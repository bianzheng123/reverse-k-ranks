//
// Created by BianZheng on 2021/12/27.
//

#ifndef REVERSE_KRANKS_SAMPLEINDEX_HPP
#define REVERSE_KRANKS_SAMPLEINDEX_HPP

#include "struct/VectorMatrix.hpp"
#include "util/TimeMemory.hpp"
#include "util/StringUtil.hpp"
#include "alg/SpaceInnerProduct.hpp"
#include "struct/IntervalVector.hpp"
#include "struct/DistancePair.hpp"
#include "alg/KMeans.hpp"
#include <fstream>
#include <cassert>
#include <utility>
//#undef NDEBUG

namespace ReverseMIPS {

    int qID = 0;
    class RankIntervalIndex {
    public:
        std::vector<int> known_rank_idx_l_; // shape: n_known_rank
        std::vector<int> user_merge_idx_l_; // shape: n_user, record which user belongs to which IntervalVector
        std::vector<DistancePair> distance_table_; // shape: n_user * n_known_rank
        std::vector<IntervalVector> interval_arr_; // shape: n_merge_user
        // IntervalVector shape: n_interval * (interval_size or final_interval_size)
        int n_known_rank_, n_user_, n_merge_user_, n_interval_, interval_size_, final_interval_size_;
        VectorMatrix user_, data_item_;


        RankIntervalIndex(std::vector<int> &known_rank_idx_l, std::vector<DistancePair> &distance_table,
                          std::vector<IntervalVector> &interval_arr, std::vector<int> &user_merge_idx_l,
                          std::vector<int> size_config) {
            this->known_rank_idx_l_ = std::move(known_rank_idx_l);
            this->distance_table_ = std::move(distance_table);
            this->interval_arr_ = std::move(interval_arr);
            this->user_merge_idx_l_ = std::move(user_merge_idx_l);

            this->n_known_rank_ = size_config[0];
            this->n_user_ = size_config[1];
            this->n_merge_user_ = size_config[2];
            this->n_interval_ = size_config[3];
            this->interval_size_ = size_config[4];
            this->final_interval_size_ = size_config[5];
        }

        void setUserItemMatrix(VectorMatrix &user, VectorMatrix &data_item) {
            this->user_ = user;
            this->data_item_ = data_item;
        }

        [[nodiscard]] std::vector<std::vector<RankElement>> Retrieval(VectorMatrix query_item, int topk) const {
            std::vector<std::vector<RankElement>> result(query_item.n_vector_, std::vector<RankElement>());
            int n_query = query_item.n_vector_;
            int vec_dim = query_item.vec_dim_;
            for (qID = 0; qID < n_query; qID++) {
//            for (int qID = 0; qID < n_query; qID++) {
                double *query_vec = query_item.getVector(qID);

                std::vector<RankElement> &minHeap = result[qID];
                minHeap.resize(topk);

                for (int userID = 0; userID < topk; userID++) {
                    int rank = GetRank(userID, query_vec, vec_dim);
                    RankElement rankElement(userID, rank);
                    minHeap[userID] = rankElement;
                }

                std::make_heap(minHeap.begin(), minHeap.end(), std::less<RankElement>());

                RankElement minHeapEle = minHeap.front();
                for (int userID = topk; userID < n_user_; userID++) {
                    int tmpRank = GetRank(userID, query_vec, vec_dim);

                    RankElement rankElement(userID, tmpRank);
                    if (minHeapEle.rank_ > rankElement.rank_) {
                        std::pop_heap(minHeap.begin(), minHeap.end(), std::less<RankElement>());
                        minHeap.pop_back();
                        minHeap.push_back(rankElement);
                        std::push_heap(minHeap.begin(), minHeap.end(), std::less<RankElement>());
                        minHeapEle = minHeap.front();
                    }

                }
                std::make_heap(minHeap.begin(), minHeap.end(), std::less<RankElement>());
                std::sort_heap(minHeap.begin(), minHeap.end(), std::less<RankElement>());
            }
            return result;
        }

        [[nodiscard]] int GetRank(const int userID, const double *query_vec, const int vec_dim) const {
            double *user = this->user_.getVector(userID);
            double queryIP = InnerProduct(query_vec, user, vec_dim);
            int interval_idx = BinarySearch(queryIP, userID);

            int interval_vector_idx = user_merge_idx_l_[userID];
            std::vector<int> candidate_l = interval_arr_[interval_vector_idx].getInterval(interval_idx);
            int loc_rk = RelativeRankInInterval(candidate_l, interval_idx, queryIP, userID, vec_dim);

            int rank = interval_idx == 0 ? loc_rk : known_rank_idx_l_[interval_idx - 1] + loc_rk;
            return rank + 1;
        }

        [[nodiscard]] int RelativeRankInInterval(const std::vector<int> &candidate_l, int interval_idx, double queryIP,
                                                 int userID, int vec_dim) const {
            //calculate all the IP, then get the lower bound
            //make different situation by the information
            int interval_size = (int) candidate_l.size();
            std::vector<DistancePair> dp_l;
            for (int i = 0; i < interval_size; i++) {
                double ip = InnerProduct(data_item_.getVector(candidate_l[i]), user_.getVector(userID), vec_dim);
                dp_l.emplace_back(ip, candidate_l[i]);
            }
            std::sort(dp_l.begin(), dp_l.end(), std::greater<DistancePair>());

            if (interval_idx == 0) {
                auto lb_ptr = std::lower_bound(dp_l.begin(), dp_l.end(), queryIP,
                                               [](const DistancePair &info, double value) {
                                                   return info.dist_ > value;
                                               });
                return (int) (lb_ptr - dp_l.begin());
            } else {
                auto lb_ptr = std::lower_bound(dp_l.begin(), dp_l.end(), queryIP,
                                               [](const DistancePair &info, double value) {
                                                   return info.dist_ > value;
                                               });
                double interval_upper_bound = distance_table_[userID * n_known_rank_ + interval_idx - 1].dist_;
                auto interval_ptr = std::lower_bound(dp_l.begin(), dp_l.end(), interval_upper_bound,
                                                     [](const DistancePair &info, double value) {
                                                         return info.dist_ > value;
                                                     });
                return (int) (lb_ptr - dp_l.begin()) - (int) (interval_ptr - dp_l.begin());
            }
        }

        //return the index of the interval it belongs to
        [[nodiscard]] int BinarySearch(double queryIP, int userID) const {
            auto iter_begin = distance_table_.begin() + userID * n_known_rank_;
            auto iter_end = distance_table_.begin() + (userID + 1) * n_known_rank_;

            auto lb_ptr = std::lower_bound(iter_begin, iter_end, queryIP,
                                           [](const DistancePair &info, double IP) {
                                               return info.dist_ > IP;
                                           });
            return (int) (lb_ptr - iter_begin);
        }

    };

    RankIntervalIndex
    BuildIndex(VectorMatrix user, VectorMatrix data_item, int n_merge_user, double &time) {

        //perform Kmeans for user vector, the label start from 0, indicates where the rank should come from
        printf("n_merge_user %d\n", n_merge_user);
        std::vector<int> label_l = BuildKMeans(user, n_merge_user);

        int n_user = user.n_vector_;
        int n_data_item = data_item.n_vector_;
        int vec_dim = user.vec_dim_;

        int n_known_rank = std::min(n_data_item / 10, 5);

        int n_interval = n_known_rank + 1;
        int interval_size = n_data_item / n_known_rank - 1;
        int final_interval_size = n_data_item % (interval_size + 1);

        printf("n_known_rank %d, n_interval %d, interval_size %d, final_interval_size %d\n", n_known_rank, n_interval,
               interval_size, final_interval_size);

        //confirm the specific rank, start from 1
        std::vector<int> known_rank_idx_l(n_known_rank);
        for (int i = 0; i < n_known_rank; i++) {
            known_rank_idx_l[i] = (i + 1) * interval_size + i - 1;
        }

        std::vector<DistancePair> distance_table(n_user * n_known_rank);
        std::vector<IntervalVector> interval_arr(n_merge_user, IntervalVector(n_interval));

        std::vector<DistancePair> distance_cache(n_data_item);
        for (int userID = 0; userID < n_user; userID++) {
            for (int itemID = 0; itemID < n_data_item; itemID++) {
                double ip = InnerProduct(data_item.getVector(itemID), user.getVector(userID), vec_dim);
                distance_cache[itemID] = DistancePair(ip, itemID);
            }
            std::sort(distance_cache.begin(), distance_cache.end(), std::greater<DistancePair>());

            std::vector<std::vector<int>> tmp_interval_vec(n_interval, std::vector<int>());
            for (int i = 0; i < n_known_rank; i++) {
                distance_table[userID * n_known_rank + i] = distance_cache[known_rank_idx_l[i]];
                int base_idx = i == 0 ? 0 : known_rank_idx_l[i-1];
                for (int j = 0; j < interval_size; j++) {
                    tmp_interval_vec[i].emplace_back(distance_cache[base_idx + j].ID_);
                }
            }
            {
                int base_idx = known_rank_idx_l[n_known_rank - 1];
                for (int j = 0; j < final_interval_size; j++) {
                    tmp_interval_vec[n_known_rank].emplace_back(distance_cache[base_idx + j].ID_);
                }
            }
            IntervalVector setVector(tmp_interval_vec, n_interval);

            interval_arr[label_l[userID]].Merge(setVector);

        }

        RankIntervalIndex rii(known_rank_idx_l, distance_table, interval_arr, label_l,
                              std::vector<int>{n_known_rank, n_user, n_merge_user, n_interval, interval_size,
                                               final_interval_size});
        rii.setUserItemMatrix(user, data_item);
        return rii;
    }

}

#endif //REVERSE_KRANKS_SAMPLEINDEX_HPP
