//
// Created by BianZheng on 2022/11/3.
//

#ifndef REVERSE_KRANKS_BITINDEX_HPP
#define REVERSE_KRANKS_BITINDEX_HPP

#include "struct/DistancePair.hpp"

#include <iostream>
#include <memory>
#include <fstream>
#include <spdlog/spdlog.h>
#include <set>
#include <numeric>

namespace ReverseMIPS {

    class BitIndex {

        size_t n_sample_, n_data_item_, n_user_, n_sample_score_;
        std::vector<bool> score_distribution_l_; // n_user * (n_sample_ - 1) * n_sample_score_
    public:

        inline BitIndex() {}

        inline BitIndex(const size_t &n_data_item, const size_t &n_user, const size_t &n_sample) {
            this->n_data_item_ = n_data_item;
            this->n_user_ = n_user;
            this->n_sample_ = n_sample;

            n_sample_score_ = 8;
            score_distribution_l_.resize(n_user_ * (n_sample_ - 1) * n_sample_score_);
        }

        void LoopPreprocess(const double *distance_ptr, const int *sample_rank_l, const int &userID) {

            for (int crankID = 0; crankID < n_sample_ - 1; crankID++) {
                const unsigned int rank_lb = sample_rank_l[crankID + 1];
                const unsigned int rank_ub = sample_rank_l[crankID];
                const double IP_ub = distance_ptr[rank_ub];
                const double IP_lb = distance_ptr[rank_lb];
                assert(IP_ub >= IP_lb);

                assert(0 <= rank_ub && rank_ub <= rank_lb && rank_lb < n_data_item_);

                const double distribution_distance = (IP_ub - IP_lb) / (double) (n_sample_score_ + 1);
                const int64_t offset = userID * (n_sample_ - 1) * n_sample_score_ + crankID * n_sample_score_;

                for (int scoreID = 0; scoreID < n_sample_score_; scoreID++) {
                    double sample_IP = IP_ub - (scoreID + 1) * distribution_distance;
                    const uint64_t pred_sample_rank =
                            (rank_lb - rank_ub) * (scoreID + 1) / (n_sample_score_ + 1) + rank_ub;
                    const double *iter_begin = distance_ptr;
                    const double *iter_end = distance_ptr + n_data_item_;

                    const double *lb_ptr = std::lower_bound(iter_begin, iter_end, sample_IP,
                                                            [](const double &arrIP, double queryIP) {
                                                                return arrIP > queryIP;
                                                            });
                    const int sample_rank = (int) (lb_ptr - iter_begin);
                    assert(0 <= sample_rank && sample_rank <= rank_lb);
                    assert(rank_ub <= pred_sample_rank && pred_sample_rank <= rank_lb);
                    const bool is_larger = sample_rank >= pred_sample_rank;
                    score_distribution_l_[offset + scoreID] = is_larger;
                }
            }

        }

        inline void ScoreDistribution(const double &queryIP, const int &userID, const int &bucketID,
                                      const double &IP_lb, const double &IP_ub,
                                      int &rank_lb, int &rank_ub) const {

            if (bucketID == 0 || bucketID == n_sample_) {
                return;
            } else { // lies between the middle
                assert(IP_ub >= queryIP && queryIP >= IP_lb);
                const double distribution_distance = (IP_ub - IP_lb) / (double) (n_sample_score_ + 1);
                const int itvID_ub = std::floor((IP_ub - queryIP) / distribution_distance);
                assert(0 <= itvID_ub && itvID_ub <= n_sample_score_);
                const int itvID_lb = itvID_ub + 1;

                unsigned int sample_rank_lb = rank_lb;
                unsigned int sample_rank_ub = rank_ub;

                const uint64_t sample_score_offset =
                        userID * (n_sample_ - 1) * n_sample_score_ + (bucketID - 1) * n_sample_score_;
                if (itvID_ub != 0) {
                    int first_true_idx = -1;
                    for (int sample_scoreID = itvID_ub - 1; sample_scoreID >= 0; sample_scoreID--) {
                        if (score_distribution_l_[sample_score_offset + sample_scoreID]) {
                            first_true_idx = sample_scoreID;
                            break;
                        }
                    }
                    if (first_true_idx != -1) {
                        const uint64_t pred_sample_rank =
                                (sample_rank_lb - sample_rank_ub) * (first_true_idx + 1) / (n_sample_score_ + 1) +
                                sample_rank_ub;
                        rank_ub = (int) pred_sample_rank;

                        assert(IP_ub - (first_true_idx + 1) * distribution_distance >= queryIP);
                    }
                    assert(first_true_idx == -1 ||
                           (0 <= first_true_idx && first_true_idx <= itvID_ub - 1));
                }

                if (itvID_lb != n_sample_score_ + 1) {
                    int first_false_idx = -1;
                    for (int sample_scoreID = itvID_lb - 1; sample_scoreID < n_sample_score_; sample_scoreID++) {
                        if (!score_distribution_l_[sample_score_offset + sample_scoreID]) {
                            first_false_idx = sample_scoreID;
                            break;
                        }
                    }
                    if (first_false_idx != -1) {
                        const uint64_t pred_sample_rank =
                                (sample_rank_lb - sample_rank_ub) * (first_false_idx + 1) / (n_sample_score_ + 1) +
                                sample_rank_ub;
                        rank_lb = (int) pred_sample_rank;

                        assert(IP_ub - (first_false_idx + 1) * distribution_distance <= queryIP);
                    }
                    assert(first_false_idx == -1 ||
                           (itvID_lb - 1 <= first_false_idx && first_false_idx <= n_sample_score_ - 1));
                }

            }
            assert(rank_ub <= rank_lb);
        }


        void ScoreDistributionRankBound(const std::vector<double> &queryIP_l,
                                        const std::vector<bool> &prune_l, const std::vector<bool> &result_l,
                                        const std::vector<int> &bucketID_l,
                                        const std::vector<std::pair<double, double>> &queryIP_bound_l,
                                        std::vector<int> &rank_lb_l, std::vector<int> &rank_ub_l) {
            assert(queryIP_l.size() == n_user_);
            assert(prune_l.size() == n_user_);
            assert(result_l.size() == n_user_);
            assert(queryIP_bound_l.size() == n_user_);
            assert(rank_lb_l.size() == n_user_);
            assert(rank_ub_l.size() == n_user_);
            for (int userID = 0; userID < n_user_; userID++) {
                if (prune_l[userID] || result_l[userID]) {
                    continue;
                }
                int lower_rank = rank_lb_l[userID];
                int upper_rank = rank_ub_l[userID];
                assert(upper_rank <= lower_rank);
                double queryIP = queryIP_l[userID];
                const double IP_lb = queryIP_bound_l[userID].first;
                const double IP_ub = queryIP_bound_l[userID].second;

                int bucketID = bucketID_l[userID];
                ScoreDistribution(queryIP, userID, bucketID,
                                  IP_lb, IP_ub,
                                  lower_rank, upper_rank);

                rank_lb_l[userID] = lower_rank;
                rank_ub_l[userID] = upper_rank;
            }


        }

        uint64_t IndexSizeByte() const {
            const uint64_t known_rank_idx_size = sizeof(int) * n_sample_;
            const uint64_t bound_distance_table_size = sizeof(double) * n_user_ * n_sample_;
            const uint64_t score_distribution_size = n_user_ * (n_sample_ - 1) * n_sample_score_ / 8;
            return known_rank_idx_size + bound_distance_table_size + score_distribution_size;
        }

    };
}
#endif //REVERSE_KRANKS_BITINDEX_HPP
