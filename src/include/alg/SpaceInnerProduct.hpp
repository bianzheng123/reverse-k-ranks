#pragma once

#include <algorithm>

namespace ReverseMIPS {
    std::tuple<int, int, int> InnerProductBound(const int *vecs1, const int *vecs2, const int dim) {
        int lb = 0;
        int ub = 0;
        int int_res = 0;
        for (unsigned i = 0; i < dim; i++) {
            int abs_vecs1 = std::abs(vecs1[i]);
            int abs_vecs2 = std::abs(vecs2[i]);
            int tmp_int_res = vecs1[i] * vecs2[i];
            int_res += tmp_int_res;
            lb += tmp_int_res - abs_vecs1 - abs_vecs2 - 1;
            ub += tmp_int_res + abs_vecs1 + abs_vecs2 + 1;
        }
        return std::make_tuple(lb, ub, int_res);
    }

    double InnerProduct(const double *pVect1, const double *pVect2, const int dim) {
        double res = 0;
        for (unsigned i = 0; i < dim; i++) {
            res += pVect1[i] * pVect2[i];
        }
        return res;

    }


}