#pragma once
namespace ReverseMIPS {

    class RankElement {

    public:
        int index_;
        int rank_;

        inline RankElement() {
            this->index_ = 0;
            this->rank_ = 0;
        }

        inline RankElement(int index, int rank) {
            this->index_ = index;
            this->rank_ = rank;
        }

        inline ~RankElement() {}

        inline bool operator==(const RankElement &other) const {
            if (this == &other)
                return true;
            return rank_ == other.rank_ && index_ == other.index_;
        };

        inline bool operator!=(const RankElement &other) const {
            if (this == &other)
                return false;
            return rank_ != other.rank_ || index_ != other.index_;
        };

        inline bool operator<(const RankElement &other) const {
            if (rank_ != other.rank_) {
                return rank_ < other.rank_;
            } else {
                return index_ < other.index_;
            }
        }

        inline bool operator<=(const RankElement &other) const {
            return rank_ <= other.rank_;
        }

        inline bool operator>(const RankElement &other) const {
            if (rank_ != other.rank_) {
                return rank_ > other.rank_;
            } else {
                return index_ > other.index_;
            }
        }

        inline bool operator>=(const RankElement &other) const {
            return rank_ >= other.rank_;
        }

    };
}