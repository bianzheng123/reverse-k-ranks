//
// Created by BianZheng on 2022/3/10.
//

#ifndef REVERSE_K_RANKS_PRUNECANDIDATEBYBOUND_HPP
#define REVERSE_K_RANKS_PRUNECANDIDATEBYBOUND_HPP

#include <cassert>
#include <vector>
#include <algorithm>
#include "struct/RankBoundElement.hpp"

namespace ReverseMIPS {
    void
    PruneCandidateByBound(const std::vector<int> &lb_l, const std::vector<int> &ub_l,
                          const int &n_user, const int &topk,
                          std::vector<bool> &prune_l, std::vector<int> &topk_lb_heap,
                          int &n_candidate) {
        assert(lb_l.size() == n_user);
        assert(ub_l.size() == n_user);
        assert(prune_l.size() == n_user);

        for (int userID = 0; userID < topk; userID++) {
            topk_lb_heap[userID] = lb_l[userID];
        }
        std::make_heap(topk_lb_heap.begin(), topk_lb_heap.end(), std::less());
        int global_lb = topk_lb_heap.front();

        int topk_1 = topk - 1;
        for (int userID = topk; userID < n_user; userID++) {
            int tmp_lb = lb_l[userID];
            if (global_lb > tmp_lb) {
                std::pop_heap(topk_lb_heap.begin(), topk_lb_heap.end(), std::less());
                topk_lb_heap[topk_1] = tmp_lb;
                std::push_heap(topk_lb_heap.begin(), topk_lb_heap.end(), std::less());
                global_lb = topk_lb_heap.front();
            }
        }

        for (int userID = 0; userID < n_user; userID++) {
            if(prune_l[userID]){
                continue;
            }
            int tmp_ub = ub_l[userID];
            if (global_lb < tmp_ub) {
                prune_l[userID] = true;
                n_candidate--;
            }
        }

    }

}
#endif //REVERSE_K_RANKS_PRUNECANDIDATEBYBOUND_HPP
