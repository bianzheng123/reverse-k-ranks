//
// Created by BianZheng on 2022/8/12.
//

#ifndef REVERSE_KRANKS_QUERYRANKSEARCHSEARCHALLRANK_HPP
#define REVERSE_KRANKS_QUERYRANKSEARCHSEARCHALLRANK_HPP

#include "struct/DistancePair.hpp"
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>

namespace ReverseMIPS {

    class SearchAllRank {
        uint64_t n_user_, n_data_item_;
        std::unique_ptr<int[]> sort_kth_rank_l_; // n_sample_query
        std::unique_ptr<int[]> sample_rank_l_; // n_data_item
        std::unique_ptr<int[]> accu_user_rank_l_; // n_sample_query * (n_data_item_ + 1)
        // stores the number of user candidates between [1, rank]

        uint64_t n_sample_query_;
        uint64_t sample_topk_;
    public:

        uint64_t n_sample_rank_;

        inline SearchAllRank() = default;

        inline SearchAllRank(const uint64_t &n_user, const uint64_t &n_data_item,
                             const char *dataset_name,
                             const int &n_sample_query, const int &sample_topk,
                             const char *index_basic_dir) {
            this->n_user_ = n_user;
            this->n_data_item_ = n_data_item;
            this->n_sample_query_ = n_sample_query;
            this->sample_topk_ = sample_topk;
            this->n_sample_rank_ = n_data_item;
            this->sort_kth_rank_l_ = std::make_unique<int[]>(n_sample_query_);
            this->sample_rank_l_ = std::make_unique<int[]>(n_data_item);
            this->accu_user_rank_l_ = std::make_unique<int[]>(n_sample_query_ * (n_data_item_ + 1));

            //init sort_kth_rank_l_, accu_user_rank_l_
            ReadQueryDistribution(index_basic_dir, dataset_name);

            std::iota(sample_rank_l_.get(), sample_rank_l_.get() + n_data_item, 0);

            spdlog::info("n_sample_query {}, sample_topk {}, n_sample_rank {}",
                         n_sample_query_, sample_topk_, n_sample_rank_);

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
//                printf("%4d ", sort_kth_rank_l_[sample_queryID]);
//            }
//            printf("\n------------------\n");
//            for (int sample_queryID = 0; sample_queryID < n_print; sample_queryID++) {
//                for (int sample_queryID2 = 0; sample_queryID2 < n_print; sample_queryID2++) {
//                    int print_thing;
//                    if (sample_queryID2 == 0) {
//                    print_thing = accu_user_rank_l_[sample_queryID * n_sample_query_ + sample_queryID2];
//                    } else {
//                        print_thing = accu_user_rank_l_[sample_queryID * n_sample_query_ + sample_queryID2] -
//                                      accu_user_rank_l_[sample_queryID * n_sample_query_ + sample_queryID2 - 1];
//                    }
//                    if (sample_queryID > sample_queryID2) {
//                        assert(accu_user_rank_l_[sample_queryID * n_sample_query_ + sample_queryID2] == 0);
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

        void ReadQueryDistribution(
                const char *index_basic_dir, const char *dataset_name) {

            {
                char sort_kth_rank_path[512];
                sprintf(sort_kth_rank_path,
                        "%s/query_distribution/%s-n_sample_item_%ld-sample_topk_%ld/sort_kth_rank_l.index",
                        index_basic_dir, dataset_name, n_sample_query_, sample_topk_);

                std::ifstream sort_kth_rank_stream = std::ifstream(sort_kth_rank_path, std::ios::binary | std::ios::in);
                if (!sort_kth_rank_stream) {
                    spdlog::error("error in reading index");
                    exit(-1);
                }

                sort_kth_rank_stream.seekg(0, std::ios::end);
                std::ios::pos_type ss = sort_kth_rank_stream.tellg();
                auto fsize = (size_t) ss;
                assert(fsize == sizeof(int) * n_sample_query_);
                sort_kth_rank_stream.seekg(0, std::ios::beg);

                sort_kth_rank_stream.read((char *) sort_kth_rank_l_.get(), sizeof(int) * n_sample_query_);
                sort_kth_rank_stream.close();
            }

            {
                char accu_user_rank_path[512];
                sprintf(accu_user_rank_path,
                        "%s/query_distribution/%s-n_sample_item_%ld-sample_topk_%ld/accu_n_user_rank_l.index",
                        index_basic_dir, dataset_name, n_sample_query_, sample_topk_);

                std::ifstream accu_user_rank_stream = std::ifstream(accu_user_rank_path,
                                                                    std::ios::binary | std::ios::in);
                if (!accu_user_rank_stream) {
                    spdlog::error("error in reading index");
                    exit(-1);
                }

                accu_user_rank_stream.seekg(0, std::ios::end);
                std::ios::pos_type ss = accu_user_rank_stream.tellg();
                auto fsize = (size_t) ss;
                assert(fsize == sizeof(int) * n_sample_query_ * (n_data_item_ + 1));
                accu_user_rank_stream.seekg(0, std::ios::beg);

                accu_user_rank_stream.read((char *) accu_user_rank_l_.get(),
                                           sizeof(int) * n_sample_query_ * (n_data_item_ + 1));
                accu_user_rank_stream.close();
            }
        }

        int64_t GetTransitUserCandidate(const int &rankID_ub, const int &rankID_lb) {
            int64_t first = GetUserCandidateEnd(rankID_ub);
            int64_t second = GetUserCandidateBetween(rankID_ub, rankID_lb);
            int64_t third = GetUserCandidateEnd(rankID_lb);
            return -first + second + third;
        }

        // it is Upsilon([rank_ub + 1, rank_lb])
        int64_t GetUserCandidateBetween(const int &rankID_ub, const int &rankID_lb) {
            assert(0 <= rankID_ub && rankID_ub < rankID_lb && rankID_lb < n_sample_rank_);
            const int rank_ub = sample_rank_l_[rankID_ub] + 1;
            const int rank_lb = sample_rank_l_[rankID_lb];
            assert(1 <= rank_ub && rank_ub <= rank_lb && rank_lb <= n_data_item_);

            int *min_queryID_ptr = std::lower_bound(sort_kth_rank_l_.get(),
                                                    sort_kth_rank_l_.get() + n_sample_query_,
                                                    rank_ub,
                                                    [](const int &info, const int &value) {
                                                        return info < value;
                                                    });

            int *max_queryID_ptr = std::lower_bound(sort_kth_rank_l_.get(),
                                                    sort_kth_rank_l_.get() + n_sample_query_,
                                                    rank_lb,
                                                    [](const int &info, const int &value) {
                                                        return info <= value;
                                                    });

            const int &queryID_begin = (int) (min_queryID_ptr - sort_kth_rank_l_.get());
            const int &queryID_end = (int) (max_queryID_ptr - sort_kth_rank_l_.get() - 1);
            if (queryID_end < 0 || max_queryID_ptr - min_queryID_ptr == 0) {
                return 0;
            }
            assert(0 <= queryID_begin && queryID_begin <= queryID_end && queryID_end < n_sample_query_);

            assert(0 < rank_ub && rank_ub <= rank_lb && rank_lb <= n_data_item_);

            int64_t n_unprune_user = 0;
            for (int queryID = queryID_begin; queryID <= queryID_end; queryID++) {
                int64_t n_user_candidate = accu_user_rank_l_[queryID * (n_data_item_ + 1) + rank_lb] -
                                           accu_user_rank_l_[queryID * (n_data_item_ + 1) + rank_ub - 1];
                n_unprune_user += n_user_candidate;
            }

            return n_unprune_user;
        }

        // it is Upsilon([rank+1, n_data_item])
        int64_t GetUserCandidateEnd(const int &rankID) {
            assert(rankID < n_sample_rank_);
            const int rank = sample_rank_l_[rankID] + 1;
            int *min_queryID_ptr = std::lower_bound(sort_kth_rank_l_.get(),
                                                    sort_kth_rank_l_.get() + n_sample_query_,
                                                    rank,
                                                    [](const int &info, const int &value) {
                                                        return info < value;
                                                    });
            const int queryID_start = (int) (min_queryID_ptr - sort_kth_rank_l_.get());

            assert(0 <= queryID_start);
            assert(1 <= rank && rank <= n_data_item_);

            int64_t n_unprune_user = 0;
            for (int queryID = queryID_start; queryID < n_sample_query_; queryID++) {
                int64_t n_user_candidate = (int64_t) n_user_ -
                                           accu_user_rank_l_[queryID * (n_data_item_ + 1) + rank - 1];
                assert(n_user_candidate >= 0);
                n_unprune_user += n_user_candidate;
            }

            return n_unprune_user;
        }

        // Upsilon([0, rank])
        int64_t GetUserCandidateBegin(const int &rankID) {
            assert(rankID < n_sample_rank_);
            const int rank = sample_rank_l_[rankID];
            int *min_queryID_ptr = std::lower_bound(sort_kth_rank_l_.get(),
                                                    sort_kth_rank_l_.get() + n_sample_query_,
                                                    rank,
                                                    [](const int &info, const int &value) {
                                                        return info <= value;
                                                    });
            const int queryID_end = (int) (min_queryID_ptr - sort_kth_rank_l_.get() - 1);
            if (queryID_end < 0) {
                return 0;
            }
            assert(0 <= queryID_end && queryID_end < n_sample_query_);

            int64_t n_unprune_user = 0;
            for (int queryID = 0; queryID <= queryID_end; queryID++) {
                int64_t n_user_candidate = accu_user_rank_l_[queryID * (n_data_item_ + 1) + rank];
                assert(n_user_candidate >= 0);
                n_unprune_user += n_user_candidate;
            }

            return n_unprune_user;
        }

        unsigned int GetRank(const unsigned int &rankID) {
            assert(0 <= rankID && rankID <= n_sample_rank_);
            return sample_rank_l_[rankID];
        }

    };

    class QueryRankSearchSearchAllRank {

        size_t n_sample_, n_data_item_, n_user_;
        std::unique_ptr<int[]> known_rank_idx_l_; // n_sample_
        std::unique_ptr<double[]> bound_distance_table_; // n_user * n_sample_
    public:

        inline QueryRankSearchSearchAllRank() {}

        inline QueryRankSearchSearchAllRank(const int &n_sample, const int &n_data_item,
                                            const int &n_user, const char *dataset_name,
                                            const int &n_sample_query, const int &sample_topk,
                                            const char *index_basic_dir) {
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

            Preprocess(dataset_name, n_sample_query, sample_topk, index_basic_dir);

        }

        inline QueryRankSearchSearchAllRank(const char *index_path) {
            LoadIndex(index_path);
        }

        void Preprocess(const char *dataset_name, const int &n_sample_query, const int &sample_topk,
                        const char *index_basic_dir) {

            SearchAllRank query_distribution_ins((int) n_user_, (int) n_data_item_, dataset_name,
                                                 n_sample_query, sample_topk, index_basic_dir);
            const uint64_t n_sample_rank = query_distribution_ins.n_sample_rank_;

            std::vector<int64_t> optimal_dp(n_sample_rank * n_sample_);
            std::vector<int> position_dp(n_sample_rank * n_sample_);
            for (int sampleID = 0; sampleID < n_sample_; sampleID++) {
                spdlog::info("sampleID {}, n_sample {}, progress {:.3f}",
                             sampleID, n_sample_, (double) sampleID / (double) n_sample_);
//                if (sampleID != 0 && sampleID % 10 == 0) {
//                    spdlog::info("sampleID {}, n_sample {}, progress {:.3f}",
//                                 sampleID, n_sample_, (double) sampleID / (double) n_sample_);
//                }
#pragma omp parallel for default(none) shared(n_sample_rank, sampleID, optimal_dp, query_distribution_ins, position_dp, std::cout)
                for (int rankID = 0; rankID < n_sample_rank; rankID++) {
                    if (sampleID == 0) {
                        optimal_dp[rankID * n_sample_ + sampleID] =
                                query_distribution_ins.GetUserCandidateBegin(rankID) +
                                query_distribution_ins.GetUserCandidateEnd(rankID);

                        position_dp[rankID * n_sample_ + sampleID] = rankID;
                    } else {
                        optimal_dp[rankID * n_sample_ + sampleID] =
                                optimal_dp[rankID * n_sample_ + sampleID - 1];
                        position_dp[rankID * n_sample_ + sampleID] =
                                position_dp[rankID * n_sample_ + sampleID - 1];
                        for (int prev_rankID = rankID - 1; prev_rankID >= 0; prev_rankID--) {
                            const int64_t unprune_user_candidate =
                                    query_distribution_ins.GetTransitUserCandidate(
                                            prev_rankID, rankID);
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
                           int &rank_lb, int &rank_ub) const {
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
#endif //REVERSE_KRANKS_QUERYRANKSEARCHSEARCHALLRANK_HPP
