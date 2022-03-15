//
// Created by BianZheng on 2022/3/15.
//

#ifndef REVERSE_KRANKS_FULLDIMPRUNE_HPP
#define REVERSE_KRANKS_FULLDIMPRUNE_HPP

#include <cassert>
#include <memory>
#include <vector>
#include "alg/SpaceInnerProduct.hpp"
#include "struct/VectorMatrix.hpp"
#include "struct/RankBoundElement.hpp"

namespace ReverseMIPS {

    class FullDimPrune {
        int n_user_, vec_dim_;
        std::unique_ptr<std::pair<double, double>[]> ip_bound_l_;
    public:
        FullDimPrune() = default;

        //make bound from offset_dim to vec_dim
        void Preprocess(const VectorMatrix &user) {
            this->n_user_ = user.n_vector_;
            this->vec_dim_ = user.vec_dim_;
            ip_bound_l_ = std::make_unique<std::pair<double, double>[]>(n_user_);
        }

        void
        QueryBound(const double *query_vecs, const VectorMatrix &user, const int &n_candidate,
                   std::vector<RankBoundElement> &candidate_l, bool assign) {
            assert(candidate_l.size() == n_user_);
            assert(n_candidate <= n_user_);
            assert(n_candidate <= candidate_l.size());

            for (int candID = 0; candID < n_candidate; candID++) {
                int userID = candidate_l[candID].userID_;
                const double *user_vecs = user.getVector(userID);
                double ip = InnerProduct(user_vecs, query_vecs, vec_dim_);

                ip_bound_l_[candID].first = ip;
                ip_bound_l_[candID].second = ip;

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
#endif //REVERSE_KRANKS_FULLDIMPRUNE_HPP
