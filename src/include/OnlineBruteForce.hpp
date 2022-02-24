#pragma once

#include "alg/SpaceInnerProduct.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/VectorMatrix.hpp"
#include "struct/MethodBase.hpp"
#include "util/TimeMemory.hpp"
#include <vector>
#include <queue>

namespace ReverseMIPS::OnlineBruteForce {

    class RetrievalResult {
    public:
        //unit: second
        double total_time, second_per_query;
        int topk;

        inline RetrievalResult(double total_time, double second_per_query, int topk) {
            this->total_time = total_time;
            this->second_per_query = second_per_query;

            this->topk = topk;
        }

        void AddMap(std::map<std::string, std::string> &performance_m) {
            char buff[256];
            sprintf(buff, "top%d total retrieval time", topk);
            std::string str1(buff);
            performance_m.emplace(str1, double2string(total_time));

            sprintf(buff, "top%d second per query time", topk);
            std::string str5(buff);
            performance_m.emplace(str5, double2string(second_per_query));
        }

        [[nodiscard]] std::string ToString() const {
            char arr[256];
            sprintf(arr, "top%d retrieval time:\n\ttotal %.3fs, million second per query %.3fms",
                    topk, total_time, second_per_query * 1000);
            std::string str(arr);
            return str;
        }

    };

    class Index : public BaseIndex {
    public:
        VectorMatrix data_item_, user_;
        int vec_dim_;
        int report_every_ = 10;

        inline Index() {}

        inline Index(VectorMatrix &data_item, VectorMatrix &user) {
            this->data_item_ = data_item;
            this->user_ = user;

            vec_dim_ = user.vec_dim_;
        }

        inline ~Index() {}

        void Preprocess() {}

        std::vector<std::vector<UserRankElement>> Retrieval(VectorMatrix &query_item, int topk) override {
            if (topk > user_.n_vector_) {
                printf("top-k is larger than user, system exit\n");
                exit(-1);
            }
            int n_query_item = query_item.n_vector_;
            int n_user = user_.n_vector_;

            std::vector<std::vector<UserRankElement>> results(n_query_item, std::vector<UserRankElement>(topk));

            TimeRecord single_query_record;
#pragma omp parallel for default(none) shared(n_query_item, query_item, results, topk, n_user, single_query_record, std::cout)
            for (int qID = 0; qID < n_query_item; qID++) {
                double *query_item_vec = query_item.getVector(qID);
                std::vector<UserRankElement> &minHeap = results[qID];

                for (int userID = 0; userID < topk; userID++) {
                    int tmp_rank = getRank(query_item_vec, user_.getVector(userID));
                    UserRankElement rankElement(userID, tmp_rank);
                    minHeap[userID] = rankElement;
                }

                std::make_heap(minHeap.begin(), minHeap.end(), std::less<UserRankElement>());
                UserRankElement minHeapEle = minHeap.front();
                for (int userID = topk; userID < n_user; userID++) {
                    int tmpRank = getRank(query_item_vec, user_.getVector(userID));
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

                if (qID % report_every_ == 0) {
                    std::cout << "retrieval top" << topk << " progress " << qID / (0.01 * n_query_item) << " %, "
                              << single_query_record.get_elapsed_time_second() << " s/iter" << " Mem: "
                              << get_current_RSS() / 1000000 << " Mb \n";
                    single_query_record.reset();
                }
            }

            return results;
        }

        int getRank(double *query_item_vec, double *user_vec) {

            double query_dist = InnerProduct(query_item_vec, user_vec, vec_dim_);
            int n_data_item = data_item_.n_vector_;
            int rank = 1;

            for (int i = 0; i < n_data_item; i++) {
                double data_dist = InnerProduct(data_item_.getVector(i), user_vec, vec_dim_);
                rank += data_dist > query_dist ? 1 : 0;
            }

            return rank;
        }

    };

}

