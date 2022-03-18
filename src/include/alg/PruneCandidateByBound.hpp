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

    void PruneCandidateByBound(const std::vector<std::pair<int, int>> &rank_bound_l, const int &n_user, const int &topk,
                               std::vector<bool> &prune_l, int &n_candidate, int queryID) {
        assert(n_candidate >= topk);
        assert(n_user == rank_bound_l.size());
        assert(n_user == prune_l.size());
        if (n_candidate == topk) {
            return;
        }

        std::vector<int> lb_max_heap;
        lb_max_heap.reserve(topk);
        int count = 0;
        int userID = 0;
        while (count < topk) {
            if (prune_l[userID]) {
                userID++;
                continue;
            }
            lb_max_heap.emplace_back(rank_bound_l[userID].first);
            userID++;
            count++;
        }
        assert(count == topk);
        assert(userID >= count);

        std::make_heap(lb_max_heap.begin(), lb_max_heap.end(), std::less());
        int top_lb = lb_max_heap.front();
        int back_pos = topk - 1;

        while (userID < n_user) {
            if (prune_l[userID]) {
                userID++;
                continue;
            }
            int tmp_lb = rank_bound_l[userID].first;
            int tmp_ub = rank_bound_l[userID].second;
            if (top_lb > tmp_lb) {
                std::pop_heap(lb_max_heap.begin(), lb_max_heap.end(), std::less());
                lb_max_heap[back_pos] = tmp_lb;
                std::push_heap(lb_max_heap.begin(), lb_max_heap.end(), std::less());
                top_lb = lb_max_heap.front();
            } else if (top_lb < tmp_ub) {
                prune_l[userID] = true;
                n_candidate--;
            }
            userID++;
        }

        top_lb = lb_max_heap.front();
        for (userID = 0; userID < n_user; userID++) {
            if (prune_l[userID]) {
                continue;
            }
            int tmp_ub = rank_bound_l[userID].second;
            if (top_lb < tmp_ub) {
                prune_l[userID] = true;
                n_candidate--;
            }
        }

    }


//    void PruneCandidateByBound(std::vector<RankBoundElement> &rank_bound_l, const int &size, const int &topk,
//                               int &n_remain, const char *can_prune_l) {
//        assert(rank_bound_l.size() >= size);
//        assert(size >= topk);
//
//        auto topk_iter = rank_bound_l.begin() + topk;
//        // max heap for lower rank
//        std::make_heap(rank_bound_l.begin(), topk_iter, RankBoundElement::LowerBoundMaxHeap);
//        RankBoundElement topk_ele = rank_bound_l.front();
//        n_remain = 0;
//
//        for (int candID = topk; candID < size; candID++) {
//            std::pair<int, int> rank_bound = rank_bound_l[candID].rank_pair();
//
//            assert(rank_bound.first >= rank_bound.second);
//            if (can_prune_l[candID]) {
//                continue;
//            }
//            if (topk_ele.upper_rank_ > rank_bound.first) {
//                //lower rank greater than top-k
//                //pop current topk heap, renew the topk heap
//                std::pop_heap(rank_bound_l.begin(), topk_iter,
//                              RankBoundElement::LowerBoundMaxHeap);
//                std::swap(rank_bound_l[topk - 1], rank_bound_l[candID]);
//                std::push_heap(rank_bound_l.begin(), topk_iter,
//                               RankBoundElement::LowerBoundMaxHeap);
//                topk_ele = rank_bound_l.front();
//                //the popped element may be in remain_heap, push into remain_heap
//                std::swap(rank_bound_l[candID], rank_bound_l[topk + n_remain]);
//                n_remain++;
//                std::push_heap(topk_iter, topk_iter + n_remain, RankBoundElement::UpperBoundMaxHeap);
//                //pop invalid element in remain_heap
//                if (n_remain > 0) {
//                    RankBoundElement remain_ele = topk_iter[0];
//                    while (n_remain > 0 and topk_ele.lower_rank_ < remain_ele.upper_rank_) {
//                        std::pop_heap(topk_iter, topk_iter + n_remain,
//                                      RankBoundElement::UpperBoundMaxHeap);
//                        n_remain--;
//                        if (n_remain > 0) {
//                            remain_ele = topk_iter[0];
//                        }
//                    }
//                }
//
//            } else if (topk_ele.lower_rank_ >= rank_bound.second) {
//                //have overlap with topk, add to remain_heap
//                std::swap(rank_bound_l[candID], rank_bound_l[topk + n_remain]);
//                n_remain++;
//                std::push_heap(topk_iter, topk_iter + n_remain, RankBoundElement::UpperBoundMaxHeap);
//            }
//        }
//    }

}
#endif //REVERSE_K_RANKS_PRUNECANDIDATEBYBOUND_HPP
