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
        std::unique_ptr<double[]> sample_IP_l_;
        int n_sample_, n_larger_;

    public:
        inline HashBucket() {
            n_sample_ = 0;
            n_larger_ = -1;
            sample_IP_l_ = nullptr;
        }

        void Preprocess(const double *IP_vecs, const std::pair<double, double> &IP_bound, const int &n_data_item,
                        const int &n_larger) {
            this->n_larger_ = n_larger;
            const double IP_lb = IP_bound.first;
            const double IP_ub = IP_bound.second;

            auto rank_lb_ptr = std::lower_bound(IP_vecs, IP_vecs + n_data_item, IP_lb,
                                                [](const double &arr_val, double query_val) {
                                                    return arr_val > query_val;
                                                });

            auto rank_ub_ptr = std::lower_bound(IP_vecs, IP_vecs + n_data_item, IP_ub,
                                                [](const double &arr_val, double query_val) {
                                                    return arr_val > query_val;
                                                });
            const int &rank_lb = rank_lb_ptr - IP_vecs;
            const int &rank_ub = rank_ub_ptr - IP_vecs;
            const int size = rank_lb - rank_ub + 1;

            sample_IP_l_ = std::make_unique<double[]>(size);
            n_sample_ = size;
            for (int rankID = rank_lb; rankID <= rank_ub; rankID++) {
                sample_IP_l_[rankID - rank_lb] = IP_vecs[rankID];
            }

        }

        std::pair<int, int>
        BinarySearch(const double &queryIP, const int &sample_every, const std::pair<int, int> &rank_pair) {
            if (n_sample_ == 0) {
                return rank_pair;
            }

            //get the rank of the first sample arr
            const int rank_ub = rank_pair.second;
            int bucket_ub = std::floor(1.0 * (rank_ub - sample_every + 1) / sample_every);
            bucket_ub = bucket_ub < 0 ? 0 : bucket_ub;
            const int sample_rank_ub = sample_every - 1 + bucket_ub * sample_every;

            auto rank_lb_ptr = std::lower_bound(sample_IP_l_.get(), sample_IP_l_.get() + n_sample_, queryIP,
                                                [](const double &arr_val, double queryIP) {
                                                    return arr_val > queryIP;
                                                });
            const int bound_dist = rank_lb_ptr - sample_IP_l_.get();
            if (bound_dist == 0) {
                return std::make_pair(sample_rank_ub, rank_pair.second);
            } else if (bound_dist == n_sample_) {
                const int sample_rank_lb = sample_rank_ub + (n_sample_ - 1) * sample_every;
                return std::make_pair(rank_pair.first, sample_rank_lb);
            } else {
                const int res_rank_lb = sample_rank_ub + (n_sample_ - 1) * sample_every;
                const int res_rank_ub = sample_rank_ub + (n_sample_ - 2) * sample_every;
                return std::make_pair(res_rank_lb, res_rank_ub);
            }

        }
    };

    class HashSearch {

        int n_data_item_, n_user_, cache_bound_every_, n_interval_, n_cache_rank_;
        std::unique_ptr<int[]> known_rank_idx_l_; // n_cache_rank
        std::unique_ptr<double[]> bkt_dist_l_; // n_user_, stores the distance of each user
        std::unique_ptr<std::pair<double, double>[]> IPbound_l_; // n_user, stores IP_lb, IP_ub bound for each user
        std::unique_ptr<HashBucket[]> bucket_l_; // n_user * n_interval_
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
            IPbound_l_ = std::make_unique<std::pair<double, double>[]>(n_user_);
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

            spdlog::info("HashSearch: cache_bound_every {}, n_cache_rank {}, n_interval {}, n_max_disk_read {}",
                         cache_bound_every_, n_cache_rank_, n_interval_, n_max_disk_read_);
        }

        void LoopPreprocess(const DistancePair *distance_ptr, const int &userID) {
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
            IPbound_l_[userID] = std::make_pair(IP_lb, IP_ub);
            bkt_dist_l_[userID] = dist;

            std::vector<double> sample_IP_l(n_cache_rank_);
            for (int crankID = 0; crankID < n_cache_rank_; crankID++) {
                int rankID = known_rank_idx_l_[crankID];
                sample_IP_l[crankID] = distance_ptr[rankID];
            }

            for (int itvID = 0; itvID < n_interval_; itvID++) {
                double tmp_IP_lb = IP_lb + itvID * dist;
                double tmp_IP_ub = IP_lb + (itvID + 1) * dist;
                std::pair<double, double> tmp_IP_pair = std::make_pair(tmp_IP_lb, tmp_IP_ub);
                bucket_l_[userID * n_interval_ + itvID].Preprocess(sample_IP_l.data(), tmp_IP_pair, n_data_item_);
            }
        }


        void RankBound(const std::vector<double> &queryIP_l, const std::vector<bool> &prune_l,
                       std::vector<int> &rank_lb_l, std::vector<int> &rank_ub_l) const {
            for (int userID = 0; userID < n_user_; userID++) {
                if (prune_l[userID]) {
                    continue;
                }
                std::pair<double, double> IPbound = IPbound_l_[userID];
                const double user_IP_lb = IPbound.first;
                const double user_IP_ub = IPbound.second;
                const double bkt_dist = bkt_dist_l_[userID];
                const int bucketID = std::floor((queryIP_l[userID] - user_IP_lb) / bkt_dist);

                bucket_l_[userID * n_interval_ + bucketID]
            }
        }

    };
}

#endif //REVERSE_KRANKS_HASHSEARCH_HPP
