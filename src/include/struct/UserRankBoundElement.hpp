//
// Created by BianZheng on 2022/3/3.
//

#ifndef REVERSE_K_RANKS_USERRANKBOUNDELEMENT_HPP
#define REVERSE_K_RANKS_USERRANKBOUNDELEMENT_HPP

#include <string>

namespace ReverseMIPS {
    class UserRankBoundElement {
    public:
        int userID_, upper_rank_, lower_rank_;
        double queryIP_;

        //by default, value of upper rank is smaller than lower rank
        UserRankBoundElement(int userID, double queryIP, std::pair<int, int> bound_pair) {
            this->userID_ = userID;
            this->queryIP_ = queryIP;
            this->lower_rank_ = bound_pair.first;
            this->upper_rank_ = bound_pair.second;
        }

        UserRankBoundElement() {
            userID_ = -1;
            upper_rank_ = -1;
            lower_rank_ = -1;
            queryIP_ = -1;
        }

        ~UserRankBoundElement() = default;

        std::string ToString() {
            char arr[256];
            sprintf(arr, "userID %d, queryIP %.3f, lower_rank %d, upper_rank %d", userID_, queryIP_, lower_rank_, upper_rank_);
            std::string str(arr);
            return str;
        }

        inline bool operator==(const UserRankBoundElement &other) const {
            if (this == &other)
                return true;
            return upper_rank_ == other.upper_rank_ && lower_rank_ == other.lower_rank_ && userID_ == other.userID_ &&
                   queryIP_ == other.queryIP_;
        };

        inline bool operator!=(const UserRankBoundElement &other) const {
            if (this == &other)
                return false;
            return upper_rank_ != other.upper_rank_ || lower_rank_ != other.lower_rank_ || userID_ != other.userID_ ||
                   queryIP_ != other.queryIP_;
        };

        inline bool operator<(const UserRankBoundElement &other) const {
            return lower_rank_ < other.lower_rank_;
        }

        inline bool operator<=(const UserRankBoundElement &other) const {
            return lower_rank_ <= other.lower_rank_;
        }

        inline bool operator>(const UserRankBoundElement &other) const {
            return lower_rank_ > other.lower_rank_;
        }

        inline bool operator>=(const UserRankBoundElement &other) const {
            return lower_rank_ >= other.lower_rank_;
        }
    };
}

#endif //REVERSE_K_RANKS_USERRANKBOUNDELEMENT_HPP
