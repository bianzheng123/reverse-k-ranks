//
// Created by BianZheng on 2021/12/22.
//

#ifndef REVERSE_KRANKS_DISKBRUTEFORCE_HPP
#define REVERSE_KRANKS_DISKBRUTEFORCE_HPP

#include "alg/SpaceInnerProduct.hpp"
#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "struct/VectorMatrix.hpp"
#include "struct/DistancePair.hpp"
#include "struct/MethodBase.hpp"
#include <fstream>
#include <vector>
#include <algorithm>
#include <cassert>
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
        TimeRecord get_rank_record_;

        Index() {}

        Index(const char *index_path, const int n_data_item, const VectorMatrix &user) {
            this->index_path_ = index_path;
            this->user_ = user;
            this->n_data_item_ = n_data_item;
            this->vec_dim_ = user.vec_dim_;
            this->n_cache = std::min(user_.n_vector_, 10000);
        }

        std::vector<std::vector<UserRankElement>> Retrieval(VectorMatrix &query_item, int topk) override {
            TimeRecord record, batch_report_record;
            ResetTimer();
            std::ifstream index_stream_ = std::ifstream(this->index_path_, std::ios::binary | std::ios::in);
            if (!index_stream_) {
                std::printf("error in writing index\n");
            }
            index_stream_.read((char *) &this->n_data_item_, sizeof(int));

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
                    int tmp_rank = getRank(query_item_vec, userID, cacheID, distance_cache);
                    query_heap_l[qID][cacheID].userID_ = userID;
                    query_heap_l[qID][cacheID].rank_ = tmp_rank;
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
                    int tmp_rank = getRank(query_item_vec, userID, cacheID, distance_cache);
                    if (min_heap_ele.rank_ > tmp_rank) {
                        std::pop_heap(tmp_heap.begin(), tmp_heap.end(), std::less<UserRankElement>());
                        tmp_heap.pop_back();
                        tmp_heap.emplace_back(userID, tmp_rank);
                        std::push_heap(tmp_heap.begin(), tmp_heap.end(), std::less<UserRankElement>());
                    }
                }
            }

            for (int bth_idx = 1; bth_idx < n_batch; bth_idx++) {
                record.reset();
                index_stream_.read((char *) distance_cache.data(), n_cache * n_data_item_ * sizeof(DistancePair));
                read_disk_time_ += record.get_elapsed_time_second();

                for (int cacheID = 0; cacheID < n_cache; cacheID++) {
                    int userID = bth_idx * n_cache + cacheID;
                    for (int qID = 0; qID < n_query_item; qID++) {
                        std::vector<UserRankElement> &tmp_heap = query_heap_l[qID];
                        UserRankElement min_heap_ele = tmp_heap.front();
                        double *query_item_vec = query_item.getVector(qID);
                        int tmp_rank = getRank(query_item_vec, userID, cacheID, distance_cache);
                        if (min_heap_ele.rank_ > tmp_rank) {
                            std::pop_heap(tmp_heap.begin(), tmp_heap.end(), std::less<UserRankElement>());
                            tmp_heap.pop_back();
                            tmp_heap.emplace_back(userID, tmp_rank);
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
                        int tmp_rank = getRank(query_item_vec, userID, cacheID, distance_cache);
                        if (min_heap_ele.rank_ > tmp_rank) {
                            std::pop_heap(tmp_heap.begin(), tmp_heap.end(), std::less<UserRankElement>());
                            tmp_heap.pop_back();
                            tmp_heap.emplace_back(userID, tmp_rank);
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

        int getRank(double *query_item_vec, int userID, int cacheID, std::vector<double> &distance_cache) {
            get_rank_record_.reset();
            double *user_vec = user_.getVector(userID);
            double query_dist = InnerProduct(query_item_vec, user_vec, vec_dim_);
            double *dpPtr = distance_cache.data() + cacheID * n_data_item_;
            this->inner_product_time_ += get_rank_record_.get_elapsed_time_second();

            get_rank_record_.reset();
            int low = 0;
            int high = n_data_item_;
            int rank = -1;
            //descending
            while (low <= high) {
                int mid = (low + high) / 2;
                if (mid == 0) {
                    if (query_dist >= dpPtr[mid]) {
                        rank = 1;
                        break;
                    } else if (query_dist < dpPtr[mid] && query_dist > dpPtr[mid + 1]) {
                        rank = 2;
                        break;
                    } else if (query_dist < dpPtr[mid] && query_dist <= dpPtr[mid + 1]) {
                        low = mid + 1;
                    }
                } else if (0 < mid && mid < n_data_item_ - 1) {
                    if (query_dist > dpPtr[mid]) {
                        high = mid - 1;
                    } else if (query_dist <= dpPtr[mid] &&
                               query_dist > dpPtr[mid + 1]) {
                        rank = mid + 2;
                        break;
                    } else if (query_dist <= dpPtr[mid] &&
                               query_dist <= dpPtr[mid + 1]) {
                        low = mid + 1;
                    }
                } else if (mid == n_data_item_ - 1) {
                    if (query_dist <= dpPtr[mid]) {
                        rank = n_data_item_ + 1;
                        break;
                    } else if (query_dist <= dpPtr[mid - 1] && query_dist > dpPtr[mid]) {
                        rank = n_data_item_;
                        break;
                    } else if (query_dist > dpPtr[mid - 1] && query_dist > dpPtr[mid]) {
                        high = mid - 1;
                    }
                }
            }
            if (rank <= 0) {
                printf("bug\n");
            }
            this->binary_search_time_ += get_rank_record_.get_elapsed_time_second();
            return rank;
        }

    };

    const int write_every_ = 30000;
    const int report_batch_every_ = 5;

    /*
     * bruteforce index
     * shape: 1, type: int, n_data_item
     * shape: n_user * n_data_item, type: double, the distance pair for each user
     */

    Index BuildIndex(const VectorMatrix &data_item, const VectorMatrix &user, const char *index_path) {
        std::ofstream out(index_path, std::ios::binary | std::ios::out);
        if (!out) {
            std::printf("error in write result\n");
        }
        const int n_data_item = data_item.n_vector_;
        std::vector<double> distance_cache(write_every_ * n_data_item);
        const int vec_dim = data_item.vec_dim_;
        const int n_batch = user.n_vector_ / write_every_;
        const int n_remain = user.n_vector_ % write_every_;
        out.write((char *) &n_data_item, sizeof(int));

        TimeRecord batch_report_record;
        batch_report_record.reset();
        for (int i = 0; i < n_batch; i++) {
#pragma omp parallel for default(none) shared(i, data_item, user, distance_cache) //shared(write_every_, n_data_item, vec_dim)
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
                double ip = InnerProduct(data_item.rawData_ + itemID * vec_dim,
                                         user.rawData_ + userID * vec_dim, vec_dim);
                distance_cache[cacheID * data_item.n_vector_ + itemID] = ip;
            }

            std::sort(distance_cache.begin() + cacheID * n_data_item,
                      distance_cache.begin() + (cacheID + 1) * n_data_item, std::greater<double>());

        }

        out.write((char *) distance_cache.data(),
                  n_remain * data_item.n_vector_ * sizeof(double));

        Index index(index_path, n_data_item, user);
        return index;
    }

}

#endif //REVERSE_KRANKS_DISKBRUTEFORCE_HPP
