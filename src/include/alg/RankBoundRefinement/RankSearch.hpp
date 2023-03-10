//
// Created by BianZheng on 2022/3/18.
//

#ifndef REVERSE_KRANKS_RANKSEARCH_HPP
#define REVERSE_KRANKS_RANKSEARCH_HPP

#include "struct/DistancePair.hpp"
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>

namespace ReverseMIPS {

    class RankSearch {

        size_t n_sample_, max_sample_every_, n_data_item_, n_user_;
        std::unique_ptr<double[]> bound_distance_table_; // n_user * n_sample_
    public:
        std::unique_ptr<int[]> known_rank_idx_l_; // n_sample_

        inline RankSearch() {}

        inline RankSearch(const int &n_sample, const int &n_data_item,
                          const int &n_user) {
            this->n_sample_ = n_sample;
            this->n_data_item_ = n_data_item;
            this->n_user_ = n_user;
            known_rank_idx_l_ = std::make_unique<int[]>(n_sample_);
            bound_distance_table_ = std::make_unique<double[]>(n_user_ * n_sample_);
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
            const int end_sample_rank = (int) n_data_item_ - 1;
            const double delta = (end_sample_rank - 0) * 1.0 / n_sample_;
            for (int sampleID = 0; sampleID < n_sample_; sampleID++) {
                known_rank_idx_l_[sampleID] = std::floor(sampleID * delta);
            }
            std::sort(known_rank_idx_l_.get(), known_rank_idx_l_.get() + n_sample_);
            assert(0 <= known_rank_idx_l_[0] && known_rank_idx_l_[0] < n_data_item_);
            int max_sample_every = 0;
            for (int sampleID = 1; sampleID < n_sample_; sampleID++) {
                assert(0 <= known_rank_idx_l_[sampleID] && known_rank_idx_l_[sampleID] < n_data_item_);
                assert(known_rank_idx_l_[sampleID - 1] < known_rank_idx_l_[sampleID]);
                max_sample_every = std::max(max_sample_every,
                                            known_rank_idx_l_[sampleID] - known_rank_idx_l_[sampleID - 1]);
            }
            max_sample_every_ = max_sample_every;
            assert(max_sample_every_ <= std::ceil(1.0 * n_data_item_ / n_sample_));

            for (int rankID = 0; rankID < n_sample_; rankID++) {
                std::cout << known_rank_idx_l_[rankID] << " ";
            }
            std::cout << std::endl;

            spdlog::info("rank bound: max_sample_every {}, n_sample {}", max_sample_every_, n_sample_);
        }

        void LoopPreprocess(const DistancePair *distance_ptr, const int &userID) {
            for (int crankID = 0; crankID < n_sample_; crankID++) {
                unsigned int rankID = known_rank_idx_l_[crankID];
                bound_distance_table_[n_sample_ * userID + crankID] = distance_ptr[rankID].dist_;
            }
        }

        void LoopPreprocess(const double *distance_ptr, const int &userID) {
            for (int crankID = 0; crankID < n_sample_; crankID++) {
                unsigned int rankID = known_rank_idx_l_[crankID];
                bound_distance_table_[n_sample_ * userID + crankID] = distance_ptr[rankID];
            }
        }

        const double *SampleData(const int &userID) const {
            return bound_distance_table_.get() + userID * n_sample_;
        }

        inline void
        CoarseBinarySearch(const double &queryIP, const int &userID,
                           int &rank_lb, int &rank_ub) const {
            double *search_iter = bound_distance_table_.get() + n_sample_ * userID;

            int bucket_ub = 0;
            int bucket_lb = (int) n_sample_ - 1;

            double *iter_begin = search_iter;
            double *iter_end = search_iter + bucket_lb + 1;

            double *lb_ptr = std::lower_bound(iter_begin, iter_end, queryIP,
                                              [](const double &arrIP, double queryIP) {
                                                  return arrIP > queryIP;
                                              });
            unsigned int bucket_idx = bucket_ub + (lb_ptr - iter_begin);
            unsigned int tmp_rank_lb = bucket_idx == n_sample_ ? n_data_item_ : known_rank_idx_l_[bucket_idx];
            unsigned int tmp_rank_ub = bucket_idx == 0 ? 0 : known_rank_idx_l_[bucket_idx - 1];

            if (lb_ptr == iter_end) {
                rank_lb = (int) n_data_item_;
                rank_ub = (int) tmp_rank_ub;
            } else if (lb_ptr == iter_begin) {
                rank_lb = (int) tmp_rank_lb;
                rank_ub = (int) 0;
            } else {
                rank_lb = (int) tmp_rank_lb;
                rank_ub = (int) tmp_rank_ub;
            }

            assert(0 <= rank_lb - rank_ub &&
                   rank_lb - rank_ub <= std::max(known_rank_idx_l_[n_sample_ - 1],
                                                 (int) n_data_item_ - known_rank_idx_l_[n_sample_ - 1]));
        }

        void RankBound(const std::vector<double> &queryIP_l,
                       const std::vector<bool> &prune_l, const std::vector<bool> &result_l,
                       std::vector<int> &rank_lb_l, std::vector<int> &rank_ub_l) const {
            assert(queryIP_l.size() == n_user_);
            assert(prune_l.size() == n_user_);
            assert(result_l.size() == n_user_);
            assert(rank_lb_l.size() == n_user_);
            assert(rank_ub_l.size() == n_user_);
            for (int userID = 0; userID < n_user_; userID++) {
                if (prune_l[userID] || result_l[userID]) {
                    continue;
                }
                int lower_rank = rank_lb_l[userID];
                int upper_rank = rank_ub_l[userID];
                assert(upper_rank <= lower_rank);
                double queryIP = queryIP_l[userID];

                CoarseBinarySearch(queryIP, userID,
                                   lower_rank, upper_rank);

                rank_lb_l[userID] = lower_rank;
                rank_ub_l[userID] = upper_rank;
            }
        }

        void RankBound(const std::vector<std::pair<double, double>> &queryIP_l,
                       std::vector<int> &rank_lb_l, std::vector<int> &rank_ub_l) const {
            for (int userID = 0; userID < n_user_; userID++) {
                const double queryIP_lb = queryIP_l[userID].first;
                int qIP_lb_tmp_lower_rank, qIP_lb_tmp_upper_rank;

                CoarseBinarySearch(queryIP_lb, userID,
                                   qIP_lb_tmp_lower_rank, qIP_lb_tmp_upper_rank);

                const double queryIP_ub = queryIP_l[userID].second;
                int qIP_ub_tmp_lower_rank, qIP_ub_tmp_upper_rank;
                CoarseBinarySearch(queryIP_ub, userID,
                                   qIP_ub_tmp_lower_rank, qIP_ub_tmp_upper_rank);

                rank_lb_l[userID] = qIP_lb_tmp_lower_rank;
                rank_ub_l[userID] = qIP_ub_tmp_upper_rank;
                assert(qIP_lb_tmp_upper_rank <= qIP_lb_tmp_lower_rank);
                assert(qIP_ub_tmp_upper_rank <= qIP_ub_tmp_lower_rank);
                assert(qIP_ub_tmp_upper_rank <= qIP_lb_tmp_lower_rank);
            }
        }

        void SaveIndex(const char *index_path) {
            std::ofstream out_stream_ = std::ofstream(index_path, std::ios::binary | std::ios::out);
            if (!out_stream_) {
                spdlog::error("error in write result");
                exit(-1);
            }
            out_stream_.write((char *) &n_sample_, sizeof(size_t));
            out_stream_.write((char *) &max_sample_every_, sizeof(size_t));
            out_stream_.write((char *) &n_data_item_, sizeof(size_t));
            out_stream_.write((char *) &n_user_, sizeof(size_t));

            out_stream_.write((char *) known_rank_idx_l_.get(), (int64_t) (n_sample_ * sizeof(int)));
            out_stream_.write((char *) bound_distance_table_.get(), (int64_t) (n_user_ * n_sample_ * sizeof(double)));

            out_stream_.close();
        }

        void SaveNewIndex(const char *index_path, const bool &save_sample_score) {
            std::ofstream out_stream_ = std::ofstream(index_path, std::ios::binary | std::ios::out);
            if (!out_stream_) {
                spdlog::error("error in write result");
                exit(-1);
            }
            out_stream_.write((char *) &n_sample_, sizeof(size_t));
            out_stream_.write((char *) &n_data_item_, sizeof(size_t));
            out_stream_.write((char *) &n_user_, sizeof(size_t));

            out_stream_.write((char *) known_rank_idx_l_.get(), (int64_t) (n_sample_ * sizeof(int)));
            if (save_sample_score) {
                out_stream_.write((char *) bound_distance_table_.get(),
                                  (int64_t) (n_user_ * n_sample_ * sizeof(double)));
            }

            out_stream_.close();
        }

        void LoadIndex(const char *index_path) {
            std::ifstream index_stream = std::ifstream(index_path, std::ios::binary | std::ios::in);
            if (!index_stream) {
                spdlog::error("error in reading index");
                exit(-1);
            }

            index_stream.read((char *) &n_sample_, sizeof(size_t));
            index_stream.read((char *) &max_sample_every_, sizeof(size_t));
            index_stream.read((char *) &n_data_item_, sizeof(size_t));
            index_stream.read((char *) &n_user_, sizeof(size_t));

            known_rank_idx_l_ = std::make_unique<int[]>(n_sample_);
            index_stream.read((char *) known_rank_idx_l_.get(), (int64_t) (sizeof(int) * n_sample_));

            bound_distance_table_ = std::make_unique<double[]>(n_user_ * n_sample_);
            index_stream.read((char *) bound_distance_table_.get(), (int64_t) (sizeof(double) * n_user_ * n_sample_));

            index_stream.close();
        }

        uint64_t IndexSizeByte() {
            const uint64_t known_rank_idx_size = sizeof(int) * n_sample_;
            const uint64_t bound_distance_table_size = sizeof(double) * n_user_ * n_sample_;
            return known_rank_idx_size + bound_distance_table_size;
        }

    };
}
#endif //REVERSE_KRANKS_RANKSEARCH_HPP
