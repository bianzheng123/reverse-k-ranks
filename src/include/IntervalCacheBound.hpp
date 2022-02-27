//
// Created by BianZheng on 2022/2/27.
//

#ifndef REVERSE_K_RANKS_INTERVALCACHEBOUND_HPP
#define REVERSE_K_RANKS_INTERVALCACHEBOUND_HPP

#include "struct/VectorMatrix.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/MethodBase.hpp"
#include "alg/SpaceInnerProduct.hpp"
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

namespace ReverseMIPS::IntervalCacheBound {

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

    class Index : public BaseIndex {
        void ResetTimer() {
            read_disk_time_ = 0;
            inner_product_time_ = 0;
            coarse_binary_search_time_ = 0;
            fine_binary_search_time_ = 0;
        }

    public:
        //bound for binary search, store in memory
        std::vector<double> bound_distance_table_; // n_user * n_cache_rank_
        std::vector<int> known_rank_idx_l_; // n_cache_rank_
        int n_cache_rank_, n_max_read_;

        //read index on disk
        const char *index_path_;

        VectorMatrix user_;
        int vec_dim_, n_data_item_, n_user_;
        double read_disk_time_, inner_product_time_, coarse_binary_search_time_, fine_binary_search_time_;
        TimeRecord read_disk_record_, inner_product_record_, coarse_binary_search_record_, fine_binary_search_record_;

        Index(const std::vector<double> &bound_distance_table,
              const std::vector<int> &known_rank_idx_l,
              const char *index_path, const int n_max_read) {
            this->bound_distance_table_ = bound_distance_table;
            this->known_rank_idx_l_ = known_rank_idx_l;
            this->index_path_ = index_path;
            this->n_cache_rank_ = (int) known_rank_idx_l.size();
            this->n_max_read_ = n_max_read;
        }

        void setUserItemMatrix(const VectorMatrix &user, const VectorMatrix &data_item) {
            this->user_ = user;
            this->n_user_ = user.n_vector_;
            this->n_data_item_ = data_item.n_vector_;
            this->vec_dim_ = user.vec_dim_;
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

            //coarse binary search
            const int n_query_item = query_item.n_vector_;

            std::vector<std::vector<UserRankElement>> query_heap_l(n_query_item, std::vector<UserRankElement>());
            for (int queryID = 0; queryID < n_query_item; ++queryID) {
                query_heap_l.reserve(topk);
            }

            // store queryIP
            std::vector<double> queryIP_l(n_user_);
            // for binary search, check the number
            std::vector<int> heap_count_map(n_cache_rank_ + 1);
            for (int queryID = 0; queryID < n_query_item; queryID++) {
                //calculate IP
                double *query_item_vec = query_item.getVector(queryID);
                inner_product_record_.reset();
                for (int userID = 0; userID < n_user_; userID++) {
                    double *user_vec = user_.getVector(userID);
                    double queryIP = InnerProduct(query_item_vec, user_vec, vec_dim_);
                    queryIP_l[userID] = queryIP;
                }
                this->inner_product_time_ += inner_product_record_.get_elapsed_time_second();

                //binary search
                coarse_binary_search_record_.reset();
                std::vector<UserRankElement> &max_heap = query_heap_l[queryID]; // small heap by rank
                heap_count_map.assign(n_cache_rank_ + 1, 0);
                for (int userID = 0; userID < topk; userID++) {
                    double queryIP = queryIP_l[userID];
                    int crank = CoarseBinarySearch(queryIP, userID);
                    max_heap.emplace_back(userID, crank, queryIP);
                    heap_count_map[crank]++;
                }

                std::make_heap(max_heap.begin(), max_heap.end(), std::less<UserRankElement>());
                for (int userID = topk; userID < n_user_; userID++) {
                    double queryIP = queryIP_l[userID];
                    UserRankElement element = max_heap.front();
                    int crank = CoarseBinarySearchBound(queryIP, userID, element.rank_);
                    if (crank != -1) {
                        if (crank == element.rank_) {
                            max_heap.emplace_back(userID, crank, queryIP);
                            std::push_heap(max_heap.begin(), max_heap.end(), std::less<UserRankElement>());
                            heap_count_map[crank]++;
                        } else { // crank < element.rank_
                            int min_rank_count = heap_count_map[element.rank_];
                            if (max_heap.size() - min_rank_count + 1 == topk) {
                                // the other greater rank is enough to fill the top-k, delete all the minimum rank
                                while (max_heap.size() >= topk) {
                                    std::pop_heap(max_heap.begin(), max_heap.end(), std::less<UserRankElement>());
                                    max_heap.pop_back();
                                }
                                max_heap.emplace_back(userID, crank, queryIP);
                                std::push_heap(max_heap.begin(), max_heap.end(), std::less<UserRankElement>());
                                heap_count_map[crank]++;
                                heap_count_map[element.rank_] = 0;
                            } else {
                                max_heap.emplace_back(userID, crank, queryIP);
                                std::push_heap(max_heap.begin(), max_heap.end(), std::less<UserRankElement>());
                                heap_count_map[crank]++;
                            }
                        }
                    }
                    if (queryID == 277 && userID == 21) {
                        printf("this crank %d, queryIP %.3f, userID %d\n\t prev crank %d, queryIP %.3f, userID %d\n",
                               crank, queryIP, userID, element.rank_, element.queryIP_, element.userID_);
                    }
                }
                coarse_binary_search_time_ += coarse_binary_search_record_.get_elapsed_time_second();

                assert(max_heap.size() >= topk);

                //read from disk

                // store the data of fine binary search
                read_disk_record_.reset();
                int max_heap_size = max_heap.size();
                std::vector<int> read_count_l(max_heap_size);
                std::vector<std::vector<double>> distance_cache(max_heap_size, std::vector<double>(n_max_read_));
                for (int candID = 0; candID < max_heap_size; candID++) {
                    UserRankElement element = max_heap[candID];

                    int crank = element.rank_;
                    int start_idx = crank == 0 ? 0 : known_rank_idx_l_[crank - 1] + 1;
                    int end_idx = crank == n_cache_rank_ ? n_data_item_ : known_rank_idx_l_[crank];
                    assert(start_idx <= end_idx);
                    int read_count = end_idx - start_idx;
                    index_stream_.seekg((element.userID_ * n_data_item_ + start_idx) * sizeof(double), std::ios::beg);
                    index_stream_.read((char *) distance_cache[candID].data(), read_count * sizeof(double));

                    read_count_l[candID] = read_count;
                }
                read_disk_time_ += read_disk_record_.get_elapsed_time_second();

                fine_binary_search_record_.reset();
                // reuse the max heap in coarse binary search
                for (int candID = 0; candID < max_heap_size; candID++) {
                    int crank = max_heap[candID].rank_;
                    int offset_rank = FineBinarySearch(max_heap[candID].queryIP_, read_count_l[candID],
                                                       distance_cache[candID]);
                    int base_rank = crank == 0 ? 0 : known_rank_idx_l_[crank - 1] + 1;
                    int rank = base_rank + offset_rank + 1;
                    max_heap[candID].rank_ = rank;
                }
                fine_binary_search_time_ += fine_binary_search_record_.get_elapsed_time_second();

                std::sort(max_heap.begin(), max_heap.end(), std::less<UserRankElement>());
                max_heap.resize(topk);
            }

            return query_heap_l;
        }

        //return the index of the bucket it belongs to
        [[nodiscard]] inline int CoarseBinarySearch(double queryIP, int userID) const {
            auto iter_begin = bound_distance_table_.begin() + userID * n_cache_rank_;
            auto iter_end = bound_distance_table_.begin() + (userID + 1) * n_cache_rank_;

            auto lb_ptr = std::lower_bound(iter_begin, iter_end, queryIP,
                                           [](const double &arrIP, double queryIP) {
                                               return arrIP > queryIP;
                                           });
            return (int) (lb_ptr - iter_begin);
        }

        //return the index of the bucket it belongs to
        [[nodiscard]] inline int CoarseBinarySearchBound(double queryIP, int userID, int bound_rank_id) const {
            auto iter_begin = bound_distance_table_.begin() + userID * n_cache_rank_;
            if (bound_rank_id != n_cache_rank_ && iter_begin[bound_rank_id] > queryIP) {
                return -1;
            }
            int offset_size = bound_rank_id == n_cache_rank_ ? n_cache_rank_ - 1 : bound_rank_id;
            auto iter_end = iter_begin + offset_size + 1;

            auto lb_ptr = std::lower_bound(iter_begin, iter_end, queryIP,
                                           [](const double &arrIP, double queryIP) {
                                               return arrIP > queryIP;
                                           });
            return (int) (lb_ptr - iter_begin);
        }

        //return the index of the bucket it belongs to
        [[nodiscard]] inline int
        FineBinarySearch(double queryIP, int read_count, std::vector<double> &distance_cache) const {
            auto iter_begin = distance_cache.begin();
            auto iter_end = distance_cache.begin() + read_count;

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

    Index BuildIndex(const VectorMatrix &data_item, const VectorMatrix &user, const char *index_path) {
        std::ofstream out(index_path, std::ios::binary | std::ios::out);
        if (!out) {
            spdlog::error("error in write result");
        }
        const int n_data_item = data_item.n_vector_;
        std::vector<double> write_distance_cache(write_every_ * n_data_item);
        const int vec_dim = data_item.vec_dim_;
        const int n_batch = user.n_vector_ / write_every_;
        const int n_remain = user.n_vector_ % write_every_;

        //
        const int cache_bound_every = 10;
        const int n_cache_rank = n_data_item / cache_bound_every;
        std::vector<int> known_rank_idx_l;
        for (int known_rank_idx = cache_bound_every - 1;
             known_rank_idx < n_data_item; known_rank_idx += cache_bound_every) {
            known_rank_idx_l.emplace_back(known_rank_idx);
        }

        assert(known_rank_idx_l[0] == known_rank_idx_l[1] - (known_rank_idx_l[0] + 1));
        int n_max_read = std::max(known_rank_idx_l[0], n_data_item - (known_rank_idx_l[n_cache_rank - 1] + 1));
        assert(known_rank_idx_l.size() == n_cache_rank);

        //used for coarse binary search
        std::vector<double> bound_distance_table(user.n_vector_ * n_cache_rank);

        TimeRecord batch_report_record;
        batch_report_record.reset();
        for (int i = 0; i < n_batch; i++) {
#pragma omp parallel for default(none) shared(i, data_item, user, write_distance_cache, bound_distance_table, known_rank_idx_l) shared(n_cache_rank, write_every_, n_data_item, vec_dim)
            for (int cacheID = 0; cacheID < write_every_; cacheID++) {
                int userID = write_every_ * i + cacheID;
                for (int itemID = 0; itemID < n_data_item; itemID++) {
                    double ip = InnerProduct(data_item.getVector(itemID), user.getVector(userID), vec_dim);
                    write_distance_cache[cacheID * n_data_item + itemID] = ip;
                }
                std::sort(write_distance_cache.begin() + cacheID * n_data_item,
                          write_distance_cache.begin() + (cacheID + 1) * n_data_item, std::greater<double>());

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
                double ip = InnerProduct(data_item.rawData_ + itemID * vec_dim,
                                         user.rawData_ + userID * vec_dim, vec_dim);
                write_distance_cache[cacheID * data_item.n_vector_ + itemID] = ip;
            }

            std::sort(write_distance_cache.begin() + cacheID * n_data_item,
                      write_distance_cache.begin() + (cacheID + 1) * n_data_item, std::greater<double>());

            auto array_begin = write_distance_cache.begin() + cacheID * n_data_item;
            for (int crankID = 0; crankID < n_cache_rank; crankID++) {
                int itemID = known_rank_idx_l[crankID];
                bound_distance_table[userID * n_cache_rank + crankID] = array_begin[itemID];
            }
        }

        out.write((char *) write_distance_cache.data(),
                  n_remain * data_item.n_vector_ * sizeof(double));
        Index index(bound_distance_table, known_rank_idx_l, index_path, n_max_read);
        index.setUserItemMatrix(user, data_item);
        return index;
    }

}

#endif //REVERSE_K_RANKS_INTERVALCACHEBOUND_HPP
