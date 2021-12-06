#pragma once
namespace ReverseMIPS {
    class VectorMatrix {

    public:
        float *rawData;
        int n_vector;
        int vec_dim;

        inline VectorMatrix() {
            this->rawData = NULL;
            this->n_vector = 0;
            this->vec_dim = 0;
        }

        inline ~VectorMatrix() {
            if (!rawData) {
                delete[] rawData;
            }
        }

        inline float *getVector(const int vec_idx) {
            return &rawData[vec_idx * vec_dim];
        }

        inline void init(float *rawData, const int n_vector, const int vec_dim) {
            this->n_vector = n_vector;
            this->vec_dim = vec_dim;
            this->rawData = rawData;
        }

    };
}
