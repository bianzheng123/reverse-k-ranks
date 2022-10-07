//
// Created by BianZheng on 2022/5/19.
//

#ifndef REVERSE_K_RANKS_BASEIPBOUND_HPP
#define REVERSE_K_RANKS_BASEIPBOUND_HPP

#include "struct/VectorMatrix.hpp"

#include <vector>

namespace ReverseMIPS {
    class BaseQueryIPBound {
    public:
        virtual void Preprocess(VectorMatrix &user, VectorMatrix &data_item) = 0;

        virtual void
        IPBound(const double *query_vecs, const VectorMatrix &user, std::vector<std::pair<double, double>> &queryIP_l) = 0;

        virtual ~BaseQueryIPBound() = default;

    };
}

#endif //REVERSE_K_RANKS_BASEIPBOUND_HPP
