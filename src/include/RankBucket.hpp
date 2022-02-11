//
// Created by BianZheng on 2021/12/27.
//

#ifndef REVERSE_KRANKS_SAMPLEINDEX_HPP
#define REVERSE_KRANKS_SAMPLEINDEX_HPP

#include "struct/VectorMatrix.hpp"
#include "util/TimeMemory.hpp"
#include "util/StringUtil.hpp"
#include "alg/SpaceInnerProduct.hpp"
#include "struct/DistancePair.hpp"
#include "struct/RankElement.hpp"
#include "alg/KMeans.hpp"
#include <fstream>
#include <cassert>
#include <utility>
#include <algorithm>

namespace ReverseMIPS {

    class BucketVector {

    public:
        std::vector<std::vector<int>> bucket_vector_;
        int dimension_;

        inline explicit BucketVector(int dimension) {
            this->dimension_ = dimension;
            this->bucket_vector_ = std::vector<std::vector<int>>(dimension, std::vector<int>());
        }

        inline BucketVector(std::vector<std::vector<int>> &set_vector, int dimension) {
            int size = (int) set_vector.size();
            for (int i = 0; i < size; i++) {
                std::sort(set_vector[i].begin(), set_vector[i].end());
            }

            this->bucket_vector_ = set_vector;
            this->dimension_ = dimension;
        }

        inline ~BucketVector() = default;

        void Merge(const BucketVector &other) {
            assert(other.dimension_ == this->dimension_);

            for (int i = 0; i < dimension_; i++) {
                //it has sorted before the assignment
                int total_size = (int) (bucket_vector_[i].size() + other.bucket_vector_[i].size());
                std::vector<int> final(total_size);

                std::vector<int>::iterator it;
                it = std::set_union(bucket_vector_[i].begin(), bucket_vector_[i].end(), other.bucket_vector_[i].begin(),
                                    other.bucket_vector_[i].end(), final.begin());
                final.resize(it - final.begin());
                bucket_vector_[i] = final;
            }
        }

        [[nodiscard]] std::vector<int> GetBucket(int id) const {
            return bucket_vector_[id];
        }


    };

    class RankBucketIndex {
    private:
        void ResetTime() {
            brute_force_rank_time_ = 0;
            self_inner_product_time_ = 0;
            binary_search_time_ = 0;
        }

    public:
        std::vector<int> known_rank_idx_l_; // shape: n_known_rank
        std::vector<int> user_merge_idx_l_; // shape: n_user, record which user belongs to which BucketVector
        std::vector<DistancePair> distance_table_; // shape: n_user * n_known_rank
        std::vector<BucketVector> bkt_vec_arr; // shape: n_merge_user
        // BucketVector shape: n_bucket * (bucket_size or final_bucket_size)
        int n_known_rank_, n_user_, n_merge_user_, n_bucket_, bucket_size_, final_bucket_size_;
        VectorMatrix user_, data_item_;
        double brute_force_rank_time_, self_inner_product_time_, binary_search_time_;
        TimeRecord record_;
#ifdef TEST
        int ip_calc = 0;
#endif


        RankBucketIndex(std::vector<int> &known_rank_idx_l, std::vector<DistancePair> &distance_table,
                        std::vector<BucketVector> &bkt_vec_arr, std::vector<int> &user_merge_idx_l,
                        std::vector<int> size_config) {
            this->known_rank_idx_l_ = std::move(known_rank_idx_l);
            this->distance_table_ = std::move(distance_table);
            this->bkt_vec_arr = std::move(bkt_vec_arr);
            this->user_merge_idx_l_ = std::move(user_merge_idx_l);

            this->n_known_rank_ = size_config[0];
            this->n_user_ = size_config[1];
            this->n_merge_user_ = size_config[2];
            this->n_bucket_ = size_config[3];
            this->bucket_size_ = size_config[4];
            this->final_bucket_size_ = size_config[5];
        }

        void setUserItemMatrix(VectorMatrix &user, VectorMatrix &data_item) {
            this->user_ = user;
            this->data_item_ = data_item;
        }

        [[nodiscard]] std::vector<std::vector<RankElement>> Retrieval(VectorMatrix query_item, int topk) {
            if (topk > user_.n_vector_) {
                printf("top-k is larger than user, system exit\n");
                exit(-1);
            }
            ResetTime();
            std::vector<std::vector<RankElement>> result(query_item.n_vector_, std::vector<RankElement>());
            int n_query = query_item.n_vector_;
            int vec_dim = query_item.vec_dim_;
            for (int qID = 0; qID < n_query; qID++) {
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

        [[nodiscard]] int GetRank(const int userID, const double *query_vec, const int vec_dim) {
            double *user = this->user_.getVector(userID);
            record_.reset();
            double queryIP = InnerProduct(query_vec, user, vec_dim);
            self_inner_product_time_ += record_.get_elapsed_time_second();
#ifdef TEST
            ip_calc++;
#endif
            record_.reset();
            int bucket_idx = BinarySearch(queryIP, userID);
            binary_search_time_ += record_.get_elapsed_time_second();

            record_.reset();
            int bucket_vector_idx = user_merge_idx_l_[userID];
            std::vector<int> candidate_l = bkt_vec_arr[bucket_vector_idx].GetBucket(bucket_idx);
            assert(0 <= bucket_idx && bucket_idx <= n_known_rank_);
            int loc_rk = RelativeRankInBucket(candidate_l, bucket_idx, queryIP, userID, vec_dim);
            brute_force_rank_time_ += record_.get_elapsed_time_second();

            int rank = bucket_idx == 0 ? loc_rk : known_rank_idx_l_[bucket_idx - 1] + loc_rk + 1;
            return rank + 1;
        }

        [[nodiscard]] int RelativeRankInBucket(const std::vector<int> &candidate_l, int bucket_idx, double queryIP,
                                               int userID, int vec_dim) const {
            //calculate all the IP, then get the lower bound
            //make different situation by the information
            int candidate_size = (int) candidate_l.size();
            std::vector<DistancePair> dp_l;
            double bucket_upper_bound =
                    bucket_idx == 0 ? DBL_MAX : distance_table_[userID * n_known_rank_ + bucket_idx - 1].dist_;
            double bucket_lower_bound =
                    bucket_idx == n_known_rank_ ? -DBL_MAX : distance_table_[userID * n_known_rank_ + bucket_idx].dist_;
            assert(bucket_lower_bound < queryIP && queryIP < bucket_upper_bound);
            for (int i = 0; i < candidate_size; i++) {
                double ip = InnerProduct(data_item_.getVector(candidate_l[i]), user_.getVector(userID), vec_dim);
                if (bucket_lower_bound < ip && ip < bucket_upper_bound) {
                    dp_l.emplace_back(ip, candidate_l[i]);
                }
            }
            std::sort(dp_l.begin(), dp_l.end(), std::greater<DistancePair>());

            auto lb_ptr = std::lower_bound(dp_l.begin(), dp_l.end(), queryIP,
                                           [](const DistancePair &info, double value) {
                                               return info.dist_ > value;
                                           });
            return (int) (lb_ptr - dp_l.begin());
        }

        //return the index of the bucket it belongs to
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

    RankBucketIndex
    BuildIndex(VectorMatrix &user, VectorMatrix &data_item) {
        int n_user = user.n_vector_;
        int n_data_item = data_item.n_vector_;
        int vec_dim = user.vec_dim_;
        if (n_data_item <= 5) {
            printf("the number of data item is too small, program exit\n");
            exit(-1);
        }

        const int n_merge_user = std::min(10000, n_user / 2);
        const int n_known_rank = n_data_item > 256 ? 128 : 5;

        //perform Kmeans for user vector, the label start from 0, indicates where the rank should come from
        printf("n_merge_user %d\n", n_merge_user);
        std::vector<int> label_l = BuildKMeans(user, n_merge_user);

        int n_bucket = n_known_rank + 1;
        int bucket_size = n_data_item / n_known_rank - 1;
        int final_bucket_size = n_data_item - (bucket_size + 1) * n_known_rank;

        printf("n_known_rank %d, n_bucket %d, bucket_size %d, final_bucket_size %d\n", n_known_rank, n_bucket,
               bucket_size, final_bucket_size);

        //confirm the specific rank, start from 1
        std::vector<int> known_rank_idx_l(n_known_rank);
        for (int i = 0; i < n_known_rank; i++) {
            known_rank_idx_l[i] = (bucket_size + 1) * (i + 1) - 1;
        }

        std::vector<DistancePair> distance_table(n_user * n_known_rank);
        std::vector<BucketVector> bkt_vec_arr(n_merge_user, BucketVector(n_bucket));

        std::vector<DistancePair> distance_cache(n_data_item);
        for (int userID = 0; userID < n_user; userID++) {
            for (int itemID = 0; itemID < n_data_item; itemID++) {
                double ip = InnerProduct(data_item.getVector(itemID), user.getVector(userID), vec_dim);
                distance_cache[itemID] = DistancePair(ip, itemID);
            }
            std::sort(distance_cache.begin(), distance_cache.end(), std::greater<DistancePair>());

            std::vector<std::vector<int>> tmp_bucket_vec(n_bucket, std::vector<int>());
            for (int i = 0; i < n_known_rank; i++) {
                distance_table[userID * n_known_rank + i] = distance_cache[known_rank_idx_l[i]];
                int base_idx = i == 0 ? 0 : known_rank_idx_l[i - 1] + 1;
                for (int j = 0; j < bucket_size; j++) {
                    tmp_bucket_vec[i].emplace_back(distance_cache[base_idx + j].ID_);
                }
            }
            {
                int base_idx = known_rank_idx_l[n_known_rank - 1] + 1;
                for (int j = 0; j < final_bucket_size; j++) {
                    tmp_bucket_vec[n_known_rank].emplace_back(distance_cache[base_idx + j].ID_);
                }
            }
            BucketVector setVector(tmp_bucket_vec, n_bucket);

            bkt_vec_arr[label_l[userID]].Merge(setVector);
        }

        RankBucketIndex rii(known_rank_idx_l, distance_table, bkt_vec_arr, label_l,
                            std::vector<int>{n_known_rank, n_user, n_merge_user, n_bucket, bucket_size,
                                             final_bucket_size});
        rii.setUserItemMatrix(user, data_item);
        return rii;
    }

}

#endif //REVERSE_KRANKS_SAMPLEINDEX_HPP
