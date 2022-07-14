//
// Created by BianZheng on 2022/7/13.
//

#ifndef REVERSE_KRANKS_CPUSCORETABLE_HPP
#define REVERSE_KRANKS_CPUSCORETABLE_HPP

#include "alg/SpaceInnerProduct.hpp"
#include <vector>
#include <Eigen/Dense>

#ifndef EIGEN_USE_MKL_ALL
#define EIGEN_USE_MKL_ALL
#endif

#ifndef EIGEN_VECTORIZE_SSE4_2
#define EIGEN_VECTORIZE_SSE4_2
#endif

namespace ReverseMIPS {

    class CPUScoreTable {

        int n_user_, n_data_item_, vec_dim_;
        std::vector<double> ip_cache_;

        Eigen::MatrixXd user_m_, data_item_m_;
    public:
        CPUScoreTable() = default;

        inline CPUScoreTable(const double *user, const double *data_item,
                             const int n_user, const int n_data_item, const int vec_dim) {
            n_user_ = n_user;
            n_data_item_ = n_data_item;
            vec_dim_ = vec_dim;
            ip_cache_.resize(n_data_item_);

            user_m_ = Eigen::Map<const Eigen::VectorXd>(user, n_user * vec_dim);
            user_m_.resize(vec_dim, n_user);

            data_item_m_ = Eigen::Map<const Eigen::VectorXd>(data_item, n_data_item * vec_dim);
            data_item_m_.resize(vec_dim, n_data_item);
        }

        void ComputeList(const int &userID, double *distance_l) {
            Eigen::VectorXd user_vecs = user_m_.col(userID);
            Eigen::VectorXd ip_res = data_item_m_.transpose() * user_vecs;
            const double *ip_ptr = ip_res.data();
            memcpy(distance_l, ip_ptr, n_data_item_ * sizeof(double));
        }

        void FinishCompute() {}
    };

}
#endif //REVERSE_KRANKS_CPUSCORETABLE_HPP
