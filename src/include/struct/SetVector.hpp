//
// Created by BianZheng on 2021/12/27.
//

#ifndef REVERSE_KRANKS_SETVECTOR_HPP
#define REVERSE_KRANKS_SETVECTOR_HPP

#include <vector>
#include <set>
#include <cassert>
#include <algorithm>

namespace ReverseMIPS {

    class SetVector {

    public:
        std::vector<std::vector<int>> set_vector_;
        int dimension_;

        inline SetVector(int dimension) {
            this->dimension_ = dimension;
            this->set_vector_ = std::vector<std::vector<int>>(dimension, std::vector<int>());
        }

        inline SetVector(std::vector<std::vector<int>> &set_vector, int dimension) {
            int size = set_vector.size();
            for (int i = 0; i < size; i++) {
                std::sort(set_vector[i].begin(), set_vector[i].end());
            }

            this->set_vector_ = set_vector;
            this->dimension_ = dimension;
        }

        inline ~SetVector() {}

        void Merge(const SetVector &other) {
            assert(other.dimension_ != this->dimension_);
            for (int i = 0; i < dimension_; i++) {
                //it has sorted before the assignment
                int total_size = set_vector_[i].size() + other.set_vector_[i].size();
                std::vector<int> final(total_size);

                std::vector<int>::iterator it;
                it = std::set_union(set_vector_[i].begin(), set_vector_[i].end(), other.set_vector_[i].begin(),
                                    other.set_vector_[i].end(), final.begin());
                final.resize(it - final.begin());
                set_vector_[i] = final;
            }

        }

    };
}

#endif //REVERSE_KRANKS_SETVECTOR_HPP
