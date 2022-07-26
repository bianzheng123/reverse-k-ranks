//
// Created by BianZheng on 2022/3/10.
//

#ifndef REVERSE_K_RANKS_PRUNECANDIDATEBYBOUND_HPP
#define REVERSE_K_RANKS_PRUNECANDIDATEBYBOUND_HPP

#include "alg/TopkLBHeap.hpp"
#include "struct/RankBoundElement.hpp"

#include <cassert>
#include <vector>
#include <algorithm>

namespace ReverseMIPS {

    void
    PruneCandidateByBound(const std::vector<int> &rank_lb_l, const std::vector<int> &rank_ub_l,
                          const int &n_user,
                          std::vector<bool> &prune_l, TopkLBHeap &topkLbHeap) {
        assert(rank_lb_l.size() == n_user);
        assert(rank_ub_l.size() == n_user);
        assert(prune_l.size() == n_user);

        for (int userID = 0; userID < n_user; userID++) {
            assert(rank_ub_l[userID] <= rank_lb_l[userID]);
            if (prune_l[userID]) {
                continue;
            }
            topkLbHeap.Update(rank_lb_l[userID]);
        }
        const int min_topk_lb_rank = topkLbHeap.Front();
        for (int userID = 0; userID < n_user; userID++) {
            if (prune_l[userID]) {
                continue;
            }
            int tmp_ub = rank_ub_l[userID];
            if (min_topk_lb_rank < tmp_ub) {
                prune_l[userID] = true;
            }
        }

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
