//
// Created by BianZheng on 2022/5/19.
//

#ifndef REVERSE_K_RANKS_FULLDIM_HPP
#define REVERSE_K_RANKS_FULLDIM_HPP

#include <cfloat>

#include "alg/ExactRankByIPBound/BaseIPBound.hpp"
#include "alg/SpaceInnerProduct.hpp"

namespace ReverseMIPS {

    class FullDim : public BaseIPBound {
        int n_user_, n_data_item_, vec_dim_;

    public:

        inline FullDim() {
            this->n_user_ = -1;
            this->n_data_item_ = -1;
            this->vec_dim_ = -1;
        }

        inline FullDim(const int &n_user, const int &n_data_item, const int &vec_dim) {
            this->n_user_ = n_user;
            this->n_data_item_ = n_data_item;
            this->vec_dim_ = vec_dim;
        }

        void Preprocess(VectorMatrix &user, VectorMatrix &data_item) {

        }

        void PreprocessQuery(const double *query_vecs, const int &vec_dim, double *query_write_vecs) {
            memcpy(query_write_vecs, query_vecs, vec_dim * sizeof(double));
        }

        double IPUpperBound(const double *user_vecs, const int &userID,
                            const double *item_vecs, const int &itemID) override {
            const double itemIP = InnerProduct(user_vecs, item_vecs, vec_dim_);
            return itemIP;
        }

        double IPLowerBound(const double *user_vecs, const int &userID,
                            const double *item_vecs, const int &itemID) override {
            const double itemIP = InnerProduct(user_vecs, item_vecs, vec_dim_);
            return itemIP;
        }

    };
}
#endif //REVERSE_K_RANKS_FULLDIM_HPP
