//
// Created by BianZheng on 2022/11/3.
//

#ifndef REVERSE_K_RANKS_SAMPLESEARCH_HPP
#define REVERSE_K_RANKS_SAMPLESEARCH_HPP

#include "struct/DistancePair.hpp"
#include <iostream>
#include <fstream>
#include <memory>
#include <cfloat>
#include <spdlog/spdlog.h>

namespace ReverseMIPS {

    class SampleSearch {

        size_t n_sample_, n_data_item_, n_user_;
        std::unique_ptr<double[]> bound_distance_table_; // n_user * n_sample_
    public:
        std::unique_ptr<int[]> known_rank_idx_l_; // n_sample_

        inline SampleSearch() {}

        inline SampleSearch(const int &n_data_item, const int &n_user,
                            const std::vector<int> &known_rank_l, const int &n_sample) {
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
            assert(known_rank_l.size() == n_sample);

            for (int sampleID = 0; sampleID < n_sample; sampleID++) {
                known_rank_idx_l_[sampleID] = known_rank_l[sampleID];
            }

        }

        inline SampleSearch(const char *index_path, const char *dataset_name, const char *method_name,
                            const size_t &n_sample,
                            const bool &load_sample_score, const bool &is_query_distribution,
                            const size_t &n_sample_query = 0, const size_t &sample_topk = 0) {
            LoadIndex(index_path, dataset_name, method_name,
                      n_sample,
                      load_sample_score, is_query_distribution,
                      n_sample_query, sample_topk);

            for (int rankID = 0; rankID < n_sample_; rankID++) {
                std::cout << known_rank_idx_l_[rankID] << " ";
            }
            std::cout << std::endl;
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

        template<class Compare>
        const double *lower_bound(const double *first, const double *last, const double &value, Compare comp) {

            int count, step;
            const double *it;
            count = std::distance(first, last);

            while (count > 0) {
                it = first;
                step = count / 2;
                std::advance(it, step);

                if (comp(*it, value)) {
                    first = ++it;
                    count -= step + 1;
                } else
                    count = step;
            }

            return first;
        }

        inline void
        CoarseBinarySearch(const double &queryIP, const int &userID,
                           int &rank_lb, int &rank_ub) const {
            double *search_iter = bound_distance_table_.get() + n_sample_ * userID;

            double *iter_begin = search_iter;
            double *iter_end = search_iter + n_sample_;

            double *lb_ptr = std::lower_bound(iter_begin, iter_end, queryIP,
                                              [](const double &arrIP, double queryIP) {
                                                  return arrIP > queryIP;
                                              });
//            double *lb_ptr = std::lower_bound(iter_begin, iter_end, queryIP,
//                                              [](const double &arrIP, double queryIP) {
//                                                  return arrIP > queryIP;
//                                              });
            unsigned int bucket_idx = (lb_ptr - iter_begin);
            unsigned int tmp_rank_lb = bucket_idx == n_sample_ ? n_data_item_ : known_rank_idx_l_[bucket_idx];
            unsigned int tmp_rank_ub = bucket_idx == 0 ? 0 : known_rank_idx_l_[bucket_idx - 1];


            if (bucket_idx == n_sample_) {
                rank_lb = (int) n_data_item_;
                rank_ub = (int) tmp_rank_ub;
            } else if (bucket_idx == 0) {
                rank_lb = (int) tmp_rank_lb;
                rank_ub = 0;
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
                int lower_rank, upper_rank;
                double queryIP = queryIP_l[userID];

                CoarseBinarySearch(queryIP, userID,
                                   lower_rank, upper_rank);

                rank_lb_l[userID] = lower_rank;
                rank_ub_l[userID] = upper_rank;
            }
        }

        inline void
        CoarseBinarySearch(const double &queryIP, const int &userID,
                           int &bucketID,
                           double &IP_lb, double &IP_ub,
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

            bucketID = (int) (lb_ptr - iter_begin);

            if (lb_ptr == iter_end) {
                rank_lb = (int) n_data_item_;
                rank_ub = (int) tmp_rank_ub;
                IP_lb = -DBL_MAX;
                IP_ub = bound_distance_table_[userID * n_sample_ + bucket_idx - 1];
            } else if (lb_ptr == iter_begin) {
                rank_lb = (int) tmp_rank_lb;
                rank_ub = (int) 0;
                IP_lb = bound_distance_table_[userID * n_sample_ + bucket_idx];
                IP_ub = DBL_MAX;
            } else {
                rank_lb = (int) tmp_rank_lb;
                rank_ub = (int) tmp_rank_ub;
                IP_lb = bound_distance_table_[userID * n_sample_ + bucket_idx];
                IP_ub = bound_distance_table_[userID * n_sample_ + bucket_idx - 1];
            }

            assert(0 <= rank_lb - rank_ub &&
                   rank_lb - rank_ub <= std::max(known_rank_idx_l_[n_sample_ - 1],
                                                 (int) n_data_item_ - known_rank_idx_l_[n_sample_ - 1]));
        }

        void RankBound(const std::vector<double> &queryIP_l,
                       const std::vector<bool> &prune_l, const std::vector<bool> &result_l,
                       std::vector<std::pair<double, double>> &queryIP_bound_l,
                       std::vector<int> &bucketID_l,
                       std::vector<int> &rank_lb_l, std::vector<int> &rank_ub_l) const {
            assert(queryIP_l.size() == n_user_);
            assert(prune_l.size() == n_user_);
            assert(result_l.size() == n_user_);
            assert(queryIP_bound_l.size() == n_user_);
            assert(bucketID_l.size() == n_user_);
            assert(rank_lb_l.size() == n_user_);
            assert(rank_ub_l.size() == n_user_);
            for (int userID = 0; userID < n_user_; userID++) {
                if (prune_l[userID] || result_l[userID]) {
                    continue;
                }
                int lower_rank, upper_rank;
                double queryIP = queryIP_l[userID];

                double IP_lb, IP_ub;
                int bucketID;
                CoarseBinarySearch(queryIP, userID,
                                   bucketID,
                                   IP_lb, IP_ub,
                                   lower_rank, upper_rank);

                bucketID_l[userID] = bucketID;
                queryIP_bound_l[userID] = std::make_pair(IP_lb, IP_ub);

                rank_lb_l[userID] = lower_rank;
                rank_ub_l[userID] = upper_rank;
            }
        }

        void SaveIndex(const char *index_basic_dir, const char *dataset_name,
                       const char *method_name,
                       const bool &save_sample_score, const bool &is_query_distribution,
                       const size_t &n_sample_query, const size_t &sample_topk) {
            char index_abs_dir[256];
            if (save_sample_score) {
                sprintf(index_abs_dir, "%s/memory_index", index_basic_dir);
            } else {
                sprintf(index_abs_dir, "%s/qrs_to_sample_index", index_basic_dir);
            }

            char index_path[512];
            if (is_query_distribution) {
                sprintf(index_path,
                        "%s/%s-%s-n_sample_%ld-n_sample_query_%ld-sample_topk_%ld.index",
                        index_abs_dir, method_name, dataset_name, n_sample_, n_sample_query, sample_topk);
            } else {
                sprintf(index_path,
                        "%s/%s-%s-n_sample_%ld.index",
                        index_abs_dir, method_name, dataset_name, n_sample_);

            }

            std::ofstream out_stream_ = std::ofstream(index_path, std::ios::binary | std::ios::out);
            if (!out_stream_) {
                spdlog::error("error in write result, not found index");
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

        void LoadIndex(const char *index_basic_dir, const char *dataset_name,
                       const char *method_name,
                       const size_t &n_sample,
                       const bool &load_sample_score, const bool &is_query_distribution,
                       const size_t &n_sample_query = 0, const size_t &sample_topk = 0) {
            char index_abs_dir[256];
            if (load_sample_score) {
                sprintf(index_abs_dir, "%s/memory_index", index_basic_dir);
            } else {
                sprintf(index_abs_dir, "%s/qrs_to_sample_index", index_basic_dir);
            }

            char index_path[512];
            if (is_query_distribution) {
                sprintf(index_path,
                        "%s/%s-%s-n_sample_%ld-n_sample_query_%ld-sample_topk_%ld.index",
                        index_abs_dir, method_name, dataset_name, n_sample, n_sample_query, sample_topk);
            } else {
                sprintf(index_path,
                        "%s/%s-%s-n_sample_%ld.index",
                        index_abs_dir, method_name, dataset_name, n_sample);
            }
            spdlog::info("index path {}", index_path);

            std::ifstream index_stream = std::ifstream(index_path, std::ios::binary | std::ios::in);
            if (!index_stream) {
                spdlog::error("error in reading index");
                exit(-1);
            }

            index_stream.read((char *) &n_sample_, sizeof(size_t));
            index_stream.read((char *) &n_data_item_, sizeof(size_t));
            index_stream.read((char *) &n_user_, sizeof(size_t));
            assert(n_sample_ == n_sample);

            known_rank_idx_l_ = std::make_unique<int[]>(n_sample_);
            index_stream.read((char *) known_rank_idx_l_.get(), (int64_t) (sizeof(int) * n_sample_));

            bound_distance_table_ = std::make_unique<double[]>(n_user_ * n_sample_);
            if (load_sample_score) {
                index_stream.read((char *) bound_distance_table_.get(),
                                  (int64_t) (sizeof(double) * n_user_ * n_sample_));
            }

            index_stream.close();
        }

        uint64_t IndexSizeByte() {
            const uint64_t known_rank_idx_size = sizeof(int) * n_sample_;
            const uint64_t bound_distance_table_size = sizeof(double) * n_user_ * n_sample_;
            return known_rank_idx_size + bound_distance_table_size;
        }

    };
}
#endif //REVERSE_K_RANKS_SAMPLESEARCH_HPP
