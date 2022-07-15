//
// Created by BianZheng on 2022/7/14.
//

#ifndef REVERSE_KRANKS_BASECANDIDATE_HPP
#define REVERSE_KRANKS_BASECANDIDATE_HPP

#include <vector>
#include "struct/VectorMatrix.hpp"

namespace ReverseMIPS {
    class BaseComputeRank {
    public:
        virtual void PreprocessData(VectorMatrix &user, VectorMatrix &data_item) = 0;

        virtual void PreprocessQuery(const double *query_vecs, const int &vec_dim, double *query_write_vecs) = 0;

        virtual int QueryRankByCandidate(const std::pair<double, double> &queryIPbound_pair, const double &queryIP,
                                         const double *user_vecs, const int &userID,
                                         const VectorMatrix &item, const std::vector<bool> &item_cand_l) const = 0;

        virtual int QueryRankByCandidate(const double *user_vecs, const int &userID,
                                         const VectorMatrix &item,
                                         const double &queryIP) const = 0;

        virtual ~BaseComputeRank() = default;

    };
}
#endif //REVERSE_KRANKS_BASECANDIDATE_HPP
