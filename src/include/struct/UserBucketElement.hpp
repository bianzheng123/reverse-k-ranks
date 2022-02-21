//
// Created by BianZheng on 2022/2/20.
//

#ifndef REVERSE_KRANKS_USERBUCKETELEMENT_HPP
#define REVERSE_KRANKS_USERBUCKETELEMENT_HPP

#include <string>

namespace ReverseMIPS {
    class UserBucketElement {
    public:
        int userID_, bucketID_;
        double queryIP_;

        UserBucketElement(int userID, int bucketID, double queryIP) {
            this->userID_ = userID;
            this->bucketID_ = bucketID;
            this->queryIP_ = queryIP;
        }

        UserBucketElement() {
            userID_ = -1;
            bucketID_ = -1;
            queryIP_ = 0;
        }

        ~UserBucketElement() = default;

        std::string ToString() {
            char arr[256];
            sprintf(arr, "userId %d, bucketID %d, queryIP %.3f", userID_, bucketID_, queryIP_);
            std::string str(arr);
            return str;
        }

        inline bool operator==(const UserBucketElement &other) const {
            if (this == &other)
                return true;
            return bucketID_ == other.bucketID_ && userID_ == other.userID_;
        };

        inline bool operator!=(const UserBucketElement &other) const {
            if (this == &other)
                return false;
            return bucketID_ != other.bucketID_ || userID_ != other.userID_;
        };

        inline bool operator<(const UserBucketElement &other) const {
            if (bucketID_ != other.bucketID_) {
                return bucketID_ < other.bucketID_;
            }
            return userID_ < other.userID_;
        }

        inline bool operator<=(const UserBucketElement &other) const {
            if (bucketID_ != other.bucketID_) {
                return bucketID_ <= other.bucketID_;
            }
            return userID_ <= other.userID_;

        }

        inline bool operator>(const UserBucketElement &other) const {
            if (bucketID_ != other.bucketID_) {
                return bucketID_ > other.bucketID_;
            }
            return userID_ > other.userID_;
        }

        inline bool operator>=(const UserBucketElement &other) const {
            if (bucketID_ != other.bucketID_) {
                return bucketID_ >= other.bucketID_;
            }
            return userID_ >= other.userID_;
        }
    };
}

#endif //REVERSE_KRANKS_USERBUCKETELEMENT_HPP
