#pragma once

#include "util/SpaceInnerProduct.hpp"
#include "struct/RankElement.hpp"
#include "struct/VectorMatrix.hpp"
#include "util/TimeMemory.hpp"
#include <vector>
#include <queue>

namespace ReverseMIPS {

    class OnlineBruteForce {
    public:
        VectorMatrix data_item_, user_;
        int vec_dim_;
        int report_every_ = 10;

        inline OnlineBruteForce() {}

        inline OnlineBruteForce(VectorMatrix &data_item, VectorMatrix &user) {
            this->data_item_ = data_item;
            this->user_ = user;

            vec_dim_ = user.vec_dim;
        }

        inline ~OnlineBruteForce() {}

        void Preprocess() {}

        std::vector<std::vector<RankElement>> Retrieval(VectorMatrix &query_item, int topk) {
            int n_query_item = query_item.n_vector;
            int n_user = user_.n_vector;

            std::vector<std::vector<RankElement>> results(n_query_item, std::vector<RankElement>());

            TimeRecord single_query_record;
            for (int qID = 0; qID < n_query_item; qID++) {
                float *query_item_vec = query_item.getVector(qID);
                std::vector<RankElement> &minHeap = results[qID];
                minHeap.resize(topk);

                for (int userID = 0; userID < topk; userID++) {
                    int tmp_rank = getRank(query_item_vec, user_.getVector(userID));
                    RankElement rankElement(userID, tmp_rank);
                    minHeap[userID] = rankElement;
                }

                std::make_heap(minHeap.begin(), minHeap.end(), std::less<RankElement>());
                RankElement minHeapEle = minHeap.front();
                for (int userID = topk; userID < n_user; userID++) {
                    int tmpRank = getRank(query_item_vec, user_.getVector(userID));
                    RankElement rankElement(userID, tmpRank);
                    if (minHeapEle.rank > rankElement.rank) {
                        std::pop_heap(minHeap.begin(), minHeap.end(), std::less<RankElement>());
                        minHeap.pop_back();
                        minHeap.push_back(rankElement);
                        std::push_heap(minHeap.begin(), minHeap.end(), std::less<RankElement>());
                        minHeapEle = minHeap.front();
                    }
                }
                std::make_heap(minHeap.begin(), minHeap.end(), std::less<RankElement>());
                std::sort_heap(minHeap.begin(), minHeap.end(), std::less<RankElement>());

                if (qID % report_every_ == 0) {
                    std::cout << qID / (0.01 * n_query_item) << " %, "
                              << 1e-6 * single_query_record.get_elapsed_time_micro() << " s/iter" << " Mem: "
                              << get_current_RSS() / 1000000 << " Mb \n";
                    single_query_record.reset();
                }
            }


            return results;
        }

        int getRank(float *query_item_vec, float *user_vec) {

            float query_dist = InnerProduct(query_item_vec, user_vec, vec_dim_);
            int n_data_item = data_item_.n_vector;
            int rank = 1;

            for (int i = 0; i < n_data_item; i++) {
                float data_dist = InnerProduct(data_item_.getVector(i), user_vec, vec_dim_);
                rank += data_dist > query_dist ? 1 : 0;
            }
            return rank;
        }

    };

}

