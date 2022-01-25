//
// Created by BianZheng on 2022/1/25.
//

#ifndef REVERSE_KRANKS_BUCKETINTERVAL_HPP
#define REVERSE_KRANKS_BUCKETINTERVAL_HPP


#include "struct/DistancePair.hpp"
#include "struct/IntervalVector.hpp"
#include "struct/BucketVector.hpp"
#include "alg/SpaceInnerProduct.hpp"
#include "alg/KMeans.hpp"
#include <vector>
#include <cfloat>
#include <cstring>
#include <fstream>

namespace ReverseMIPS {
    class BucketIntervalIndex {

    public:
        std::vector<int> user_merge_idx_l_; // shape: n_user, record which user belongs to which IntervalVector
        std::vector<BucketVector> bucket_vector_l_; // shape: n_merge_user
        // IntervalVector shape: n_bucket * unknown_size
        std::vector<int> bucket_size_l_;
        /*
         * shape: n_user * (n_bucket - 1), for each user, record the lower bound of rank in each bucket
         * start record from the second bucket since the first is always 0
         */
        int n_user_, n_merge_user_, n_bucket_;
        VectorMatrix user_, data_item_;

        [[nodiscard]] std::vector<std::vector<RankElement>> Retrieval(VectorMatrix query_item, int topk) const {
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

        [[nodiscard]] int RelativeRankInBucket(const std::vector<int> &candidate_l, double queryIP,
                                               int userID, int vec_dim, double upper_bound_ip) const {
            //calculate all the IP, then get the lower bound
            //make different situation by the information
            int bucket_size = (int) candidate_l.size();
            int rank = 0;

            for (int i = 0; i < bucket_size; i++) {
                double data_dist = InnerProduct(data_item_.getVector(candidate_l[i]), user_.getVector(userID), vec_dim);
                rank += data_dist > queryIP && data_dist < upper_bound_ip ? 1 : 0;
            }

            return rank;
        }

        [[nodiscard]] int GetRank(const int userID, const double *query_vec, const int vec_dim) const {
            double *user_vec = this->user_.getVector(userID);
            double queryIP = InnerProduct(query_vec, user_vec, vec_dim);

            int bucket_vector_idx = user_merge_idx_l_[userID];
            std::vector<int> candidate_l = bucket_vector_l_[bucket_vector_idx].getBucketByIP(queryIP);
            double upper_bound_ip = bucket_vector_l_[bucket_vector_idx].getUpperBoundByIP(queryIP);
            int bucketID = bucket_vector_l_[bucket_vector_idx].getBucketIndexByIP(queryIP);
            int loc_rk = RelativeRankInBucket(candidate_l, queryIP, userID, vec_dim, upper_bound_ip);

            int rank;
            if (bucketID == n_bucket_) {
                rank = data_item_.n_vector_;
            } else {
                rank = bucketID == 0 ? loc_rk : bucket_size_l_[userID * (n_bucket_ - 1) + bucketID - 1] + loc_rk;
            }
            return rank + 1;
        }


        inline BucketIntervalIndex(const std::vector<int> &user_merge_idx_l,
                                   const std::vector<BucketVector> &bucket_vector_l,
                                   const std::vector<int> &bucket_size_l) {
            this->user_merge_idx_l_ = user_merge_idx_l;
            this->bucket_vector_l_ = bucket_vector_l;
            this->n_user_ = (int) user_merge_idx_l.size();
            this->n_merge_user_ = (int) bucket_vector_l.size();
            this->n_bucket_ = bucket_vector_l[0].n_bucket_;
            this->bucket_size_l_ = bucket_size_l;
        }

        void setUserItemMatrix(VectorMatrix &user, VectorMatrix &data_item) {
            this->user_ = user;
            this->data_item_ = data_item;
        }

    };


    /*
     * bruteforce index
     * shape: 1, type: int, n_user
     * shape: 1, type: int, n_data_item
     * shape: n_user * n_data_item, type: DistancePair, the distance pair for each user
     */

    void BuildSaveBruteForceIndex(const VectorMatrix &user, const VectorMatrix &data_item, const char *index_path,
                                  const std::vector<int> &user_merge_idx_l,
                                  std::vector<std::pair<double, double>> &ip_bound_l) {
        const int write_every_ = 10000;
        const int report_batch_every_ = 5;

        std::ofstream out(index_path, std::ios::binary | std::ios::out);
        if (!out) {
            std::printf("error in write result\n");
        }
        const int n_data_item = data_item.n_vector_;
        std::vector<DistancePair> distance_cache(write_every_ * n_data_item);
        const int vec_dim = data_item.vec_dim_;
        const int n_batch = user.n_vector_ / write_every_;
        const int n_remain = user.n_vector_ % write_every_;
        out.write((char *) &user.n_vector_, sizeof(int));
        out.write((char *) &n_data_item, sizeof(int));

        TimeRecord batch_report_record;
        batch_report_record.reset();
        for (int i = 0; i < n_batch; i++) {
            for (int cacheID = 0; cacheID < write_every_; cacheID++) {
                int userID = write_every_ * i + cacheID;
                for (int itemID = 0; itemID < n_data_item; itemID++) {
                    double ip = InnerProduct(data_item.getVector(itemID), user.getVector(userID), vec_dim);
                    distance_cache[cacheID * n_data_item + itemID] = DistancePair(ip, itemID);
                }
                std::sort(distance_cache.begin() + cacheID * n_data_item,
                          distance_cache.begin() + (cacheID + 1) * n_data_item, std::greater<DistancePair>());

                int merge_idx = user_merge_idx_l[userID];
                double ip_lower_bound = std::min(distance_cache[(cacheID + 1) * n_data_item - 1].dist_,
                                                 ip_bound_l[merge_idx].first);
                double ip_upper_bound = std::max(distance_cache[cacheID * n_data_item].dist_,
                                                 ip_bound_l[merge_idx].second);
                ip_bound_l[merge_idx].first = ip_lower_bound;
                ip_bound_l[merge_idx].second = ip_upper_bound;
            }
            out.write((char *) distance_cache.data(), distance_cache.size() * sizeof(DistancePair));

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
                distance_cache[cacheID * data_item.n_vector_ + itemID] = DistancePair(ip, itemID);
            }

            std::sort(distance_cache.begin() + cacheID * n_data_item,
                      distance_cache.begin() + (cacheID + 1) * n_data_item, std::greater<DistancePair>());

            int merge_idx = user_merge_idx_l[userID];
            double ip_lower_bound = std::min(distance_cache[(cacheID + 1) * n_data_item - 1].dist_,
                                             ip_bound_l[merge_idx].first);
            double ip_upper_bound = std::max(distance_cache[cacheID * n_data_item].dist_,
                                             ip_bound_l[merge_idx].second);
            ip_bound_l[merge_idx].first = ip_lower_bound;
            ip_bound_l[merge_idx].second = ip_upper_bound;
        }

        out.write((char *) distance_cache.data(),
                  n_remain * data_item.n_vector_ * sizeof(DistancePair));
    }

    void BuildBucketIndex(const std::vector<int> &user_merge_idx_l, const char *index_path,
                          std::vector<BucketVector> &bkt_vec_l, std::vector<int> &bucket_size_l, const int n_bucket) {
        std::ifstream index_stream_ = std::ifstream(index_path, std::ios::binary | std::ios::in);
        if (!index_stream_) {
            std::printf("error in writing index\n");
        }
        int n_user, n_data_item;
        index_stream_.read((char *) &n_user, sizeof(int));
        index_stream_.read((char *) &n_data_item, sizeof(int));

        const int n_cache = std::min(n_user, 10000);
        const int report_user_every_ = 10000;

        std::vector<DistancePair> distance_cache(n_cache * n_data_item);
        const int n_batch = n_user / n_cache;
        const int n_remain = n_user % n_cache;

        TimeRecord batch_report_record;
        batch_report_record.reset();
        for (int batchID = 0; batchID < n_batch; batchID++) {
            index_stream_.read((char *) distance_cache.data(), n_cache * n_data_item * sizeof(DistancePair));
            for (int cacheID = 0; cacheID < n_cache; cacheID++) {
                int userID = batchID * n_cache + cacheID;
                int bkt_l_idx = user_merge_idx_l[userID];
                BucketVector &tmp_bkt_vec = bkt_vec_l[bkt_l_idx];

                // get index of every data item, used for calculate the size of bucket in each user
                std::vector<int> bkt_idx_l(n_data_item);
                std::memset(bkt_idx_l.data(), 0, sizeof(int) * n_data_item);
                for (int d_itemID = 0; d_itemID < n_data_item; d_itemID++) {
                    int dp_idx = cacheID * n_data_item + d_itemID;
                    const DistancePair &dp = distance_cache[dp_idx];
                    int bkt_idx = tmp_bkt_vec.addUniqueElement(dp.dist_, dp.ID_);
                    bkt_idx_l[d_itemID] = bkt_idx;
                }

                int size_count = 0;
                int bkt_ptr = 0;
                int d_itemID = 0;
                if (bkt_idx_l[d_itemID] != bkt_ptr) {
                    for (; bkt_ptr < bkt_idx_l[d_itemID]; bkt_ptr++) {
                        bucket_size_l[userID * (n_bucket - 1) + bkt_ptr] = size_count;
                    }
                } else {
                    size_count++;
                }
                for (d_itemID = 1; d_itemID < n_data_item; d_itemID++) {
                    if (bkt_ptr < bkt_idx_l[d_itemID]) {
                        bucket_size_l[userID * (n_bucket - 1) + bkt_ptr] = size_count;
                        bkt_ptr++;
                    }
                    size_count++;
                }
                for (; bkt_ptr < n_bucket - 1; bkt_ptr++) {
                    bucket_size_l[userID * (n_bucket - 1) + bkt_ptr] = size_count;
                }

                if (userID % report_user_every_ == 0) {
                    std::cout << "read and process bucket vector " << userID / (0.01 * n_user) << " %, "
                              << batch_report_record.get_elapsed_time_second() << " s/iter" << " Mem: "
                              << get_current_RSS() / 1000000 << " Mb \n";
                    batch_report_record.reset();
                }
            }

        }

        index_stream_.read((char *) distance_cache.data(), n_remain * n_data_item * sizeof(DistancePair));
        for (int cacheID = 0; cacheID < n_remain; cacheID++) {
            int userID = n_batch * n_cache + cacheID;
            int bkt_l_idx = user_merge_idx_l[userID];
            BucketVector &tmp_bkt_vec = bkt_vec_l[bkt_l_idx];
            // get index of every data item, used for calculate the size of bucket in each user
            std::vector<int> bkt_idx_l(n_data_item);
            std::memset(bkt_idx_l.data(), 0, sizeof(int) * n_data_item);

            for (int d_itemID = 0; d_itemID < n_data_item; d_itemID++) {
                int dp_idx = cacheID * n_data_item + d_itemID;
                const DistancePair &dp = distance_cache[dp_idx];
                int bkt_idx = tmp_bkt_vec.addUniqueElement(dp.dist_, dp.ID_);
                bkt_idx_l[d_itemID] = bkt_idx;
            }

            int size_count = 0;
            int bkt_ptr = 0;
            int d_itemID = 0;
            if (bkt_idx_l[d_itemID] != bkt_ptr) {
                for (; bkt_ptr < bkt_idx_l[d_itemID]; bkt_ptr++) {
                    bucket_size_l[userID * (n_bucket - 1) + bkt_ptr] = size_count;
                }
            } else {
                size_count++;
            }
            for (d_itemID = 1; d_itemID < n_data_item; d_itemID++) {
                if (bkt_ptr < bkt_idx_l[d_itemID]) {
                    bucket_size_l[userID * (n_bucket - 1) + bkt_ptr] = size_count;
                    bkt_ptr++;
                }
                size_count++;
            }
            for (; bkt_ptr < n_bucket - 1; bkt_ptr++) {
                bucket_size_l[userID * (n_bucket - 1) + bkt_ptr] = size_count;
            }

            if (userID % report_user_every_ == 0) {
                std::cout << "read and process bucket vector " << userID / (0.01 * n_user) << " %, "
                          << batch_report_record.get_elapsed_time_second() << " s/iter" << " Mem: "
                          << get_current_RSS() / 1000000 << " Mb \n";
                batch_report_record.reset();
            }
        }

        for (auto &tmp_bkt_vec: bkt_vec_l) {
            tmp_bkt_vec.stopAddUniqueElement();
        }

    }


    BucketIntervalIndex
    BuildIndex(VectorMatrix &user, VectorMatrix &data_item, int n_merge_user, const char *dataset_name,
               std::vector<double> &component_time_l) {

        int n_user = user.n_vector_;
        int n_data_item = data_item.n_vector_;
        int vec_dim = user.vec_dim_;

        int n_bucket = std::min(n_data_item / 10, 5);

        //perform Kmeans for user vector, the label start from 0, indicates where the rank should come from
        printf("n_merge_user %d\n", n_merge_user);
        std::vector<int> user_merge_idx_l = BuildKMeans(user, n_merge_user);

        char index_path[256];
        sprintf(index_path, "../index/%s.bkt_itv_idx", dataset_name);

        //left: lower bound, right: upper bound
        std::vector<std::pair<double, double>> ip_bound_l(n_merge_user, std::pair<double, double>(DBL_MAX, DBL_MIN));

        TimeRecord record;
        BuildSaveBruteForceIndex(user, data_item, index_path, user_merge_idx_l, ip_bound_l);
        double bruteforce_index_time = record.get_elapsed_time_second();
        component_time_l.push_back(bruteforce_index_time);

        std::vector<BucketVector> bkt_vec_l;
        for (std::pair<double, double> tmp_bound: ip_bound_l) {
            double lb = tmp_bound.first;
            double ub = tmp_bound.second;
            bkt_vec_l.emplace_back(n_bucket, lb, ub);
        }
        std::vector<int> bucket_size_l(n_user * (n_bucket - 1));
        BuildBucketIndex(user_merge_idx_l, index_path, bkt_vec_l, bucket_size_l, n_bucket);

        printf("n_bucket %d\n", n_bucket);

        BucketIntervalIndex bucketIntervalIndex(user_merge_idx_l, bkt_vec_l, bucket_size_l);
        bucketIntervalIndex.setUserItemMatrix(user, data_item);

        return bucketIntervalIndex;
    }
}
#endif //REVERSE_KRANKS_BUCKETINTERVAL_HPP
