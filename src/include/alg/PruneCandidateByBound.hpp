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

    void PruneCandidateByBound(std::vector<RankBoundElement> &rank_bound_l, const int &size, const int &topk,
                               int &n_remain, const char *can_prune_l, int queryID, bool test = false) {
        assert(rank_bound_l.size() >= size);
        assert(size >= topk);

        auto topk_iter = rank_bound_l.begin() + topk;
        // max heap for lower rank
        std::make_heap(rank_bound_l.begin(), topk_iter, RankBoundElement::LowerBoundMaxHeap);
        RankBoundElement topk_ele = rank_bound_l.front();
        n_remain = 0;

        for (int candID = topk; candID < size; candID++) {
            std::pair<int, int> rank_bound = rank_bound_l[candID].rank_pair();
            if (test && queryID == 987 && rank_bound_l[candID].userID_ == 609) {
                printf("in prune method rank_bound_l\n");
                std::cout << "\t" << "topk_ele " << topk_ele.ToString() << std::endl;
                std::cout << "\t" << "this_rank_bound_l " << rank_bound_l[candID].ToString() << std::endl;
            }
            if (test && queryID == 987 && topk_ele.userID_ == 609) {
                printf("in prune method topk_ele\n");
                std::cout << "\t" << "topk_ele " << topk_ele.ToString() << std::endl;
                std::cout << "\t" << "this_rank_bound_l " << rank_bound_l[candID].ToString() << std::endl;
            }

            assert(rank_bound.first >= rank_bound.second);
            if (can_prune_l[candID]) {
                continue;
            }
            if (topk_ele.upper_rank_ > rank_bound.first) {
                //lower rank greater than top-k
                //pop current topk heap, renew the topk heap
                std::pop_heap(rank_bound_l.begin(), topk_iter,
                              RankBoundElement::LowerBoundMaxHeap);
                std::swap(rank_bound_l[topk - 1], rank_bound_l[candID]);
                std::push_heap(rank_bound_l.begin(), topk_iter,
                               RankBoundElement::LowerBoundMaxHeap);
                topk_ele = rank_bound_l.front();
                //the popped element may be in remain_heap, push into remain_heap
                std::swap(rank_bound_l[candID], rank_bound_l[topk + n_remain]);
                n_remain++;
                std::push_heap(topk_iter, topk_iter + n_remain, RankBoundElement::UpperBoundMaxHeap);
                //pop invalid element in remain_heap
                if (n_remain > 0) {
                    RankBoundElement remain_ele = topk_iter[0];
                    while (n_remain > 0 and topk_ele.lower_rank_ < remain_ele.upper_rank_) {
                        std::pop_heap(topk_iter, topk_iter + n_remain,
                                      RankBoundElement::UpperBoundMaxHeap);
                        n_remain--;
                        if (n_remain > 0) {
                            remain_ele = topk_iter[0];
                        }
                    }
                }

            } else if (topk_ele.lower_rank_ >= rank_bound.second) {
                //have overlap with topk, add to remain_heap
                std::swap(rank_bound_l[candID], rank_bound_l[topk + n_remain]);
                n_remain++;
                std::push_heap(topk_iter, topk_iter + n_remain, RankBoundElement::UpperBoundMaxHeap);
            }
        }
    }

}
#endif //REVERSE_K_RANKS_PRUNECANDIDATEBYBOUND_HPP
