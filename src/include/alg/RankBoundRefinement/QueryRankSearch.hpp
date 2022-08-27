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

        std::vector<int> min_rank2queryID_l_;//n_sample_query, points to the minimum index of the same rank
        std::vector<int> max_rank2queryID_l_;//n_sample_query, points to the maximum index of the same rank
        std::vector<int> distinct_rank_l_;//n_distinct_rank_

        uint64_t n_sample_query_;
        uint64_t sample_topk_;
    public:

        uint64_t n_distinct_rank_;

        inline SampleQueryDistributionBelowTopk() = default;

        inline SampleQueryDistributionBelowTopk(const int &n_data_item, const char *dataset_name,
                                                const int &n_sample_query, const int &sample_topk,
                                                const char *index_basic_dir) {
            n_data_item_ = n_data_item;
            this->n_sample_query_ = n_sample_query;
            this->sample_topk_ = sample_topk;
            topk_rank_l_ = std::make_unique<int[]>(n_sample_query_);
            sample_rank_l_ = std::make_unique<int[]>(n_sample_query_ * n_sample_query_);

            ReadQueryDistribution(index_basic_dir, dataset_name);

            //find distinct rank
            std::vector<int> distinct_sample_rank_l(n_sample_query_);
            distinct_sample_rank_l.assign(topk_rank_l_.get(), topk_rank_l_.get() + n_sample_query_);
            std::sort(distinct_sample_rank_l.begin(), distinct_sample_rank_l.end());
            distinct_sample_rank_l.erase(std::unique(distinct_sample_rank_l.begin(), distinct_sample_rank_l.end()),
                                         distinct_sample_rank_l.end());
            n_distinct_rank_ = distinct_sample_rank_l.size();

            distinct_rank_l_.resize(n_distinct_rank_);
            distinct_rank_l_.assign(distinct_sample_rank_l.begin(), distinct_sample_rank_l.end());
            assert(0 < n_distinct_rank_ && n_distinct_rank_ <= n_sample_query_);

            min_rank2queryID_l_.resize(n_distinct_rank_);
            max_rank2queryID_l_.resize(n_distinct_rank_);
            min_rank2queryID_l_.assign(n_distinct_rank_, -1);
            max_rank2queryID_l_.assign(n_distinct_rank_, -1);
            spdlog::info("n_sample_query {}, sample_topk {}, n_distinct_rank {}",
                         n_sample_query_, sample_topk_, n_distinct_rank_);

            for (int rankID = 0; rankID < n_distinct_rank_; rankID++) {
                int *min_queryID_ptr = std::lower_bound(topk_rank_l_.get(), topk_rank_l_.get() + n_sample_query_,
                                                        distinct_rank_l_[rankID],
                                                        [](const int &info, const int &value) {
                                                            return info < value;
                                                        });

                int *max_queryID_ptr = std::lower_bound(topk_rank_l_.get(), topk_rank_l_.get() + n_sample_query_,
                                                        distinct_rank_l_[rankID],
                                                        [](const int &info, const int &value) {
                                                            return info <= value;
                                                        });
                const long min_queryID = min_queryID_ptr - topk_rank_l_.get();
                const long max_queryID = max_queryID_ptr - topk_rank_l_.get();
                assert(min_queryID < max_queryID);
                assert(0 <= min_queryID && min_queryID < n_sample_query_);
                assert(1 <= max_queryID && max_queryID <= n_sample_query_);
                min_rank2queryID_l_[rankID] = (int) min_queryID;
                max_rank2queryID_l_[rankID] = (int) max_queryID - 1;
                assert(min_rank2queryID_l_[rankID] >= 0);
                assert(max_rank2queryID_l_[rankID] >= 0);
            }

            for (int rankID = 1; rankID < n_distinct_rank_; rankID++) {
                assert(min_rank2queryID_l_[rankID] > min_rank2queryID_l_[rankID - 1]);
                assert(max_rank2queryID_l_[rankID] > max_rank2queryID_l_[rankID - 1]);
            }

            /*
             * {
                std::vector<int> sample_queryID_l(n_sample_query_);
                assert(sample_queryID_l.size() == n_sample_query_);
                char resPath[256];
                std::sprintf(resPath, "%s/index/query_distribution/%s-sample-itemID-n_sample_query_%ld-sample_topk_%ld.txt",
                             index_basic_dir, dataset_name, n_sample_query_, sample_topk_);

                std::ifstream in_stream = std::ifstream(resPath, std::ios::binary | std::ios::in);
                if (!in_stream.is_open()) {
                    spdlog::error("error in open file");
                    exit(-1);
                }

                in_stream.read((char *) sample_queryID_l.data(), sizeof(int) * n_sample_query_);
                printf("sample queryID l\n");
                for (int sample_queryID = 0; sample_queryID < n_sample_query_; sample_queryID++) {
                    printf("%d ", sample_queryID_l[sample_queryID]);
                }
                printf("\n");
            }
             */

//            const int n_print = 10; //n_print <= n_sample_query_
//            for (int sample_queryID = 0; sample_queryID < n_print; sample_queryID++) {
//                printf("%4d ", sample_queryID);
//            }
//            printf("\n------------------\n");
//            for (int sample_queryID = 0; sample_queryID < n_print; sample_queryID++) {
//                printf("%4d ", topk_rank_l_[sample_queryID]);
//            }
//            printf("\n------------------\n");
//            for (int sample_queryID = 0; sample_queryID < n_print; sample_queryID++) {
//                for (int sample_queryID2 = 0; sample_queryID2 < n_print; sample_queryID2++) {
//                    int print_thing;
//                    if (sample_queryID2 == 0) {
//                    print_thing = sample_rank_l_[sample_queryID * n_sample_query_ + sample_queryID2];
//                    } else {
//                        print_thing = sample_rank_l_[sample_queryID * n_sample_query_ + sample_queryID2] -
//                                      sample_rank_l_[sample_queryID * n_sample_query_ + sample_queryID2 - 1];
//                    }
//                    if (sample_queryID > sample_queryID2) {
//                        assert(sample_rank_l_[sample_queryID * n_sample_query_ + sample_queryID2] == 0);
//                    }
//
//                    if (sample_queryID == sample_queryID2) {
//                        printf("\033[0;31m"); //Set the text to the color red
//                        printf("%4d ", print_thing);
//                        printf("\033[0m"); //Resets the text to default color
//
//                    } else {
//                        printf("%4d ", print_thing);
//                    }
//                }
//                printf("\n");
//            }

        }

        void ReadQueryDistribution(const char *index_basic_dir, const char *dataset_name) {
            {
                char topk_rank_path[512];
                sprintf(topk_rank_path,
                        "%s/index/query_distribution/%s-kth-rank-n_sample_query_%ld-sample_topk_%ld.index",
                        index_basic_dir, dataset_name, n_sample_query_, sample_topk_);

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
                        "%s/index/query_distribution/%s-below-topk-n_sample_query_%ld-sample_topk_%ld.index",
                        index_basic_dir, dataset_name, n_sample_query_, sample_topk_);

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
        }

        int64_t GetUnpruneCandidate(const int &rankID_ub, const int &rankID_lb) {
            assert(rankID_ub <= rankID_lb && rankID_lb < n_distinct_rank_);
            int64_t n_unprune_user = 0;
            const int &queryID_ub = min_rank2queryID_l_[rankID_ub];
            const int &queryID_lb = max_rank2queryID_l_[rankID_lb];
            for (int queryID = queryID_ub; queryID <= queryID_lb; queryID++) {
                n_unprune_user += sample_rank_l_[queryID * n_sample_query_ + queryID_lb];
            }

            return n_unprune_user;
        }

        int64_t GetUnpruneCandidate(const int &rankID) {
            assert(rankID <= n_distinct_rank_);
            if (rankID == n_distinct_rank_) {
                return 0;
            }
            const int &queryID = min_rank2queryID_l_[rankID];
            int64_t n_unprune_user = (int64_t) (n_data_item_ - sample_topk_) * (n_sample_query_ - queryID);

            return n_unprune_user;
        }

        unsigned int GetRank(const unsigned int &rankID) {
            assert(0 <= rankID && rankID <= n_distinct_rank_);
            return distinct_rank_l_[rankID];
        }

    };

    class QueryRankSearch {

        size_t n_sample_, n_data_item_, n_user_;
        std::unique_ptr<int[]> known_rank_idx_l_; // n_sample_
        std::unique_ptr<double[]> bound_distance_table_; // n_user * n_sample_
    public:

        inline QueryRankSearch() {}

        inline QueryRankSearch(const int &n_sample, const int &n_data_item,
                               const int &n_user, const char *dataset_name,
                               const int &n_sample_query, const int &sample_topk,
                               const char *index_size_basic_dir = "..") {
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

            Preprocess(dataset_name, n_sample_query, sample_topk, index_size_basic_dir);

        }

        inline QueryRankSearch(const char *index_path) {
            LoadIndex(index_path);
        }

        void Preprocess(const char *dataset_name, const int &n_sample_query, const int &sample_topk,
                        const char *index_size_basic_dir) {
            SampleQueryDistributionBelowTopk query_distribution_ins((int) n_data_item_, dataset_name,
                                                                    n_sample_query, sample_topk, index_size_basic_dir);
            const uint64_t n_sample_rank = query_distribution_ins.n_distinct_rank_;

            std::vector<int64_t> optimal_dp(n_sample_rank * n_sample_);
            std::vector<int> position_dp(n_sample_rank * n_sample_);
            for (int sampleID = 0; sampleID < n_sample_; sampleID++) {
                if (sampleID != 0 && sampleID % 100 == 0) {
                    spdlog::info("sampleID {}, n_sample {}, progress {:.3f}",
                                 sampleID, n_sample_, (double) sampleID / (double) n_sample_);
                }
#pragma omp parallel for default(none) shared(n_sample_rank, sampleID, optimal_dp, query_distribution_ins, position_dp, std::cout)
                for (int rankID = 0; rankID < n_sample_rank; rankID++) {
                    if (sampleID == 0) {
                        optimal_dp[rankID * n_sample_ + sampleID] =
                                query_distribution_ins.GetUnpruneCandidate(0, rankID) +
                                query_distribution_ins.GetUnpruneCandidate(rankID + 1);

                        position_dp[rankID * n_sample_ + sampleID] = rankID;
                    } else {
                        optimal_dp[rankID * n_sample_ + sampleID] =
                                optimal_dp[rankID * n_sample_ + sampleID - 1];
                        position_dp[rankID * n_sample_ + sampleID] =
                                position_dp[rankID * n_sample_ + sampleID - 1];
                        for (int prev_rankID = rankID - 1; prev_rankID >= 0; prev_rankID--) {
                            const int64_t unprune_user_candidate =
                                    -query_distribution_ins.GetUnpruneCandidate(prev_rankID + 1) +
                                    query_distribution_ins.GetUnpruneCandidate(prev_rankID + 1, rankID) +
                                    query_distribution_ins.GetUnpruneCandidate(rankID + 1);
                            assert(unprune_user_candidate <= 0);

                            if (optimal_dp[rankID * n_sample_ + sampleID] >
                                optimal_dp[prev_rankID * n_sample_ + sampleID - 1] + unprune_user_candidate) {

                                optimal_dp[rankID * n_sample_ + sampleID] =
                                        optimal_dp[prev_rankID * n_sample_ + sampleID - 1] + unprune_user_candidate;

                                position_dp[rankID * n_sample_ + sampleID] = prev_rankID;
                            }
                            assert(optimal_dp[rankID * n_sample_ + sampleID] >= 0);
                            assert(position_dp[rankID * n_sample_ + sampleID] >= 0);
                        }
//                        if (rankID - 1 >= 0 && sampleID < rankID) {
//                            if (optimal_dp[rankID * n_sample_ + sampleID] >=
//                                optimal_dp[rankID * n_sample_ + sampleID - 1]) {
//                                printf("this sample %lu, prev sample %lu, sampleID %d, rankID %d, position_dp %d, rank %d\n",
//                                       optimal_dp[rankID * n_sample_ + sampleID],
//                                       optimal_dp[rankID * n_sample_ + sampleID - 1],
//                                       sampleID, rankID,
//                                       position_dp[rankID * n_sample_ + sampleID],
//                                       query_distribution_ins.GetRank(rankID));
//                            }
//
//                            assert(optimal_dp[rankID * n_sample_ + sampleID] <
//                                   optimal_dp[rankID * n_sample_ + sampleID - 1]);
//                        }
                        assert(optimal_dp[rankID * n_sample_ + sampleID] >= 0);
                        assert(position_dp[rankID * n_sample_ + sampleID] >= 0);
                    }
                }
            }

            uint64_t min_cost = UINT64_MAX;
            unsigned int min_cost_idx = -1;
            for (int rankID = 0; rankID < n_sample_rank; rankID++) {
                const uint64_t tmp_cost = optimal_dp[rankID * n_sample_ + n_sample_ - 1];
                if (tmp_cost < min_cost) {
                    min_cost_idx = rankID;
                    min_cost = tmp_cost;
                }
            }
            assert(min_cost_idx != -1);
            std::vector<int> sample_idx_l(n_sample_);
            for (int sampleID = (int) n_sample_ - 1; sampleID >= 0; sampleID--) {
                sample_idx_l[sampleID] = (int) min_cost_idx;

                known_rank_idx_l_[sampleID] = (int) query_distribution_ins.GetRank(min_cost_idx);
                min_cost_idx = position_dp[min_cost_idx * n_sample_ + sampleID];
            }

            for (int rankID = 0; rankID < n_sample_; rankID++) {
                std::cout << known_rank_idx_l_[rankID] << " ";
            }
            std::cout << std::endl;

            std::vector<int> known_distinct_rank_l(n_sample_);
            known_distinct_rank_l.assign(known_rank_idx_l_.get(), known_rank_idx_l_.get() + n_sample_);
            std::sort(known_distinct_rank_l.begin(), known_distinct_rank_l.end());
            known_distinct_rank_l.erase(std::unique(known_distinct_rank_l.begin(), known_distinct_rank_l.end()),
                                        known_distinct_rank_l.end());

            if (query_distribution_ins.n_distinct_rank_ >= n_sample_) {
                assert(known_distinct_rank_l.size() == n_sample_);
            }
            spdlog::info("n_distinct_rank {}, n_sample {}, n_sample_distinct_rank {}",
                   query_distribution_ins.n_distinct_rank_, n_sample_,
                   known_distinct_rank_l.size());


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
            if (rank_lb == rank_ub) {
                rank_lb++;
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
