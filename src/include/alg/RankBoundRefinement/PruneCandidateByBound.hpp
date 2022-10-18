//
// Created by BianZheng on 2022/3/10.
//

#ifndef REVERSE_K_RANKS_PRUNECANDIDATEBYBOUND_HPP
#define REVERSE_K_RANKS_PRUNECANDIDATEBYBOUND_HPP

#include "alg/TopkMaxHeap.hpp"
#include "struct/RankBoundElement.hpp"

#include <cassert>
#include <vector>
#include <algorithm>

namespace ReverseMIPS {

    void
    PruneCandidateByBound(const std::vector<int> &rank_lb_l, const std::vector<int> &rank_ub_l,
                          const int &n_user, const int &topk,
                          std::vector<int> &refine_seq_l, int &refine_user_size,
                          int &n_result_user, int &n_prune_user,
                          std::vector<bool> &prune_l, std::vector<bool> &result_l) {

        assert(rank_lb_l.size() == n_user);
        assert(rank_ub_l.size() == n_user);
        assert(refine_seq_l.size() == n_user);
        assert(prune_l.size() == n_user);
        assert(result_l.size() == n_user);
        assert(n_result_user <= topk);
        assert(topk - n_result_user <= refine_user_size);

        TopkMaxHeap lbr_heap(topk - n_result_user);
        TopkMaxHeap ubr_heap(topk - n_result_user);
        lbr_heap.Reset();
        ubr_heap.Reset();

        for (int userID = 0; userID < n_user; userID++) {
            assert(rank_ub_l[userID] <= rank_lb_l[userID]);
            assert(!(prune_l[userID] == true && result_l[userID] == true));
            if (prune_l[userID] || result_l[userID]) {
                continue;
            }
            lbr_heap.Update(rank_lb_l[userID]);
            ubr_heap.Update(rank_ub_l[userID]);
        }
        const int min_topk_lb_rank = lbr_heap.Front();
        const int min_topk_ub_rank = ubr_heap.Front();
        if (min_topk_lb_rank == -1 && min_topk_ub_rank == -1) {
            return;
        }
        assert(min_topk_lb_rank != -1 && min_topk_ub_rank != -1);
        refine_user_size = 0;
        for (int userID = 0; userID < n_user; userID++) {
            if (prune_l[userID] || result_l[userID]) {
                assert(!(prune_l[userID] && result_l[userID]));
                continue;
            }

            assert(!prune_l[userID] && !result_l[userID]);
            const int &tmp_lb = rank_lb_l[userID];
            if (tmp_lb <= min_topk_ub_rank) {
                result_l[userID] = true;
                n_result_user++;
            }
            if (n_result_user == topk) {
                for (int tmp_userID = userID + 1; tmp_userID < n_user; tmp_userID++) {
                    if (prune_l[tmp_userID] || result_l[tmp_userID]) {
                        continue;
                    }else{
                        prune_l[tmp_userID] = true;
                        n_prune_user++;
                    }
                }
                break;
            }

            const int &tmp_ub = rank_ub_l[userID];
            if (min_topk_lb_rank <= tmp_ub && min_topk_lb_rank != min_topk_ub_rank) {
                prune_l[userID] = true;
                n_prune_user++;
            }
            assert((prune_l[userID] ^ result_l[userID]) || (!prune_l[userID] && !result_l[userID]));

            if (!prune_l[userID] && !result_l[userID]) {
                refine_seq_l[refine_user_size] = userID;
                refine_user_size++;
            }
        }

        std::sort(refine_seq_l.begin(), refine_seq_l.begin() + refine_user_size,
                  [&rank_lb_l, &rank_ub_l](const int i1, const int i2) {
                      if (rank_lb_l[i1] == rank_lb_l[i2]) {
                          return rank_ub_l[i1] < rank_ub_l[i2];
                      } else {
                          return rank_lb_l[i1] < rank_lb_l[i2];
                      }
                  });
        assert(refine_user_size <= n_user);

    }

    void
    PruneCandidateByBound(const std::vector<int> &rank_l,
                          const int &n_user, const int &topk,
                          std::vector<bool> &prune_l) {
        assert(rank_l.size() == n_user);
        assert(prune_l.size() == n_user);
        std::vector<int> topk_heap_l(topk);

        int n_candidate = 0;
        int userID = 0;
        while (n_candidate < topk) {
            if (prune_l[userID]) {
                userID++;
                continue;
            }
            topk_heap_l[n_candidate] = rank_l[userID];
            n_candidate++;
            userID++;
        }

        std::make_heap(topk_heap_l.begin(), topk_heap_l.end(), std::less());
        int global_lb = topk_heap_l.front();

        int topk_1 = topk - 1;
        for (; userID < n_user; userID++) {
            if (prune_l[userID]) {
                continue;
            }
            int tmp_lb = rank_l[userID];
            if (global_lb > tmp_lb) {
                std::pop_heap(topk_heap_l.begin(), topk_heap_l.end(), std::less());
                topk_heap_l[topk_1] = tmp_lb;
                std::push_heap(topk_heap_l.begin(), topk_heap_l.end(), std::less());
                global_lb = topk_heap_l.front();
            }
        }

        for (userID = 0; userID < n_user; userID++) {
            if (prune_l[userID]) {
                continue;
            }
            int tmp_ub = rank_l[userID];
            if (global_lb < tmp_ub) {
                prune_l[userID] = true;
            }
        }

    }

}
#endif //REVERSE_K_RANKS_PRUNECANDIDATEBYBOUND_HPP
