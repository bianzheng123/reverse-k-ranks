#pragma once

#include "alg/SpaceInnerProduct.hpp"
#include "util/TimeMemory.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/VectorMatrix.hpp"
#include "struct/MethodBase.hpp"
#include <vector>
#include <algorithm>
#include <cassert>
#include <queue>

namespace ReverseMIPS::MemoryBruteForce {

    class Index : public BaseIndex {
    private:
        void ResetTime() {
            this->inner_product_time_ = 0;
            this->binary_search_time_ = 0;
            this->total_retrieval_time_ = 0;
        }

    public:
        VectorMatrix data_item_, user_;
        std::vector<double> distance_table_; // n_user_ * n_data_item_
        int n_user_, n_data_item_;

        int vec_dim_;
        int preprocess_report_every_ = 100;
        double total_retrieval_time_, inner_product_time_, binary_search_time_;
        TimeRecord total_retrieval_record_, preprocess_record_, inner_product_record_, binary_search_record_;

        Index() {}

        Index(VectorMatrix &data_item, VectorMatrix &user) {
            this->vec_dim_ = user.vec_dim_;
            this->data_item_ = std::move(data_item);
            this->user_ = std::move(user);
        }

        ~Index() {}

        void Preprocess() {
            int n_data_item = data_item_.n_vector_;
            int n_user = user_.n_vector_;
            std::vector<double> preprocess_matrix(n_user * n_data_item);
            user_.vectorNormalize();

            preprocess_record_.reset();
//#pragma omp parallel for default(none) shared(n_user, preprocess_matrix, std::cout, n_data_item)
            for (int userID = 0; userID < n_user; userID++) {

                for (int itemID = 0; itemID < n_data_item; itemID++) {
                    double query_dist = InnerProduct(data_item_.getVector(itemID), user_.getVector(userID), vec_dim_);
                    preprocess_matrix[userID * n_data_item + itemID] = query_dist;
                }

                std::make_heap(preprocess_matrix.begin() + userID * n_data_item,
                               preprocess_matrix.begin() + (userID + 1) * n_data_item, std::greater<double>());
                std::sort_heap(preprocess_matrix.begin() + userID * n_data_item,
                               preprocess_matrix.begin() + (userID + 1) * n_data_item, std::greater<double>());

                if (userID % preprocess_report_every_ == 0) {
                    std::cout << "preprocessed " << userID / (0.01 * n_user) << " %, "
                              << preprocess_record_.get_elapsed_time_second() << " s/iter" << " Mem: "
                              << get_current_RSS() / 1000000 << " Mb \n";
                    preprocess_record_.reset();
                }

            }

            this->distance_table_ = preprocess_matrix;
            this->n_data_item_ = n_data_item;
            this->n_user_ = n_user;

        }

        std::vector<std::vector<UserRankElement>>
        Retrieval(const VectorMatrix &query_item, const int &topk, const int &n_execute_query,
                  std::vector<SingleQueryPerformance> &query_performance_l) override {
            if (topk > user_.n_vector_) {
                spdlog::error("top-k is larger than user, system exit");
                exit(-1);
            }
            ResetTime();

            int n_query_item = n_execute_query;
            int n_user = user_.n_vector_;

            if (n_execute_query > query_item.n_vector_) {
                spdlog::error("n_execute_query larger than n_query_item, program exit");
                exit(-1);
            }

            std::vector<std::vector<UserRankElement>> results(n_query_item, std::vector<UserRankElement>());

            for (int qID = 0; qID < n_query_item; qID++) {
                total_retrieval_record_.reset();
                double *query_item_vec = query_item.getVector(qID);
                std::vector<UserRankElement> &minHeap = results[qID];
                minHeap.resize(topk);

                for (int userID = 0; userID < topk; userID++) {
                    inner_product_record_.reset();
                    double *user_vec = user_.getVector(userID);
                    double queryIP = InnerProduct(query_item_vec, user_vec, vec_dim_);
                    this->inner_product_time_ += inner_product_record_.get_elapsed_time_second();

                    binary_search_record_.reset();
                    int tmp_rank = BinarySearch(queryIP, userID);
                    this->binary_search_time_ += binary_search_record_.get_elapsed_time_second();

                    UserRankElement rankElement(userID, tmp_rank, queryIP);
                    minHeap[userID] = rankElement;
                }

                std::make_heap(minHeap.begin(), minHeap.end(), std::less<UserRankElement>());

                UserRankElement minHeapEle = minHeap.front();
                for (int userID = topk; userID < n_user; userID++) {

                    inner_product_record_.reset();
                    double *user_vec = user_.getVector(userID);
                    double queryIP = InnerProduct(query_item_vec, user_vec, vec_dim_);
                    this->inner_product_time_ += inner_product_record_.get_elapsed_time_second();

                    binary_search_record_.reset();
                    int tmp_rank = BinarySearch(queryIP, userID);
                    this->binary_search_time_ += binary_search_record_.get_elapsed_time_second();

                    UserRankElement rankElement(userID, tmp_rank, queryIP);
                    if (minHeapEle > rankElement) {
                        std::pop_heap(minHeap.begin(), minHeap.end(), std::less<UserRankElement>());
                        minHeap.pop_back();
                        minHeap.push_back(rankElement);
                        std::push_heap(minHeap.begin(), minHeap.end(), std::less<UserRankElement>());
                        minHeapEle = minHeap.front();
                    }

                }
                std::make_heap(minHeap.begin(), minHeap.end(), std::less<UserRankElement>());
                std::sort_heap(minHeap.begin(), minHeap.end(), std::less<UserRankElement>());
                total_retrieval_time_ += total_retrieval_record_.get_elapsed_time_second();
            }

            return results;
        }

        int BinarySearch(double queryIP, int userID) {
            int n_data_item = data_item_.n_vector_;
            auto iter_begin = distance_table_.begin() + userID * n_data_item_;
            auto iter_end = distance_table_.begin() + (userID + 1) * n_data_item_;


            auto lb_ptr = std::lower_bound(iter_begin, iter_end, queryIP,
                                           [](const double &arrIP, double queryIP) {
                                               return arrIP > queryIP;
                                           });
            return (int) (lb_ptr - iter_begin) + 1;
        }

        std::string
        PerformanceStatistics(const int &topk) override {
            // int topk;
            //double total_time,
            //          inner_product_time, binary_search_time;
            //unit: second

            char buff[1024];

            sprintf(buff,
                    "top%d retrieval time:\n\ttotal %.3fs\n\tinner product %.3fs, binary search %.3fs",
                    topk, total_retrieval_time_,
                    inner_product_time_, binary_search_time_);
            std::string str(buff);
            return str;
        }

        uint64_t IndexSizeByte() override {
            return sizeof(double) * n_user_ * n_data_item_;
        }


    };

    std::unique_ptr<Index> BuildIndex(VectorMatrix &data_item, VectorMatrix &user) {
        std::unique_ptr<Index> index_ptr = std::make_unique<Index>(data_item, user);
        index_ptr->Preprocess();
        return index_ptr;
    }

}

