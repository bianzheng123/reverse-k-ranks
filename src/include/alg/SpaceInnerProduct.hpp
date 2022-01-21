#pragma once


namespace ReverseMIPS {

    double InnerProduct(const double *pVect1, const double *pVect2, const int dim) {
        double res = 0;
        for (unsigned i = 0; i < dim; i++) {
            res += pVect1[i] * pVect2[i];
        }
        return res;

    }


}