#include <cmath>

#pragma once
namespace ReverseMIPS {
    class VectorMatrix {

    public:
        double *rawData_;
        int n_vector_;
        int vec_dim_;

        VectorMatrix() {
            this->rawData_ = nullptr;
            this->n_vector_ = 0;
            this->vec_dim_ = 0;
        }

        ~VectorMatrix() = default;

        [[nodiscard]] double *getVector(const int vec_idx) const {
            return rawData_ + vec_idx * vec_dim_;
        }

        void init(double *rawData, const int n_vector, const int vec_dim) {
            this->n_vector_ = n_vector;
            this->vec_dim_ = vec_dim;
            this->rawData_ = rawData;
        }

        void vectorNormalize() {
#pragma omp parallel for default(none)
            for (int i = 0; i < n_vector_; i++) {
                double l2norm = 0;
                for (int j = 0; j < vec_dim_; j++) {
                    l2norm += rawData_[i * vec_dim_ + j] * rawData_[i * vec_dim_ + j];
                }
                l2norm = std::sqrt(l2norm);
                for (int j = 0; j < vec_dim_; j++) {
                    rawData_[i * vec_dim_ + j] /= l2norm;
                }
            }
        }

    };
}
