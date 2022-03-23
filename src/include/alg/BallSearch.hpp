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

            center_l_ = std::make_unique<double[]>(n_node_ * vec_dim_);
            range_l_ = std::make_unique<double[]>(n_node_);

            const double *user_vecs = user.getVector(0);

            BuildBall(0, 0);
        }

        void BuildBall(const int &nodeID, const int &start_idx) {
            /**对一整个数组进行partition, 分成两份, 将user数量最多的那一份放到最远的地方**/
            const int end_idx = n_user_;
            assert(0 <= nodeID && nodeID < n_node_);
            assert(0 <= start_idx && start_idx < end_idx);

            //randomly choose a userID
            std::random_device rd;  //Will be used to obtain a seed for the random number engine
            std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
            std::uniform_int_distribution<> distrib(start_idx, end_idx - 1);
            int rand_candID = distrib(gen);
            int rand_userID = user_idx_l_[rand_candID];

            //calc most far userID
//            int tmp_
            for (int candID = start_idx; candID < end_idx; candID++) {
                int userID = user_idx_l_[candID];
//                if(userID == )
//                double ip = InnerProduct()
            }

        }

    };

}
#endif //REVERSE_KRANKS_TREESEARCH_HPP
