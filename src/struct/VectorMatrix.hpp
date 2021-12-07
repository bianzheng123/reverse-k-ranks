#pragma once
namespace ReverseMIPS {
    class VectorMatrix {

    public:
        float *rawData_;
        int n_vector_;
        int vec_dim_;

        VectorMatrix() {
            this->rawData_ = NULL;
            this->n_vector_ = 0;
            this->vec_dim_ = 0;
        }

        ~VectorMatrix() {
            if (!rawData_) {
                delete[] rawData_;
            }
        }

        float *getVector(const int vec_idx) {
            return &rawData_[vec_idx * vec_dim_];
        }

        void init(float *rawData, const int n_vector, const int vec_dim) {
            this->n_vector_ = n_vector;
            this->vec_dim_ = vec_dim;
            this->rawData_ = rawData;
        }

    };
}
