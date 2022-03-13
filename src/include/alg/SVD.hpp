//
// Created by BianZheng on 2022/3/1.
//

#ifndef REVERSE_KRANKS_SVD_HPP
#define REVERSE_KRANKS_SVD_HPP

#include <cassert>
#include <armadillo>
#include <memory>
#include "alg/SpaceInnerProduct.hpp"
#include "struct/VectorMatrix.hpp"

namespace ReverseMIPS::SVD {
    void TransferItem(double *item_vecs, const VectorMatrix &transfer_item, const int vec_dim) {
        assert(transfer_item.n_vector_ == transfer_item.vec_dim_);

        std::vector<double> data_item_cache(vec_dim);
        for (int trans_dim = 0; trans_dim < vec_dim; trans_dim++) {
            double *transfer_vecs = transfer_item.getVector(trans_dim);
            data_item_cache[trans_dim] = InnerProduct(transfer_vecs, item_vecs, vec_dim);
        }
        memcpy(item_vecs, data_item_cache.data(), vec_dim * sizeof(double));

    }

    int SVD(VectorMatrix &user, VectorMatrix &data_item, VectorMatrix &transfer_item, const double &SIGMA) {
        const int vec_dim = user.vec_dim_; // p->colNum, m
        const int n_user = user.n_vector_; // p->rowNum, n
        const int n_data_item = data_item.n_vector_;

        std::unique_ptr<double[]> transfer_ptr = std::make_unique<double[]>(vec_dim * vec_dim);
        transfer_item.init(transfer_ptr, vec_dim, vec_dim);

        //Q is item, since a new query would be added
        //U is user, since user matrix would not changed
        arma::mat P_t(user.getRawData(), user.vec_dim_, user.n_vector_, false, true);

        arma::mat U_t;
        arma::vec s;
        arma::mat V;

        // see: http://arma.sourceforge.net/docs.html#svd_econ
        //	svd_econ(U_t, s, V, P_t, "both", "std");
        arma::svd(U_t, s, V, P_t, "std"); // P = U * sigma * V_t

        U_t = U_t.t();

        double *uData = transfer_item.getRawData();

        for (int rowIndex = 0; rowIndex < vec_dim; rowIndex++) {
            for (int colIndex = 0; colIndex < vec_dim; colIndex++) {
                uData[rowIndex * vec_dim + colIndex] = s[rowIndex] * U_t(rowIndex, colIndex);
            }
        }

        for (int rowIndex = 0; rowIndex < n_user; rowIndex++) {
            for (int colIndex = 0; colIndex < vec_dim; colIndex++) {
                user.getRawData()[rowIndex * vec_dim + colIndex] = V(rowIndex, colIndex);
            }
        }

        for (int itemID = 0; itemID < n_data_item; itemID++) {
            TransferItem(data_item.getVector(itemID), transfer_item, vec_dim);
        }

        std::vector<double> sum(vec_dim);
        sum[0] = s[0];
        for (int colIndex = 1; colIndex < vec_dim; colIndex++) {
            sum[colIndex] = sum[colIndex - 1] + s[colIndex];
        }

        int check_dim = 0;
        for (int colIndex = 0; colIndex < vec_dim; colIndex++) {
            if (sum[colIndex] / sum[vec_dim - 1] >= SIGMA) {
                check_dim = colIndex;
                break;
            }
        }
        return check_dim;
    }

}

#endif //REVERSE_KRANKS_SVD_HPP