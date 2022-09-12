//
// Created by BianZheng on 2022/9/11.
//

#ifndef REVERSE_K_RANKS_SIMPFERRETRIEVAL_HPP
#define REVERSE_K_RANKS_SIMPFERRETRIEVAL_HPP

#include "simpfer/SimpferBuildIndex.hpp"
#include "simpfer/SimpferData.hpp"

#include "fexipro/alg/SVDIntUpperBoundIncrPrune2.h"
#include "fexipro/structs/VectorElement.h"

#include "alg/SpaceInnerProduct.hpp"
#include "util/TimeMemory.hpp"


namespace ReverseMIPS {
    class SimpferIndex {
        // verification by sequential scan
        // return value is whether contains in the reverse top-k result
        bool SequentialScan(SimpferData &user, const double &ip_, const int &rtk_topk, int64_t &ip_count) {

            for (unsigned int itemID = 0; itemID < n_data_item_; ++itemID) {
                const double item_norm_ub = user.norm_ * data_item_sd_l_[itemID].norm_;
                // guarantee to be top-k
                if (ip_ >= item_norm_ub || user.threshold_ >= item_norm_ub) {
                    return true;
                }

                // compute ip
                const double ip = InnerProduct(user.vec_.data(), data_item_sd_l_[itemID].vec_.data(), (int) vec_dim_);
                ++ip_count;

                // update top-k
                user.UpdateTopk(ip, data_item_sd_l_[itemID].ID_, rtk_topk);

                // out from top-k
                if (user.threshold_ > ip_) {
                    return false;
                }
            }
            if (user.threshold_ > ip_) {
                return false;
            } else {
                return true;
            }
        }

    public:
        int64_t n_user_, n_data_item_, vec_dim_;
        int64_t k_max_;
        std::vector<SimpferData> user_sd_l_;
        std::vector<SimpferData> data_item_sd_l_;
        std::vector<SimpferBlock> block_l_;
        SIRPrune sir_prune_;

        inline SimpferIndex() = default;

        inline SimpferIndex(std::vector<SimpferData> &user_sd_l,
                            std::vector<SimpferData> &data_item_sd_l,
                            std::vector<SimpferBlock> &block_l,
                            SIRPrune &&sirPrune,
                            const int &k_max,
                            const int &n_user, const int &n_data_item, const int &vec_dim)
                : user_sd_l_(std::move(user_sd_l)), data_item_sd_l_(std::move(data_item_sd_l)),
                  block_l_(std::move(block_l)), sir_prune_(std::move(sirPrune)) {
            this->n_user_ = n_user;
            this->n_data_item_ = n_data_item;
            this->vec_dim_ = vec_dim;
            this->k_max_ = k_max;
        }

        void ComputeByIndex(const SimpferData &query_item, const int &rtk_topk,
                            std::vector<int> &result_userID_l,
                            int &n_block_prune, int &sample_prune, int &norm_prune, int64_t &ip_count,
                            int &result_size, const int &queryID) {
            // init
            for (unsigned int userID = 0; userID < n_user_; ++userID) {
                user_sd_l_[userID].init();
            }

            // query_norm computation
            double query_norm = InnerProduct(query_item.vec_.data(), query_item.vec_.data(), (int) vec_dim_);
            query_norm = std::sqrt(query_norm);

            if (queryID == 11) {
                for (int sample_userID = 0; sample_userID < n_user_; sample_userID++) {
                    if (user_sd_l_[sample_userID].ID_ == 833) {
                        printf("queryID %d, userID %d, blockID %d\n",
                               queryID, user_sd_l_[sample_userID].ID_, user_sd_l_[sample_userID].block_id);
                    }
                }
                bool is_find = false;
                for (unsigned int blockID = 0; blockID < block_l_.size(); ++blockID) {
                    const SimpferBlock block = block_l_[blockID];
                    for (const int sample_userID: block.userID_l) {
                        if(user_sd_l_[sample_userID].ID_ == 833){
                            printf("appear userID %d, blockID %d\n", 833, blockID);
                        }
                    }
                }
                printf("n_block %ld\n", block_l_.size());
            }


            for (unsigned int blockID = 0; blockID < block_l_.size(); ++blockID) {
                // compute upper-bound in this block
                const SimpferBlock block = block_l_[blockID];
                double upperbound = user_sd_l_[block.userID_l[0]].norm_ * query_norm;
                assert(user_sd_l_[block.userID_l[0]].norm_ >= user_sd_l_[block.userID_l[1]].norm_);
                const int n_user_member = (int) block.userID_l.size();
                if (queryID == 11) {
                    for (const int sample_userID: block.userID_l) {
                        if (user_sd_l_[sample_userID].ID_ == 833) {
                            int is_prune_next = upperbound > block_l_[blockID].lowerbound_array[rtk_topk - 1];
                            printf("queryID %d, userID %d, blockID %d, is_prune_next %d\n",
                                   queryID, 833, block.identifier, is_prune_next);
                            break;
                        }
                    }
                    if (block.identifier == 52) {
                        printf("queryID %d, block ID %d, user size %ld: userID ", queryID, blockID,
                               block_l_[blockID].userID_l.size());
                        for (const int sample_userID: block_l_[blockID].userID_l) {
                            printf("%d ", user_sd_l_[sample_userID].ID_);
                        }
                        printf("\n");
                        std::fflush(stdout);
                    }
                }
                // block-level filtering
                if (upperbound > block_l_[blockID].lowerbound_array[rtk_topk - 1]) {
                    for (unsigned int memberID = 0; memberID < n_user_member; ++memberID) {
                        // get user-vec
                        SimpferData user = user_sd_l_[block.userID_l[memberID]];
                        if (queryID == 11 && user.ID_ == 833) {
                            printf("queryID %d, userID %d, contains", queryID, user.ID_);
                        }
                        // compute ip
                        const double ip = InnerProduct(query_item.vec_.data(), user.vec_.data(), (int) vec_dim_);
                        ++ip_count;
                        // comparison with lower-bound
                        if (ip > user.lowerbound_array_[rtk_topk - 1]) {
                            upperbound = user.norm_ * data_item_sd_l_[rtk_topk - 1].norm_;
                            // comparison with upper-bound
                            if (upperbound > ip) {
                                bool is_result = SequentialScan(user, ip, rtk_topk, ip_count);
                                if (is_result) {
                                    result_userID_l.emplace_back(user.ID_);
                                    result_size++;
                                }
                            } else {
                                norm_prune++;
                                result_userID_l.emplace_back(user.ID_);
                                result_size++;
                            }
                        } else {
                            sample_prune++;
                            norm_prune++;
                        }
                    }
                } else {
                    n_block_prune += n_user_member;
                    sample_prune += n_user_member;
                    norm_prune += n_user_member;
                }
            }
        }

        void ComputeByBruteforce(const SimpferData &query_item, const Matrix &user_matrix,
                                 const int &rtk_topk,
                                 std::vector<int> &result_userID_l,
                                 int64_t &ip_count,
                                 int &result_size) {

            std::vector<VectorElement> topk_res_l(n_user_ * rtk_topk);

            sir_prune_.topK(user_matrix, rtk_topk, topk_res_l, ip_count);

            for (int userID = 0; userID < n_user_; userID++) {
                const int64_t topk_start_idx = userID * rtk_topk;
                const int64_t topk_end_idx = (userID + 1) * rtk_topk;
                int64_t topk_min_id = topk_start_idx;
                double topk_min_ip = topk_res_l[topk_start_idx].data;
                for (int64_t topk_idx = topk_start_idx + 1; topk_idx < topk_end_idx; topk_idx++) {
                    if (topk_min_ip > topk_res_l[topk_idx].data) {
                        topk_min_ip = topk_res_l[topk_idx].data;
                        topk_min_id = topk_idx;
                    }
                }
                const double queryIP = InnerProduct(query_item.vec_.data(), user_matrix.getRowPtr(userID),
                                                    (int) vec_dim_);

                ip_count++;
                if (queryIP > topk_min_ip) {
                    result_size++;
                    result_userID_l.push_back(userID);
                }
            }
        }


        // main operation
        void RTopKRetrieval(const SimpferData &query_item, Matrix &user_matrix, const int &rtk_topk,
                            std::vector<int> &result_userID_l,
                            int &n_block_prune, int &sample_prune, int &norm_prune, int64_t &ip_count,
                            int &result_size, const int &queryID) {

            result_userID_l.clear();
            n_block_prune = 0;
            sample_prune = 0;
            norm_prune = 0;
            ip_count = 0;
            result_size = 0;
            assert((int) result_userID_l.size() == 0);

            if (rtk_topk > k_max_) {
                ComputeByBruteforce(query_item, user_matrix, rtk_topk,
                                    result_userID_l,
                                    ip_count,
                                    result_size);
            } else {
                ComputeByIndex(query_item, rtk_topk,
                               result_userID_l,
                               n_block_prune, sample_prune, norm_prune, ip_count,
                               result_size, queryID);

            }

        }
    };
}
#endif //REVERSE_K_RANKS_SIMPFERRETRIEVAL_HPP
