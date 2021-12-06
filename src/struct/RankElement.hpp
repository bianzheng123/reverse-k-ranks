#pragma once
namespace ReverseMIPS {

    class RankElement {

    public:
        int index;
        int rank;

        inline RankElement(){
            this->index = 0;
            this->rank = 0;
        }

        inline RankElement(int _index, int _rank) {
            this->index = _index;
            this->rank = _rank;
        }

        inline ~RankElement() {}

        inline bool operator==(const RankElement &other) const {
            if (this == &other)
                return true;
            return rank == other.rank && index == other.index;
        };

        inline bool operator!=(const RankElement &other) const {
            if (this == &other)
                return false;
            return rank != other.rank || index != other.index;
        };

        inline bool operator<(const RankElement &other) const {
            return rank < other.rank;
        }

        inline bool operator<=(const RankElement &other) const {
            return rank <= other.rank;
        }

        inline bool operator>(const RankElement &other) const {
            return rank > other.rank;
        }

        inline bool operator>=(const RankElement &other) const {
            return rank >= other.rank;
        }

    };
}