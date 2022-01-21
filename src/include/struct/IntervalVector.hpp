//
// Created by BianZheng on 2021/12/27.
//

#ifndef REVERSE_KRANKS_INTERVALVECTOR_HPP
#define REVERSE_KRANKS_INTERVALVECTOR_HPP

#include <vector>
#include <set>
#include <cassert>
#include <algorithm>

namespace ReverseMIPS {

    class IntervalVector {

    public:
        std::vector<std::vector<int>> set_vector_;
        int dimension_;

        inline IntervalVector(int dimension) {
            this->dimension_ = dimension;
            this->set_vector_ = std::vector<std::vector<int>>(dimension, std::vector<int>());
        }

        inline IntervalVector(std::vector<std::vector<int>> &set_vector, int dimension) {
            int size = set_vector.size();
            for (int i = 0; i < size; i++) {
                std::sort(set_vector[i].begin(), set_vector[i].end());
            }

            this->set_vector_ = set_vector;
            this->dimension_ = dimension;
        }

        inline ~IntervalVector() {}

        void Merge(const IntervalVector &other) {
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

//            for(int i=0;i<set_vector_.size();i++){
//                for(int j=0;j<set_vector_[i].size();j++){
//                    if(set_vector_[i][j] <0 || set_vector_[i][j] >= 1000){
//                        printf("%d\n", set_vector_[i][j]);
//                    }
//                }
//            }

        }

        [[nodiscard]] std::vector<int> getInterval(int id) const {
            return set_vector_[id];
        }


    };
}

#endif //REVERSE_KRANKS_INTERVALVECTOR_HPP
