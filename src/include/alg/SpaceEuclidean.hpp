//
// Created by BianZheng on 2022/3/26.
//

#ifndef REVERSE_KRANKS_SPACEEUCLIDEAN_HPP
#define REVERSE_KRANKS_SPACEEUCLIDEAN_HPP

#include <cmath>
#include <algorithm>

namespace ReverseMIPS {
    double EuclideanDistance(const double *pVect1, const double *pVect2, const int dim) {
        double res = 0;
        for (unsigned i = 0; i < dim; i++) {
            double tmp_euc = pVect1[i] - pVect2[i];
            res += tmp_euc * tmp_euc;
        }
        assert(res >= 0);
        res = std::sqrt(res);
        return res;
    }

}
#endif //REVERSE_KRANKS_SPACEEUCLIDEAN_HPP
