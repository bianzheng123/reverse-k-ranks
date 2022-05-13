//
// Created by BianZheng on 2022/5/11.
//

#ifndef REVERSE_K_RANKS_GRIDINDEX_HPP
#define REVERSE_K_RANKS_GRIDINDEX_HPP

#include "alg/DiskIndex/ReadAll.hpp"
#include "alg/Prune/PruneCandidateByBound.hpp"
#include "alg/Prune/HashSearch.hpp"
#include "alg/SpaceInnerProduct.hpp"
#include "struct/VectorMatrix.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/MethodBase.hpp"
#include "util/TimeMemory.hpp"
#include "util/VectorIO.hpp"
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <map>
#include <set>
#include <cassert>
#include <spdlog/spdlog.h>

namespace ReverseMIPS::GridIndex {

    class Grid {
        int n_user_, n_data_item_, vec_dim_, n_codeword_;
        // (n_codeword + 1) * (n_codeword + 1), each cell stores the bound of index, it is + 1 because it should obtain the upper bound
        // first stores the user, then the item
        std::unique_ptr<double[]> codebook_;
        std::unique_ptr<unsigned char[]> user_codeword_; //n_user_ * vec_dim_
        std::unique_ptr<unsigned char[]> item_codeword_; //n_data_item * vec_dim_
    public:
        //build IP bound, compute the IP bound of item directly,

        inline Grid() = default;

        inline Grid(const VectorMatrix &data_item, const VectorMatrix &user, const int &n_codeword) {

        }
    };

    class Index : public BaseIndex {
        void ResetTimer() {
            inner_product_time_ = 0;
            inner_product_bound_time_ = 0;
            inner_product_prune_ratio_ = 0;
        }


        Grid grid;
        VectorMatrix user_, data_item_;
        int vec_dim_, n_data_item_, n_user_;
        double inner_product_time_, inner_product_bound_time_;
        TimeRecord inner_product_record_, inner_product_bound_record_;
        double inner_product_prune_ratio_;
    public:

        //temporary retrieval variable
        std::vector<double> queryIP_l_;
        std::vector<std::pair<double, double>> queryIPbound_l_;

        Index(
                Grid &grid,
                //general retrieval
                VectorMatrix &data_item,
                VectorMatrix &user
        ) {
            this->grid = std::move(grid);
            this->data_item_ = std::move(data_item);
            this->n_data_item_ = this->data_item_.n_vector_;
            this->user_ = std::move(user);
            this->vec_dim_ = this->user_.vec_dim_;
            this->n_user_ = this->user_.n_vector_;

            //retrieval variable
            queryIP_l_.resize(n_user_);
            queryIPbound_l_.resize(n_user_);
        }

        std::vector<std::vector<UserRankElement>> Retrieval(VectorMatrix &query_item, const int &topk) override {
            ResetTimer();

            if (topk > user_.n_vector_) {
                spdlog::error("top-k is too large, program exit");
                exit(-1);
            }

            //coarse binary search
            const int n_query_item = query_item.n_vector_;

            std::vector<std::vector<UserRankElement>> query_heap_l(n_query_item);
            for (int qID = 0; qID < n_query_item; qID++) {
                query_heap_l[qID].reserve(topk);
            }

            // for binary search, check the number
            std::vector<int> rank_topk_max_heap(topk);
            for (int queryID = 0; queryID < n_query_item; queryID++) {

                //calculate IP
                double *query_item_vec = query_item.getVector(queryID);
                inner_product_record_.reset();
                for (int userID = 0; userID < n_user_; userID++) {
                    double *user_vec = user_.getVector(userID);
                    double queryIP = InnerProduct(query_item_vec, user_vec, vec_dim_);
                    queryIP_l_[userID] = queryIP;
                }
                this->inner_product_time_ += inner_product_record_.get_elapsed_time_second();

                //rank search
                inner_product_bound_record_.reset();
                rank_ins_.RankBound(queryIP_l_, prune_l_, rank_lb_l_, rank_ub_l_, queryIPbound_l_, queryID);

                PruneCandidateByBound(rank_lb_l_, rank_ub_l_,
                                      n_user_, topk,
                                      prune_l_, rank_topk_max_heap);
                inner_product_bound_time_ += inner_product_bound_record_.get_elapsed_time_second();

                int n_candidate = 0;
                for (int userID = 0; userID < n_user_; userID++) {
                    if (!prune_l_[userID]) {
                        n_candidate++;
                    }
                }
                assert(n_candidate >= topk);
                inner_product_prune_ratio_ += 1.0 * (n_user_ - n_candidate) / n_user_;

                for (int candID = 0; candID < topk; candID++) {
                    query_heap_l[queryID].emplace_back(disk_ins_.user_topk_cache_l_[candID]);
                }
                assert(query_heap_l[queryID].size() == topk);
            }

            inner_product_prune_ratio_ /= n_query_item;

            return query_heap_l;
        }

        std::string
        PerformanceStatistics(const int &topk, const double &retrieval_time, const double &second_per_query) override {
            // int topk;
            //double total_time,
            //          inner_product_time, rank bound prune, read_disk_time
            //          exact rank refinement;
            //double rank_prune_ratio;
            //double second_per_query;
            //unit: second

            char buff[1024];

            sprintf(buff,
                    "top%d retrieval time: total %.3fs\n\tinner product %.3fs, inner product bound %.3fs\n\tinner product prune ratio %.4f\n\tmillion second per query %.3fms",
                    topk, retrieval_time,
                    inner_product_time_, inner_product_bound_time_,
                    inner_product_prune_ratio_,
                    second_per_query);
            std::string str(buff);
            return str;
        }

    };

    /*
     * bruteforce index
     * shape: n_user * n_data_item, type: double, the distance pair for each user
     */

    std::unique_ptr<Index>
    BuildIndex(VectorMatrix &data_item, VectorMatrix &user) {
        user.vectorNormalize();
        assert(user.vec_dim_ == data_item.vec_dim_);

        const int min_codeword = std::floor(std::sqrt(1.0 * 80 * std::sqrt(3 * user.vec_dim_)));
        int n_codeword = 1;
        while (n_codeword < min_codeword) {
            n_codeword = n_codeword << 1;
        }
        spdlog::info("min_codeword {}, codeword {}", min_codeword, n_codeword);

        Grid grid(data_item, user, n_codeword);


        std::unique_ptr<Index> index_ptr = std::make_unique<Index>(grid, data_item, user);
        return index_ptr;
    }

}

#endif //REVERSE_K_RANKS_GRIDINDEX_HPP
