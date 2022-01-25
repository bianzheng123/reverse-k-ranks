//
// Created by BianZheng on 2022/1/25.
//

#ifndef REVERSE_KRANKS_BUCKETVECTOR_HPP
#define REVERSE_KRANKS_BUCKETVECTOR_HPP

#include <vector>
#include <set>
#include <cassert>
#include <algorithm>
#include <cfloat>
#include <cmath>

namespace ReverseMIPS {

    class BucketVector {

    public:
        std::vector<std::set<int>> tmp_set_vector_;
        std::vector<std::vector<int>> set_vector_;
        int n_bucket_;
        double lower_bound_inner_product_, upper_bound_inner_product_;
        double bucket_width_;

        inline BucketVector() = default;

        inline BucketVector(const int dimension, const double lb_ip, const double ub_ip) {
            this->n_bucket_ = dimension;
            this->lower_bound_inner_product_ = lb_ip - 0.01;
            this->upper_bound_inner_product_ = ub_ip + 0.01;
            this->set_vector_.resize(dimension);
            this->tmp_set_vector_.resize(dimension);
            this->bucket_width_ = (upper_bound_inner_product_ - lower_bound_inner_product_) / n_bucket_;
        }

        inline ~BucketVector() = default;

        [[nodiscard]] std::vector<int> getBucketByIP(double inner_product) const {
            int index = std::floor((upper_bound_inner_product_ - inner_product) / bucket_width_);
            if (index < 0 || index >= n_bucket_) {
                return std::vector<int>{};
            }
            return set_vector_[index];
        }

        [[nodiscard]] int getBucketIndexByIP(double inner_product) const {
            int index = std::floor((upper_bound_inner_product_ - inner_product) / bucket_width_);
            if (index < 0) {
                return 0;
            }
            if (index >= n_bucket_) {
                return n_bucket_;
            }
            return index;
        }

        [[nodiscard]] double getUpperBoundByIP(double inner_product) const {
            int index = std::floor((upper_bound_inner_product_ - inner_product) / bucket_width_);
            if (index < 0) {
                return DBL_MAX;
            } else if (index >= n_bucket_) {
                return lower_bound_inner_product_;
            } else {
                return upper_bound_inner_product_ - bucket_width_ * index;
            }
        }

        int addUniqueElement(double inner_product, int itemID) {
            int idx = std::floor((upper_bound_inner_product_ - inner_product) / bucket_width_);
            if (tmp_set_vector_[idx].find(itemID) == tmp_set_vector_[idx].end()) {
                tmp_set_vector_[idx].insert(itemID);
            }
            return idx;
        }

        void stopAddUniqueElement() {
            for (int i = 0; i < n_bucket_; i++) {
                std::set<int> &tmp_itemID_set = tmp_set_vector_[i];
                std::vector<int> &tmp_itemID_arr = set_vector_[i];
                for (auto &itemID: tmp_itemID_set) {
                    tmp_itemID_arr.emplace_back(itemID);
                }

                std::sort(tmp_itemID_arr.begin(), tmp_itemID_arr.end(), std::greater<int>());
                tmp_itemID_set.clear();
            }

        }


    };
}
#endif //REVERSE_KRANKS_BUCKETVECTOR_HPP
