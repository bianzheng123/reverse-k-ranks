//
// Created by BianZheng on 2022/3/8.
//

#ifndef REVERSE_K_RANKS_INTEGERPRUNE_HPP
#define REVERSE_K_RANKS_INTEGERPRUNE_HPP

#include <cassert>
#include <memory>
#include <vector>
#include "alg/SpaceInnerProduct.hpp"
#include "struct/IntVectorMatrix.hpp"
#include "struct/VectorMatrix.hpp"

namespace ReverseMIPS::ScaleIntegerPrune {

    const double scale_ = 100;

    void ConvertQuery(double *query_vecs, const int &vec_dim,
                      std::vector<int> &query_int, const int &check_dim) {
        for (int dim = 0; dim < vec_dim; dim++) {
            query_vecs[dim] *= scale_;
        }
        for (int dim = check_dim; dim < vec_dim; dim++) {
            query_int[dim - check_dim] = std::floor(query_vecs[dim]);
            assert(query_vecs[dim] - 1 <= query_int[dim - check_dim] &&
                   query_int[dim - check_dim] <= query_vecs[dim]);
        }

    }

    void
    ScaleIntegerPrune(VectorMatrix &user, VectorMatrix &data_item, IntVectorMatrix &user_int, const int &check_dim) {
        const int vec_dim = user.vec_dim_; // p->colNum, m
        const int n_user = user.n_vector_; // p->rowNum, n
        const int n_data_item = data_item.n_vector_;

        std::unique_ptr<int[]> user_int_uni_ptr = std::make_unique<int[]>(n_user * check_dim);

        user_int.init(user_int_uni_ptr, n_user, check_dim);

        for (int userID = 0; userID < n_user; userID++) {
            double *user_vecs = user.getVector(userID);
            int *user_int_vecs = user_int.getVector(userID);
            for (int dim = 0; dim < vec_dim; dim++) {
                user_vecs[dim] *= scale_;
            }
            for (int dim = check_dim; dim < vec_dim; dim++) {
                user_int_vecs[dim - check_dim] = std::floor(user_vecs[dim]);
                assert(user_vecs[dim] - 1 <= user_int_vecs[dim - check_dim] &&
                       user_int_vecs[dim - check_dim] <= user_vecs[dim]);
            }
        }

        for (int itemID = 0; itemID < n_data_item; itemID++) {
            double *item_vecs = data_item.getVector(itemID);
            for (int dim = 0; dim < vec_dim; dim++) {
                item_vecs[dim] *= scale_;
            }
        }

    }

    void
    ScaleTransferItem(VectorMatrix &transfer_item) {
        const int vec_dim = transfer_item.vec_dim_; // p->colNum, m
        assert(transfer_item.n_vector_ == transfer_item.vec_dim_);

        for (int vecID = 0; vecID < vec_dim; vecID++) {
            double *vecs = transfer_item.getVector(vecID);
            for (int dim = 0; dim < vec_dim; dim++) {
                vecs[dim] *= scale_;
            }
        }

    }

}

#endif //REVERSE_K_RANKS_INTEGERPRUNE_HPP
