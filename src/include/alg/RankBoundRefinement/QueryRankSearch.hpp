//
// Created by BianZheng on 2022/8/12.
//

#ifndef REVERSE_KRANKS_QUERYRANKSEARCH_HPP
#define REVERSE_KRANKS_QUERYRANKSEARCH_HPP

#include "struct/DistancePair.hpp"
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>

namespace ReverseMIPS {

    class SampleQueryDistributionBelowTopk {
        int n_data_item_;
        std::unique_ptr<int[]> topk_rank_l_; // n_sample_query
        std::unique_ptr<int[]> sample_rank_l_; // n_sample_query * n_sample_query, stores the number of un-pruned user in the rank of sampled query
    public:
        const int n_sample_query_ = 100;
        const int topk_ = 10;

        inline SampleQueryDistributionBelowTopk() = default;

        inline SampleQueryDistributionBelowTopk(const int n_data_item, const char *dataset_name) {
            n_data_item_ = n_data_item;
            topk_rank_l_ = std::make_unique<int[]>(n_sample_query_);
            sample_rank_l_ = std::make_unique<int[]>(n_sample_query_ * n_sample_query_);

            {
                char topk_rank_path[512];
                sprintf(topk_rank_path,
                        "../index/query_distribution/%s-query-distribution-kth-rank-n_sample_query_%d-topk_%d.index",
                        dataset_name, n_sample_query_, topk_);

                std::ifstream topk_rank_stream = std::ifstream(topk_rank_path, std::ios::binary | std::ios::in);
                if (!topk_rank_stream) {
                    spdlog::error("error in reading index");
                    exit(-1);
                }

                topk_rank_stream.read((char *) topk_rank_l_.get(), sizeof(int) * n_sample_query_);
                topk_rank_stream.close();
            }

            {
                char rank_below_topk_path[512];
                sprintf(rank_below_topk_path,
                        "../index/query_distribution/%s-query-distribution-below-topk-n_sample_query_%d-topk_%d.index",
                        dataset_name, n_sample_query_, topk_);

                std::ifstream rank_below_topk_stream = std::ifstream(rank_below_topk_path,
                                                                     std::ios::binary | std::ios::in);
                if (!rank_below_topk_stream) {
                    spdlog::error("error in reading index");
                    exit(-1);
                }

                rank_below_topk_stream.read((char *) sample_rank_l_.get(),
                                            sizeof(int) * n_sample_query_ * n_sample_query_);
                rank_below_topk_stream.close();
            }

//            for (int sample_queryID = 0; sample_queryID < n_sample_query_; sample_queryID++) {
//                printf("%4d ", topk_rank_l_[sample_queryID]);
//            }
//            printf("\n------------------\n");
//
//            for (int sample_queryID = 0; sample_queryID < n_sample_query_; sample_queryID++) {
//                for (int sample_queryID2 = 0; sample_queryID2 < n_sample_query_; sample_queryID2++) {
//                    if (sample_queryID2 == 0) {
//                        printf("%4d ", sample_rank_l_[sample_queryID * n_sample_query_ + sample_queryID2]);
//                    } else {
//                        const int n_user_candidate =
//                                sample_rank_l_[sample_queryID * n_sample_query_ + sample_queryID2] -
//                                sample_rank_l_[sample_queryID * n_sample_query_ + sample_queryID2 - 1];
//                        printf("%4d ", n_user_candidate);
//                    }
//                }
//                printf("\n");
//            }

        }

        unsigned int GetUnpruneCandidate(const int &sample_rank_ub, const int &sample_rank_lb) {
            assert(sample_rank_ub <= sample_rank_lb && sample_rank_lb < n_sample_query_);

            unsigned int n_unprune_user = 0;
            for (int queryID = sample_rank_ub; queryID <= sample_rank_lb; queryID++) {
                n_unprune_user += sample_rank_l_[queryID * n_sample_query_ + sample_rank_lb];
            }

            return n_unprune_user;
        }

        unsigned int GetRank(const unsigned int &sample_rank) {
            assert(0 <= sample_rank && sample_rank <= n_sample_query_);
            return topk_rank_l_[sample_rank];
        }

    };

    class QueryRankSearch {

        size_t n_sample_, n_data_item_, n_user_;
        std::unique_ptr<int[]> known_rank_idx_l_; // n_sample_
        std::unique_ptr<double[]> bound_distance_table_; // n_user * n_sample_
    public:

        inline QueryRankSearch() {}

        inline QueryRankSearch(const int &n_sample, const int &n_data_item,
                               const int &n_user, const char *dataset_name) {
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

            Preprocess(dataset_name);

        }

        inline QueryRankSearch(const char *index_path) {
            LoadIndex(index_path);
        }

        void Preprocess(const char *dataset_name) {
            SampleQueryDistributionBelowTopk query_distribution_ins((int) n_data_item_, dataset_name);

            const int n_sample_query = query_distribution_ins.n_sample_query_;

            std::vector<unsigned int> optimal_dp(n_sample_query * n_sample_);
            std::vector<int> position_dp(n_sample_query * n_sample_);
            for (int sampleID = 0; sampleID < n_sample_; sampleID++) {
                printf("sampleID %d\n", sampleID);
                for (int sample_rankID = 0; sample_rankID < n_sample_query; sample_rankID++) {
                    if (sampleID == 0) {
                        optimal_dp[sample_rankID * n_sample_ + sampleID] =
                                query_distribution_ins.GetUnpruneCandidate(0, sample_rankID) +
                                (n_sample_query - sample_rankID - 1) * (n_data_item_ - query_distribution_ins.topk_);

                        position_dp[sample_rankID * n_sample_ + sampleID] = sample_rankID;
                    } else {
                        optimal_dp[sample_rankID * n_sample_ + sampleID] = UINT32_MAX;
                        for (int t = sample_rankID - 1; t >= 0; t--) {
                            const unsigned int unprune_user_candidate = query_distribution_ins.GetUnpruneCandidate(
                                    t + 1, sample_rankID) - (sample_rankID - t) *
                                                            (n_data_item_ - query_distribution_ins.topk_);
                            assert(unprune_user_candidate >= 0);
                            assert(optimal_dp[sample_rankID * n_sample_ + sampleID] >= 0);
                            assert(position_dp[sample_rankID * n_sample_ + sampleID] >= 0);
                            if (optimal_dp[sample_rankID * n_sample_ + sampleID] >
                                optimal_dp[t * n_sample_ + sampleID - 1] +
                                unprune_user_candidate) {
                                optimal_dp[sample_rankID * n_sample_ + sampleID] =
                                        optimal_dp[t * n_sample_ + sampleID - 1] + unprune_user_candidate;
                                position_dp[sample_rankID * n_sample_ + sampleID] = t;
                            } else if (unprune_user_candidate >= optimal_dp[sample_rankID * n_sample_ + sampleID]) {
                                break;
                            }

                        }

                    }
                    if (sample_rankID % 500 == 0) {
                        printf("sample_rankID %d, n_sample_query %d\n", sample_rankID, n_sample_query);
                    }
                }
            }

            unsigned int min_cost = UINT32_MAX;
            unsigned int min_cost_idx = -1;
            for (int sample_queryID = 0; sample_queryID < n_sample_query; sample_queryID++) {
                const unsigned int tmp_cost = optimal_dp[sample_queryID * n_sample_ + n_sample_ - 1];
                if (tmp_cost < min_cost) {
                    min_cost_idx = sample_queryID;
                    min_cost = tmp_cost;
                }
            }
            printf("min_cost %d, min_cost_idx %d\n", min_cost, min_cost_idx);
//            unsigned int min_cost_idx = n_sample_query - 1;
//            assert(0 <= min_cost && min_cost < n_data_item_);
            for (int sampleID = (int) n_sample_ - 1; sampleID >= 0; sampleID--) {
                printf("min cost idx %d\n", min_cost_idx);
                known_rank_idx_l_[sampleID] = (int) query_distribution_ins.GetRank(min_cost_idx);
                min_cost_idx = position_dp[min_cost_idx * n_sample_ + sampleID];
            }

            for (int rankID = 0; rankID < n_sample_; rankID++) {
                std::cout << known_rank_idx_l_[rankID] << " ";
            }
            std::cout << std::endl;

            spdlog::info("rank bound: n_sample {}", n_sample_);
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

        inline void
        CoarseBinarySearch(const double &queryIP, const int &userID,
                           int &rank_lb, int &rank_ub, double &IP_lb, double &IP_ub) const {
            double *search_iter = bound_distance_table_.get() + userID * n_sample_;

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
            assert(rank_lb - rank_ub <=
                   std::max(known_rank_idx_l_[n_sample_ - 1], (int) n_data_item_ - known_rank_idx_l_[n_sample_ - 1]));
        }

        void RankBound(const std::vector<double> &queryIP_l,
                       std::vector<int> &rank_lb_l, std::vector<int> &rank_ub_l,
                       std::vector<std::pair<double, double>> &queryIPbound_l) const {
            for (int userID = 0; userID < n_user_; userID++) {
                int lower_rank = rank_lb_l[userID];
                int upper_rank = rank_ub_l[userID];
                assert(upper_rank <= lower_rank);
                double queryIP = queryIP_l[userID];

                double &IP_lb = queryIPbound_l[userID].first;
                double &IP_ub = queryIPbound_l[userID].second;

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
            out_stream_.write((char *) &n_data_item_, sizeof(size_t));
            out_stream_.write((char *) &n_user_, sizeof(size_t));

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
            index_stream.read((char *) &n_data_item_, sizeof(size_t));
            index_stream.read((char *) &n_user_, sizeof(size_t));

            known_rank_idx_l_ = std::make_unique<int[]>(n_sample_);
            index_stream.read((char *) known_rank_idx_l_.get(), (int64_t) (sizeof(int) * n_sample_));

            bound_distance_table_ = std::make_unique<double[]>(n_user_ * n_sample_);
            index_stream.read((char *) bound_distance_table_.get(), (int64_t) (sizeof(double) * n_user_ * n_sample_));

            index_stream.close();
        }

    };
}
#endif //REVERSE_KRANKS_QUERYRANKSEARCH_HPP
