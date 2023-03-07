//
// Created by BianZheng on 2022/6/27.
//

#ifndef REVERSE_KRANKS_COMPUTEALL_HPP
#define REVERSE_KRANKS_COMPUTEALL_HPP

#include "alg/SpaceInnerProduct.hpp"
#include "struct/UserRankElement.hpp"

#include <memory>
#include <omp.h>
#include <spdlog/spdlog.h>

namespace ReverseMIPS {

    class ComputeAll {
        int n_data_item_, n_user_, vec_dim_;
    public:
        TimeRecord exact_rank_refinement_record_;
        double exact_rank_refinement_time_;

        std::vector<UserRankElement> user_topk_cache_l_;

        inline ComputeAll() = default;

        inline ComputeAll(const int &n_user, const int &n_data_item, const int &vec_dim) {
            this->n_user_ = n_user;
            this->n_data_item_ = n_data_item;
            this->vec_dim_ = vec_dim;
            this->user_topk_cache_l_.resize(n_user);
        }

        void RetrievalPreprocess() {
            exact_rank_refinement_time_ = 0;
        }

        void GetRank(const VectorMatrix &user, const VectorMatrix &data_item,
                     const std::vector<double> &queryIP_l,
                     const std::vector<bool> &prune_l, const std::vector<bool> &result_l,
                     const int &n_remain_result, size_t &n_compute, int &n_refine_user) {

            //read disk and fine binary search
            n_compute = 0;
            n_refine_user = 0;
            if (n_remain_result == 0) {
                return;
            }

            int n_candidate = 0;
            exact_rank_refinement_record_.reset();
            const int n_thread = omp_get_num_procs();
#pragma omp parallel for default(none) shared(user, prune_l, result_l, queryIP_l, data_item, n_compute, n_candidate) num_threads(n_thread)
            for (int userID = 0; userID < n_user_; userID++) {
                if (prune_l[userID] || result_l[userID]) {
                    continue;
                }
                const double *user_vecs = user.getVector(userID);
                const double queryIP = queryIP_l[userID];
                int base_rank = 1;
                for (int itemID = 0; itemID < n_data_item_; itemID++) {
                    const double *item_vecs = data_item.getVector(itemID);
                    const double ip = InnerProduct(user_vecs, item_vecs, vec_dim_);
                    if (ip > queryIP) {
                        base_rank++;
                    }

                }

#pragma omp critical
                {
                    n_compute += n_data_item_;
                    user_topk_cache_l_[n_candidate] = UserRankElement(userID, base_rank, queryIP);
                    n_candidate++;
                };

            }
            exact_rank_refinement_time_ += exact_rank_refinement_record_.get_elapsed_time_second();

            std::sort(user_topk_cache_l_.begin(), user_topk_cache_l_.begin() + n_candidate,
                      std::less());
            n_refine_user = n_candidate;
        }

    };
}
#endif //REVERSE_KRANKS_COMPUTEALL_HPP
