#pragma once
namespace ReverseMIPS {
    class DistancePair {
    public:
        float dist_;
        int ID_;

        DistancePair(float dist, int ID) {
            this->dist_ = dist;
            this->ID_ = ID;
        }

        DistancePair() {
            dist_ = 0;
            ID_ = 0;
        }

        ~DistancePair() {}

        inline bool operator==(const DistancePair &other) const {
            if (this == &other)
                return true;
            return dist_ == other.dist_ && ID_ == other.ID_;
        };

        inline bool operator!=(const DistancePair &other) const {
            if (this == &other)
                return false;
            return dist_ != other.dist_ || ID_ != other.ID_;
        };

        inline bool operator<(const DistancePair &other) const {
            return dist_ < other.dist_;
        }

        inline bool operator<=(const DistancePair &other) const {
            return dist_ <= other.dist_;
        }

        inline bool operator>(const DistancePair &other) const {
            return dist_ > other.dist_;
        }

        inline bool operator>=(const DistancePair &other) const {
            return dist_ >= other.dist_;
        }
    };
}