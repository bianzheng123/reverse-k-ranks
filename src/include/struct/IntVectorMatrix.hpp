//
// Created by BianZheng on 2022/3/8.
//

#ifndef REVERSE_KRANKS_INTVECTORMATRIX_HPP
#define REVERSE_KRANKS_INTVECTORMATRIX_HPP

#include <memory>

class IntVectorMatrix {
    std::unique_ptr<int[]> rawData_;
public:
    int n_vector_;
    int vec_dim_;

    IntVectorMatrix() {
        this->rawData_ = nullptr;
        this->n_vector_ = 0;
        this->vec_dim_ = 0;
    }

    ~IntVectorMatrix() = default;

    [[nodiscard]] int *getVector(const int vec_idx) const {
        return rawData_.get() + vec_idx * vec_dim_;
    }

    void init(std::unique_ptr<int[]> &rawData, const int n_vector, const int vec_dim) {
        this->rawData_ = std::move(rawData);
        this->n_vector_ = n_vector;
        this->vec_dim_ = vec_dim;
    }

    IntVectorMatrix &operator=(IntVectorMatrix &&other) noexcept {
        this->rawData_ = std::move(other.rawData_);
        this->n_vector_ = other.n_vector_;
        this->vec_dim_ = other.vec_dim_;
        return *this;
    }

    [[nodiscard]] int *getRawData() const {
        return this->getVector(0);
    }

};

#endif //REVERSE_KRANKS_INTVECTORMATRIX_HPP
