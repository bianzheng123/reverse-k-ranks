#pragma once

#include <string>

namespace ReverseMIPS {
    class DistancePair {
    public:
        double dist_;
        int ID_;

        DistancePair(double dist, int ID) {
            this->dist_ = dist;
            this->ID_ = ID;
        }

        DistancePair() {
            dist_ = 0;
            ID_ = 0;
        }

        ~DistancePair() {}

        std::string ToString() {
            char arr[256];
            sprintf(arr, "%.3f %d", dist_, ID_);
            std::string str(arr);
            return str;
        }

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