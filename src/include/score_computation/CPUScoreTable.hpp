//
// Created by BianZheng on 2022/7/13.
//

#ifndef REVERSE_KRANKS_CPUSCORETABLE_HPP
#define REVERSE_KRANKS_CPUSCORETABLE_HPP

#include "alg/SpaceInnerProduct.hpp"
#include <vector>

namespace ReverseMIPS {

    class CPUScoreTable {

        uint64_t n_user_, n_data_item_, vec_dim_;

        const double *user_vecs_;
        const double *data_item_vecs_;

    public:
        CPUScoreTable() = default;

        inline CPUScoreTable(const double *user_vecs, const double *data_item_vecs,
                             const uint64_t n_user, const uint64_t n_data_item, const uint64_t vec_dim) {
            this->user_vecs_ = user_vecs;
            this->data_item_vecs_ = data_item_vecs;
            this->n_user_ = n_user;
            this->n_data_item_ = n_data_item;
            this->vec_dim_ = vec_dim;
        }

        void ComputeList(const int &userID, double *distance_l) {
            assert(0 <= userID && userID < n_user_);
            const double *tmp_user_vecs = user_vecs_ + userID * vec_dim_;
#pragma omp parallel for default(none) shared(tmp_user_vecs, distance_l, n_data_item_)
            for (int itemID = 0; itemID < n_data_item_; itemID++) {
                const double *tmp_data_item_vecs = data_item_vecs_ + itemID * vec_dim_;
                const double ip = InnerProduct(tmp_user_vecs, tmp_data_item_vecs, (int) vec_dim_);
                distance_l[itemID] = ip;
            }
        }

        void ComputeList(const int &start_userID, const int &batch_n_user, double *distance_l) {
            assert(0 <= start_userID && start_userID + batch_n_user <= n_user_);

#pragma omp parallel for default(none) shared(batch_n_user, distance_l, start_userID)
            for (int batch_userID = 0; batch_userID < batch_n_user; batch_userID++) {
                const double *tmp_user_vecs = user_vecs_ + (start_userID + batch_userID) * vec_dim_;

                for (int itemID = 0; itemID < n_data_item_; itemID++) {
                    const double *tmp_data_item_vecs = data_item_vecs_ + itemID * vec_dim_;
                    const double ip = InnerProduct(tmp_user_vecs, tmp_data_item_vecs, (int) vec_dim_);
                    distance_l[itemID + batch_userID * n_data_item_] = ip;
                }
            }

        }

        void FinishCompute() {}
    };

}
#endif //REVERSE_KRANKS_CPUSCORETABLE_HPP
