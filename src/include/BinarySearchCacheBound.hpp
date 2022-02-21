//
// Created by BianZheng on 2022/2/20.
//

#ifndef REVERSE_KRANKS_BINARYSEARCHCACHEBOUND_HPP
#define REVERSE_KRANKS_BINARYSEARCHCACHEBOUND_HPP

#include "struct/VectorMatrix.hpp"
#include "struct/DistancePair.hpp"
#include "struct/UserBucketElement.hpp"
#include "alg/SpaceInnerProduct.hpp"
#include "util/TimeMemory.hpp"
#include "util/VectorIO.hpp"
#include <fstream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <set>
#include <cassert>
#include <spdlog/spdlog.h>

namespace ReverseMIPS {

    class BinarySearchCacheBoundIndex {
        void ResetTimer() {
            read_disk_time_ = 0;
            inner_product_time_ = 0;
            binary_search_time_ = 0;
        }

    public:
        //bound for binary search, store in memory
        std::vector<double> bound_distance_table_; // n_user * n_cache_rank_
        std::vector<int> known_rank_idx_l_; // n_cache_rank_
        int n_cache_rank_;

        //read index on disk
        const char *index_path_;

        VectorMatrix user_;
        int vec_dim_, n_data_item_, n_user_;
        double read_disk_time_, inner_product_time_, binary_search_time_;
        TimeRecord read_disk_record, inner_product_record, binary_search_record;

        BinarySearchCacheBoundIndex(const std::vector<double> &bound_distance_table,
                                    const std::vector<int> &known_rank_idx_l,
                                    const char *index_path) {
            this->bound_distance_table_ = bound_distance_table;
            this->known_rank_idx_l_ = known_rank_idx_l;
            this->index_path_ = index_path;
            this->n_cache_rank_ = (int) known_rank_idx_l.size();
        }

        void setUserItemMatrix(const VectorMatrix &user, const VectorMatrix &data_item) {
            this->user_ = user;
            this->n_user_ = user.n_vector_;
            this->n_data_item_ = data_item.n_vector_;
            this->vec_dim_ = user.vec_dim_;
        }

        ~BinarySearchCacheBoundIndex() {}

        std::vector<std::vector<UserRankElement>> Retrieval(VectorMatrix &query_item, const int topk) {
            TimeRecord batch_report_record;
            ResetTimer();
            std::ifstream index_stream_ = std::ifstream(this->index_path_, std::ios::binary | std::ios::in);
            if (!index_stream_) {
                std::printf("error in writing index\n");
            }

            if (topk > user_.n_vector_) {
                printf("top-k is too large, program exit\n");
                exit(-1);
            }

            //coarse binary search
            //store bucketID in the set
            const int n_query_item = query_item.n_vector_;

            //first dimension: userID, key: bucketID, value: queryItemID, queryIP, shape: n_user * unordered_map
            std::vector<std::unordered_map<int, std::vector<std::pair<int, double>>>> candidates_invert_index_l(n_user_,
                                                                                                                std::unordered_map<int, std::vector<std::pair<int, double>>>());
            //store the bucketID that queryIP fall in, for each query. used for coarse binary search
            std::vector<UserBucketElement> user_bucket_l(n_user_);

            for (int queryID = 0; queryID < n_query_item; ++queryID) {
                for (int userID = 0; userID < n_user_; ++userID) {
                    inner_product_record.reset();
                    double *user_vec = user_.getVector(userID);
                    double *query_item_vec = query_item.getVector(queryID);
                    double queryIP = InnerProduct(query_item_vec, user_vec, vec_dim_);
                    this->inner_product_time_ += inner_product_record.get_elapsed_time_second();
                    int bucketID = MemoryBinarySearch(queryIP, userID);
                    assert(0 <= bucketID && bucketID <= n_cache_rank_);
                    user_bucket_l[userID] = UserBucketElement(userID, bucketID, queryIP);
                }
                //small bucketID means higher rank
                std::sort(user_bucket_l.begin(), user_bucket_l.end(), std::less<UserBucketElement>());
                int topk_bucketID = user_bucket_l[topk - 1].bucketID_;
                int end_ptr = topk;
                while (end_ptr < n_user_ && topk_bucketID == user_bucket_l[end_ptr].bucketID_) {
                    ++end_ptr;
                }
                for (int i = 0; i < end_ptr; ++i) {
                    int tmp_userID = user_bucket_l[i].userID_;
                    int tmp_bucketID = user_bucket_l[i].bucketID_;
                    double tmp_queryIP = user_bucket_l[i].queryIP_;

                    auto find_iter = candidates_invert_index_l[tmp_userID].find(tmp_bucketID);
                    if (find_iter == candidates_invert_index_l[tmp_userID].end()) {
                        candidates_invert_index_l[tmp_userID].insert(
                                std::make_pair(tmp_bucketID, std::vector<std::pair<int, double>>{
                                        std::make_pair(queryID, tmp_queryIP)}));
                    } else {
                        find_iter->second.emplace_back(queryID, tmp_queryIP);
                    }

                }
            }
            user_bucket_l.clear();

            std::vector<std::vector<UserRankElement>> query_heap_l(n_query_item, std::vector<UserRankElement>());
            for (int queryID = 0; queryID < n_query_item; ++queryID) {
                query_heap_l.reserve(topk);
            }

            //read the candidate rank all in one time
            std::vector<double> distance_cache(n_data_item_);
            for (int userID = 0; userID < n_user_; userID++) {
                std::unordered_map<int, std::vector<std::pair<int, double>>> &invert_index = candidates_invert_index_l[userID];
                for (auto &iter: invert_index) {
                    int bucketID = iter.first;
                    int start_idx = bucketID == 0 ? 0 : known_rank_idx_l_[bucketID - 1];
                    int end_idx = bucketID == n_cache_rank_ ? n_data_item_ : known_rank_idx_l_[bucketID];
                    assert(start_idx < end_idx);
                    index_stream_.seekg(sizeof(double) * (userID * n_data_item_ + start_idx), std::ios::beg);
                    index_stream_.read((char *) distance_cache.data(), (end_idx - start_idx) * sizeof(double));
                    auto start_iter = distance_cache.begin();
                    auto end_iter = distance_cache.begin() + end_idx - start_idx;
                    for (auto &queryIter: iter.second) {
                        int queryID = queryIter.first;
                        double queryIP = queryIter.second;

                        auto lb_ptr = std::lower_bound(start_iter, end_iter, queryIP,
                                                       [](const double &info, double value) {
                                                           return info > value;
                                                       });
                        int offset_rank = (int) (lb_ptr - start_iter);
                        int base_rank = bucketID == 0 ? 0 : known_rank_idx_l_[bucketID - 1];
                        int rank = base_rank + offset_rank + 1;
                        if (query_heap_l[queryID].size() < topk) {
                            query_heap_l[queryID].emplace_back(userID, rank);
                        } else {
                            std::vector<UserRankElement> &minHeap = query_heap_l[queryID];
                            std::make_heap(minHeap.begin(), minHeap.end(), std::less<UserRankElement>());
                            UserRankElement minHeapEle = minHeap.front();
                            UserRankElement rankElement(userID, rank);
                            if (minHeapEle.rank_ > rankElement.rank_) {
                                std::pop_heap(minHeap.begin(), minHeap.end(), std::less<UserRankElement>());
                                minHeap.pop_back();
                                minHeap.push_back(rankElement);
                                std::push_heap(minHeap.begin(), minHeap.end(), std::less<UserRankElement>());
                            }

                        }
                    }
                }
            }

            index_stream_.close();

            for (int qID = 0; qID < n_query_item; qID++) {
                std::sort(query_heap_l[qID].begin(), query_heap_l[qID].end(), std::less<UserRankElement>());
                assert(query_heap_l[qID].size() == topk);
            }
            return query_heap_l;
        }

        //return the index of the bucket it belongs to
        [[nodiscard]] int MemoryBinarySearch(double queryIP, int userID) const {
            auto iter_begin = bound_distance_table_.begin() + userID * n_cache_rank_;
            auto iter_end = bound_distance_table_.begin() + (userID + 1) * n_cache_rank_;

            auto lb_ptr = std::lower_bound(iter_begin, iter_end, queryIP,
                                           [](const double &arrIP, double queryIP) {
                                               return arrIP > queryIP;
                                           });
            return (int) (lb_ptr - iter_begin);
        }

    };

    const int write_every_ = 100;
    const int report_batch_every_ = 5;

    /*
     * bruteforce index
     * shape: n_user * n_data_item, type: double, the distance pair for each user
     */

    BinarySearchCacheBoundIndex
    BuildBinarySearchCacheBoundIndex(const VectorMatrix &data_item, const VectorMatrix &user, const char *index_path) {
        std::ofstream out(index_path, std::ios::binary | std::ios::out);
        if (!out) {
            std::printf("error in write result\n");
        }
        const int n_data_item = data_item.n_vector_;
        std::vector<double> write_distance_cache(write_every_ * n_data_item);
        const int vec_dim = data_item.vec_dim_;
        const int n_batch = user.n_vector_ / write_every_;
        const int n_remain = user.n_vector_ % write_every_;

        //隔着多少个建模
        const int cache_bound_every = 10;
        const int n_cache_rank = n_data_item / cache_bound_every;
        std::vector<int> known_rank_idx_l;
        for (int known_rank_idx = cache_bound_every - 1;
             known_rank_idx < n_data_item; known_rank_idx += cache_bound_every) {
            known_rank_idx_l.emplace_back(known_rank_idx);
        }
        assert(known_rank_idx_l.size() == n_cache_rank);

        //used for coarse binary search
        std::vector<double> bound_distance_table(user.n_vector_ * n_cache_rank);

        TimeRecord batch_report_record;
        batch_report_record.reset();
        for (int i = 0; i < n_batch; i++) {
//#pragma omp parallel for default(none) shared(i, data_item, user, write_distance_cache, bound_distance_table, known_rank_idx_l) //shared(n_cache_rank, write_every_, n_data_item, vec_dim)
            for (int cacheID = 0; cacheID < write_every_; cacheID++) {
                int userID = write_every_ * i + cacheID;
                for (int itemID = 0; itemID < n_data_item; itemID++) {
                    double ip = InnerProduct(data_item.getVector(itemID), user.getVector(userID), vec_dim);
                    write_distance_cache[cacheID * n_data_item + itemID] = ip;
                }
                std::sort(write_distance_cache.begin() + cacheID * n_data_item,
                          write_distance_cache.begin() + (cacheID + 1) * n_data_item, std::greater<double>());

                auto array_begin = write_distance_cache.begin() + cacheID * n_data_item;
                for (int bucketID = 0; bucketID < n_cache_rank; bucketID++) {
                    int itemID = known_rank_idx_l[bucketID];
                    bound_distance_table[userID * n_cache_rank + bucketID] = array_begin[itemID];
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
                double ip = InnerProduct(data_item.rawData_ + itemID * vec_dim,
                                         user.rawData_ + userID * vec_dim, vec_dim);
                write_distance_cache[cacheID * data_item.n_vector_ + itemID] = ip;
            }

            std::sort(write_distance_cache.begin() + cacheID * n_data_item,
                      write_distance_cache.begin() + (cacheID + 1) * n_data_item, std::greater<double>());

            auto array_begin = write_distance_cache.begin() + cacheID * n_data_item;
            for (int bucketID = 0; bucketID < n_cache_rank; bucketID++) {
                int itemID = known_rank_idx_l[bucketID];
                bound_distance_table[userID * n_cache_rank + bucketID] = array_begin[itemID];
            }
        }

        out.write((char *) write_distance_cache.data(),
                  n_remain * data_item.n_vector_ * sizeof(double));
        BinarySearchCacheBoundIndex index(bound_distance_table, known_rank_idx_l, index_path);
        index.setUserItemMatrix(user, data_item);
        return index;
    }

}
#endif //REVERSE_KRANKS_BINARYSEARCHCACHEBOUND_HPP
