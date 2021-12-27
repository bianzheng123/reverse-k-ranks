#pragma once


namespace ReverseMIPS {

    float InnerProduct(const float *pVect1, const float *pVect2, const int dim) {
        float res = 0;
        for (unsigned i = 0; i < dim; i++) {
            res += pVect1[i] * pVect2[i];
        }
        return res;

    }


}