//
// Created by BianZheng on 2022/4/19.
//

#ifndef REVERSE_KRANKS_CANDIDATEBRUTEFORCE_HPP
#define REVERSE_KRANKS_CANDIDATEBRUTEFORCE_HPP

#include "alg/DiskIndex/ComputeRank/BaseComputeRank.hpp"
#include "alg/SpaceInnerProduct.hpp"
#include "struct/VectorMatrix.hpp"
#include "struct/UserRankBound.hpp"

namespace ReverseMIPS {

    class CandidateBruteForce : public BaseComputeRank {
        int n_data_item_, vec_dim_;
    public:

        inline CandidateBruteForce() = default;

        inline CandidateBruteForce(const int &n_data_item, const int &vec_dim) {
            this->n_data_item_ = n_data_item;
            this->vec_dim_ = vec_dim;
        }

        void PreprocessData(VectorMatrix &user, VectorMatrix &data_item) override {

        };

        void PreprocessQuery(const double *query_vecs, const int &vec_dim, double *query_write_vecs) override {
            memcpy(query_write_vecs, query_vecs, vec_dim * sizeof(double));
        };

        int QueryRankByCandidate(const std::pair<double, double> &queryIPbound_pair, const double &queryIP,
                                 const double *user_vecs, const int &userID,
                                 const VectorMatrix &item, const std::vector<bool> &item_cand_l) const override {
            //calculate all the IP, then get the lower bound
            //make different situation by the information
            int rank = 0;
            const double &queryIP_lb = queryIPbound_pair.first;
            const double &queryIP_ub = queryIPbound_pair.second;

            assert(item_cand_l.size() == n_data_item_);
            assert(queryIP_lb <= queryIP && queryIP <= queryIP_ub);
            for (int itemID = 0; itemID < n_data_item_; itemID++) {
                if (!item_cand_l[itemID]) {
                    continue;
                }
                double ip = InnerProduct(item.getVector(itemID), user_vecs, vec_dim_);
                if (queryIP_lb <= ip && ip <= queryIP_ub) {
                    if (ip >= queryIP) {
                        rank++;
                    }
                }
            }

            return rank;
        };

        int QueryRankByCandidate(const double *user_vecs, const int &userID,
                                 const VectorMatrix &item,
                                 const double &queryIP) const override {
            //calculate all the IP, then get the lower bound
            //make different situation by the information
            int rank = 0;
            for (int itemID = 0; itemID < n_data_item_; itemID++) {
                double ip = InnerProduct(item.getVector(itemID), user_vecs, vec_dim_);
                if (ip >= queryIP) {
                    rank++;
                }
            }

            return rank;
        };

    };
}
#endif //REVERSE_KRANKS_CANDIDATEBRUTEFORCE_HPP
