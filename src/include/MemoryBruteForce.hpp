#pragma once

#include "alg/SpaceInnerProduct.hpp"
#include "util/TimeMemory.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/VectorMatrix.hpp"
#include "struct/DistancePair.hpp"
#include "struct/MethodBase.hpp"
#include <vector>
#include <cassert>
#include <queue>

namespace ReverseMIPS::MemoryBruteForce {

    class RetrievalResult {
    public:
        //unit: second
        double total_time, inner_product_time, binary_search_time, second_per_query;
        int topk;

        inline RetrievalResult(double total_time, double inner_product_time,
                               double binary_search_time, double second_per_query, int topk) {
            this->total_time = total_time;
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

            sprintf(buff, "top%d retrieval inner product time", topk);
            std::string str2(buff);
            performance_m.emplace(str2, double2string(inner_product_time));

            sprintf(buff, "top%d retrieval binary search time", topk);
            std::string str3(buff);
            performance_m.emplace(str3, double2string(binary_search_time));

            sprintf(buff, "top%d second per query", topk);
            std::string str4(buff);
            performance_m.emplace(str4, double2string(second_per_query));
        }

        [[nodiscard]] std::string ToString() const {
            char arr[256];
            sprintf(arr,
                    "top%d retrieval time:\n\ttotal %.3fs\n\tinner product %.3fs, binary search %.3fs, million second per query %.3fms",
                    topk, total_time, inner_product_time, binary_search_time, second_per_query * 1000);
            std::string str(arr);
            return str;
        }


    };

    class DataIndex {
    public:
        std::vector<DistancePair> dist_arr_;
        //n_user_ is row, n_data_item_ is column
        int n_user_, n_data_item_;

        DataIndex() {
            this->n_data_item_ = 0;
            this->n_user_ = 0;
        }

        void init(std::vector<DistancePair> &dist_arr, int n_data_item, int n_user) {
            this->dist_arr_ = dist_arr;
            this->n_data_item_ = n_data_item;
            this->n_user_ = n_user;
        }

        DistancePair *getUserDistPtr(int userID) {
            assert(userID < n_user_);
            return dist_arr_.data() + userID * n_data_item_;
        }
    };

    class Index : public BaseIndex {
    private:
        void ResetTime() {
            this->inner_product_calculation_time_ = 0;
            this->binary_search_time_ = 0;
        }

    public:
        VectorMatrix data_item_, user_;
        DataIndex index_;
        int vec_dim_;
        int preprocess_report_every_ = 100;
        double inner_product_calculation_time_, binary_search_time_;
        TimeRecord record_;

        Index() {}

        Index(VectorMatrix &data_item, VectorMatrix &user) {
            this->data_item_ = data_item;
            this->user_ = user;
            this->vec_dim_ = user.vec_dim_;
        }

        ~Index() {}

        void Preprocess() {
            int n_data_item = data_item_.n_vector_;
            int n_user = user_.n_vector_;
            std::vector<DistancePair> preprocess_matrix(n_user * n_data_item);

            record_.reset();
            for (int userID = 0; userID < n_user; userID++) {

                for (int itemID = 0; itemID < n_data_item; itemID++) {
                    double query_dist = InnerProduct(data_item_.getVector(itemID), user_.getVector(userID), vec_dim_);
                    preprocess_matrix[userID * n_data_item + itemID] = DistancePair(query_dist, itemID);
                }

                std::make_heap(preprocess_matrix.begin() + userID * n_data_item,
                               preprocess_matrix.begin() + (userID + 1) * n_data_item, std::greater<DistancePair>());
                std::sort_heap(preprocess_matrix.begin() + userID * n_data_item,
                               preprocess_matrix.begin() + (userID + 1) * n_data_item, std::greater<DistancePair>());

                if (userID % preprocess_report_every_ == 0) {
                    std::cout << "preprocessed " << userID / (0.01 * n_user) << " %, "
                              << record_.get_elapsed_time_second() << " s/iter" << " Mem: "
                              << get_current_RSS() / 1000000 << " Mb \n";
                    record_.reset();
                }

            }

            index_.init(preprocess_matrix, n_data_item, n_user);

        }

        std::vector<std::vector<UserRankElement>> Retrieval(VectorMatrix &query_item, int topk) override {
            if (topk > user_.n_vector_) {
                printf("top-k is larger than user, system exit\n");
                exit(-1);
            }
            ResetTime();
            int n_query_item = query_item.n_vector_;
            int n_user = user_.n_vector_;

            std::vector<std::vector<UserRankElement>> results(n_query_item, std::vector<UserRankElement>());

            for (int qID = 0; qID < n_query_item; qID++) {
                double *query_item_vec = query_item.getVector(qID);
                std::vector<UserRankElement> &minHeap = results[qID];
                minHeap.resize(topk);

                for (int userID = 0; userID < topk; userID++) {
                    int tmp_rank = getRank(query_item_vec, userID);

                    UserRankElement rankElement(userID, tmp_rank);
                    minHeap[userID] = rankElement;
                }

                std::make_heap(minHeap.begin(), minHeap.end(), std::less<UserRankElement>());

                UserRankElement minHeapEle = minHeap.front();
                for (int userID = topk; userID < n_user; userID++) {
                    int tmpRank = getRank(query_item_vec, userID);

                    UserRankElement rankElement(userID, tmpRank);
                    if (minHeapEle.rank_ > rankElement.rank_) {
                        std::pop_heap(minHeap.begin(), minHeap.end(), std::less<UserRankElement>());
                        minHeap.pop_back();
                        minHeap.push_back(rankElement);
                        std::push_heap(minHeap.begin(), minHeap.end(), std::less<UserRankElement>());
                        minHeapEle = minHeap.front();
                    }

                }
                std::make_heap(minHeap.begin(), minHeap.end(), std::less<UserRankElement>());
                std::sort_heap(minHeap.begin(), minHeap.end(), std::less<UserRankElement>());

            }

//            this->inner_product_calculation_time_ /= (double) n_query_item;
//            this->binary_search_time_ /= (double) n_query_item;
            return results;
        }

        int getRank(double *query_item_vec, int userID) {
            double *user_vec = user_.getVector(userID);
            record_.reset();
            double query_dist = InnerProduct(query_item_vec, user_vec, vec_dim_);
            this->inner_product_calculation_time_ += record_.get_elapsed_time_second();
            int n_data_item = data_item_.n_vector_;
            DistancePair *dpPtr = index_.getUserDistPtr(userID);

            record_.reset();
            int low = 0;
            int high = n_data_item;
            int rank = -1;
            //descending
            while (low <= high) {
                int mid = (low + high) / 2;
                if (mid == 0) {
                    if (query_dist >= dpPtr[mid].dist_) {
                        rank = 1;
                        break;
                    } else if (query_dist < dpPtr[mid].dist_ && query_dist > dpPtr[mid + 1].dist_) {
                        rank = 2;
                        break;
                    } else if (query_dist < dpPtr[mid].dist_ && query_dist <= dpPtr[mid + 1].dist_) {
                        low = mid + 1;
                    }
                } else if (0 < mid && mid < n_data_item - 1) {
                    if (query_dist > dpPtr[mid].dist_) {
                        high = mid - 1;
                    } else if (query_dist <= dpPtr[mid].dist_ &&
                               query_dist > dpPtr[mid + 1].dist_) {
                        rank = mid + 2;
                        break;
                    } else if (query_dist <= dpPtr[mid].dist_ &&
                               query_dist <= dpPtr[mid + 1].dist_) {
                        low = mid + 1;
                    }
                } else if (mid == n_data_item - 1) {
                    if (query_dist <= dpPtr[mid].dist_) {
                        rank = n_data_item + 1;
                        break;
                    } else if (query_dist <= dpPtr[mid - 1].dist_ && query_dist > dpPtr[mid].dist_) {
                        rank = n_data_item;
                        break;
                    } else if (query_dist > dpPtr[mid - 1].dist_ && query_dist > dpPtr[mid].dist_) {
                        high = mid - 1;
                    }
                }
            }
            this->binary_search_time_ += record_.get_elapsed_time_second();
            if (rank <= 0) {
                printf("bug\n");
            }
            return rank;
        }

    };

}

