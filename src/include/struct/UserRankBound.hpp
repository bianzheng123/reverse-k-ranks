//
// Created by BianZheng on 2022/4/19.
//

#ifndef REVERSE_KRANKS_USERRANKBOUND_HPP
#define REVERSE_KRANKS_USERRANKBOUND_HPP
#include <cassert>

class UserRankBound {
public:
    int rank_lb_, rank_ub_;

    inline UserRankBound() {
        rank_lb_ = -1;
        rank_ub_ = -1;
    }

    inline UserRankBound(const int &rank_lb, const int rank_ub) {
        this->rank_lb_ = rank_lb;
        this->rank_ub_ = rank_ub;
    }

    void Merge(int rank) {
        assert(rank_lb_ != -1 && rank_ub_ == -1);
        assert(rank_lb_ == -1 && rank_ub_ != -1);
        assert(rank_ub_ <= rank_lb_);
        if (rank_lb_ == -1 && rank_ub_ == -1) {
            rank_lb_ = rank;
            rank_ub_ = rank;
        } else if (rank_lb_ != -1 && rank_ub_ != -1) {
            if (rank < rank_ub_) {
                rank_ub_ = rank;
            } else if (rank > rank_lb_) {
                rank_lb_ = rank;
            }
        }
    }

    inline void Reset() {
        this->rank_lb_ = -1;
        this->rank_ub_ = -1;
    }
};

#endif //REVERSE_KRANKS_USERRANKBOUND_HPP
