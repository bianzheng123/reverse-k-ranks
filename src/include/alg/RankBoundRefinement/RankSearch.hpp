//
// Created by BianZheng on 2022/3/18.
//

#ifndef REVERSE_KRANKS_RANKSEARCH_HPP
#define REVERSE_KRANKS_RANKSEARCH_HPP

#include "struct/DistancePair.hpp"
#include <memory>
#include <spdlog/spdlog.h>

namespace ReverseMIPS {

    class RankSearch {

        size_t n_sample_, sample_every_, n_data_item_, n_user_;
        std::unique_ptr<int[]> known_rank_idx_l_; // n_sample_
        std::unique_ptr<double[]> bound_distance_table_; // n_user * n_sample_
    public:
        size_t n_max_disk_read_;

        inline RankSearch() {}

        inline RankSearch(const int &n_sample, const int &n_data_item,
                          const int &n_user) {
            const int sample_every = n_data_item / n_sample;
            this->n_sample_ = n_sample;
            this->sample_every_ = sample_every;
            this->n_data_item_ = n_data_item;
            this->n_user_ = n_user;
            known_rank_idx_l_ = std::make_unique<int[]>(n_sample_);
            bound_distance_table_ = std::make_unique<double[]>(n_user_ * n_sample_);
            if (sample_every >= n_data_item) {
                spdlog::error("sample every larger than n_data_item, sample ratio too small, program exit");
                exit(-1);
            }
            if (n_sample <= 0 || n_sample >= n_data_item) {
                spdlog::error("n_sample too small or too large, program exit");
                exit(-1);
            }
            assert(n_sample > 0);

            Preprocess();

        }

        inline RankSearch(const char *index_path) {
            LoadIndex(index_path);
        }

        void Preprocess() {
            for (size_t known_rank_idx = 0, idx = 0;
                 known_rank_idx < n_data_item_ && idx < n_sample_; known_rank_idx += sample_every_, idx++) {
                known_rank_idx_l_[idx] = known_rank_idx;
                assert(idx < n_sample_);
            }

//            for (int rankID = 0; rankID < n_sample_; rankID++) {
//                std::cout << known_rank_idx_l_[rankID] << " ";
//            }
//            std::cout << std::endl;

            n_max_disk_read_ = n_data_item_;

            spdlog::info("rank bound: sample_every {}, n_sample {}, n_max_disk_read {}", sample_every_, n_sample_,
                         n_max_disk_read_);
        }

        void LoopPreprocess(const DistancePair *distance_ptr, const int &userID) {
            std::vector<double> IP_l(n_data_item_);
            for (int itemID = 0; itemID < n_data_item_; itemID++) {
                IP_l[itemID] = distance_ptr[itemID].dist_;
            }
            LoopPreprocess(IP_l.data(), userID);
        }

        void LoopPreprocess(const double *distance_ptr, const int &userID) {
            for (int crankID = 0; crankID < n_sample_; crankID++) {
                unsigned int rankID = known_rank_idx_l_[crankID];
                bound_distance_table_[userID * n_sample_ + crankID] = distance_ptr[rankID];
            }
        }

        inline bool
        CoarseBinarySearch(const double &queryIP, const int &userID,
                           int &rank_lb, int &rank_ub, double &IP_lb, double &IP_ub) const {
            double *search_iter = bound_distance_table_.get() + userID * n_sample_;

            int bucket_ub = 0;
            int bucket_lb = n_sample_ - 1;

            double *iter_begin = search_iter;
            double *iter_end = search_iter + bucket_lb + 1;

            double *lb_ptr = std::lower_bound(iter_begin, iter_end, queryIP,
                                              [](const double &arrIP, double queryIP) {
                                                  return arrIP > queryIP;
                                              });
            unsigned int bucket_idx = bucket_ub + (lb_ptr - iter_begin);
            unsigned int tmp_rank_lb = known_rank_idx_l_[bucket_idx];
            unsigned int tmp_rank_ub = bucket_idx == 0 ? 0 : known_rank_idx_l_[bucket_idx - 1];

            if (tmp_rank_ub <= rank_ub && rank_lb <= tmp_rank_lb) {
                return false;
            }

            if (lb_ptr == iter_end) {
                rank_ub = (int) tmp_rank_ub;
                IP_ub = bound_distance_table_[userID * n_sample_ + bucket_idx - 1];
            } else if (lb_ptr == iter_begin) {
                rank_lb = (int) tmp_rank_lb;
                IP_lb = bound_distance_table_[userID * n_sample_ + bucket_idx];
            } else {
                rank_lb = (int) tmp_rank_lb;
                rank_ub = (int) tmp_rank_ub;
                IP_lb = bound_distance_table_[userID * n_sample_ + bucket_idx];
                IP_ub = bound_distance_table_[userID * n_sample_ + bucket_idx - 1];
            }

            assert(IP_lb <= queryIP && queryIP <= IP_ub);
            assert(rank_lb - rank_ub <= n_max_disk_read_);

            return false;
        }

        void RankBound(const std::vector<double> &queryIP_l, const int &topk,
                       std::vector<int> &rank_lb_l, std::vector<int> &rank_ub_l,
                       std::vector<std::pair<double, double>> &queryIPbound_l,
                       const std::vector<bool> &prune_l) const {
            for (int userID = 0; userID < n_user_; userID++) {
                int lower_rank = rank_lb_l[userID];
                int upper_rank = rank_ub_l[userID];
                assert(upper_rank <= lower_rank);
                double queryIP = queryIP_l[userID];

                double IP_lb = queryIPbound_l[userID].first;
                double IP_ub = queryIPbound_l[userID].second;

                CoarseBinarySearch(queryIP, userID,
                                   lower_rank, upper_rank, IP_lb, IP_ub);
                queryIPbound_l[userID] = std::make_pair(IP_lb, IP_ub);

                rank_lb_l[userID] = lower_rank;
                rank_ub_l[userID] = upper_rank;
            }
        }

        void SaveIndex(const char *index_path) {
            std::ofstream out_stream_ = std::ofstream(index_path, std::ios::binary | std::ios::out);
            if (!out_stream_) {
                spdlog::error("error in write result");
                exit(-1);
            }
            out_stream_.write((char *) &n_sample_, sizeof(size_t));
            out_stream_.write((char *) &sample_every_, sizeof(size_t));
            out_stream_.write((char *) &n_data_item_, sizeof(size_t));
            out_stream_.write((char *) &n_user_, sizeof(size_t));
            out_stream_.write((char *) &n_max_disk_read_, sizeof(size_t));

            out_stream_.write((char *) known_rank_idx_l_.get(), (int64_t) (n_sample_ * sizeof(int)));
            out_stream_.write((char *) bound_distance_table_.get(), (int64_t) (n_user_ * n_sample_ * sizeof(double)));

            out_stream_.close();
        }

        void LoadIndex(const char *index_path) {
            std::ifstream index_stream = std::ifstream(index_path, std::ios::binary | std::ios::in);
            if (!index_stream) {
                spdlog::error("error in reading index");
                exit(-1);
            }

            index_stream.read((char *) &n_sample_, sizeof(size_t));
            index_stream.read((char *) &sample_every_, sizeof(size_t));
            index_stream.read((char *) &n_data_item_, sizeof(size_t));
            index_stream.read((char *) &n_user_, sizeof(size_t));
            index_stream.read((char *) &n_max_disk_read_, sizeof(size_t));

            known_rank_idx_l_ = std::make_unique<int[]>(n_sample_);
            index_stream.read((char *) known_rank_idx_l_.get(), (int64_t) (sizeof(int) * n_sample_));

            bound_distance_table_ = std::make_unique<double[]>(n_user_ * n_sample_);
            index_stream.read((char *) bound_distance_table_.get(), (int64_t) (sizeof(double) * n_user_ * n_sample_));

            index_stream.close();
        }

    };
}
#endif //REVERSE_KRANKS_RANKSEARCH_HPP
