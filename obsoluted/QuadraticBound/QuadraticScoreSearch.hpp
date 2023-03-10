//
// Created by BianZheng on 2022/6/3.
//

#ifndef REVERSE_KRANKS_QUADRATICSCORESEARCH_HPP
#define REVERSE_KRANKS_QUADRATICSCORESEARCH_HPP

#include "struct/DistancePair.hpp"

#include <cassert>
#include <vector>
#include <algorithm>
#include "spdlog/spdlog.h"

namespace ReverseMIPS {

    class QuadraticScoreSearch {
    public:

        int n_sample_, n_user_, n_data_item_;
        // n_user * n_interval, the last element of an interval column must be n_data_item
        std::unique_ptr<int[]> interval_table_;
        // n_user, stores the distance of interval for each user
        std::unique_ptr<double[]> interval_dist_l_;
        // n_user, bound for column, first is lower bound, second is upper bound
        std::unique_ptr<std::pair<double, double>[]> user_ip_bound_l_;


        inline QuadraticScoreSearch() = default;

        inline QuadraticScoreSearch(const int &n_sample, const int n_data_item, const int &n_user) {
            this->n_sample_ = n_sample;
            this->n_data_item_ = n_data_item;
            this->n_user_ = n_user;

            interval_table_ = std::make_unique<int[]>(n_user_ * n_sample_);
            std::memset(interval_table_.get(), 0, n_user_ * n_sample_ * sizeof(int));

            interval_dist_l_ = std::make_unique<double[]>(n_user_);
            user_ip_bound_l_ = std::make_unique<std::pair<double, double>[]>(n_user_);
        }

        void LoopPreprocess(const DistancePair *distance_ptr, const int &userID) {
            std::vector<double> IP_l(n_data_item_);
            for (int itemID = 0; itemID < n_data_item_; itemID++) {
                IP_l[itemID] = distance_ptr[itemID].dist_;
            }
            LoopPreprocess(IP_l.data(), userID);
        }

        void
        LoopPreprocess(const double *distance_ptr, const int &userID) {

            double upper_bound = distance_ptr[0] + 0.01;
            double lower_bound = distance_ptr[n_data_item_ - 1] - 0.01;
            const std::pair<double, double> &bound_pair = std::make_pair(lower_bound, upper_bound);

            double lb = bound_pair.first;
            double ub = bound_pair.second;
            user_ip_bound_l_[userID] = std::make_pair(lb, ub);
            double interval_distance = (ub - lb) / (n_sample_ * n_sample_);
            interval_dist_l_[userID] = interval_distance;

            int *interval_ptr = interval_table_.get() + userID * n_sample_;
            for (int itemID = 0; itemID < n_data_item_; itemID++) {
                double ip = distance_ptr[itemID];
                int itv_idx = std::floor(std::sqrt((ub - ip) / interval_distance));
                assert(0 <= itv_idx && itv_idx < n_sample_);
                interval_ptr[itv_idx]++;
            }
            for (int intervalID = 1; intervalID < n_sample_; intervalID++) {
                interval_ptr[intervalID] += interval_ptr[intervalID - 1];
            }
            assert(interval_ptr[n_sample_ - 1] == n_data_item_);

        }

        //convert ip_bound to rank_bound
        void RankBound(const std::vector<double> &queryIP_l, const int &topk,
                       std::vector<int> &rank_lb_l, std::vector<int> &rank_ub_l,
                       std::vector<std::pair<double, double>> &queryIPbound_l) {

            assert(rank_lb_l.size() == n_user_ && rank_ub_l.size() == n_user_);
            assert(queryIP_l.size() == n_user_);

            for (int userID = 0; userID < n_user_; userID++) {
                const double queryIP = queryIP_l[userID];
                std::pair<double, double> user_IPbound = user_ip_bound_l_[userID];
                const double user_IP_ub = user_IPbound.second;
                const double itv_dist = interval_dist_l_[userID];
                const int itvID = std::floor(std::sqrt((user_IP_ub - queryIP) / itv_dist));
                if (itvID < 0) {
                    assert(rank_ub_l[userID] <= 0 && 0 <= rank_lb_l[userID]);
                    rank_ub_l[userID] = 0;
                    rank_lb_l[userID] = 0;
                    queryIPbound_l[userID] = std::make_pair(queryIP, queryIP);
                    continue;
                } else if (itvID >= n_sample_) {
                    assert(rank_ub_l[userID] <= n_data_item_ && n_data_item_ <= rank_lb_l[userID]);
                    rank_ub_l[userID] = n_data_item_;
                    rank_lb_l[userID] = n_data_item_;
                    queryIPbound_l[userID] = std::make_pair(queryIP, queryIP);
                    continue;
                }

                int bkt_rank_ub = itvID == 0 ? 0 : interval_table_[userID * n_sample_ + itvID - 1];
                int bkt_rank_lb = interval_table_[userID * n_sample_ + itvID];
                rank_lb_l[userID] = bkt_rank_lb;
                rank_ub_l[userID] = bkt_rank_ub;

                double bkt_query_lb = user_IP_ub - (itvID + 1) * (itvID + 1) * itv_dist;
                double bkt_query_ub = user_IP_ub - itvID * itvID * itv_dist;
                queryIPbound_l[userID] = std::make_pair(bkt_query_lb, bkt_query_ub);
                assert(bkt_query_lb <= queryIP && queryIP <= bkt_query_ub);
            }
        }

    };
}

#endif //REVERSE_KRANKS_QUADRATICSCORESEARCH_HPP
