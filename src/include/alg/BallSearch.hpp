//
// Created by BianZheng on 2022/3/22.
//

#ifndef REVERSE_KRANKS_TREESEARCH_HPP
#define REVERSE_KRANKS_TREESEARCH_HPP

#include <algorithm>
#include <vector>
#include <memory>
#include "struct/VectorMatrix.hpp"

namespace ReverseMIPS {

    /**
     * Preprocess: 先选择一个两个最远的点进行划分, 然后选择归类, 判断其半径大小
     * 讲半径更大的进行拆分, 直到总共的长度不超过N
     * **/
    class BallSearch {
        int n_user_, vec_dim_, n_node_;
        std::unique_ptr<int[]> user_idx_l_;  // n_user_, store the total index which belongs to
        std::unique_ptr<double[]> center_l_; // n_node_ * vec_dim_
        std::unique_ptr<double[]> range_l_;  //n_node_, stores the last user index of a node
        inline BallSearch(const VectorMatrix &user, const int n_node) {
            this->n_user_ = user.n_vector_;
            this->vec_dim_ = user.vec_dim_;
            this->n_node_ = n_node;

            user_idx_l_ = std::make_unique<int[]>(n_user_);
            const int *user_idx_vecs = user_idx_l_.get();
            std::iota(user_idx_vecs, user_idx_vecs + n_user_, 0);
//            user_idx_l_
//
//            const double *user_vecs = user.getVector(0);
        }

    };

}
#endif //REVERSE_KRANKS_TREESEARCH_HPP
