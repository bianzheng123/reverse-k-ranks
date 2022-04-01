//
// Created by BianZheng on 2022/3/1.
//

#ifndef REVERSE_K_RANKS_INTERVALBOUND_HPP
#define REVERSE_K_RANKS_INTERVALBOUND_HPP

#include "struct/VectorMatrix.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/MethodBase.hpp"
#include "alg/SpaceInnerProduct.hpp"
#include "alg/SVD.hpp"
#include "MonotonicityReduction.hpp"
#include "util/TimeMemory.hpp"
#include "util/VectorIO.hpp"
#include "util/FileIO.hpp"
#include "struct/RankBoundElement.hpp"
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <map>
#include <set>
#include <cassert>
#include <spdlog/spdlog.h>

namespace ReverseMIPS::IntervalBinarySearchBoundMono {

    class RetrievalResult {
    public:
        //unit: second
        double total_time, read_disk_time, inner_product_time, coarse_binary_search_time, fine_binary_search_time, second_per_query;
        int topk;

        inline RetrievalResult(double total_time, double read_disk_time, double inner_product_time,
                               double coarse_binary_search_time, double fine_binary_search_time,
                               double second_per_query, int topk) {
            this->total_time = total_time;
            this->read_disk_time = read_disk_time;
            this->inner_product_time = inner_product_time;
            this->coarse_binary_search_time = coarse_binary_search_time;
            this->fine_binary_search_time = fine_binary_search_time;
            this->second_per_query = second_per_query;

            this->topk = topk;
        }

        void AddMap(std::map<std::string, std::string> &performance_m) const {
            char buff[256];
            sprintf(buff, "top%d retrieval\t\t total time", topk);
            std::string str1(buff);
            performance_m.emplace(str1, double2string(total_time));

            sprintf(buff, "top%d retrieval\t\t read disk time", topk);
            std::string str2(buff);
            performance_m.emplace(str2, double2string(read_disk_time));

            sprintf(buff, "top%d retrieval\t\t inner product time", topk);
            std::string str3(buff);
            performance_m.emplace(str3, double2string(inner_product_time));

            sprintf(buff, "top%d retrieval\t\t coarse binary search time", topk);
            std::string str4(buff);
            performance_m.emplace(str4, double2string(coarse_binary_search_time));

            sprintf(buff, "top%d retrieval\t\t fine binary search time", topk);
            std::string str5(buff);
            performance_m.emplace(str5, double2string(fine_binary_search_time));

            sprintf(buff, "top%d retrieval\t\t second per query time", topk);
            std::string str6(buff);
            performance_m.emplace(str6, double2string(second_per_query));
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

        inline int CountRank() {
            if (disk_cache_.size() == 0) {
                return base_rank_ + 1;
            }
            auto iter_begin = disk_cache_.begin();
            auto iter_end = disk_cache_.begin() + read_count_;

            auto lb_ptr = std::lower_bound(iter_begin, iter_end, queryIP_,
                                           [](const double &arrIP, double queryIP) {
                                               return arrIP > queryIP;
                                           });
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

    inline bool UserRankBoundElementUpperBoundMaxHeap(const RankBoundElement &userRankBoundElement,
                                                      RankBoundElement &other) {
        return userRankBoundElement.upper_rank_ < other.upper_rank_;
    }

    class Index : public BaseIndex {
        void ResetTimer() {
            read_disk_time_ = 0;
            inner_product_time_ = 0;
            coarse_binary_search_time_ = 0;
            fine_binary_search_time_ = 0;
            interval_prune_ratio_ = 0;
            binary_search_prune_ratio_ = 0;
        }

    public:
        //for interval search, store in memory
        std::vector<int> interval_table_; // n_user * n_interval, the last element must be n_user
        std::vector<double> interval_dist_l_;
        std::vector<std::pair<double, double>> user_ip_bound_l_; // first is lower bound, second is upper bound
        int n_interval_;
        std::vector<double> user_part_norm_l_;

        //bound for binary search, store in memory
        std::vector<double> bound_distance_table_; // n_user * n_cache_rank_
        std::vector<int> known_rank_idx_l_; // n_cache_rank_
        int n_cache_rank_, n_max_disk_read_, cache_bound_every_;

        //read index on disk
        const char *index_path_;

        VectorMatrix user_, transfer_item_;
        std::vector<double> eigen_l_;
        int check_dim_;
        int vec_dim_, n_data_item_, n_user_, old_vec_dim_;
        double read_disk_time_, inner_product_time_, coarse_binary_search_time_, fine_binary_search_time_, interval_search_time_;
        TimeRecord read_disk_record_, inner_product_record_, coarse_binary_search_record_, fine_binary_search_record_, interval_search_record_;
        double interval_prune_ratio_, binary_search_prune_ratio_;

        Index(const std::vector<int> &interval_table, const std::vector<double> &interval_dist_l,
              const std::vector<std::pair<double, double>> &bound_l,
              const int n_interval, const std::vector<double> &user_part_norm_l,
              const std::vector<double> &bound_distance_table, const std::vector<int> &known_rank_idx_l,
              const char *index_path, const int n_max_disk_read, const int cache_bound_every) {
            this->interval_table_ = interval_table;
            this->interval_dist_l_ = interval_dist_l;
            this->user_ip_bound_l_ = bound_l;
            this->n_interval_ = n_interval;
            this->user_part_norm_l_ = user_part_norm_l;

            this->bound_distance_table_ = bound_distance_table;
            this->known_rank_idx_l_ = known_rank_idx_l;
            this->index_path_ = index_path;
            this->n_cache_rank_ = (int) known_rank_idx_l.size();
            this->n_max_disk_read_ = n_max_disk_read;
            this->cache_bound_every_ = cache_bound_every;
        }

        void
        setUserItemMatrix(VectorMatrix &user, const VectorMatrix &data_item, VectorMatrix &transfer_item,
                          const std::vector<double> &eigen_l, const int &check_dim) {
            this->n_user_ = user.n_vector_;
            this->vec_dim_ = user.vec_dim_;
            this->old_vec_dim_ = user.vec_dim_ - 2;
            this->user_ = std::move(user);
            this->n_data_item_ = data_item.n_vector_;
            this->transfer_item_ = std::move(transfer_item);
            this->eigen_l_ = eigen_l;
            this->check_dim_ = check_dim;
            assert(eigen_l.size() == this->old_vec_dim_);
            assert(interval_dist_l_.size() == n_user_);
        }

        std::vector<std::vector<UserRankElement>> Retrieval(VectorMatrix &query_item, const int topk) override {
            ResetTimer();
            std::ifstream index_stream_ = std::ifstream(this->index_path_, std::ios::binary | std::ios::in);
            if (!index_stream_) {
                spdlog::error("error in writing index");
            }

            if (topk > user_.n_vector_) {
                spdlog::error("top-k is too large, program exit");
                exit(-1);
            }

            const int n_query_item = query_item.n_vector_;
            std::vector<std::vector<UserRankElement>> query_heap_l(n_query_item, std::vector<UserRankElement>());
            for (int queryID = 0; queryID < n_query_item; ++queryID) {
                query_heap_l.reserve(topk);
            }

            assert(check_dim_ <= vec_dim_);
            // store queryIP
            std::vector<double> queryIP_l(n_user_);
            std::vector<std::pair<int, int>> rank_bound_l(n_user_);
            std::vector<RankBoundElement> bound_topk_heap;
            // additional heap, store the other candidates
            std::vector<RankBoundElement> bound_addi_heap;
            std::vector<RankBoundElement> binary_search_bound_addi_heap;
            for (int queryID = 0; queryID < n_query_item; queryID++) {

                SVD::TransferItem(query_item.getVector(queryID), transfer_item_, old_vec_dim_);
                std::vector<double> &query = MonotonicityReduction::ConvertQuery(query_item.getVector(queryID),
                                                                                 old_vec_dim_);
                assert(query.size() == vec_dim_);

                //calculate part dimension IP
                double *query_item_vec = query.data();
                inner_product_record_.reset();
                for (int userID = 0; userID < n_user_; userID++) {
                    double *user_vec = user_.getVector(userID);
                    double queryIP = InnerProduct(query_item_vec, user_vec, check_dim_);
                    queryIP_l[userID] = queryIP;
                }
                this->inner_product_time_ += inner_product_record_.get_elapsed_time_second();

                interval_search_record_.reset();
                double *part_query_vec = query_item_vec + check_dim_;
                double query_part_norm = InnerProduct(part_query_vec, part_query_vec, vec_dim_ - check_dim_);
                //interval search, use dimension directly
                bound_topk_heap.clear();
                bound_topk_heap.reserve(topk);
                for (int userID = 0; userID < topk; userID++) {
                    double query_ub = query_part_norm * user_part_norm_l_[userID] + queryIP_l[userID];
                    double query_lb = queryIP_l[userID];
                    assert(query_lb <= query_ub);
                    std::pair<int, int> rank_pair = GetRankPairByInterval(query_lb, query_ub, userID);
                    bound_topk_heap.emplace_back(userID, queryIP_l[userID], rank_pair);
                }
                // max heap for lower rank
                std::make_heap(bound_topk_heap.begin(), bound_topk_heap.end(), std::less<RankBoundElement>());
                RankBoundElement topk_heap_ele = bound_topk_heap.front();
                bound_addi_heap.clear();
                bound_addi_heap.reserve(topk);

                for (int userID = topk; userID < n_user_; userID++) {
                    double query_ub = query_part_norm * user_part_norm_l_[userID] + queryIP_l[userID];
                    double query_lb = queryIP_l[userID];
                    assert(query_lb <= query_ub);
                    std::pair<int, int> rank_pair = GetRankPairByInterval(query_lb, query_ub, userID);
                    if (topk_heap_ele.lower_rank_ > rank_pair.first) {
                        //lower rank greater than top-k
                        //pop and push topk heap
                        std::pop_heap(bound_topk_heap.begin(), bound_topk_heap.end(),
                                      std::less<RankBoundElement>());
                        bound_topk_heap.pop_back();
                        bound_topk_heap.emplace_back(userID, queryIP_l[userID], rank_pair);
                        std::push_heap(bound_topk_heap.begin(), bound_topk_heap.end(),
                                       std::less<RankBoundElement>());
                        topk_heap_ele = bound_topk_heap.front();
                        //pop invalid element in max heap

                        if (not bound_addi_heap.empty()) {
                            RankBoundElement addi_heap_ele = bound_addi_heap.front();
                            while (not bound_addi_heap.empty() and
                                   topk_heap_ele.lower_rank_ >= addi_heap_ele.upper_rank_) {
                                std::pop_heap(bound_addi_heap.begin(), bound_addi_heap.end(),
                                              UserRankBoundElementUpperBoundMaxHeap);
                                bound_addi_heap.pop_back();
                                if (not bound_addi_heap.empty()) {
                                    addi_heap_ele = bound_addi_heap.front();
                                }
                            }
                        }

                    } else if (topk_heap_ele.lower_rank_ >= rank_pair.second) {
                        //have overlap with topk, add to addi_heap
                        bound_addi_heap.emplace_back(userID, queryIP_l[userID], rank_pair);
                        std::push_heap(bound_addi_heap.begin(), bound_addi_heap.end(),
                                       UserRankBoundElementUpperBoundMaxHeap);
                    }
                }
                this->interval_search_time_ += interval_search_record_.get_elapsed_time_second();


                int bound_topk_heap_size = (int) bound_topk_heap.size();
                int bound_addi_heap_size = (int) bound_addi_heap.size();
                //calculate the other part dimension IP
                {
                    inner_product_record_.reset();
                    query_item_vec = query.data() + check_dim_;
                    assert(bound_topk_heap_size == topk);
                    int remain_dim = vec_dim_ - check_dim_;
                    for (int candID = 0; candID < bound_topk_heap_size; candID++) {
                        int userID = bound_topk_heap[candID].userID_;
                        double *user_vec = user_.getVector(userID) + check_dim_;
                        double queryIP = InnerProduct(query_item_vec, user_vec, remain_dim);
                        assert(queryIP > 0);
                        bound_topk_heap[candID].queryIP_ += queryIP;
                    }
                    for (int candID = 0; candID < bound_addi_heap_size; candID++) {
                        int userID = bound_addi_heap[candID].userID_;
                        double *user_vec = user_.getVector(userID) + check_dim_;
                        double queryIP = InnerProduct(query_item_vec, user_vec, remain_dim);
                        assert(queryIP > 0);
                        bound_addi_heap[candID].queryIP_ += queryIP;
                    }
                    this->inner_product_time_ += inner_product_record_.get_elapsed_time_second();
                    interval_prune_ratio_ = 1.0 * (bound_topk_heap_size + bound_addi_heap_size) / n_user_;
                }

                //coarse binary search
                coarse_binary_search_record_.reset();
                for (int candID = 0; candID < bound_topk_heap_size; candID++) {
                    RankBoundElement element = bound_topk_heap[candID];
                    int &lower_rank = element.lower_rank_;
                    int &upper_rank = element.upper_rank_;
                    double &queryIP = element.queryIP_;
                    int &userID = element.userID_;
                    CoarseBinarySearch(queryIP, userID, lower_rank, upper_rank);
                    bound_topk_heap[candID].lower_rank_ = lower_rank;
                    bound_topk_heap[candID].upper_rank_ = upper_rank;
//                    if (userID == 0) {
//                        std::cout << element.ToString() << std::endl;
//                    }
                    assert(upper_rank <= lower_rank);
                }
                // max heap for lower rank
                std::make_heap(bound_topk_heap.begin(), bound_topk_heap.end(), std::less<RankBoundElement>());
                topk_heap_ele = bound_topk_heap.front();
                binary_search_bound_addi_heap.clear();
                binary_search_bound_addi_heap.reserve(bound_addi_heap_size);

                for (int candID = 0; candID < bound_addi_heap_size; candID++) {
                    RankBoundElement element = bound_addi_heap[candID];
                    int &lower_rank = element.lower_rank_;
                    int &upper_rank = element.upper_rank_;
                    double &queryIP = element.queryIP_;
                    int &userID = element.userID_;
//                    if (userID == 13) {
//                        printf("before low rank %d, before upper rank %d\n", lower_rank, upper_rank);
//                    }
                    CoarseBinarySearch(queryIP, userID, lower_rank, upper_rank);
                    bound_addi_heap[candID].lower_rank_ = lower_rank;
                    bound_addi_heap[candID].upper_rank_ = upper_rank;
//                    if (userID == 13) {
//                        printf("after low rank %d, after upper rank %d\n", lower_rank, upper_rank);
//                    }
                    assert(upper_rank <= lower_rank);
                    if (topk_heap_ele.lower_rank_ > lower_rank) {
                        //lower rank greater than top-k
                        //pop and push topk heap
                        std::pop_heap(bound_topk_heap.begin(), bound_topk_heap.end(),
                                      std::less<RankBoundElement>());
                        bound_topk_heap.pop_back();
                        bound_topk_heap.emplace_back(userID, queryIP_l[userID], std::make_pair(lower_rank, upper_rank));
                        std::push_heap(bound_topk_heap.begin(), bound_topk_heap.end(),
                                       std::less<RankBoundElement>());
                        topk_heap_ele = bound_topk_heap.front();
                        //pop invalid element in max heap

                        if (not binary_search_bound_addi_heap.empty()) {
                            RankBoundElement addi_heap_ele = binary_search_bound_addi_heap.front();
                            while (not binary_search_bound_addi_heap.empty() and
                                   topk_heap_ele.lower_rank_ >= addi_heap_ele.upper_rank_) {
                                std::pop_heap(binary_search_bound_addi_heap.begin(),
                                              binary_search_bound_addi_heap.end(),
                                              UserRankBoundElementUpperBoundMaxHeap);
                                binary_search_bound_addi_heap.pop_back();
                                if (not binary_search_bound_addi_heap.empty()) {
                                    addi_heap_ele = binary_search_bound_addi_heap.front();
                                }
                            }
                        }

                    } else if (topk_heap_ele.lower_rank_ >= upper_rank) {
                        //have overlap with topk, add to addi_heap
                        binary_search_bound_addi_heap.emplace_back(userID, queryIP_l[userID],
                                                                   std::make_pair(lower_rank, upper_rank));
                        std::push_heap(binary_search_bound_addi_heap.begin(), binary_search_bound_addi_heap.end(),
                                       UserRankBoundElementUpperBoundMaxHeap);
                    }
                }
                coarse_binary_search_time_ += coarse_binary_search_record_.get_elapsed_time_second();
                int binary_search_bound_addi_heap_size = (int) binary_search_bound_addi_heap.size();

                binary_search_prune_ratio_ = 1.0 * (bound_topk_heap_size + binary_search_bound_addi_heap_size) /
                                             (bound_topk_heap_size + bound_addi_heap_size);
                assert(bound_topk_heap.size() == topk);

                //read from disk
                // store the data of fine binary search
                read_disk_record_.reset();
                int total_read_disk_size = topk + binary_search_bound_addi_heap_size;
                std::vector<BinarySearchBoundElement> bound_cache_l(topk + binary_search_bound_addi_heap_size,
                                                                    BinarySearchBoundElement(2 * n_max_disk_read_));
                for (int candID = 0; candID < total_read_disk_size; candID++) {
                    RankBoundElement element;
                    if (candID >= topk) {
                        int _candID = candID - topk;
                        element = binary_search_bound_addi_heap[_candID];
                    } else {
                        element = bound_topk_heap[candID];
                    }
                    int &end_idx = element.lower_rank_;
                    int &start_idx = element.upper_rank_;
                    int &userID = element.userID_;
                    double &queryIP = element.queryIP_;
                    if (queryID == 0 && userID == 546) {
                        printf("have no1\n");
                        std::cout << element.ToString() << std::endl;
                    }

                    assert(start_idx <= end_idx);
                    int read_count = end_idx - start_idx;
                    index_stream_.seekg((element.userID_ * n_data_item_ + start_idx) * sizeof(double), std::ios::beg);
                    bound_cache_l[candID].ReadDisk(index_stream_, read_count);

                    bound_cache_l[candID].base_rank_ = start_idx;
                    bound_cache_l[candID].userID_ = userID;
                    bound_cache_l[candID].queryIP_ = queryIP;
                }
                read_disk_time_ += read_disk_record_.get_elapsed_time_second();

                fine_binary_search_record_.reset();
                // reuse the max heap in coarse binary search
                int total_cand_size = (int) bound_cache_l.size();
                std::vector<UserRankElement> &max_heap = query_heap_l[queryID];
                for (int candID = 0; candID < total_cand_size; candID++) {
                    BinarySearchBoundElement element = bound_cache_l[candID];
                    int rank = element.CountRank();
                    int userID = element.userID_;
                    double queryIP = element.queryIP_;
                    if (queryID == 0 && userID == 546) {
                        printf("have no1\n");
                        std::cout << element.ToString() << std::endl;
                    }
                    max_heap.emplace_back(userID, rank, queryIP);
                }
                fine_binary_search_time_ += fine_binary_search_record_.get_elapsed_time_second();

                std::sort(max_heap.begin(), max_heap.end(), std::less<UserRankElement>());
                max_heap.resize(topk);
            }

            return query_heap_l;
        }

        //return the index of the bucket it belongs to
        inline void CoarseBinarySearch(const double &queryIP, const int &userID, int &rank_lb, int &rank_ub) const {
            int bucket_ub = std::ceil(1.0 * (rank_ub - cache_bound_every_ + 1) / cache_bound_every_);
            int bucket_lb = std::floor(1.0 * (rank_lb - cache_bound_every_ + 1) / cache_bound_every_);
            if (bucket_lb - bucket_ub <= 0) {
                return;
            }

            auto iter_begin = bound_distance_table_.begin() + userID * n_cache_rank_ + bucket_ub;
            auto iter_end = bound_distance_table_.begin() + userID * n_cache_rank_ + bucket_lb + 1;

            auto lb_ptr = std::lower_bound(iter_begin, iter_end, queryIP,
                                           [](const double &arrIP, double queryIP) {
                                               return arrIP > queryIP;
                                           });
            int bucket_idx = bucket_ub + (int) (lb_ptr - iter_begin);
            int tmp_rank_lb = known_rank_idx_l_[bucket_idx];
            int tmp_rank_ub = bucket_idx == 0 ? 0 : known_rank_idx_l_[bucket_idx - 1];
            if (lb_ptr == iter_end) {
                rank_ub = tmp_rank_ub;
            } else if (lb_ptr == iter_begin) {
                rank_lb = tmp_rank_lb;
            } else {
                rank_lb = tmp_rank_lb;
                rank_ub = tmp_rank_ub;
            }
        }

        std::pair<int, int> GetRankPairByInterval(const double &query_lb, const double &query_ub, const int &userID) {
            std::pair<double, double> &ip_bound_pair = user_ip_bound_l_[userID];
            //for interval id, the higher rank value means the lower queryiP
            int itv_lb_idx = std::floor((ip_bound_pair.second - query_lb) / interval_dist_l_[userID]);
            int itv_ub_idx = std::floor((ip_bound_pair.second - query_ub) / interval_dist_l_[userID]);
            assert(itv_ub_idx <= itv_lb_idx);

            if (itv_ub_idx <= 0) {
                itv_ub_idx = 0;
            } else if (itv_ub_idx >= n_interval_ - 1) {
                itv_ub_idx = n_interval_ - 1;
            }
            if (itv_lb_idx <= 0) {
                itv_lb_idx = 0;
            } else if (itv_lb_idx >= n_interval_ - 1) {
                itv_lb_idx = n_interval_ - 1;
            }
            int *rank_ptr = interval_table_.data() + userID * n_interval_;
            int rank_ub = itv_ub_idx == 0 ? 0 : rank_ptr[itv_ub_idx - 1];
            int rank_lb = rank_ptr[itv_lb_idx];
            return std::make_pair(rank_lb, rank_ub);
        }

    };

    const double SIGMA = 0.7;

    int TransformUserItem(VectorMatrix &user, VectorMatrix &data_item, VectorMatrix &transfer_item,
                          std::vector<double> &eigen_l) {
        //SVD
        int check_dim = SVD::SVD(user, data_item, transfer_item, eigen_l, SIGMA);
        //monotonicity  reduction
        MonotonicityReduction::ConvertUserItem(user, data_item, eigen_l);
        //user normalization
        user.vectorNormalize();

        return check_dim;
    }

    const int write_every_ = 100;
    const int report_batch_every_ = 5;

    /*
     * bruteforce index
     * shape: n_user * n_data_item, type: double, the distance pair for each user
     */

    Index &BuildIndex(VectorMatrix &data_item, VectorMatrix &user, const char *index_path) {
        VectorMatrix transfer_item;
        std::vector<double> eigen_l;
        int check_dim = TransformUserItem(user, data_item, transfer_item, eigen_l);

        const int n_data_item = data_item.n_vector_;
        const int vec_dim = data_item.vec_dim_;
        const int n_user = user.n_vector_;

        //used for interval cache bound
        const int n_interval = std::min(n_data_item / 10, 5000);
        std::vector<int> interval_table(n_user * n_interval);
        std::memset(interval_table.data(), 0, n_user * n_interval * sizeof(int));
        std::vector<std::pair<double, double>> bound_l(n_user);
        std::vector<double> interval_dist_l(n_user);
        std::vector<double> user_part_norm_l(n_user);
        for (int userID = 0; userID < n_user; userID++) {
            double *user_vecs = user.getVector(userID) + check_dim;
            double part_norm = InnerProduct(user_vecs, user_vecs, vec_dim - check_dim);
            part_norm = std::sqrt(part_norm);
            user_part_norm_l[userID] = part_norm;
        }
        spdlog::info("interval bound: n_interval {}", n_interval);

        //used for coarse binary search
        const int cache_bound_every = 10;
        const int n_cache_rank = n_data_item / cache_bound_every;
        std::vector<int> known_rank_idx_l;
        for (int known_rank_idx = cache_bound_every - 1;
             known_rank_idx < n_data_item; known_rank_idx += cache_bound_every) {
            known_rank_idx_l.emplace_back(known_rank_idx);
        }
        spdlog::info("binary search bound: cache_bound_every {}, n_cache_rank {}", cache_bound_every, n_cache_rank);

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

        TimeRecord batch_report_record;
        batch_report_record.reset();
        for (int i = 0; i < n_batch; i++) {
#pragma omp parallel for default(none) shared(i, data_item, user, write_distance_cache, bound_distance_table, known_rank_idx_l, bound_l, interval_table, interval_dist_l, n_interval) shared(n_cache_rank, write_every_, n_data_item, vec_dim)
            for (int cacheID = 0; cacheID < write_every_; cacheID++) {
                int userID = write_every_ * i + cacheID;
                for (int itemID = 0; itemID < n_data_item; itemID++) {
                    double ip = InnerProduct(data_item.getVector(itemID), user.getVector(userID), vec_dim);
                    write_distance_cache[cacheID * n_data_item + itemID] = ip;
                }
                std::sort(write_distance_cache.begin() + cacheID * n_data_item,
                          write_distance_cache.begin() + (cacheID + 1) * n_data_item, std::greater<double>());

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
                    int itv_idx = std::floor((ip - lower_bound) / interval_distance);
                    assert(0 <= itv_idx && itv_idx <= n_interval);
                    interval_ptr[itv_idx]++;
                }
                for (int intervalID = 1; intervalID < n_interval; intervalID++) {
                    interval_ptr[intervalID] += interval_ptr[intervalID - 1];
                }

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
            for (int itemID = 0; itemID < data_item.n_vector_; itemID++) {
                double ip = InnerProduct(data_item.getRawData() + itemID * vec_dim,
                                         user.getRawData() + userID * vec_dim, vec_dim);
                write_distance_cache[cacheID * data_item.n_vector_ + itemID] = ip;
            }

            std::sort(write_distance_cache.begin() + cacheID * n_data_item,
                      write_distance_cache.begin() + (cacheID + 1) * n_data_item, std::greater<double>());

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
                int itv_idx = std::floor((ip - lower_bound) / interval_distance);
                assert(0 <= itv_idx && itv_idx <= n_interval);
                interval_ptr[itv_idx]++;
            }
            for (int intervalID = 1; intervalID < n_interval; intervalID++) {
                interval_ptr[intervalID] += interval_ptr[intervalID - 1];
            }

            auto array_begin = write_distance_cache.begin() + cacheID * n_data_item;
            for (int crankID = 0; crankID < n_cache_rank; crankID++) {
                int itemID = known_rank_idx_l[crankID];
                bound_distance_table[userID * n_cache_rank + crankID] = array_begin[itemID];
            }
        }

        out.write((char *) write_distance_cache.data(),
                  n_remain * data_item.n_vector_ * sizeof(double));
        static Index index(interval_table, interval_dist_l, bound_l, n_interval, user_part_norm_l,
                           bound_distance_table, known_rank_idx_l, index_path, n_max_disk_read, cache_bound_every);
        index.setUserItemMatrix(user, data_item, transfer_item, eigen_l, check_dim);
        return index;
    }

}

#endif //REVERSE_K_RANKS_INTERVALBOUND_HPP
