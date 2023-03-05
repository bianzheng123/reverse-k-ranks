//
// Created by BianZheng on 2022/6/27.
//

#ifndef REVERSE_KRANKS_COMPUTEALL_HPP
#define REVERSE_KRANKS_COMPUTEALL_HPP

#include "alg/SpaceInnerProduct.hpp"
#include "struct/UserRankElement.hpp"

#include <memory>
#include <spdlog/spdlog.h>

namespace ReverseMIPS {

    class ComputeAll {
        int n_data_item_, n_user_, vec_dim_;
    public:
        TimeRecord exact_rank_refinement_record_;
        double exact_rank_refinement_time_;
        uint64_t n_refine_user_;

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
            n_refine_user_ = 0;
        }

        void GetRank(const VectorMatrix &user, const VectorMatrix &data_item,
                     const std::vector<double> &queryIP_l,
                     const std::vector<bool> &prune_l, const std::vector<bool> &result_l,
                     size_t &n_compute, const int &queryID) {

            //read disk and fine binary search
            n_compute = 0;
            int n_candidate = 0;
            exact_rank_refinement_record_.reset();
//#pragma omp parallel for default(none) shared(user, userID, data_item, queryIP, base_rank)
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
//#pragma omp critical
                    if (ip > queryIP) {
                        base_rank++;
                    }

                }
                if (queryID == 0 && userID == 790) {
                    spdlog::error("found userID 790, rank {}", base_rank);
                }

                n_compute += n_data_item_;
                user_topk_cache_l_[n_candidate] = UserRankElement(userID, base_rank, queryIP);
                n_candidate++;
            }
            exact_rank_refinement_time_ += exact_rank_refinement_record_.get_elapsed_time_second();

            std::sort(user_topk_cache_l_.begin(), user_topk_cache_l_.begin() + n_candidate,
                      std::less());
            n_refine_user_ = n_candidate;
        }

    };
}
#endif //REVERSE_KRANKS_COMPUTEALL_HPP
