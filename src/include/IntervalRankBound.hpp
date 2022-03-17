//
// Created by BianZheng on 2022/3/1.
//

#ifndef REVERSE_K_RANKS_INTERVALBOUND_HPP
#define REVERSE_K_RANKS_INTERVALBOUND_HPP

#include "alg/FullDimPrune.hpp"
#include "alg/FullIntPrune.hpp"
#include "alg/FullNormPrune.hpp"

#include "alg/PartDimPartIntPrune.hpp"
#include "alg/PartDimPartNormPrune.hpp"
#include "alg/PartIntPartNormPrune.hpp"

#include "alg/PruneCandidateByBound.hpp"
#include "alg/SpaceInnerProduct.hpp"
#include "alg/SVD.hpp"
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

    class RetrievalResult : public RetrievalResultBase {
    public:
        //unit: second
        //double total_time, read_disk_time, inner_product_time,
        //          coarse_binary_search_time, fine_binary_search_time, interval_search_time;
        //double interval_prune_ratio, binary_search_prune_ratio;
        //double second_per_query;
        //int topk;

        inline RetrievalResult() {
        }

        void AddPreprocess(double build_index_time) {
            char buff[1024];
            sprintf(buff, "build index time %.3f", build_index_time);
            std::string str(buff);
            this->config_l.emplace_back(str);
        }

        std::string AddResultConfig(const int &topk,
                                    const double &total_time, const double &interval_search_time,
                                    const double &inner_product_time,
                                    const double &coarse_binary_search_time, const double &read_disk_time,
                                    const double &fine_binary_search_time,
                                    const double &full_norm_prune_ratio, const double &part_int_part_norm_prune_ratio,
                                    const double &binary_search_prune_ratio,
                                    const double &second_per_query) {
            char buff[1024];

            sprintf(buff,
                    "top%d retrieval time:\n\ttotal %.3fs, interval search %.3fs, inner product %.3fs\n\tcoarse binary search %.3fs, read disk time %.3f, fine binary search %.3fs\n\tfull norm prune ratio %.7f, part int part norm prune ratio %.7f, binary search prune ratio %.7f\n\tmillion second per query %.3fms",
                    topk,
                    total_time, interval_search_time, inner_product_time,
                    coarse_binary_search_time, read_disk_time, fine_binary_search_time,
                    full_norm_prune_ratio, part_int_part_norm_prune_ratio, binary_search_prune_ratio,
                    second_per_query);
            std::string str(buff);
            this->config_l.emplace_back(str);
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
            if (disk_cache_.empty()) {
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

    class Index : public BaseIndex {
        void ResetTimer() {
            read_disk_time_ = 0;
            inner_product_time_ = 0;
            coarse_binary_search_time_ = 0;
            fine_binary_search_time_ = 0;
            interval_search_time_ = 0;
            full_norm_prune_ratio_ = 0;
            part_int_part_norm_prune_ratio_ = 0;
            binary_search_prune_ratio_ = 0;
        }

    public:
        //for interval search, store in memory
        std::vector<int> interval_table_; // n_user * n_interval, the last element must be n_user
        std::vector<double> interval_dist_l_;
        std::vector<std::pair<double, double>> user_ip_bound_l_; // first is lower bound, second is upper bound
        int n_interval_;
        //interval search bound
        SVD svd_ins_;
        FullNormPrune full_norm_prune_;
        PartIntPartNormPrune part_int_part_norm_prune_;

        //for rank search, store in memory
        std::vector<double> bound_distance_table_; // n_user * n_cache_rank_
        std::vector<int> known_rank_idx_l_; // n_cache_rank_
        int n_cache_rank_, n_max_disk_read_, cache_bound_every_;

        //read index on disk
        const char *index_path_;

        VectorMatrix user_;
        int vec_dim_, n_data_item_, n_user_;
        double interval_search_time_, inner_product_time_, coarse_binary_search_time_, read_disk_time_, fine_binary_search_time_;
        TimeRecord read_disk_record_, inner_product_record_, coarse_binary_search_record_, fine_binary_search_record_, interval_search_record_;
        double full_norm_prune_ratio_, part_int_part_norm_prune_ratio_, binary_search_prune_ratio_;

        Index(
                //interval search
                const std::vector<int> &interval_table, const std::vector<double> &interval_dist_l,
                const std::vector<std::pair<double, double>> &user_ip_bound_l,
                const int &n_interval,
                //interval search bound
                SVD &svd_ins, FullNormPrune &full_norm_prune, PartIntPartNormPrune &part_int_part_norm_prune,
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
            this->svd_ins_ = std::move(svd_ins);
            this->full_norm_prune_ = std::move(full_norm_prune);
            this->part_int_part_norm_prune_ = std::move(part_int_part_norm_prune);
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
        setUserItemMatrix(VectorMatrix &user, const int n_data_item) {
            this->n_user_ = user.n_vector_;
            this->vec_dim_ = user.vec_dim_;
            this->user_ = std::move(user);
            this->n_data_item_ = n_data_item;
            assert(0 < this->user_.vec_dim_);
            assert(interval_dist_l_.size() == n_user_);
        }

        std::vector<std::vector<UserRankElement>> Retrieval(VectorMatrix &query_item, const int &topk) override {
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

            // store queryIP
            std::vector<UserRankElement> user_topk_cache_l(n_user_);
            std::vector<RankBoundElement> rank_bound_l(n_user_);
            std::vector<std::pair<double, double>> ip_bound_l(n_user_);
            std::vector<char> prune_l(n_user_);
            std::unique_ptr<double[]> query_ptr = std::make_unique<double[]>(vec_dim_);
            std::vector<int> rank_search_topk_max_heap(topk);
            std::vector<double> disk_cache(2 * n_max_disk_read_);
            for (int queryID = 0; queryID < n_query_item; queryID++) {

                for (int userID = 0; userID < n_user_; userID++) {
                    prune_l[userID] = 0;
                }

                double *query_vecs = query_ptr.get();
                svd_ins_.TransferQuery(query_item.getVector(queryID), vec_dim_, query_vecs);
                assert(rank_bound_l.size() == n_user_);
                for (int userID = 0; userID < n_user_; userID++) {
                    rank_bound_l[userID].userID_ = userID;
                }

                interval_search_record_.reset();
//                printf("happen before query bound queryID %d, topk %d\n", queryID, topk);
                //full norm
                full_norm_prune_.QueryBound(query_vecs, user_, n_user_, rank_bound_l, true);
//                printf("happen after query bound queryID %d, topk %d\n", queryID, topk);

                int n_candidate = n_user_;
                AllIntervalSearch(rank_bound_l, prune_l, n_candidate, topk);
//                printf("happen after all interval search queryID %d, topk %d\n", queryID, topk);

                full_norm_prune_ratio_ += 1.0 * (n_user_ - n_candidate) / n_user_;

                part_int_part_norm_prune_.QueryBound(query_vecs, user_, n_candidate, rank_bound_l, true);

                AllIntervalSearch(rank_bound_l, prune_l, n_candidate, topk);

                this->interval_search_time_ += interval_search_record_.get_elapsed_time_second();
                part_int_part_norm_prune_ratio_ += 1.0 * (n_user_ - n_candidate) / n_user_;

                //calculate the exact IP
                inner_product_record_.reset();
                for (int candID = 0; candID < n_candidate; candID++) {
                    RankBoundElement &element = rank_bound_l[candID];
                    int userID = element.userID_;
                    element.lower_bound_ = InnerProduct(user_.getVector(userID), query_vecs, vec_dim_);
                }
                this->inner_product_time_ += inner_product_record_.get_elapsed_time_second();
//                printf("happen after inner product queryID %d, topk %d\n", queryID, topk);

                //coarse binary search
                coarse_binary_search_record_.reset();
                int global_lower_rank = 0;
                for (int candID = 0; candID < n_candidate; candID++) {
                    int userID = rank_bound_l[candID].userID_;
                    int &lower_rank = rank_bound_l[candID].lower_rank_;
                    int &upper_rank = rank_bound_l[candID].upper_rank_;
                    assert(upper_rank <= lower_rank);
                    double queryIP = rank_bound_l[candID].lower_bound_;

                    if (candID < topk) {
                        rank_search_topk_max_heap[candID] = lower_rank;
                        if (candID == topk - 1) {
                            std::make_heap(rank_search_topk_max_heap.begin(),
                                           rank_search_topk_max_heap.end(),
                                           std::less());
                            global_lower_rank = rank_search_topk_max_heap.front();
                        }
                    } else if (lower_rank < global_lower_rank) {
                        std::pop_heap(rank_search_topk_max_heap.begin(), rank_search_topk_max_heap.end(),
                                      std::less());
                        rank_search_topk_max_heap[topk - 1] = global_lower_rank;
                        std::push_heap(rank_search_topk_max_heap.begin(), rank_search_topk_max_heap.end(),
                                       std::less());
                        global_lower_rank = rank_search_topk_max_heap.front();
                    }

                    if (candID >= topk && global_lower_rank < upper_rank) {
                        prune_l[candID] = 1; //true
                        continue;
                    }
                    CoarseBinarySearch(queryIP, userID, lower_rank, upper_rank);
                }
//                printf("happen after coarse binary search queryID %d, topk %d\n", queryID, topk);
                int n_remain;
                PruneCandidateByBound(rank_bound_l, n_candidate, topk, n_remain, prune_l.data());
//                printf("happen after coarse binary search prune queryID %d, topk %d\n", queryID, topk);
                n_candidate = topk + n_remain;
                coarse_binary_search_time_ += coarse_binary_search_record_.get_elapsed_time_second();
                binary_search_prune_ratio_ = 1.0 * (n_user_ - n_candidate) / n_user_;

                //read disk and fine binary search
                for (int candID = 0; candID < n_candidate; candID++) {
                    RankBoundElement element = rank_bound_l[candID];
                    int end_idx = element.lower_rank_;
                    int start_idx = element.upper_rank_;
                    int userID = element.userID_;
                    double queryIP = element.lower_bound_;
                    int &base_rank = start_idx;
                    int read_count = end_idx - start_idx;

                    assert(start_idx <= end_idx);

                    read_disk_record_.reset();
                    ReadDisk(index_stream_, userID, start_idx, read_count, disk_cache);
                    read_disk_time_ += read_disk_record_.get_elapsed_time_second();

                    fine_binary_search_record_.reset();
                    int rank = FineBinarySearch(disk_cache, queryIP, userID, base_rank, read_count);
                    fine_binary_search_time_ += fine_binary_search_record_.get_elapsed_time_second();

                    user_topk_cache_l[candID] = UserRankElement(userID, rank, queryIP);
                }
                std::sort(user_topk_cache_l.begin(), user_topk_cache_l.begin() + n_candidate,
                          std::less<UserRankElement>());
                for (int candID = 0; candID < topk; candID++) {
                    query_heap_l[queryID].emplace_back(user_topk_cache_l[candID]);
                }

//                // read from disk
//                // store the data of fine binary search
//                read_disk_record_.reset();
//                std::vector<BinarySearchBoundElement> bound_cache_l(n_candidate,
//                                                                    BinarySearchBoundElement(2 * n_max_disk_read_));
//                for (int candID = 0; candID < n_candidate; candID++) {
//                    RankBoundElement element = rank_bound_l[candID];
//                    int end_idx = element.lower_rank_;
//                    int start_idx = element.upper_rank_;
//                    int userID = element.userID_;
//                    double queryIP = element.lower_bound_;
//
//                    assert(start_idx <= end_idx);
//                    int read_count = end_idx - start_idx;
//                    index_stream_.seekg((element.userID_ * n_data_item_ + start_idx) * sizeof(double),
//                                        std::ios::beg);
//                    bound_cache_l[candID].ReadDisk(index_stream_, read_count);
//
//                    bound_cache_l[candID].base_rank_ = start_idx;
//                    bound_cache_l[candID].userID_ = userID;
//                    bound_cache_l[candID].queryIP_ = queryIP;
//
//                }
//                read_disk_time_ += read_disk_record_.get_elapsed_time_second();
//                printf("happen after read disk queryID %d, topk %d\n", queryID, topk);
//
//                fine_binary_search_record_.reset();
//                // reuse the max heap in coarse binary search
//                int total_cand_size = (int) bound_cache_l.size();
//
//                std::vector<UserRankElement> &max_heap = query_heap_l[queryID];
//                for (int candID = 0; candID < total_cand_size; candID++) {
//                    BinarySearchBoundElement element = bound_cache_l[candID];
//                    int rank = element.CountRank();
//                    int userID = element.userID_;
//                    double queryIP = element.queryIP_;
//                    max_heap.emplace_back(userID, rank, queryIP);
//                }
//                fine_binary_search_time_ += fine_binary_search_record_.get_elapsed_time_second();
//                printf("happen after fine binary search queryID %d, topk %d\n", queryID, topk);
//
//                std::sort(max_heap.begin(), max_heap.end(), std::less<UserRankElement>());
//                max_heap.resize(topk);
            }

            full_norm_prune_ratio_ /= n_query_item;
            part_int_part_norm_prune_ratio_ /= n_query_item;

            return query_heap_l;
        }

        inline void
        ReadDisk(std::ifstream &index_stream, const int &userID, const int &start_idx, const int &read_count,
                 std::vector<double> &disk_cache) const {
            index_stream.seekg((userID * n_data_item_ + start_idx) * sizeof(double), std::ios::beg);
            index_stream.read((char *) disk_cache.data(), read_count * sizeof(double));
        }

        inline int FineBinarySearch(const std::vector<double> &disk_cache, const double &queryIP, const int &userID,
                                    const int &base_rank,
                                    const int &read_count) {
            if (read_count == 0) {
                return base_rank + 1;
            }
            auto iter_begin = disk_cache.begin();
            auto iter_end = disk_cache.begin() + read_count;

            auto lb_ptr = std::lower_bound(iter_begin, iter_end, queryIP,
                                           [](const double &arrIP, double queryIP) {
                                               return arrIP > queryIP;
                                           });
            return (int) (lb_ptr - iter_begin) + base_rank + 1;
        }

        //return the index of the bucket it belongs to
        inline void
        CoarseBinarySearch(const double &queryIP, const int &userID, int &rank_lb, int &rank_ub) const {
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

            if (lb_ptr == iter_end) {
                rank_ub = tmp_rank_ub;
            } else if (lb_ptr == iter_begin) {
                rank_lb = tmp_rank_lb;
            } else {
                rank_lb = tmp_rank_lb;
                rank_ub = tmp_rank_ub;
            }

        }

        void AllIntervalSearch(std::vector<RankBoundElement> &candidate_l, std::vector<char> &prune_l,
                               int &n_candidate, const int &topk) {
            assert(candidate_l.size() >= n_candidate);
            for (int candID = 0; candID < n_candidate; candID++) {
                RankBoundElement tmp_element = candidate_l[candID];
                int userID = tmp_element.userID_;
                std::pair<double, double> IP_bound = tmp_element.IPBound();
                assert(IP_bound.first <= IP_bound.second);
                std::pair<int, int> pair = IntervalSearch(IP_bound, userID);
                candidate_l[candID].lower_rank_ = pair.first;
                candidate_l[candID].upper_rank_ = pair.second;
            }

            int n_remain;
            PruneCandidateByBound(candidate_l, n_candidate, topk, n_remain, prune_l.data());
            n_candidate = topk + n_remain;
        }

        std::pair<int, int>
        IntervalSearch(const std::pair<double, double> &IPbound, const int &userID) {
            std::pair<double, double> ip_bound_pair = user_ip_bound_l_[userID];
            double itv_dist = interval_dist_l_[userID];
            assert(ip_bound_pair.first <= ip_bound_pair.second);
            //for interval id, the higher rank value means the lower queryiP

            long long l_lb = std::ceil((ip_bound_pair.second - IPbound.first) / itv_dist);
            long long l_ub = (long long) std::floor((ip_bound_pair.second - IPbound.second) / itv_dist) - 1;

            int itv_lb_idx = (int) (l_lb % 1000000000);
            int itv_ub_idx = (int) (l_ub % 1000000000);

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
            assert(itv_ub_idx <= itv_lb_idx);
            int *rank_ptr = interval_table_.data() + userID * n_interval_;
            int rank_lb = itv_lb_idx == -1 ? 0 : rank_ptr[itv_lb_idx];
            int rank_ub = itv_ub_idx == -1 ? 0 : rank_ptr[itv_ub_idx];

            return std::make_pair(rank_lb, rank_ub);
        }

    };

    const int write_every_ = 100;
    const int report_batch_every_ = 5;

    /*
     * bruteforce index
     * shape: n_user * n_data_item, type: double, the distance pair for each user
     */

    Index &BuildIndex(VectorMatrix &user, VectorMatrix &data_item, const char *index_path) {
        const int n_data_item = data_item.n_vector_;
        const int vec_dim = data_item.vec_dim_;
        const int n_user = user.n_vector_;

        user.vectorNormalize();

        const double SIGMA = 0.7;
        const double scale = 100;
        SVD svd_ins;
        int check_dim = svd_ins.Preprocess(user, data_item, SIGMA);

        FullNormPrune full_norm_prune;
        full_norm_prune.Preprocess(user);

        PartIntPartNormPrune part_int_part_norm_prune;
        part_int_part_norm_prune.Preprocess(user, check_dim, scale);

        //interval search
        const int n_interval = std::min(n_data_item / 10, 5000);
        std::vector<int> interval_table(n_user * n_interval);
        std::memset(interval_table.data(), 0, n_user * n_interval * sizeof(int));

        std::vector<std::pair<double, double>> bound_l(n_user);
        std::vector<double> interval_dist_l(n_user);
        spdlog::info("interval bound: n_interval {}", n_interval);

        //rank search
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
#pragma omp parallel for default(none) shared(i, data_item, user, write_distance_cache, bound_distance_table, known_rank_idx_l, bound_l, interval_table, interval_dist_l, check_dim) shared(n_cache_rank, write_every_, n_data_item, vec_dim, n_interval)
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
                svd_ins, full_norm_prune, part_int_part_norm_prune,
                //rank search
                bound_distance_table, known_rank_idx_l, n_max_disk_read, cache_bound_every,
                //general retrieval
                index_path);
        index.setUserItemMatrix(user, n_data_item);
        return index;
    }

}

#endif //REVERSE_K_RANKS_INTERVALBOUND_HPP
