//
// Created by BianZheng on 2022/3/1.
//

#ifndef REVERSE_K_RANKS_INTERVALBOUND_HPP
#define REVERSE_K_RANKS_INTERVALBOUND_HPP

#include "alg/PruneCandidateByBound.hpp"
#include "alg/SpaceInnerProduct.hpp"
#include "alg/SVD.hpp"
#include "alg/ScaleIntegerPrune.hpp"
#include "struct/VectorMatrix.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/MethodBase.hpp"
#include "struct/RankBoundElement.hpp"
#include "struct/IntVectorMatrix.hpp"
#include "util/TimeMemory.hpp"
#include "util/VectorIO.hpp"
#include "util/FileIO.hpp"
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <map>
#include <set>
#include <cassert>
#include <spdlog/spdlog.h>

namespace ReverseMIPS::IntervalRankBound {

    class RetrievalResult {
    public:
        //unit: second
        double total_time, read_disk_time, inner_product_time, coarse_binary_search_time, fine_binary_search_time, interval_search_time;
        double interval_prune_ratio, binary_search_prune_ratio;
        double second_per_query;
        int topk;

        inline RetrievalResult(double total_time, double interval_search_time,
                               double inner_product_time, double coarse_binary_search_time,
                               double read_disk_time, double fine_binary_search_time,

                               double interval_prune_ratio, double binary_search_prune_ratio,
                               double second_per_query, int topk) {
            this->total_time = total_time;
            this->interval_search_time = interval_search_time;
            this->inner_product_time = inner_product_time;
            this->coarse_binary_search_time = coarse_binary_search_time;
            this->read_disk_time = read_disk_time;
            this->fine_binary_search_time = fine_binary_search_time;

            this->interval_prune_ratio = interval_prune_ratio;
            this->binary_search_prune_ratio = binary_search_prune_ratio;
            this->second_per_query = second_per_query;

            this->topk = topk;
        }

        void ConvertAddString(std::map<std::string, std::string> &performance_m, double val, const char *string) {
            char buff[256];
            sprintf(buff, "top%d retrieval\t\t %s", topk, string);
            std::string str(buff);
            performance_m.emplace(str, double2string(val));
        }

        void AddMap(std::map<std::string, std::string> &performance_m) {
            ConvertAddString(performance_m, total_time, "total time");
            ConvertAddString(performance_m, interval_search_time, "interval search time");
            ConvertAddString(performance_m, inner_product_time, "inner product time");
            ConvertAddString(performance_m, coarse_binary_search_time, "coarse binary search time");
            ConvertAddString(performance_m, read_disk_time, "read disk time");
            ConvertAddString(performance_m, fine_binary_search_time, "fine binary search time");

            ConvertAddString(performance_m, interval_prune_ratio, "interval prune ratio");
            ConvertAddString(performance_m, binary_search_prune_ratio, "binary search prune ratio");

            ConvertAddString(performance_m, second_per_query, "second per query time");
        }

        [[nodiscard]] std::string ToString() const {
            char arr[512];
            sprintf(arr,
                    "top%d retrieval time:\n\ttotal %.3fs, read disk %.3fs\n\tinner product %.3fs, coarse binary search %.3fs, fine binary search %.3fs, million second per query %.3fms",
                    topk, total_time, read_disk_time, inner_product_time, coarse_binary_search_time,
                    fine_binary_search_time,
                    second_per_query * 1000);
            std::string str(arr);
            return str;
        }

    };

    class BinarySearchBoundElement {
    public:
        int read_count_, base_rank_, userID_;
        double queryIP_;
        std::vector<double> disk_cache_;

        inline BinarySearchBoundElement(const int max_n_cache) {
            disk_cache_.reserve(max_n_cache);
        }

        inline void ReadDisk(std::ifstream &index_stream, int read_count) {
            this->read_count_ = read_count;
            disk_cache_.resize(read_count);
            index_stream.read((char *) disk_cache_.data(), read_count * sizeof(double));
        }

        inline int CountRank(int queryID) {
            if (disk_cache_.empty()) {
                return base_rank_ + 1;
            }
            auto iter_begin = disk_cache_.begin();
            auto iter_end = disk_cache_.begin() + read_count_;

            auto lb_ptr = std::lower_bound(iter_begin, iter_end, queryIP_,
                                           [](const double &arrIP, double queryIP) {
                                               return arrIP > queryIP;
                                           });
            if (userID_ == 964 && queryID == 0) {
                printf("count rank: read_count %d, base_rank %d, queryIP %.3f\n", read_count_, base_rank_, queryIP_);
                for (int i = 0; i < read_count_; i++) {
                    printf("%.3f ", disk_cache_[i]);
                }
                printf("\n");
                printf("offset %d\n", (int) (lb_ptr - iter_begin));
            }
            return (int) (lb_ptr - iter_begin) + base_rank_ + 1;
        }

        std::string ToString() {
            char arr[256];
            sprintf(arr, "userID %d, queryIP %.3f, read_count_ %d, base_rank %d", userID_, queryIP_, read_count_,
                    base_rank_);
            std::string str(arr);
            return str;
        }
    };

    class Index : public BaseIndex {
        void ResetTimer() {
            read_disk_time_ = 0;
            inner_product_time_ = 0;
            coarse_binary_search_time_ = 0;
            fine_binary_search_time_ = 0;
            interval_search_time_ = 0;
            interval_prune_ratio_ = 0;
            binary_search_prune_ratio_ = 0;
        }

    public:
        //for interval search, store in memory
        std::vector<int> interval_table_; // n_user * n_interval, the last element must be n_user
        std::vector<double> interval_dist_l_;
        std::vector<std::pair<double, double>> user_ip_bound_l_; // first is lower bound, second is upper bound
        int n_interval_;
        //interval bound (SVD)
        std::vector<double> user_part_norm_l_;
        VectorMatrix transfer_item_;
        int check_dim_, remain_dim_;

        //for rank search, store in memory
        std::vector<double> bound_distance_table_; // n_user * n_cache_rank_
        std::vector<int> known_rank_idx_l_; // n_cache_rank_
        int n_cache_rank_, n_max_disk_read_, cache_bound_every_;

        //read index on disk
        const char *index_path_;

        VectorMatrix user_double_;
        IntVectorMatrix user_int_;
        int vec_dim_, n_data_item_, n_user_;
        double read_disk_time_, inner_product_time_, coarse_binary_search_time_, fine_binary_search_time_, interval_search_time_;
        TimeRecord read_disk_record_, inner_product_record_, coarse_binary_search_record_, fine_binary_search_record_, interval_search_record_;
        double interval_prune_ratio_, binary_search_prune_ratio_;

        Index(
                //interval search
                const std::vector<int> &interval_table, const std::vector<double> &interval_dist_l,
                const std::vector<std::pair<double, double>> &user_ip_bound_l,
                const int &n_interval,
                //interval search bound
                std::vector<double> &user_part_norm_l, VectorMatrix &transfer_item, const int &check_dim,
                // rank search
                const std::vector<double> &bound_distance_table, const std::vector<int> &known_rank_idx_l,
                const int &n_max_disk_read, const int &cache_bound_every,
                //general retrieval
                const char *index_path) {
            //interval search
            this->interval_table_ = interval_table;
            this->interval_dist_l_ = interval_dist_l;
            this->user_ip_bound_l_ = user_ip_bound_l;
            this->n_interval_ = n_interval;
            //interval search bound
            this->user_part_norm_l_ = user_part_norm_l;
            this->transfer_item_ = std::move(transfer_item);
            this->check_dim_ = check_dim;
            //rank search
            this->bound_distance_table_ = bound_distance_table;
            this->known_rank_idx_l_ = known_rank_idx_l;
            this->n_max_disk_read_ = n_max_disk_read;
            this->cache_bound_every_ = cache_bound_every;
            this->n_cache_rank_ = (int) known_rank_idx_l.size();
            //general retrieval
            this->index_path_ = index_path;
        }

        void
        setUserItemMatrix(VectorMatrix &user_double, IntVectorMatrix &user_int, const int n_data_item) {
            this->n_user_ = user_double.n_vector_;
            this->vec_dim_ = user_double.vec_dim_;
            this->user_double_ = std::move(user_double);
            this->user_int_ = std::move(user_int);
            this->n_data_item_ = n_data_item;
            assert(this->user_int_.n_vector_ == this->user_double_.n_vector_);
            assert(0 < this->user_int_.vec_dim_ && this->user_int_.vec_dim_ <= this->user_double_.vec_dim_);
            assert(interval_dist_l_.size() == n_user_);
            assert(this->transfer_item_.n_vector_ == this->vec_dim_);
            assert(this->transfer_item_.vec_dim_ == this->vec_dim_);
            this->remain_dim_ = vec_dim_ - check_dim_;
        }

        std::vector<std::vector<UserRankElement>> Retrieval(VectorMatrix &query_item, const int topk) override {
            ResetTimer();
            std::ifstream index_stream_ = std::ifstream(this->index_path_, std::ios::binary | std::ios::in);
            if (!index_stream_) {
                spdlog::error("error in writing index");
            }

            if (topk > user_double_.n_vector_) {
                spdlog::error("top-k is too large, program exit");
                exit(-1);
            }

            const int n_query_item = query_item.n_vector_;
            std::vector<std::vector<UserRankElement>> query_heap_l(n_query_item, std::vector<UserRankElement>());
            for (int queryID = 0; queryID < n_query_item; ++queryID) {
                query_heap_l.reserve(topk);
            }

            // store queryIP
            std::vector<int> query_int(vec_dim_ - check_dim_);
            std::vector<RankBoundElement> rank_bound_l(n_user_);
            std::vector<double> queryIP_l(n_user_);
            std::vector<char> prune_l(n_user_, 0);
            std::vector<std::pair<double, double>> queryIP_bound_l(n_user_);
            for (int queryID = 0; queryID < n_query_item; queryID++) {
                double *query_double_vecs = query_item.getVector(queryID);
                ConveryQuery(query_double_vecs, query_int);
                assert(query_int.size() == remain_dim_);

                //calculate the interval search bound, by computing part dimension, integer and cauchy inequality
                int *query_int_vecs = query_int.data();
                double *query_remain_vecs = query_double_vecs + check_dim_;
                double query_part_norm = std::sqrt(InnerProduct(query_remain_vecs, query_remain_vecs, remain_dim_));
                inner_product_record_.reset();
                for (int userID = 0; userID < n_user_; userID++) {
                    double *user_double_vecs = user_double_.getVector(userID);
                    double partIP = InnerProduct(query_double_vecs, user_double_vecs, check_dim_);
                    queryIP_l[userID] = partIP;
                    int *user_int_vecs = user_int_.getVector(userID);
                    std::pair<int, int> bound_int_pair = InnerProductBound(query_int_vecs, user_int_vecs,
                                                                           remain_dim_);
                    double part_norm = query_part_norm * user_part_norm_l_[userID];
                    std::pair<double, double> bound_double_pair = std::make_pair(-part_norm, part_norm);
                    if (userID == 964 && queryID == 0) {
                        printf("userID %d, int lowerIP %.3f, double lowerIP %.3f, int upperIP %.3f, double upperIP %.3f\n",
                               userID, bound_int_pair.first + partIP, bound_double_pair.first + partIP,
                               bound_int_pair.second + partIP, bound_double_pair.second + partIP);
                    }
                    bound_double_pair.first =
                            std::max(bound_double_pair.first, (double) bound_int_pair.first) + partIP;
                    bound_double_pair.second =
                            std::min(bound_double_pair.second, (double) bound_int_pair.second) + partIP;
                    queryIP_bound_l[userID] = bound_double_pair;
                }
                this->inner_product_time_ += inner_product_record_.get_elapsed_time_second();

                interval_search_record_.reset();
                //interval search
                for (int userID = 0; userID < n_user_; userID++) {
                    std::pair<double, double> IP_bound = queryIP_bound_l[userID];
                    if (userID == 964 && queryID == 0) {
                        printf("userID %d, lowerIP: %.3f, upperIP %.3f\n", userID, IP_bound.first, IP_bound.second);
                        printf("global %.3f %.3f\n", user_ip_bound_l_[userID].first, user_ip_bound_l_[userID].second);
                    }
                    std::pair<int, int> pair = IntervalSearch(IP_bound, userID, queryID);
                    rank_bound_l[userID] = RankBoundElement(userID, pair);
                    if (userID == 964 && queryID == 0) {
                        std::cout << "after interval search: " << rank_bound_l[userID].ToString() << std::endl;
                    }
                }

                int n_remain;
                int n_rank_bound = n_user_;
                std::memset(prune_l.data(), 0, sizeof(char) * n_user_);
                PruneCandidateByBound(rank_bound_l, n_rank_bound, topk, n_remain, prune_l.data(), queryID);
                n_rank_bound = topk + n_remain;
                this->interval_search_time_ += interval_search_record_.get_elapsed_time_second();
                interval_prune_ratio_ = 1.0 * n_rank_bound / n_user_;

                //calculate the exact IP
                inner_product_record_.reset();
                for (int candID = 0; candID < n_rank_bound; candID++) {
                    int userID = rank_bound_l[candID].userID_;
                    queryIP_l[userID] += InnerProduct(user_double_.getVector(userID) + vec_dim_, query_remain_vecs,
                                                      remain_dim_);
                    if (userID == 964 && queryID == 0) {
                        printf("userID %d, actual queryIP %.3f\n", userID, queryIP_l[userID]);
                    }
                }

                this->inner_product_time_ += inner_product_record_.get_elapsed_time_second();

                //coarse binary search
                coarse_binary_search_record_.reset();
                int global_lower_rank = 0;
                for (int candID = 0; candID < n_rank_bound; candID++) {
                    int userID = rank_bound_l[candID].userID_;
                    int &lower_rank = rank_bound_l[candID].lower_rank_;
                    int &upper_rank = rank_bound_l[candID].upper_rank_;
                    double queryIP = queryIP_l[userID];

                    if (candID < topk) {
                        global_lower_rank = std::max(global_lower_rank, lower_rank);
                    } else {
                        global_lower_rank = std::min(global_lower_rank, lower_rank);
                    }

                    if (candID >= topk && global_lower_rank < upper_rank) {
                        prune_l[candID] = 1; //true

                        if (userID == 964 && queryID == 0) {
                            printf("global lower rank %d\n", global_lower_rank);
                            std::cout << "pruned coarse binary search:" << rank_bound_l[candID].ToString() << std::endl;
                        }
                        continue;
                    }
                    CoarseBinarySearch(queryIP, userID, lower_rank, upper_rank, queryID);
                }
                PruneCandidateByBound(rank_bound_l, n_rank_bound, topk, n_remain, prune_l.data(), queryID, true);
                if (queryID == 0) {
                    printf("is pruned by coarse binary search userID %d, is pruned %d\n", 964, prune_l[964]);
                }
                int origin_size = n_rank_bound;
                n_rank_bound = topk + n_remain;
                coarse_binary_search_time_ += coarse_binary_search_record_.get_elapsed_time_second();
                binary_search_prune_ratio_ = 1.0 * n_rank_bound / origin_size;

                // read from disk
                // store the data of fine binary search
                read_disk_record_.reset();
                std::vector<BinarySearchBoundElement> bound_cache_l(n_rank_bound,
                                                                    BinarySearchBoundElement(2 * n_max_disk_read_));
                for (int candID = 0; candID < n_rank_bound; candID++) {
                    RankBoundElement element = rank_bound_l[candID];
                    int end_idx = element.lower_rank_;
                    int start_idx = element.upper_rank_;
                    int userID = element.userID_;
                    double queryIP = queryIP_l[userID];

                    assert(start_idx <= end_idx);
                    int read_count = end_idx - start_idx;
                    index_stream_.seekg((element.userID_ * n_data_item_ + start_idx) * sizeof(double), std::ios::beg);
                    bound_cache_l[candID].ReadDisk(index_stream_, read_count);

                    bound_cache_l[candID].base_rank_ = start_idx;
                    bound_cache_l[candID].userID_ = userID;
                    bound_cache_l[candID].queryIP_ = queryIP;

                    if (userID == 964 && queryID == 0) {
                        printf("read disk: start %d, end %d\n", start_idx, end_idx);
                    }
                }
                read_disk_time_ += read_disk_record_.get_elapsed_time_second();

                fine_binary_search_record_.reset();
                // reuse the max heap in coarse binary search
                int total_cand_size = (int) bound_cache_l.size();
                std::vector<UserRankElement> &max_heap = query_heap_l[queryID];
                for (int candID = 0; candID < total_cand_size; candID++) {
                    BinarySearchBoundElement element = bound_cache_l[candID];
                    int rank = element.CountRank(queryID);
                    int userID = element.userID_;
                    double queryIP = element.queryIP_;
                    max_heap.emplace_back(userID, rank, queryIP);
                    if (userID == 964 && queryID == 0) {
                        printf("final rank userID %d, rank %d\n", userID, rank);
                    }
                }
                fine_binary_search_time_ += fine_binary_search_record_.get_elapsed_time_second();

                std::sort(max_heap.begin(), max_heap.end(), std::less<UserRankElement>());
                max_heap.resize(topk);
            }

            return query_heap_l;
        }

        void ConveryQuery(double *query_vecs, std::vector<int> &query_int) const {
            SVD::TransferItem(query_vecs, transfer_item_, vec_dim_);
            ScaleIntegerPrune::ConvertQuery(query_vecs, vec_dim_, query_int, check_dim_);
        }

        //return the index of the bucket it belongs to
        inline void
        CoarseBinarySearch(const double &queryIP, const int &userID, int &rank_lb, int &rank_ub, int queryID) const {
            int bucket_ub = std::ceil(1.0 * (rank_ub - cache_bound_every_ + 1) / cache_bound_every_);
            int bucket_lb = std::floor(1.0 * (rank_lb - cache_bound_every_ + 1) / cache_bound_every_);
            if (bucket_lb - bucket_ub < 0) {
                return;
            }

            auto search_iter = bound_distance_table_.begin() + userID * n_cache_rank_;
            auto iter_begin = search_iter + bucket_ub;
            auto iter_end = search_iter + bucket_lb + 1;

            auto lb_ptr = std::lower_bound(iter_begin, iter_end, queryIP,
                                           [](const double &arrIP, double queryIP) {
                                               return arrIP > queryIP;
                                           });
            int bucket_idx = bucket_ub + (int) (lb_ptr - iter_begin);
            int tmp_rank_lb = known_rank_idx_l_[bucket_idx];
            int tmp_rank_ub = bucket_idx == 0 ? 0 : known_rank_idx_l_[bucket_idx - 1];

            if (queryID == 0 && userID == 964) {
                printf("coarse binary search before convert userID %d, bucket_ub %d, bucket_lb %d\n", userID, bucket_ub, bucket_lb);
                printf("iter_begin %.3f\n", *iter_begin);
            }

            if (lb_ptr == iter_end) {
                rank_ub = tmp_rank_ub;
            } else if (lb_ptr == iter_begin) {
                rank_lb = tmp_rank_lb;
            } else {
                rank_lb = tmp_rank_lb;
                rank_ub = tmp_rank_ub;
            }

            if (queryID == 0 && userID == 964) {
                printf("coarse binary search userID %d, bucket_ub %d, bucket_lb %d\n", userID, bucket_ub, bucket_lb);
                printf("coarse binary search userID %d, rank_ub %d, rank_lb %d\n", userID, rank_ub, rank_lb);
                printf("coarse binary search offset %d\n", (int) (lb_ptr - iter_begin));
            }
        }

        std::pair<int, int> IntervalSearch(const std::pair<double, double> &IPbound, const int &userID, int queryID) {
            std::pair<double, double> &ip_bound_pair = user_ip_bound_l_[userID];
            double itv_dist = interval_dist_l_[userID];
            //for interval id, the higher rank value means the lower queryiP
            int itv_lb_idx = std::ceil((ip_bound_pair.second - IPbound.first) / itv_dist);
            int itv_ub_idx = (int) std::floor((ip_bound_pair.second - IPbound.second) / itv_dist) - 1;
            assert(itv_ub_idx <= itv_lb_idx);

            if (itv_ub_idx <= -1) {
                itv_ub_idx = -1;
            } else if (itv_ub_idx >= n_interval_ - 1) {
                itv_ub_idx = n_interval_ - 1;
            }
            if (itv_lb_idx <= -1) {
                itv_lb_idx = -1;
            } else if (itv_lb_idx >= n_interval_ - 1) {
                itv_lb_idx = n_interval_ - 1;
            }
            int *rank_ptr = interval_table_.data() + userID * n_interval_;
            int rank_lb = itv_lb_idx == -1 ? 0 : rank_ptr[itv_lb_idx];
            int rank_ub = itv_ub_idx == -1 ? 0 : rank_ptr[itv_ub_idx];

            if (userID == 964 && queryID == 0) {
                printf("interval search, rank_ub %d, rank_lb %d\n", rank_ub, rank_lb);
                printf("interval search, itv_ub_idx %d, itv_lb_idx %d\n", itv_ub_idx, itv_lb_idx);
            }

            return std::make_pair(rank_lb, rank_ub);
        }

    };

    void TransformUserItem(VectorMatrix &user, VectorMatrix &data_item, IntVectorMatrix &user_int,
                           VectorMatrix &transfer_item, int &check_dim) {
        //user normalization
        user.vectorNormalize();

        const double SIGMA = 0.7;
        check_dim = SVD::SVD(user, data_item, transfer_item, SIGMA);

        //monotonicity  reduction
        ScaleIntegerPrune::ScaleIntegerPrune(user, data_item, user_int, check_dim);

//        ScaleIntegerPrune::ScaleTransferItem(transfer_item);
    }

    const int write_every_ = 100;
    const int report_batch_every_ = 5;

    /*
     * bruteforce index
     * shape: n_user * n_data_item, type: double, the distance pair for each user
     */

    Index &BuildIndex(VectorMatrix &user, VectorMatrix &data_item, const char *index_path) {
        IntVectorMatrix user_int;
        VectorMatrix transfer_item;
        int check_dim;
        TransformUserItem(user, data_item, user_int, transfer_item, check_dim);
        std::vector<double> user_part_norm_l(user.n_vector_);

        const int n_data_item = data_item.n_vector_;
        const int vec_dim = data_item.vec_dim_;
        const int n_user = user.n_vector_;

        //used for interval cache bound
        const int n_interval = std::min(n_data_item / 10, 5000);
        std::vector<int> interval_table(n_user * n_interval);
        std::memset(interval_table.data(), 0, n_user * n_interval * sizeof(int));

        std::vector<std::pair<double, double>> bound_l(n_user);
        std::vector<double> interval_dist_l(n_user);
        spdlog::info("interval bound: n_interval {}", n_interval);

        //used for coarse binary search
        const int cache_bound_every = 10;
        const int n_cache_rank = n_data_item / cache_bound_every;
        std::vector<int> known_rank_idx_l;
        for (int known_rank_idx = cache_bound_every - 1;
             known_rank_idx < n_data_item; known_rank_idx += cache_bound_every) {
            known_rank_idx_l.emplace_back(known_rank_idx);
        }
        spdlog::info("rank bound: cache_bound_every {}, n_cache_rank {}", cache_bound_every, n_cache_rank);

        assert(known_rank_idx_l[0] == known_rank_idx_l[1] - (known_rank_idx_l[0] + 1));
        int n_max_disk_read = std::max(known_rank_idx_l[0], n_data_item - (known_rank_idx_l[n_cache_rank - 1] + 1));
        assert(known_rank_idx_l.size() == n_cache_rank);

        std::vector<double> bound_distance_table(n_user * n_cache_rank);

        //build and write index
        std::ofstream out(index_path, std::ios::binary | std::ios::out);
        if (!out) {
            spdlog::error("error in write result");
        }

        std::vector<double> write_distance_cache(write_every_ * n_data_item);
        const int n_batch = n_user / write_every_;
        const int n_remain = n_user % write_every_;
        spdlog::info("write_every_ {}, n_batch {}, n_remain {}", write_every_, n_batch, n_remain);

        TimeRecord batch_report_record;
        batch_report_record.reset();
        for (int i = 0; i < n_batch; i++) {
#pragma omp parallel for default(none) shared(i, data_item, user, write_distance_cache, bound_distance_table, known_rank_idx_l, bound_l, interval_table, interval_dist_l, user_part_norm_l, check_dim) shared(n_cache_rank, write_every_, n_data_item, vec_dim, n_interval)
            for (int cacheID = 0; cacheID < write_every_; cacheID++) {
                int userID = write_every_ * i + cacheID;
                for (int itemID = 0; itemID < n_data_item; itemID++) {
                    double ip = InnerProduct(data_item.getVector(itemID), user.getVector(userID), vec_dim);
                    write_distance_cache[cacheID * n_data_item + itemID] = ip;
                }
                std::sort(write_distance_cache.begin() + cacheID * n_data_item,
                          write_distance_cache.begin() + (cacheID + 1) * n_data_item, std::greater<double>());

                //interval search
                double upper_bound = write_distance_cache[cacheID * n_data_item] + 0.01;
                double lower_bound = write_distance_cache[(cacheID + 1) * n_data_item - 1] - 0.01;
                bound_l[userID].first = lower_bound;
                bound_l[userID].second = upper_bound;
                auto distance_ptr = write_distance_cache.begin() + cacheID * n_data_item;
                double interval_distance = (upper_bound - lower_bound) / n_interval;
                interval_dist_l[userID] = interval_distance;
                auto interval_ptr = interval_table.begin() + userID * n_interval;
                for (int itemID = 0; itemID < n_data_item; itemID++) {
                    double ip = distance_ptr[itemID];
                    int itv_idx = std::floor((upper_bound - ip) / interval_distance);
                    assert(0 <= itv_idx && itv_idx < n_interval);
                    interval_ptr[itv_idx]++;
                }
                for (int intervalID = 1; intervalID < n_interval; intervalID++) {
                    interval_ptr[intervalID] += interval_ptr[intervalID - 1];
                }
                assert(interval_ptr[n_interval - 1] == n_data_item);
                //interval search bound
                double norm2IP = InnerProduct(user.getVector(userID) + check_dim,
                                              user.getVector(userID) + check_dim, vec_dim - check_dim);
                user_part_norm_l[userID] = std::sqrt(norm2IP);

                //rank search
                auto array_begin = write_distance_cache.begin() + cacheID * n_data_item;
                for (int crankID = 0; crankID < n_cache_rank; crankID++) {
                    int itemID = known_rank_idx_l[crankID];
                    bound_distance_table[userID * n_cache_rank + crankID] = array_begin[itemID];
                }
            }
            out.write((char *) write_distance_cache.data(), write_distance_cache.size() * sizeof(double));

            if (i % report_batch_every_ == 0) {
                std::cout << "preprocessed " << i / (0.01 * n_batch) << " %, "
                          << batch_report_record.get_elapsed_time_second() << " s/iter" << " Mem: "
                          << get_current_RSS() / 1000000 << " Mb \n";
                batch_report_record.reset();
            }

        }

        for (int cacheID = 0; cacheID < n_remain; cacheID++) {
            int userID = write_every_ * n_batch + cacheID;
            for (int itemID = 0; itemID < n_data_item; itemID++) {
                double ip = InnerProduct(data_item.getVector(itemID), user.getVector(userID), vec_dim);
                write_distance_cache[cacheID * n_data_item + itemID] = ip;
            }

            std::sort(write_distance_cache.begin() + cacheID * n_data_item,
                      write_distance_cache.begin() + (cacheID + 1) * n_data_item, std::greater<double>());

            //interval search
            double upper_bound = write_distance_cache[cacheID * n_data_item] + 0.01;
            double lower_bound = write_distance_cache[(cacheID + 1) * n_data_item - 1] - 0.01;
            bound_l[userID].first = lower_bound;
            bound_l[userID].second = upper_bound;
            auto distance_ptr = write_distance_cache.begin() + cacheID * n_data_item;
            double interval_distance = (upper_bound - lower_bound) / n_interval;
            interval_dist_l[userID] = interval_distance;
            auto interval_ptr = interval_table.begin() + userID * n_interval;
            for (int itemID = 0; itemID < n_data_item; itemID++) {
                double ip = distance_ptr[itemID];
                int itv_idx = std::floor((upper_bound - ip) / interval_distance);
                assert(0 <= itv_idx && itv_idx < n_interval);
                interval_ptr[itv_idx]++;
            }
            for (int intervalID = 1; intervalID < n_interval; intervalID++) {
                interval_ptr[intervalID] += interval_ptr[intervalID - 1];
            }
            assert(interval_ptr[n_interval - 1] == n_data_item);
            //interval search bound
            double norm2IP = InnerProduct(user.getVector(userID) + check_dim,
                                          user.getVector(userID) + check_dim, vec_dim - check_dim);
            user_part_norm_l[userID] = std::sqrt(norm2IP);

            if (userID == 964) {
                printf("build index lower bound %.3f, upper bound %.3f, interval distance %.3f\n",
                       lower_bound, upper_bound, interval_distance);
                for (int j = 150; j < 200; j++) {
                    printf("%d %.3f %d\n", j, upper_bound - interval_distance * (j + 1), interval_ptr[j]);
                }

                for (int j = 0; j < 70; j++) {
                    printf("%.3f ", write_distance_cache[cacheID * n_data_item + j]);
                }
                printf("\n");
            }

            //rank search
            auto array_begin = write_distance_cache.begin() + cacheID * n_data_item;
            for (int crankID = 0; crankID < n_cache_rank; crankID++) {
                int itemID = known_rank_idx_l[crankID];
                bound_distance_table[userID * n_cache_rank + crankID] = array_begin[itemID];
            }
        }

        out.write((char *) write_distance_cache.data(),
                  n_remain * data_item.n_vector_ * sizeof(double));
        static Index index(
                //interval search
                interval_table, interval_dist_l, bound_l, n_interval,
                //interval search bound
                user_part_norm_l, transfer_item, check_dim,
                //rank search
                bound_distance_table, known_rank_idx_l, n_max_disk_read, cache_bound_every,
                //general retrieval
                index_path);
        index.setUserItemMatrix(user, user_int, n_data_item);
        return index;
    }

}

#endif //REVERSE_K_RANKS_INTERVALBOUND_HPP
