#pragma once
namespace ReverseMIPS {

    class UserRankElement {

    public:
        int userID_;
        int rank_;

        inline UserRankElement() {
            this->userID_ = 0;
            this->rank_ = 0;
        }

        inline UserRankElement(int userID, int rank) {
            this->userID_ = userID;
            this->rank_ = rank;
        }

        inline ~UserRankElement() = default;

        inline bool operator==(const UserRankElement &other) const {
            if (this == &other)
                return true;
            return rank_ == other.rank_ && userID_ == other.userID_;
        };

        inline bool operator!=(const UserRankElement &other) const {
            if (this == &other)
                return false;
            return rank_ != other.rank_ || userID_ != other.userID_;
        };

        inline bool operator<(const UserRankElement &other) const {
            if (rank_ != other.rank_) {
                return rank_ < other.rank_;
            } else {
                return userID_ < other.userID_;
            }
        }

        inline bool operator<=(const UserRankElement &other) const {
            return rank_ <= other.rank_;
        }

        inline bool operator>(const UserRankElement &other) const {
            if (rank_ != other.rank_) {
                return rank_ > other.rank_;
            } else {
                return userID_ > other.userID_;
            }
        }

        inline bool operator>=(const UserRankElement &other) const {
            return rank_ >= other.rank_;
        }

    };
}