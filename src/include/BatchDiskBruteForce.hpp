//
// Created by BianZheng on 2021/12/22.
//

#ifndef REVERSE_KRANKS_BATCH_DISKBRUTEFORCE_HPP
#define REVERSE_KRANKS_BATCH_DISKBRUTEFORCE_HPP

#include "alg/SpaceInnerProduct.hpp"
#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/VectorMatrix.hpp"
#include "struct/MethodBase.hpp"
#include <fstream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <map>
#include <spdlog/spdlog.h>

namespace ReverseMIPS::DiskBruteForce {
    class RetrievalResult {
    public:
        //unit: second
        double total_time, read_disk_time, inner_product_time, binary_search_time, second_per_query;
        int topk;

        inline RetrievalResult(double total_time, double read_disk_time, double inner_product_time,
                               double binary_search_time, double second_per_query, int topk) {
            this->total_time = total_time;
            this->read_disk_time = read_disk_time;
            this->inner_product_time = inner_product_time;
            this->binary_search_time = binary_search_time;
            this->second_per_query = second_per_query;

            this->topk = topk;
        }

        void AddMap(std::map<std::string, std::string> &performance_m) {
            char buff[256];
            sprintf(buff, "top%d total retrieval time", topk);
            std::string str1(buff);
            performance_m.emplace(str1, double2string(total_time));

            sprintf(buff, "top%d retrieval read disk time", topk);
            std::string str2(buff);
            performance_m.emplace(str2, double2string(read_disk_time));

            sprintf(buff, "top%d retrieval inner product time", topk);
            std::string str3(buff);
            performance_m.emplace(str3, double2string(inner_product_time));

            sprintf(buff, "top%d retrieval binary search time", topk);
            std::string str4(buff);
            performance_m.emplace(str4, double2string(binary_search_time));

            sprintf(buff, "top%d second per query time", topk);
            std::string str5(buff);
            performance_m.emplace(str5, double2string(second_per_query));
        }

        [[nodiscard]] std::string ToString() const {
            char arr[256];
            sprintf(arr,
                    "top%d retrieval time:\n\ttotal %.3fs, read disk %.3fs\n\tinner product %.3fs, binary search %.3fs, million second per query %.3fms",
                    topk, total_time, read_disk_time, inner_product_time, binary_search_time, second_per_query * 1000);
            std::string str(arr);
            return str;
        }

    };

    class Index : public BaseIndex {
        void ResetTimer() {
            read_disk_time_ = 0;
            inner_product_time_ = 0;
            binary_search_time_ = 0;
        }

    public:
        VectorMatrix user_;
        int vec_dim_, n_data_item_;
        size_t n_cache; //should larger than top-k
        double read_disk_time_, inner_product_time_, binary_search_time_;
        const char *index_path_;
        TimeRecord read_disk_record_, inner_product_record_, binary_search_record_;

        Index() {}

        Index(const char *index_path, const int n_data_item, VectorMatrix &user) {
            this->index_path_ = index_path;
            this->vec_dim_ = user.vec_dim_;
            this->user_ = std::move(user);
            this->n_data_item_ = n_data_item;
            this->n_cache = std::min(user_.n_vector_, 10000);
        }

        std::vector<std::vector<UserRankElement>> Retrieval(VectorMatrix &query_item, int topk) override {
            TimeRecord record, batch_report_record;
            ResetTimer();
            std::ifstream index_stream_ = std::ifstream(this->index_path_, std::ios::binary | std::ios::in);
            if (!index_stream_) {
                std::printf("error in writing index\n");
            }

            if (topk > user_.n_vector_) {
                printf("top-k is too large, program exit\n");
                exit(-1);
            }

//            size_t avail_memory = get_avail_memory();
            size_t a = user_.n_vector_;
            size_t b = (size_t) 2000000000 / n_data_item_;
            n_cache = a > b ? b : a;
            std::vector<double> distance_cache(n_cache * n_data_item_);
            int n_query_item = query_item.n_vector_;
            int n_user = user_.n_vector_;
            int n_batch = (int) (n_user / n_cache);
            int n_remain = (int) (n_user % n_cache);
            const int report_batch_every_ = 5;

            std::vector<std::vector<UserRankElement>> query_heap_l(n_query_item, std::vector<UserRankElement>(topk));

            record.reset();
            index_stream_.read((char *) distance_cache.data(), n_cache * n_data_item_ * sizeof(double));
            read_disk_time_ += record.get_elapsed_time_second();

            for (int cacheID = 0; cacheID < topk; cacheID++) {
                int userID = cacheID;
                for (int qID = 0; qID < n_query_item; qID++) {
                    double *query_item_vec = query_item.getVector(qID);

                    inner_product_record_.reset();
                    double *user_vec = user_.getVector(userID);
                    double queryIP = InnerProduct(query_item_vec, user_vec, vec_dim_);
                    this->inner_product_time_ += inner_product_record_.get_elapsed_time_second();

                    binary_search_record_.reset();
                    int tmp_rank = BinarySearch(queryIP, cacheID, distance_cache);
                    this->binary_search_time_ += binary_search_record_.get_elapsed_time_second();

                    query_heap_l[qID][cacheID].userID_ = userID;
                    query_heap_l[qID][cacheID].rank_ = tmp_rank;
                    query_heap_l[qID][cacheID].queryIP_ = queryIP;
                }
            }
            for (int qID = 0; qID < n_query_item; qID++) {
                std::make_heap(query_heap_l[qID].begin(), query_heap_l[qID].end(), std::less<UserRankElement>());
            }

            for (int cacheID = topk; cacheID < n_cache; cacheID++) {
//                spdlog::info("processing cache {} of total {}", cacheID, n_cache);
                int userID = cacheID;
                for (int qID = 0; qID < n_query_item; qID++) {
                    std::vector<UserRankElement> &tmp_heap = query_heap_l[qID];
                    UserRankElement min_heap_ele = tmp_heap.front();
                    double *query_item_vec = query_item.getVector(qID);

                    inner_product_record_.reset();
                    double *user_vec = user_.getVector(userID);
                    double queryIP = InnerProduct(query_item_vec, user_vec, vec_dim_);
                    this->inner_product_time_ += inner_product_record_.get_elapsed_time_second();

                    binary_search_record_.reset();
                    int tmp_rank = BinarySearch(queryIP, cacheID, distance_cache);
                    this->binary_search_time_ += binary_search_record_.get_elapsed_time_second();
                    UserRankElement element(userID, tmp_rank, queryIP);

                    if (min_heap_ele > element) {
                        std::pop_heap(tmp_heap.begin(), tmp_heap.end(), std::less<UserRankElement>());
                        tmp_heap.pop_back();
                        tmp_heap.push_back(element);
                        std::push_heap(tmp_heap.begin(), tmp_heap.end(), std::less<UserRankElement>());
                    }
                }
            }

            for (int bth_idx = 1; bth_idx < n_batch; bth_idx++) {
                record.reset();
                index_stream_.read((char *) distance_cache.data(), n_cache * n_data_item_ * sizeof(double));
                read_disk_time_ += record.get_elapsed_time_second();

                for (int cacheID = 0; cacheID < n_cache; cacheID++) {
                    int userID = bth_idx * n_cache + cacheID;
                    for (int qID = 0; qID < n_query_item; qID++) {
                        std::vector<UserRankElement> &tmp_heap = query_heap_l[qID];
                        UserRankElement min_heap_ele = tmp_heap.front();
                        double *query_item_vec = query_item.getVector(qID);

                        inner_product_record_.reset();
                        double *user_vec = user_.getVector(userID);
                        double queryIP = InnerProduct(query_item_vec, user_vec, vec_dim_);
                        this->inner_product_time_ += inner_product_record_.get_elapsed_time_second();

                        binary_search_record_.reset();
                        int tmp_rank = BinarySearch(queryIP, cacheID, distance_cache);
                        this->binary_search_time_ += binary_search_record_.get_elapsed_time_second();
                        UserRankElement element(userID, tmp_rank, queryIP);

                        if (min_heap_ele > element) {
                            std::pop_heap(tmp_heap.begin(), tmp_heap.end(), std::less<UserRankElement>());
                            tmp_heap.pop_back();
                            tmp_heap.push_back(element);
                            std::push_heap(tmp_heap.begin(), tmp_heap.end(),
                                           std::less<UserRankElement>());
                        }
                    }

                }

                if (bth_idx % report_batch_every_ == 0) {
                    std::cout << "top-" << topk << " retrieval batch " << bth_idx / (0.01 * n_batch) << " %, "
                              << batch_report_record.get_elapsed_time_second() << " s/iter" << " Mem: "
                              << get_current_RSS() / 1000000 << " Mb \n";
                    batch_report_record.reset();
                }

            }

            if (n_remain != 0) {
                record.reset();
                index_stream_.read((char *) distance_cache.data(),
                                   n_remain * n_data_item_ * sizeof(double));
                read_disk_time_ += record.get_elapsed_time_second();

                for (int cacheID = 0; cacheID < n_remain; cacheID++) {
                    int userID = n_batch * n_cache + cacheID;
                    for (int qID = 0; qID < n_query_item; qID++) {
                        std::vector<UserRankElement> &tmp_heap = query_heap_l[qID];
                        UserRankElement min_heap_ele = tmp_heap.front();
                        double *query_item_vec = query_item.getVector(qID);

                        inner_product_record_.reset();
                        double *user_vec = user_.getVector(userID);
                        double queryIP = InnerProduct(query_item_vec, user_vec, vec_dim_);
                        this->inner_product_time_ += inner_product_record_.get_elapsed_time_second();

                        binary_search_record_.reset();
                        int tmp_rank = BinarySearch(queryIP, cacheID, distance_cache);
                        this->binary_search_time_ += binary_search_record_.get_elapsed_time_second();
                        UserRankElement element(userID, tmp_rank, queryIP);

                        if (min_heap_ele > element) {
                            std::pop_heap(tmp_heap.begin(), tmp_heap.end(), std::less<UserRankElement>());
                            tmp_heap.pop_back();
                            tmp_heap.push_back(element);
                            std::push_heap(tmp_heap.begin(), tmp_heap.end(),
                                           std::less<UserRankElement>());
                        }
                    }
                }
            }

            index_stream_.close();

            for (int qID = 0; qID < n_query_item; qID++) {
                std::sort(query_heap_l[qID].begin(), query_heap_l[qID].end(), std::less<UserRankElement>());
            }

            return query_heap_l;
        }

        int BinarySearch(double queryIP, int cacheID, std::vector<double> &distance_cache) const {
            auto iter_begin = distance_cache.begin() + cacheID * n_data_item_;
            auto iter_end = distance_cache.begin() + (cacheID + 1) * n_data_item_;
            auto lb_ptr = std::lower_bound(iter_begin, iter_end, queryIP,
                                           [](const double &arrIP, double queryIP) {
                                               return arrIP > queryIP;
                                           });
            return (int) (lb_ptr - iter_begin) + 1;
        }

    };

    const int write_every_ = 30000;
    const int report_batch_every_ = 5;

    /*
     * bruteforce index
     * shape: 1, type: int, n_data_item
     * shape: n_user * n_data_item, type: double, the distance pair for each user
     */

    Index &BuildIndex(VectorMatrix &data_item, VectorMatrix &user, const char *index_path) {
        std::ofstream out(index_path, std::ios::binary | std::ios::out);
        if (!out) {
            std::printf("error in write result\n");
        }
        const int n_data_item = data_item.n_vector_;
        std::vector<double> distance_cache(write_every_ * n_data_item);
        const int vec_dim = data_item.vec_dim_;
        const int n_batch = user.n_vector_ / write_every_;
        const int n_remain = user.n_vector_ % write_every_;

        TimeRecord batch_report_record;
        batch_report_record.reset();
        for (int i = 0; i < n_batch; i++) {
#pragma omp parallel for default(none) shared(i, data_item, user, distance_cache) shared(write_every_, n_data_item, vec_dim)
            for (int cacheID = 0; cacheID < write_every_; cacheID++) {
                int userID = write_every_ * i + cacheID;
                for (int itemID = 0; itemID < n_data_item; itemID++) {
                    double ip = InnerProduct(data_item.getVector(itemID), user.getVector(userID), vec_dim);
                    distance_cache[cacheID * n_data_item + itemID] = ip;
                }
                std::sort(distance_cache.begin() + cacheID * n_data_item,
                          distance_cache.begin() + (cacheID + 1) * n_data_item, std::greater<double>());
            }
            out.write((char *) distance_cache.data(), distance_cache.size() * sizeof(double));

            if (i % report_batch_every_ == 0) {
                std::cout << "preprocessed " << i / (0.01 * n_batch) << " %, "
                          << batch_report_record.get_elapsed_time_second() << " s/iter" << " Mem: "
                          << get_current_RSS() / 1000000 << " Mb \n";
                batch_report_record.reset();
            }

        }

        for (int cacheID = 0; cacheID < n_remain; cacheID++) {
            int userID = cacheID + write_every_ * n_batch;
            for (int itemID = 0; itemID < data_item.n_vector_; itemID++) {
                double ip = InnerProduct(data_item.getRawData() + itemID * vec_dim,
                                         user.getRawData() + userID * vec_dim, vec_dim);
                distance_cache[cacheID * data_item.n_vector_ + itemID] = ip;
            }

            std::sort(distance_cache.begin() + cacheID * n_data_item,
                      distance_cache.begin() + (cacheID + 1) * n_data_item, std::greater<double>());

        }

        out.write((char *) distance_cache.data(),
                  n_remain * data_item.n_vector_ * sizeof(double));

        static Index index(index_path, n_data_item, user);
        return index;
    }

}

#endif //REVERSE_KRANKS_BATCH_DISKBRUTEFORCE_HPP
