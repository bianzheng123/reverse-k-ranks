//
// Created by BianZheng on 2022/8/17.
//

#ifndef REVERSE_K_RANKS_KTHUSERRANK_HPP
#define REVERSE_K_RANKS_KTHUSERRANK_HPP

#include <vector>
#include <random>
#include <algorithm>
#include <cassert>
#include <queue>

namespace ReverseMIPS {
    void SampleItem(const int &n_data_item, const int &n_sample_item, std::vector<int> &sample_itemID_l) {
        assert(sample_itemID_l.size() == n_sample_item);
        std::vector<int> shuffle_item_idx_l(n_data_item);
        std::iota(shuffle_item_idx_l.begin(), shuffle_item_idx_l.end(), 0);

//        std::shuffle(shuffle_item_idx_l.begin(), shuffle_item_idx_l.end(), std::default_random_engine(100));

        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(shuffle_item_idx_l.begin(), shuffle_item_idx_l.end(), g);

        for (int sampleID = 0; sampleID < n_sample_item; sampleID++) {
            sample_itemID_l[sampleID] = shuffle_item_idx_l[sampleID];
        }
    }

    void ComputeKthRank(const VectorMatrix &user, const VectorMatrix &data_item,
                        const int &n_sample_item, const int &sample_topk,
                        const std::vector<int> &sample_itemID_l,
                        std::vector<int> &sort_kth_rank_l,
                        std::vector<int> &sort_sample_itemID_l) {

        assert(sort_kth_rank_l.size() == n_sample_item);
        assert(sort_sample_itemID_l.size() == n_sample_item);

        const int n_data_item = data_item.n_vector_;
        const int n_user = user.n_vector_;
        ComputeItemIDScoreTable cst(user, data_item);
        std::vector<double> distance_l(n_data_item);
        std::vector<double> sample_itemIP_l(n_sample_item);

        std::vector<std::priority_queue<int, std::vector<int>, std::less<int>>> result_rank_l(n_sample_item);

        TimeRecord record;
        record.reset();

        for (int userID = 0; userID < sample_topk; userID++) {
            cst.ComputeItems(userID, distance_l.data());
            for (int sampleID = 0; sampleID < n_sample_item; sampleID++) {
                const int sampleItemID = sample_itemID_l[sampleID];
                sample_itemIP_l[sampleID] = distance_l[sampleItemID];
            }
            cst.SortItems(userID, distance_l.data());
            for (int sampleID = 0; sampleID < n_sample_item; sampleID++) {
                const double itemIP = sample_itemIP_l[sampleID];
                const double *distance_ptr = distance_l.data();

                const double *lb_ptr = std::lower_bound(distance_ptr, distance_ptr + n_data_item, itemIP,
                                                        [](const double &arrIP, const double& queryIP) {
                                                            return arrIP > queryIP;
                                                        });
                const long rank = lb_ptr - distance_ptr;
                result_rank_l[sampleID].push((int) rank);
            }
        }

        for (int userID = sample_topk; userID < n_user; userID++) {
            cst.ComputeItems(userID, distance_l.data());
            for (int sampleID = 0; sampleID < n_sample_item; sampleID++) {
                const int sampleItemID = sample_itemID_l[sampleID];
                sample_itemIP_l[sampleID] = distance_l[sampleItemID];
            }
            cst.SortItems(userID, distance_l.data());
            for (int sampleID = 0; sampleID < n_sample_item; sampleID++) {
                const double itemIP = sample_itemIP_l[sampleID];
                const double *distance_ptr = distance_l.data();

                const double *lb_ptr = std::lower_bound(distance_ptr, distance_ptr + n_data_item, itemIP,
                                                        [](const double &arrIP, const double &queryIP) {
                                                            return arrIP > queryIP;
                                                        });
                const long rank = lb_ptr - distance_ptr;
                const int heap_max_rank = result_rank_l[sampleID].top();

                if (heap_max_rank > rank) {
                    result_rank_l[sampleID].pop();
                    result_rank_l[sampleID].push((int) rank);
                }
            }

            if (userID % cst.report_every_ == 0 && userID != 0) {
                spdlog::info(
                        "Compute second score table {:.2f}%, {:.2f} s/iter, Mem: {} Mb, Compute Score Time {}s, Sort Score Time {}s",
                        userID / (0.01 * n_user), record.get_elapsed_time_second(), get_current_RSS() / (1024 * 1024),
                        cst.compute_time_, cst.sort_time_);
                cst.compute_time_ = 0;
                cst.sort_time_ = 0;
                record.reset();
            }
        }

        std::vector<int> topk_rank_l(n_sample_item);
        for (int sampleID = 0; sampleID < n_sample_item; sampleID++) {
            assert(result_rank_l[sampleID].size() == sample_topk);
            topk_rank_l[sampleID] = result_rank_l[sampleID].top();
        }

        std::vector<int> topk_rank_idx_l(n_sample_item);
        std::iota(topk_rank_idx_l.begin(), topk_rank_idx_l.end(), 0);
        std::sort(topk_rank_idx_l.begin(), topk_rank_idx_l.end(),
                  [&topk_rank_l](int i1, int i2) { return topk_rank_l[i1] < topk_rank_l[i2]; });

        for (int sampleID = 0; sampleID < n_sample_item; sampleID++) {
            sort_kth_rank_l[sampleID] = topk_rank_l[topk_rank_idx_l[sampleID]];
        }
        assert(std::is_sorted(sort_kth_rank_l.begin(), sort_kth_rank_l.end()));

        for (int sampleID = 0; sampleID < n_sample_item; sampleID++) {
            sort_sample_itemID_l[sampleID] = sample_itemID_l[topk_rank_idx_l[sampleID]];
        }
    }

}
#endif //REVERSE_K_RANKS_KTHUSERRANK_HPP
