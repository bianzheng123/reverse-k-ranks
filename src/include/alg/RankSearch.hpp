//
// Created by BianZheng on 2022/3/18.
//

#ifndef REVERSE_KRANKS_RANKSEARCH_HPP
#define REVERSE_KRANKS_RANKSEARCH_HPP

#include <memory>
#include <spdlog/spdlog.h>

namespace ReverseMIPS {

    constexpr bool
    UserRankMaxHeap(const std::pair<int, int> &user_rank, const std::pair<int, int> &other) {
        return user_rank.second < other.second;
    }

    class RankSearch {

        int n_cache_rank_, cache_bound_every_, n_data_item_, n_user_;
        std::unique_ptr<int[]> known_rank_idx_l_; // n_cache_rank
        std::unique_ptr<double[]> bound_distance_table_; // n_user * n_cache_rank
        std::unique_ptr<int[]> count_max_heap_;
    public:
        int n_max_disk_read_;

        inline RankSearch() {}

        inline RankSearch(const int &cache_bound_every, const int &n_data_item,
                          const int &n_user) {
            const int n_cache_rank = n_data_item / cache_bound_every;
            this->n_cache_rank_ = n_cache_rank;
            this->cache_bound_every_ = cache_bound_every;
            this->n_data_item_ = n_data_item;
            this->n_user_ = n_user;
            known_rank_idx_l_ = std::make_unique<int[]>(n_cache_rank_);
            bound_distance_table_ = std::make_unique<double[]>(n_user_ * n_cache_rank_);
            count_max_heap_ = std::make_unique<int[]>(n_cache_rank_ + 1);

            Preprocess();

        }

        void Preprocess() {
            for (int known_rank_idx = cache_bound_every_ - 1, idx = 0;
                 known_rank_idx < n_data_item_; known_rank_idx += cache_bound_every_, idx++) {
                known_rank_idx_l_[idx] = known_rank_idx;
            }

            assert(known_rank_idx_l_[0] == known_rank_idx_l_[1] - (known_rank_idx_l_[0] + 1));
            n_max_disk_read_ = std::max(known_rank_idx_l_[0] + 1,
                                        n_data_item_ - known_rank_idx_l_[n_cache_rank_ - 1]);

            spdlog::info("rank bound: cache_bound_every {}, n_cache_rank {}", cache_bound_every_, n_cache_rank_);
        }

        void LoopPreprocess(const double *distance_ptr, const int &userID) {
            for (int crankID = 0; crankID < n_cache_rank_; crankID++) {
                int itemID = known_rank_idx_l_[crankID];
                bound_distance_table_[userID * n_cache_rank_ + crankID] = distance_ptr[itemID];
            }
        }

        inline int
        CoarseBinarySearch(const double &queryIP, const int &userID, const int &rank_lb, const int &rank_ub) const {
            double *search_iter = bound_distance_table_.get() + userID * n_cache_rank_;

            int bucket_ub = std::ceil(1.0 * (rank_ub - cache_bound_every_ + 1) / cache_bound_every_);
            int bucket_lb = std::floor(1.0 * (rank_lb - cache_bound_every_ + 1) / cache_bound_every_);

            double *iter_begin = search_iter + bucket_ub;
            double *iter_end = search_iter + bucket_lb + 1;

            double *lb_ptr = std::lower_bound(iter_begin, iter_end, queryIP,
                                              [](const double &arrIP, double queryIP) {
                                                  return arrIP > queryIP;
                                              });
            int bucket_idx = bucket_ub + (int) (lb_ptr - iter_begin);
            return bucket_idx;
        }

        //return the index of the bucket it belongs to
        [[nodiscard]] inline int
        CoarseBinarySearchBound(const double &queryIP, const int &userID, const int &bound_rank_id,
                                const int &rank_lb, const int &rank_ub) const {
            auto iter_begin = bound_distance_table_.get() + userID * n_cache_rank_;
            if (bound_rank_id != n_cache_rank_ && iter_begin[bound_rank_id] > queryIP) {
                return -1;
            }
            int bucket_ub = std::ceil(1.0 * (rank_ub - cache_bound_every_ + 1) / cache_bound_every_);
            int bucket_lb = std::floor(1.0 * (rank_lb - cache_bound_every_ + 1) / cache_bound_every_);
            iter_begin += bucket_ub;

            int offset_size = bound_rank_id == n_cache_rank_ ? n_cache_rank_ - 1 : bound_rank_id;
            auto iter_end = iter_begin + std::min(offset_size, bucket_lb) + 1;

            auto lb_ptr = std::lower_bound(iter_begin, iter_end, queryIP,
                                           [](const double &arrIP, double queryIP) {
                                               return arrIP > queryIP;
                                           });
            return (int) (lb_ptr - iter_begin) + bucket_ub;
        }

        void RankBound(const std::vector<double> &queryIP_l, const int &topk,
                       std::vector<int> &rank_lb_l,
                       std::vector<int> &rank_ub_l,
                       std::vector<bool> &prune_l) {
            std::vector<std::pair<int, int>> topk_max_heap;
            topk_max_heap.reserve(topk);
            std::memset(count_max_heap_.get(), 0, sizeof(int) * (n_cache_rank_ + 1));
            for (int userID = 0; userID < topk; userID++) {
                if (prune_l[userID]) {
                    continue;
                }
                double queryIP = queryIP_l[userID];
                int crank = CoarseBinarySearch(queryIP, userID, rank_lb_l[userID], rank_ub_l[userID]);
                topk_max_heap.emplace_back(userID, crank);
                count_max_heap_[crank]++;
            }

            std::make_heap(topk_max_heap.begin(), topk_max_heap.end(), UserRankMaxHeap);
            for (int userID = topk; userID < n_user_; userID++) {
                if (prune_l[userID]) {
                    continue;
                }
                double queryIP = queryIP_l[userID];
                int global_crank = topk_max_heap.front().second;
                int crank = CoarseBinarySearchBound(queryIP, userID, global_crank, rank_lb_l[userID],
                                                    rank_ub_l[userID]);
                if (crank != -1) {
                    if (crank == global_crank) {
                        topk_max_heap.emplace_back(userID, crank);
                        std::push_heap(topk_max_heap.begin(), topk_max_heap.end(), UserRankMaxHeap);
                        count_max_heap_[crank]++;
                    } else { // crank < global_crank
                        int min_rank_count = count_max_heap_[global_crank];
                        if (topk_max_heap.size() - min_rank_count + 1 == topk) {
                            // the other greater rank is enough to fill the top-k, delete all the minimum rank
                            while (topk_max_heap.size() >= topk) {
                                int del_userID = topk_max_heap.front().first;
                                prune_l[del_userID] = true;
                                std::pop_heap(topk_max_heap.begin(), topk_max_heap.end(), UserRankMaxHeap);
                                topk_max_heap.pop_back();
                            }
                            topk_max_heap.emplace_back(userID, crank);
                            std::push_heap(topk_max_heap.begin(), topk_max_heap.end(), UserRankMaxHeap);
                            count_max_heap_[crank]++;
                            count_max_heap_[global_crank] = 0;
                        } else {
                            topk_max_heap.emplace_back(userID, crank);
                            std::push_heap(topk_max_heap.begin(), topk_max_heap.end(), UserRankMaxHeap);
                            count_max_heap_[crank]++;
                        }
                    }
                }
            }

            int n_candidate = (int) topk_max_heap.size();
            for (int candID = 0; candID < n_candidate; candID++) {
                int userID = topk_max_heap[candID].first;
                int crank = topk_max_heap[candID].second;
                int start_idx = crank == 0 ? 0 : known_rank_idx_l_[crank - 1] + 1;
                int end_idx = crank == n_cache_rank_ ? n_data_item_ : known_rank_idx_l_[crank];
                rank_lb_l[userID] = end_idx;
                rank_ub_l[userID] = start_idx;
            }
        }

    };
}
#endif //REVERSE_KRANKS_RANKSEARCH_HPP
