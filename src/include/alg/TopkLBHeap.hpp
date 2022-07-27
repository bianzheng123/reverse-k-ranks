//
// Created by BianZheng on 2022/7/26.
//

#ifndef REVERSE_K_RANKS_TOPKLBHEAP_HPP
#define REVERSE_K_RANKS_TOPKLBHEAP_HPP

#include <vector>
#include <cassert>
#include <algorithm>

namespace ReverseMIPS {
    class TopkLBHeap {
        int topk_, cur_size_, heap_lb_rank_;
        std::vector<int> topk_lb_heap_;
    public:
        TopkLBHeap() = default;

        TopkLBHeap(const int &topk) {
            this->topk_ = topk;
            this->cur_size_ = 0;
            this->heap_lb_rank_ = -1;
            this->topk_lb_heap_.resize(topk);
        }

        void Update(const int new_lb_rank) {
            if (cur_size_ < topk_) {
                topk_lb_heap_[cur_size_] = new_lb_rank;
                cur_size_++;
                if (cur_size_ == topk_) {
                    std::make_heap(topk_lb_heap_.begin(), topk_lb_heap_.end(), std::less());
                    heap_lb_rank_ = topk_lb_heap_.front();
                }
            } else {
                assert(cur_size_ == topk_);
                if (heap_lb_rank_ > new_lb_rank) {
                    std::pop_heap(topk_lb_heap_.begin(), topk_lb_heap_.end(), std::less());
                    topk_lb_heap_[topk_ - 1] = new_lb_rank;
                    std::push_heap(topk_lb_heap_.begin(), topk_lb_heap_.end(), std::less());
                    heap_lb_rank_ = topk_lb_heap_.front();
                    assert(topk_lb_heap_[0] == topk_lb_heap_.front());
                }
            }
            assert(cur_size_ <= topk_);

        }

        int Front() {
            if (cur_size_ < topk_) {
                return -1;
            } else {
                assert(cur_size_ == topk_);
                return this->heap_lb_rank_;
            }
        }

        void Reset() {
            this->cur_size_ = 0;
            this->heap_lb_rank_ = -1;
        }
    };
}
#endif //REVERSE_K_RANKS_TOPKLBHEAP_HPP
