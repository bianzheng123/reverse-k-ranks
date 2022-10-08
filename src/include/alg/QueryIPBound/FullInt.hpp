//
// Created by BianZheng on 2022/3/17.
//

#ifndef REVERSE_KRANKS_FULLINT_HPP
#define REVERSE_KRANKS_FULLINT_HPP

#include <cassert>
#include <memory>
#include <vector>

#include "alg/QueryIPBound/BaseQueryIPBound.hpp"
#include "alg/SpaceInnerProduct.hpp"
#include "struct/VectorMatrix.hpp"
#include "struct/RankBoundElement.hpp"

namespace ReverseMIPS {

    class FullInt : public BaseQueryIPBound {
        int n_user_, vec_dim_;
        double scale_;
        std::unique_ptr<int[]> user_int_ptr_;
        std::unique_ptr<int[]> user_int_sum_ptr_;

        std::unique_ptr<int[]> query_int_ptr_;
        double user_convert_coe_;

        double convert_coe_;
    public:
        FullInt() = default;

        FullInt(const int &n_user, const int &vec_dim, const double &scale) {
            this->n_user_ = n_user;
            this->vec_dim_ = vec_dim;
            this->scale_ = scale;
            user_int_ptr_ = std::make_unique<int[]>(n_user_ * vec_dim_);
            user_int_sum_ptr_ = std::make_unique<int[]>(n_user_);
            query_int_ptr_ = std::make_unique<int[]>(vec_dim_);
        }

        //make bound from offset_dim to vec_dim
        void Preprocess(VectorMatrix &user, VectorMatrix &data_item) override {

            double user_max_dim_ = user.getVector(0)[0];

            //compute the integer bound for the first part
            for (int userID = 0; userID < n_user_; userID++) {
                double *user_vecs = user.getVector(userID);
                for (int dim = 0; dim < vec_dim_; dim++) {
                    user_max_dim_ = std::max(user_max_dim_, user_vecs[dim]);
                }
            }
            user_max_dim_ = std::abs(user_max_dim_);

#pragma omp parallel for default(none) shared(user, user_max_dim_)
            for (int userID = 0; userID < n_user_; userID++) {
                int *user_int_vecs = user_int_ptr_.get() + userID * vec_dim_;
                double *user_double_vecs = user.getVector(userID);
                user_int_sum_ptr_[userID] = 0;
                for (int dim = 0; dim < vec_dim_; dim++) {
                    user_int_vecs[dim] = std::floor(user_double_vecs[dim] * scale_ / user_max_dim_);
                    user_int_sum_ptr_[userID] += std::abs(user_int_vecs[dim]) + 1;
                }

            }

            user_convert_coe_ = user_max_dim_ / (scale_ * scale_);
        }

        void
        IPBound(const double *query_vecs, const VectorMatrix &user,
                std::vector<std::pair<double, double>> &ip_bound_l) {
            assert(ip_bound_l.size() == n_user_);

            double query_max_dim = query_vecs[0];
            for (int dim = 1; dim < vec_dim_; dim++) {
                query_max_dim = std::max(query_max_dim, query_vecs[dim]);
            }
            query_max_dim = std::abs(query_max_dim);
            convert_coe_ = user_convert_coe_ * query_max_dim;

            const double qratio = scale_ / query_max_dim;

            int query_int_sum = 0;
            int *query_int_vecs = query_int_ptr_.get();
            for (int dim = 0; dim < vec_dim_; dim++) {
                query_int_vecs[dim] = std::floor(query_vecs[dim] * qratio);
                query_int_sum += std::abs(query_int_vecs[dim]);
            }

            for (int userID = 0; userID < n_user_; userID++) {
                int *user_int_vecs = user_int_ptr_.get() + userID * vec_dim_;
                int intIP = InnerProduct(user_int_vecs, query_int_vecs, vec_dim_);
                int int_otherIP = user_int_sum_ptr_[userID] + query_int_sum;
                int lb_part = intIP - int_otherIP;
                int ub_part = intIP + int_otherIP;

                double lower_bound = convert_coe_ * lb_part;
                double upper_bound = convert_coe_ * ub_part;

                ip_bound_l[userID] = std::make_pair(lower_bound, upper_bound);
                assert(lower_bound <= upper_bound);
            }

        }

    };

}
#endif //REVERSE_KRANKS_FULLINT_HPP
