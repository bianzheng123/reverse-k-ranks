#pragma once


namespace ReverseMIPS {

    float InnerProduct(const float *pVect1, const float *pVect2, const int dim) {
        size_t qty = (size_t) dim;
        float res = 0;
        for (unsigned i = 0; i < qty; i++) {
            res += ((float *) pVect1)[i] * ((float *) pVect2)[i];
        }
        return res;

    }


}