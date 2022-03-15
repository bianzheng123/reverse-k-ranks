//
// Created by BianZheng on 2022/3/15.
//

#ifndef REVERSE_KRANKS_PARTDIMPARTINTPRUNE_HPP
#define REVERSE_KRANKS_PARTDIMPARTINTPRUNE_HPP


#include <cassert>
#include <memory>
#include <vector>
#include "alg/SpaceInnerProduct.hpp"
#include "struct/VectorMatrix.hpp"
#include "struct/RankBoundElement.hpp"

namespace ReverseMIPS {

    class PartDimPartIntPrune {
        int n_user_, vec_dim_, check_dim_, remain_dim_;
        std::unique_ptr<std::pair<double, double>[]> ip_bound_l_;
        //part int
        std::unique_ptr<int[]> user_int_ptr_;
        std::unique_ptr<int[]> user_int_sum_ptr_;
        double user_convert_coe_;
        double integer_scale_;
        std::unique_ptr<int[]> query_int_ptr_;
    public:
        PartDimPartIntPrune() = default;

        //make bound from offset_dim to vec_dim
        void Preprocess(const VectorMatrix &user, const int &check_dim, const double &integer_scale) {

            this->n_user_ = user.n_vector_;
            this->vec_dim_ = user.vec_dim_;
            this->check_dim_ = check_dim;
            this->integer_scale_ = integer_scale;
            this->remain_dim_ = vec_dim_ - check_dim_;

            user_int_ptr_ = std::make_unique<int[]>(n_user_ * remain_dim_);
            user_int_sum_ptr_ = std::make_unique<int[]>(n_user_);
            ip_bound_l_ = std::make_unique<std::pair<double, double>[]>(n_user_);
            query_int_ptr_ = std::make_unique<int[]>(remain_dim_);
            double user_max_part_dim = user.getVector(0)[0];

            //compute the integer bound for the first part
            for (int userID = 0; userID < n_user_; userID++) {
                double *user_vecs = user.getVector(userID);
                for (int dim = check_dim_; dim < vec_dim_; dim++) {
                    user_max_part_dim = std::max(user_max_part_dim, user_vecs[dim]);
                }
            }
            for (int userID = 0; userID < n_user_; userID++) {
                double *user_vecs = user.getVector(userID);
                int *user_int_vecs = user_int_ptr_.get() + userID * remain_dim_;

                user_int_sum_ptr_[userID] = 0;
                for (int dim = check_dim_; dim < vec_dim_; dim++) {
                    user_int_vecs[dim - check_dim_] = std::floor(user_vecs[dim] * integer_scale / user_max_part_dim);
                    user_int_sum_ptr_[userID] += std::abs(user_int_vecs[dim - check_dim_]) + 1;
                }
            }
            user_convert_coe_ = user_max_part_dim / (integer_scale_ * integer_scale_);
        }

        void
        QueryBound(const double *query_vecs, const VectorMatrix &user, const int &n_candidate,
                   std::vector<RankBoundElement> &candidate_l, bool assign) {

            double query_max_part_dim = query_vecs[0];
            const double *query_offset_vecs = query_vecs + check_dim_;

            for (int dim = 1; dim < remain_dim_; dim++) {
                query_max_part_dim = std::max(query_max_part_dim, query_offset_vecs[dim]);
            }

            double qratio = integer_scale_ / query_max_part_dim;
            int query_int_sum = 0;
            for (int dim = 0; dim < remain_dim_; dim++) {
                query_int_ptr_[dim] = std::floor(query_offset_vecs[dim] * qratio);
                query_int_sum += std::abs(query_int_ptr_[dim]);
            }
            double convert_coe = user_convert_coe_ * query_max_part_dim;

            for (int candID = 0; candID < n_candidate; candID++) {
                int userID = candidate_l[candID].userID_;
                int *user_int_vecs = user_int_ptr_.get() + userID * remain_dim_;
                int *query_int_vecs = query_int_ptr_.get();
                int intIP = InnerProduct(user_int_vecs, query_int_vecs, remain_dim_);
                int int_otherIP = user_int_sum_ptr_[userID] + query_int_sum;

                int lb_int_part = intIP - int_otherIP;
                int ub_int_part = intIP + int_otherIP;
                double lb_part = convert_coe * lb_int_part;
                double ub_part = convert_coe * ub_int_part;

                double *user_vecs = user.getVector(userID);
                double ip = InnerProduct(user_vecs, query_vecs, check_dim_);

                double lb = ip + lb_part;
                double ub = ip + ub_part;
                ip_bound_l_[candID].first = lb;
                ip_bound_l_[candID].second = ub;
                assert(lb <= ub);
            }

            if (assign) {
                for (int candID = 0; candID < n_candidate; candID++) {
                    candidate_l[candID].lower_bound_ = ip_bound_l_[candID].first;
                    candidate_l[candID].upper_bound_ = ip_bound_l_[candID].second;
                }
            } else {
                for (int candID = 0; candID < n_candidate; candID++) {
                    candidate_l[candID].lower_bound_ = std::max(ip_bound_l_[candID].first,
                                                                candidate_l[candID].lower_bound_);
                    candidate_l[candID].upper_bound_ = std::min(ip_bound_l_[candID].second,
                                                                candidate_l[candID].upper_bound_);
                }
            }

        }

    };

}
#endif //REVERSE_KRANKS_PARTDIMPARTINTPRUNE_HPP
