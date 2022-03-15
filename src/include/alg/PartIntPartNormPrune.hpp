//
// Created by BianZheng on 2022/3/15.
//

#ifndef REVERSE_KRANKS_PARTINTPARTNORMPRUNE_HPP
#define REVERSE_KRANKS_PARTINTPARTNORMPRUNE_HPP


#include <cassert>
#include <memory>
#include <vector>
#include "alg/SpaceInnerProduct.hpp"
#include "struct/VectorMatrix.hpp"
#include "struct/RankBoundElement.hpp"

namespace ReverseMIPS {

    class PartIntPartNormPrune {
        int n_user_, vec_dim_, check_dim_, remain_dim_;
        std::unique_ptr<std::pair<double, double>[]> ip_bound_l_;
        //part int
        std::unique_ptr<int[]> user_int_ptr_;
        std::unique_ptr<int[]> user_int_sum_ptr_;
        double user_convert_coe_;
        double integer_scale_;
        //part norm
        std::unique_ptr<double[]> user_norm_ptr_;
        std::unique_ptr<int[]> query_int_ptr;
    public:
        PartIntPartNormPrune() = default;

        //make bound from offset_dim to vec_dim
        void Preprocess(const VectorMatrix &user, const int &check_dim, const double &integer_scale) {

            this->n_user_ = user.n_vector_;
            this->vec_dim_ = user.vec_dim_;
            this->check_dim_ = check_dim;
            this->integer_scale_ = integer_scale;
            this->remain_dim_ = vec_dim_ - check_dim;

            user_int_ptr_ = std::make_unique<int[]>(n_user_ * check_dim);
            user_int_sum_ptr_ = std::make_unique<int[]>(n_user_);
            query_int_ptr = std::make_unique<int[]>(check_dim_);
            ip_bound_l_ = std::make_unique<std::pair<double, double>[]>(n_user_);
            double user_max_part_dim = user.getVector(0)[0];

            //compute the integer bound for the first part
            for (int userID = 0; userID < n_user_; userID++) {
                double *user_vecs = user.getVector(userID);
                for (int dim = 0; dim < check_dim; dim++) {
                    user_max_part_dim = std::max(user_max_part_dim, user_vecs[dim]);
                }
            }
            for (int userID = 0; userID < n_user_; userID++) {
                double *user_vecs = user.getVector(userID);
                int *user_int_vecs = user_int_ptr_.get() + userID * check_dim;

                user_int_sum_ptr_[userID] = 0;
                for (int dim = 0; dim < check_dim; dim++) {
                    user_int_vecs[dim] = std::floor(user_vecs[dim] * integer_scale / user_max_part_dim);
                    user_int_sum_ptr_[userID] += std::abs(user_int_vecs[dim]) + 1;
                }
            }
            user_convert_coe_ = user_max_part_dim / (integer_scale_ * integer_scale_);

            this->user_norm_ptr_ = std::make_unique<double[]>(n_user_);

            for (int userID = 0; userID < n_user_; userID++) {
                double norm = InnerProduct(user.getVector(userID, check_dim),
                                           user.getVector(userID, check_dim),
                                           remain_dim_);
                this->user_norm_ptr_[userID] = std::sqrt(norm);

            }
        }

        void
        QueryBound(const double *query_vecs, const VectorMatrix &user, const int &n_candidate,
                   std::vector<RankBoundElement> &candidate_l, bool assign) {

            double query_max_part_dim = query_vecs[0];

            for (int dim = 1; dim < check_dim_; dim++) {
                query_max_part_dim = std::max(query_max_part_dim, query_vecs[dim]);
            }

            double qratio = integer_scale_ / query_max_part_dim;
            int query_int_sum = 0;
            for (int dim = 0; dim < check_dim_; dim++) {
                query_int_ptr[dim] = std::floor(query_vecs[dim] * qratio);
                query_int_sum += std::abs(query_int_ptr[dim]);
            }
            double convert_coe = user_convert_coe_ * query_max_part_dim;

            const double *query_norm_vecs = query_vecs + check_dim_;
            double part_query_norm = InnerProduct(query_norm_vecs, query_norm_vecs, remain_dim_);
            part_query_norm = std::sqrt(part_query_norm);

            for (int candID = 0; candID < n_candidate; candID++) {
                int userID = candidate_l[candID].userID_;
                int *user_int_vecs = user_int_ptr_.get() + userID * check_dim_;
                int *query_int_vecs = query_int_ptr.get();
                int intIP = InnerProduct(user_int_vecs, query_int_vecs, check_dim_);
                int int_otherIP = user_int_sum_ptr_[userID] + query_int_sum;

                int lb_int_part = intIP - int_otherIP;
                int ub_int_part = intIP + int_otherIP;
                double lb_part = convert_coe * lb_int_part;
                double ub_part = convert_coe * ub_int_part;

                double norm_times_res = part_query_norm * user_norm_ptr_[userID];
                double lb = -norm_times_res + lb_part;
                double ub = norm_times_res + ub_part;
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
#endif //REVERSE_KRANKS_PARTINTPARTNORMPRUNE_HPP
