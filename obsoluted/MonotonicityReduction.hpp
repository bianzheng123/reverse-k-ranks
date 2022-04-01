//
// Created by BianZheng on 2022/3/2.
//

#ifndef REVERSE_K_RANKS_MONOTONICITYREDUCTION_HPP
#define REVERSE_K_RANKS_MONOTONICITYREDUCTION_HPP

#include "alg/SpaceInnerProduct.hpp"
#include "struct/VectorMatrix.hpp"
#include <cstring>
#include <memory>
#include <vector>
#include <cfloat>

namespace ReverseMIPS::MonotonicityReduction {

    double topk_b2;
    std::vector<double> topk_c;

    std::vector<double> &ConvertQuery(const double *item_vecs, const int vec_dim) {
        int new_vec_dim = vec_dim + 2;
        static std::vector<double> new_item_l(new_vec_dim);

        double norm = InnerProduct(item_vecs, item_vecs, vec_dim);
        new_item_l[1] = std::sqrt(topk_b2 - norm);

        for (int dim = 2; dim < new_vec_dim; dim++) {
            new_item_l[dim] = item_vecs[dim - 2] + topk_c[dim - 2];
        }

        norm = InnerProduct(new_item_l.data() + 1, new_item_l.data() + 1, vec_dim + 1);
        new_item_l[0] = norm;

        return new_item_l;
    }

    //p: item, q: user
    void ConvertUserItem(VectorMatrix &user, VectorMatrix &data_item, const std::vector<double> &eigen_l) {
        const int n_user = user.n_vector_;
        const int n_data_item = data_item.n_vector_;
        const int vec_dim = user.vec_dim_;

        //top-k conversion
        double topk_global_c = 1.0;
        topk_b2 = 0;
        for (int itemID = 0; itemID < n_data_item; itemID++) {
            double *item_vec = data_item.getVector(itemID);
            for (int dim = 0; dim < vec_dim; dim++) {
                double abs_val = std::abs(item_vec[dim]);
                topk_global_c = std::min(abs_val, topk_global_c);
            }
            double norm = InnerProduct(item_vec, item_vec, vec_dim);
            topk_b2 = std::max(norm, topk_b2);
        }
        topk_b2 *= 10;

        topk_global_c = std::max(1.0, topk_global_c);

        double eigen_total = 0;
        for (const double tmp_eigen: eigen_l) {
            eigen_total += tmp_eigen;
        }

        topk_c.resize(vec_dim);
        for (int dim = 0; dim < vec_dim; dim++) {
//            topk_c[dim] = eigen_l[dim] / eigen_l[vec_dim - 1] + topk_global_c;
            topk_c[dim] = 1 + topk_global_c;
        }

        int new_vec_dim = vec_dim + 2;
        std::unique_ptr<double[]> new_user_rawdata = std::make_unique<double[]>(n_user * new_vec_dim);
        std::unique_ptr<double[]> new_data_item_rawdata = std::make_unique<double[]>(n_data_item * new_vec_dim);

        for (int itemID = 0; itemID < n_user; itemID++) {
            double *new_item_vecs = new_data_item_rawdata.get() + itemID * new_vec_dim;

            double *item_vecs = data_item.getVector(itemID);

            std::vector<double> &new_query = ConvertQuery(item_vecs, vec_dim);
            std::memcpy(new_item_vecs, new_query.data(), new_vec_dim * sizeof(double));
        }

        for (int userID = 0; userID < n_user; userID++) {
            double *new_user_vecs = new_user_rawdata.get() + userID * new_vec_dim;
            new_user_vecs[0] = -1;
            new_user_vecs[1] = 0;

            double *user_vecs = user.getVector(userID);
            double norm = InnerProduct(user_vecs, user_vecs, vec_dim);
            norm = std::sqrt(norm);

            for (int dim = 2; dim < new_vec_dim; dim++) {
                new_user_vecs[dim] = 2 * (topk_c[dim - 2] + user_vecs[dim - 2] / norm);
            }
        }

        VectorMatrix new_user, new_data_item;
        new_user.init(new_user_rawdata, n_user, new_vec_dim);
        new_data_item.init(new_data_item_rawdata, n_data_item, new_vec_dim);

        user = std::move(new_user);
        data_item = std::move(new_data_item);
    }


}

#endif //REVERSE_K_RANKS_MONOTONICITYREDUCTION_HPP
