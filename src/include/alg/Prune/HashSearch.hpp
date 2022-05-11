//
// Created by BianZheng on 2022/5/5.
//

#ifndef REVERSE_KRANKS_HASHSEARCH_HPP
#define REVERSE_KRANKS_HASHSEARCH_HPP

#include "struct/DistancePair.hpp"
#include <memory>
#include <spdlog/spdlog.h>

namespace ReverseMIPS {

    class HashBucket {

    public:
        int n_sample_;
        std::unique_ptr<double[]> sample_IP_l_;

        inline HashBucket() {
            n_sample_ = 0;
            sample_IP_l_ = nullptr;
        }

        void Preprocess(const double *IP_vecs, const std::pair<int, int> &bkt_rank_pair, const int &sample_every,
                        const int &n_data_item) {
            const int &bkt_rank_lb = bkt_rank_pair.first;
            const int &bkt_rank_ub = bkt_rank_pair.second;
            assert(bkt_rank_ub <= bkt_rank_lb);

            const int &sampleID_end = std::floor(1.0 * (bkt_rank_lb - sample_every + 1) / sample_every);
            const int &end_sample_rank = sample_every - 1 + sampleID_end * sample_every;

            int size = 0;
            if(end_sample_rank == bkt_rank_pair.first){
                size--;
            }
            for (int rankID = end_sample_rank; rankID >= bkt_rank_ub; rankID -= sample_every) {
                size++;
            }

            sample_IP_l_ = std::make_unique<double[]>(size);
            n_sample_ = size;
            for (int candID = 0; candID < size; candID++) {
                sample_IP_l_[candID] = IP_vecs[end_sample_rank - candID * sample_every];

                assert(0 <= end_sample_rank - candID * sample_every &&
                       end_sample_rank - candID * sample_every <= n_data_item);
                if (not(bkt_rank_pair.first >= end_sample_rank - candID * sample_every &&
                        end_sample_rank - candID * sample_every >= bkt_rank_pair.second)) {
                    printf("bkt_rank_pair_lb %d, bkt_rank_pair_ub %d\n", bkt_rank_pair.first, bkt_rank_pair.second);
                    const int this_rank = end_sample_rank - candID * sample_every;
                    printf("this_rank %d\n", this_rank);
                    printf("sample_ID_end %d\n", sampleID_end);
                    printf("end_sample_rank %d\n", end_sample_rank);
                }

                assert(bkt_rank_pair.first >= end_sample_rank - candID * sample_every &&
                       end_sample_rank - candID * sample_every >= bkt_rank_pair.second);
            }

        }

        void BinarySearch(const double &queryIP, const int &sample_every,
                          const std::pair<int, int> &bkt_rank_pair, const std::pair<double, double> &bkt_IPbound_pair,
                          std::pair<int, int> &sample_rank_pair, std::pair<double, double> &queryIPbound_pair,
                          int queryID, int userID) {
            if (n_sample_ == 0) {
                sample_rank_pair = bkt_rank_pair;
                queryIPbound_pair = bkt_IPbound_pair;
                return;
            }

            //get the rank of the first sample arr
            const int rank_ub = bkt_rank_pair.second;
            int sampleID_lb = std::ceil(1.0 * (rank_ub - sample_every + 1) / sample_every);
            const int sample_rank_ub = sample_every - 1 + sampleID_lb * sample_every;

            auto sample_IP_ptr = std::lower_bound(sample_IP_l_.get(), sample_IP_l_.get() + n_sample_, queryIP,
                                                  [](const double &arr_val, double queryIP) {
                                                      return arr_val > queryIP;
                                                  });
            const int sample_loc_rk = sample_IP_ptr - sample_IP_l_.get();
            if (queryID == 0 && userID == 198) {
                printf("queryID %d, userID %d\n", queryID, userID);
                printf("sample_loc_rk %d\n", sample_loc_rk);
            }

            if (sample_loc_rk == 0) {
                const int res_rank_lb = sample_rank_ub;
                const int res_rank_ub = bkt_rank_pair.second;
                assert(res_rank_lb >= res_rank_ub);
                sample_rank_pair = std::make_pair(res_rank_lb, res_rank_ub);

                const double bkt_IP_lb = sample_IP_l_[0];
                const double bkt_IP_ub = bkt_IPbound_pair.second;
                assert(bkt_IP_lb <= bkt_IP_ub);
                queryIPbound_pair = std::make_pair(bkt_IP_lb, bkt_IP_ub);

            } else if (sample_loc_rk == n_sample_) {
                const int res_rank_lb = bkt_rank_pair.first;
                const int res_rank_ub = sample_rank_ub + (n_sample_ - 1) * sample_every;
                assert(res_rank_lb >= res_rank_ub);
                sample_rank_pair = std::make_pair(res_rank_lb, res_rank_ub);

                const double bkt_IP_lb = bkt_IPbound_pair.first;
                const double bkt_IP_ub = sample_IP_l_[n_sample_ - 1];
                assert(bkt_IP_lb <= bkt_IP_ub);
                queryIPbound_pair = std::make_pair(bkt_IP_lb, bkt_IP_ub);

            } else {
                const int res_rank_lb = sample_rank_ub + sample_loc_rk * sample_every;
                const int res_rank_ub = sample_rank_ub + (sample_loc_rk - 1) * sample_every;
                assert(res_rank_lb >= res_rank_ub);
                sample_rank_pair = std::make_pair(res_rank_lb, res_rank_ub);

                const double bkt_IP_lb = *sample_IP_ptr;
                const double bkt_IP_ub = *(sample_IP_ptr - 1);
                assert(bkt_IP_lb <= bkt_IP_ub);
                queryIPbound_pair = std::make_pair(bkt_IP_lb, bkt_IP_ub);
            }

        }
    };

    class HashSearch {

        int n_data_item_, n_user_, cache_bound_every_, n_interval_, n_cache_rank_;
        // n_cache_rank, store the sample rank
        std::unique_ptr<int[]> known_rank_idx_l_;
        // n_user_, stores the score bound of each user
        std::unique_ptr<double[]> bkt_dist_l_;
        // n_user, stores IP_lb, IP_ub bound of user and every item
        std::unique_ptr<std::pair<double, double>[]> glb_IPbound_l_;
        // n_user * n_interval, the last element of an interval column must be n_data_item
        std::unique_ptr<int[]> interval_table_;
        // n_user * n_interval_, stores the sample information in an interval
        std::unique_ptr<HashBucket[]> bucket_l_;
    public:
        int n_max_disk_read_;

        inline HashSearch() = default;

        inline HashSearch(const int &n_data_item, const int &n_user,
                          const int &cache_bound_every, const int &n_interval) {
            this->n_data_item_ = n_data_item;
            this->n_user_ = n_user;
            this->cache_bound_every_ = cache_bound_every;
            this->n_interval_ = n_interval;

            const int n_cache_rank = n_data_item / cache_bound_every;
            this->n_cache_rank_ = n_cache_rank;

            known_rank_idx_l_ = std::make_unique<int[]>(n_cache_rank_);
            bkt_dist_l_ = std::make_unique<double[]>(n_user_);
            glb_IPbound_l_ = std::make_unique<std::pair<double, double>[]>(n_user_);
            interval_table_ = std::make_unique<int[]>(n_user_ * n_interval_);
            bucket_l_ = std::make_unique<HashBucket[]>(n_user_ * n_interval_);

            if (cache_bound_every >= n_data_item) {
                spdlog::error("cache bound every larger than n_data_item, program exit");
                exit(-1);
            }
            if (n_cache_rank_ <= 0) {
                spdlog::error("cache rank size is too small, program exit");
                exit(-1);
            }
            assert(n_cache_rank_ > 0);

            Preprocess();

        }

        void Preprocess() {
            for (int known_rank_idx = cache_bound_every_ - 1, idx = 0;
                 known_rank_idx < n_data_item_; known_rank_idx += cache_bound_every_, idx++) {
                known_rank_idx_l_[idx] = known_rank_idx;
            }

            if (n_cache_rank_ >= 2) {
                assert(known_rank_idx_l_[0] == known_rank_idx_l_[1] - (known_rank_idx_l_[0] + 1));
            }
            n_max_disk_read_ = std::max(known_rank_idx_l_[0] + 1,
                                        n_data_item_ - known_rank_idx_l_[n_cache_rank_ - 1]);

            std::cout << "known_rank_idx_l" << std::endl;
            for (int sampleID = 0; sampleID < n_cache_rank_; sampleID++) {
                std::cout << known_rank_idx_l_[sampleID] << " ";
            }
            std::cout << std::endl;

            spdlog::info("HashSearch: cache_bound_every {}, n_cache_rank {}, n_interval {}, n_max_disk_read {}",
                         cache_bound_every_, n_cache_rank_, n_interval_, n_max_disk_read_);
        }

        [[maybe_unused]] void LoopPreprocess(const DistancePair *distance_ptr, const int &userID) {
            std::vector<double> IP_l(n_data_item_);
            for (int itemID = 0; itemID < n_data_item_; itemID++) {
                IP_l[itemID] = distance_ptr[itemID].dist_;
            }
            LoopPreprocess(IP_l.data(), userID);
        }

        void LoopPreprocess(const double *distance_ptr, const int &userID) {
            double IP_ub = distance_ptr[0] + 0.01;
            double IP_lb = distance_ptr[n_data_item_ - 1] - 0.01;
            double dist = (IP_ub - IP_lb) / n_interval_;
            glb_IPbound_l_[userID] = std::make_pair(IP_lb, IP_ub);
            bkt_dist_l_[userID] = dist;

            for (int itvID = 0; itvID < n_interval_; itvID++) {
                double tmp_IP_lb = IP_ub - (itvID + 1) * dist;
                auto rank_lb_ptr = std::lower_bound(distance_ptr, distance_ptr + n_data_item_, tmp_IP_lb,
                                                    [](const double &arr_val, double query_val) {
                                                        return arr_val > query_val;
                                                    });
                int size = rank_lb_ptr - distance_ptr;
                if(userID == 1 && itvID== 315){
                    printf("tmp_IP_lb %.3f, size %d, item_IP[510] %.3f, item_IP[511] %.3f\n", tmp_IP_lb, size, distance_ptr[510], distance_ptr[511]);
                }
                interval_table_[userID * n_interval_ + itvID] = size;
            }

            for (int itvID = 0; itvID < n_interval_; itvID++) {
                const int &bkt_rank_ub = itvID == 0 ? 0 : interval_table_[userID * n_interval_ + itvID - 1];
                const int &bkt_rank_lb = interval_table_[userID * n_interval_ + itvID];
                const std::pair<int, int> &bkt_rank_pair = std::make_pair(bkt_rank_lb, bkt_rank_ub);

                bucket_l_[userID * n_interval_ + itvID].Preprocess(distance_ptr, bkt_rank_pair, cache_bound_every_,
                                                                   n_data_item_);
            }

            assert(interval_table_[userID * n_interval_] >= 0);
            for (int itvID = 1; itvID < n_interval_; itvID++) {
                assert(interval_table_[userID * n_interval_ + itvID - 1] <=
                       interval_table_[userID * n_interval_ + itvID]);
            }
            assert(interval_table_[(userID + 1) * n_interval_ - 1] == n_data_item_);

            for (int itvID = 0; itvID < n_interval_; itvID++) {
                const double &bkt_IP_lb = IP_ub - (itvID + 1) * dist;
                const double &bkt_IP_ub = IP_ub - itvID * dist;
                const int &bkt_rank_ub = itvID == 0 ? 0 : interval_table_[userID * n_interval_ + itvID - 1];
                const int &bkt_rank_lb = interval_table_[userID * n_interval_ + itvID];

                int n_sample = bucket_l_[userID * n_interval_ + itvID].n_sample_;
                const double *sample_l = bucket_l_[userID * n_interval_ + itvID].sample_IP_l_.get();
                for (int sampleID = 0; sampleID < n_sample; sampleID++) {
                    if(not(bkt_IP_lb <= sample_l[sampleID] && sample_l[sampleID] <= bkt_IP_ub)){
                        printf("userID %d, itvID %d\n", userID, itvID);
                        printf("bkt_IP_lb %.3f, bkt_IP_ub %.3f\n", bkt_IP_lb, bkt_IP_ub);
                        printf("bkt_rank_lb %d, bkt_rank_ub %d\n", bkt_rank_lb, bkt_rank_ub);
                        printf("sample_l\n");
                        for (int sampleID1 = 0; sampleID1 < n_sample; sampleID1++) {
                            printf("%.3f ", sample_l[sampleID1]);
                        }
                        printf("\n");
                    }

                    assert(bkt_IP_lb <= sample_l[sampleID] && sample_l[sampleID] <= bkt_IP_ub);
                }
            }
        }


        void RankBound(const std::vector<double> &queryIP_l, const std::vector<bool> &prune_l,
                       std::vector<int> &rank_lb_l, std::vector<int> &rank_ub_l,
                       std::vector<std::pair<double, double>> &queryIPbound_l, int queryID) const {

            assert(queryIP_l.size() == prune_l.size() && prune_l.size() == rank_lb_l.size() &&
                   rank_lb_l.size() == rank_ub_l.size() && rank_ub_l.size() == queryIPbound_l.size());
            for (int userID = 0; userID < n_user_; userID++) {
                if (prune_l[userID]) {
                    continue;
                }
                const double queryIP = queryIP_l[userID];
                std::pair<double, double> IPbound = glb_IPbound_l_[userID];
                const double user_IP_ub = IPbound.second;
                const double bkt_dist = bkt_dist_l_[userID];
                const int bucketID = std::floor((user_IP_ub - queryIP_l[userID]) / bkt_dist);
                if (bucketID < 0) {
                    assert(rank_ub_l[userID] <= 0 && 0 <= rank_lb_l[userID]);
                    rank_ub_l[userID] = 0;
                    rank_lb_l[userID] = 0;
                    queryIPbound_l[userID] = std::make_pair(queryIP, queryIP);
                    continue;
                } else if (bucketID >= n_interval_) {
                    assert(rank_ub_l[userID] <= n_data_item_ && n_data_item_ <= rank_lb_l[userID]);
                    rank_ub_l[userID] = n_data_item_;
                    rank_lb_l[userID] = n_data_item_;
                    queryIPbound_l[userID] = std::make_pair(queryIP, queryIP);
                    continue;
                }

                int bkt_rank_ub = bucketID == 0 ? 0 : interval_table_[userID * n_interval_ + bucketID - 1];
                int bkt_rank_lb = interval_table_[userID * n_interval_ + bucketID];

                std::pair<int, int> bkt_rank_pair = std::make_pair(bkt_rank_lb, bkt_rank_ub);
                std::pair<int, int> sample_rank_pair;

                const double bkt_IP_lb = user_IP_ub - bkt_dist * (bucketID + 1);
                const double bkt_IP_ub = user_IP_ub - bkt_dist * bucketID;
                std::pair<double, double> bkt_IPbound_pair = std::make_pair(bkt_IP_lb, bkt_IP_ub);
                std::pair<double, double> queryIPbound_pair;
                bucket_l_[userID * n_interval_ + bucketID].BinarySearch(queryIP, cache_bound_every_,
                                                                        bkt_rank_pair, bkt_IPbound_pair,
                                                                        sample_rank_pair, queryIPbound_pair, queryID,
                                                                        userID);

                if (not(bkt_IPbound_pair.first <= queryIPbound_pair.first && queryIPbound_pair.first <= queryIP)) {
                    printf("known_rank_idx_l:\n");
                    for (int sampleID = 0; sampleID < n_cache_rank_; sampleID++) {
                        printf("%d ", known_rank_idx_l_[sampleID]);
                    }
                    printf("\n");

                    printf("queryID %d, userID %d, queryIP %.3f\n", queryID, userID, queryIP);
                    printf("bkt_rank_pair_lb %d, bkt_rank_pair_ub %d\n",
                           bkt_rank_pair.first, bkt_rank_pair.second);
                    printf("sample_rank_pair_lb %d, sample_rank_pair_ub %d\n",
                           sample_rank_pair.first, sample_rank_pair.second);
                    printf("bkt_IPbound_pair_lb %.3f, bkt_IPbound_pair_ub %.3f\n",
                           bkt_IPbound_pair.first, bkt_IPbound_pair.second);
                    printf("queryIPbound_pair_lb %.3f, queryIPbound_pair_ub %.3f\n",
                           queryIPbound_pair.first, queryIPbound_pair.second);
                    printf("bucket n_sample %d\n", bucket_l_[userID * n_interval_ + bucketID].n_sample_);
                }

                assert(rank_ub_l[userID] <= bkt_rank_pair.second && bkt_rank_pair.second <= sample_rank_pair.second);
                assert(sample_rank_pair.second <= sample_rank_pair.first);
                assert(sample_rank_pair.first <= bkt_rank_pair.first && bkt_rank_pair.first <= rank_lb_l[userID]);

                assert(bkt_IPbound_pair.first <= queryIPbound_pair.first && queryIPbound_pair.first <= queryIP);
                assert(queryIP <= queryIPbound_pair.second && queryIPbound_pair.second <= bkt_IPbound_pair.second);

                rank_lb_l[userID] = sample_rank_pair.first;
                rank_ub_l[userID] = sample_rank_pair.second;

                queryIPbound_l[userID] = queryIPbound_pair;

                const int &this_read_count = rank_lb_l[userID] - rank_ub_l[userID];
                assert(0 <= this_read_count && this_read_count <= n_max_disk_read_);

            }
        }

    };
}

#endif //REVERSE_KRANKS_HASHSEARCH_HPP
