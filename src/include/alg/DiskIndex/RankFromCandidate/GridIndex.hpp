//
// Created by BianZheng on 2022/4/19.
//

#ifndef REVERSE_KRANKS_GRIDINDEX_HPP
#define REVERSE_KRANKS_GRIDINDEX_HPP

#include "struct/VectorMatrix.hpp"
#include "struct/UserRankBound.hpp"
#include <spdlog/spdlog.h>
#include <cfloat>

namespace ReverseMIPS {

    inline std::pair<double, double> AbsMinMaxValue(const VectorMatrix &matrix) {
        double max_val = -DBL_MAX;
        double min_val = DBL_MAX;
        const int n_vecs = matrix.n_vector_;
        const int vec_dim = matrix.vec_dim_;
        for (int vecsID = 0; vecsID < n_vecs; vecsID++) {
            const double *vecs = matrix.getVector(vecsID);
            for (int dim = 0; dim < vec_dim; dim++) {
                const double abs_val = std::abs(vecs[dim]);
                max_val = std::max(max_val, abs_val);
                min_val = std::min(min_val, abs_val);
            }
        }
        return std::make_pair(min_val, max_val);
    }


    class GridIndex {
        inline void CalcCodeword(const VectorMatrix &matrix, const std::pair<double, double> &abs_val_pair,
                                 const double &cell_dist, int *codeword_) const {
            //codeword_ shape: n_vecs * vec_dim_
            const double abs_min = abs_val_pair.first;
            const double abs_max = abs_val_pair.second;
            const int n_vecs = matrix.n_vector_;
            const int vec_dim = matrix.vec_dim_;
            for (int vecsID = 0; vecsID < n_vecs; vecsID++) {
                const double *vecs = matrix.getVector(vecsID);
                for (int dim = 0; dim < vec_dim; dim++) {
                    const double abs_val = std::abs(vecs[dim]);
                    assert(abs_min <= abs_val && abs_val <= abs_max);
                    const int lb = std::floor((abs_val - abs_min) / cell_dist);
                    assert(0 <= lb && lb <= lb + 1 && lb + 1 <= n_codeword_ + 1);
                    if (vecs[dim] > 0) {
                        codeword_[vecsID * vec_dim + dim] = lb;
                    } else {
                        codeword_[vecsID * vec_dim + dim] = -lb;
                    }

                }
            }

        }

        void BuildGrid(const VectorMatrix &user, const VectorMatrix &item) {
            //get the maximum and minimum absolute value of user and item
            //partition it with different size and split the inner product
            user_abs_pair_ = AbsMinMaxValue(user);
            item_abs_pair = AbsMinMaxValue(item);

            const double user_min = user_abs_pair_.first;
            const double user_max = user_abs_pair_.second;
            const double item_min = item_abs_pair.first;
            const double item_max = item_abs_pair.second;

            assert(user_min >= 0 && user_max >= 0);
            assert(item_min >= 0 && item_max >= 0);

            //record the quantized value of user and item
            user_dist_ = (user_max - user_min) / (n_codeword_ - 1);
            for (int codeID = 0; codeID < n_codeword_ + 1; codeID++) {
                user_quant_l_[codeID] = user_min + user_dist_ * codeID;
            }
//            printf("%.3f %.3f %.3f %.3f\n", user_quant_l_[0], user_min, user_quant_l_[n_codeword_ - 1], user_max);
            assert(user_quant_l_[0] == user_min && user_quant_l_[n_codeword_ - 1] == user_max);

            item_dist_ = (item_max - item_min) / (n_codeword_ - 1);
            for (int codeID = 0; codeID < n_codeword_ + 1; codeID++) {
                item_quant_l_[codeID] = item_min + item_dist_ * codeID;
            }
            assert(item_quant_l_[0] == item_min && item_quant_l_[n_codeword_ - 1] == item_max);

            //calculate the codebook
            for (int user_codeID = 0; user_codeID < n_codeword_ + 1; user_codeID++) {
                for (int item_codeID = 0; item_codeID < n_codeword_ + 1; item_codeID++) {
                    codebook_[user_codeID * (n_codeword_ + 1) + item_codeID] =
                            user_quant_l_[user_codeID] * item_quant_l_[item_codeID];
                }
            }

            //calculate the user
            CalcCodeword(user, user_abs_pair_, user_dist_, user_codeword_.get());
            CalcCodeword(item, item_abs_pair, item_dist_, item_codeword_.get());
        }

    public:
        int n_user_, n_data_item_, vec_dim_, n_codeword_;
        std::unique_ptr<double[]> user_quant_l_; // n_codeword + 1, stores the quantized value of user
        std::unique_ptr<double[]> item_quant_l_; // n_codeword + 1, stores the quantized value of item
        // (n_codeword + 1) * (n_codeword + 1), each cell stores the bound of index, it is + 1 because it should obtain the upper bound
        // first stores the user, then the item
        std::unique_ptr<double[]> codebook_;
        std::pair<double, double> user_abs_pair_, item_abs_pair;
        double user_dist_, item_dist_;
        std::unique_ptr<int[]> user_codeword_; //n_user_ * vec_dim_, the absolute maximum value is n_codeword
        std::unique_ptr<int[]> item_codeword_; //n_data_item * vec_dim_, the absolute maximum value is n_codeword

        inline GridIndex() = default;

        inline void Preprocess(const VectorMatrix &user, const VectorMatrix &item, const int &n_codeword) {
            assert(user.vec_dim_ == item.vec_dim_);
            this->n_user_ = user.n_vector_;
            this->n_data_item_ = item.n_vector_;
            this->vec_dim_ = user.vec_dim_;
            this->n_codeword_ = n_codeword;

            if (n_codeword_ <= 1) {
                spdlog::error("the codeword is too small, program exit");
                exit(-1);
            }

            user_quant_l_ = std::make_unique<double[]>(n_codeword + 1);
            item_quant_l_ = std::make_unique<double[]>(n_codeword + 1);
            codebook_ = std::make_unique<double[]>((n_codeword + 1) * (n_codeword + 1));
            user_codeword_ = std::make_unique<int[]>(n_user_ * vec_dim_);
            item_codeword_ = std::make_unique<int[]>(n_data_item_ * vec_dim_);

            BuildGrid(user, item);
        }

        void CalcIPBound(const std::vector<int> &item_candidate_l, const int &userID,
                         std::vector<std::pair<double, double>> &IPbound_l) {
            assert(IPbound_l.size() == n_data_item_);
            const int *user_code_l = user_codeword_.get() + userID * vec_dim_;
            for (int itemID = 0; itemID < n_data_item_; itemID++) {
                double lb = 0;
                double ub = 0;
                const int *item_code_l = item_codeword_.get() + itemID * vec_dim_;
                for (int dim = 0; dim < vec_dim_; dim++) {
                    const int user_code = user_code_l[dim];
                    const int item_code = item_code_l[dim];
                    bool all_positive = user_code >= 0 && item_code >= 0;
                    bool all_negative = user_code < 0 && item_code < 0;
                    const int user_abs_code = std::abs(user_code);
                    const int item_abs_code = std::abs(item_code);
                    if (all_positive || all_negative) {
                        lb += codebook_[user_abs_code * (n_codeword_ + 1) + item_abs_code];
                        ub += codebook_[(user_abs_code + 1) * (n_codeword_ + 1) + item_abs_code + 1];
                    } else { // only one of them are negative
                        ub += codebook_[user_abs_code * (n_codeword_ + 1) + item_abs_code];
                        lb += codebook_[(user_abs_code + 1) * (n_codeword_ + 1) + item_abs_code + 1];
                    }
                }
                IPbound_l[itemID] = std::make_pair(lb, ub);
            }

        }

        int RankByCandidate
                (const VectorMatrix &user, const VectorMatrix &item, const double &queryIP,
                 const std::vector<UserRankBound> &item_rank_bound_l, const int &userID,
                 const std::pair<double, double> &IPbound_pair, const std::pair<int, int> &rank_bound_pair) {
            //judge negative or positive
            //get the IP bound
            //prune
            return 1;
        }

    };

}
#endif //REVERSE_KRANKS_GRIDINDEX_HPP
