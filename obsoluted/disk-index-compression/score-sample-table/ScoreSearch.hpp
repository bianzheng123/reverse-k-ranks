//
// Created by BianZheng on 2022/3/18.
//

#ifndef REVERSE_KRANKS_INTERVALSEARCH_HPP
#define REVERSE_KRANKS_INTERVALSEARCH_HPP

#include "struct/DistancePair.hpp"

#include <cassert>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include "spdlog/spdlog.h"

namespace ReverseMIPS {

    class ScoreSearch {
    public:

        int n_interval_, n_user_, n_data_item_;
        // n_user * n_interval, the last element of an interval column must be n_data_item
        std::unique_ptr<int[]> interval_table_;
        // n_user, stores the distance of interval for each user
        std::unique_ptr<double[]> interval_dist_l_;
        // n_user, bound for column, first is lower bound, second is upper bound
        std::unique_ptr<std::pair<double, double>[]> user_ip_bound_l_;


        inline ScoreSearch() = default;

        inline ScoreSearch(const int &n_interval, const int &n_user, const int n_data_item) {
            this->n_interval_ = n_interval;
            this->n_user_ = n_user;
            this->n_data_item_ = n_data_item;

            interval_table_ = std::make_unique<int[]>(n_user_ * n_interval_);
            std::memset(interval_table_.get(), 0, n_user_ * n_interval_ * sizeof(int));

            interval_dist_l_ = std::make_unique<double[]>(n_user_);
            user_ip_bound_l_ = std::make_unique<std::pair<double, double>[]>(n_user_);
            spdlog::info("interval bound: n_interval {}", n_interval);
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
            double interval_distance = (ub - lb) / n_interval_;
            interval_dist_l_[userID] = interval_distance;

            int *interval_ptr = interval_table_.get() + userID * n_interval_;
            for (int itemID = 0; itemID < n_data_item_; itemID++) {
                double ip = distance_ptr[itemID];
                int itv_idx = std::floor((ub - ip) / interval_distance);
                assert(0 <= itv_idx && itv_idx < n_interval_);
                interval_ptr[itv_idx]++;
            }
            for (int intervalID = 1; intervalID < n_interval_; intervalID++) {
                interval_ptr[intervalID] += interval_ptr[intervalID - 1];
            }
            assert(interval_ptr[n_interval_ - 1] == n_data_item_);

        }

        void WriteIntervalTable(const char *dataset_name) const {
            char resPath[256];
            std::sprintf(resPath, "../../result/attribution/ScoreSampleTable/IntervalTable-%s.txt",
                         dataset_name);
            std::ofstream file(resPath);
            if (!file) {
                spdlog::error("error in write result");
            }

            for (int userID = 0; userID < n_user_; userID++) {
                for (int itvID = 0; itvID < n_interval_ - 1; itvID++) {
                    file << std::setw(8) << interval_table_[userID * n_interval_ + itvID] << ",";
                }
                file << std::setw(8) << interval_table_[userID * n_interval_ + n_interval_ - 1] << std::endl;
            }
            file.close();

        }

    };
}
#endif //REVERSE_KRANKS_INTERVALSEARCH_HPP
