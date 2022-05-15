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
#include <cfloat>
#include <cassert>
#include <spdlog/spdlog.h>

namespace ReverseMIPS::GridIndex {

    //compute the min value and max value that the number > 0
    std::pair<double, double> MinMaxValue(const VectorMatrix &vm) {
        const int &n_vector = vm.n_vector_;
        const int &vec_dim = vm.vec_dim_;
        double min_val = DBL_MAX;
        double max_val = 0;
        for (int vecsID = 0; vecsID < n_vector; vecsID++) {
            const double *vecs = vm.getVector(vecsID);
            for (int dim = 0; dim < vec_dim; dim++) {
                double vecs_val = vecs[dim];
                if (vecs_val >= 0) {
                    min_val = std::min(vecs_val, min_val);
                    max_val = std::max(vecs_val, max_val);
                }
            }
        }
        assert(min_val <= max_val);
        return std::make_pair(min_val, max_val);
    }

    class Grid {
        int n_user_, n_data_item_, vec_dim_, n_partition_;
        // n_partition_ * n_partition_, each cell stores the IP lower_bound and upper_bound index
        // first dimension is the user, second dimension is the item
        std::unique_ptr<std::pair<double, double>[]> grid_;
        std::unique_ptr<unsigned char[]> user_codeword_; //n_user_ * vec_dim_
        std::unique_ptr<unsigned char[]> item_codeword_; //n_data_item * vec_dim_

        //retrieval
        std::vector<int> cand_l_;
        int n_cand_;

    public:

        inline Grid() = default;

        inline Grid(const VectorMatrix &data_item, const VectorMatrix &user, const int &n_partition) {
            this->n_user_ = user.n_vector_;
            this->vec_dim_ = user.vec_dim_;
            this->n_data_item_ = data_item.n_vector_;
            this->n_partition_ = n_partition;
            if (n_partition >= (2 << 8)) {
                spdlog::error("n_partition too large, program exit");
                exit(-1);
            }

            grid_ = std::make_unique<std::pair<double, double>[]>(n_partition_ * n_partition_);
            user_codeword_ = std::make_unique<unsigned char[]>(n_user_ * vec_dim_);
            item_codeword_ = std::make_unique<unsigned char[]>(n_data_item_ * vec_dim_);
            cand_l_.resize(n_data_item_);
            Preprocess(data_item, user);
        }

        void Preprocess(const VectorMatrix &data_item, const VectorMatrix &user) {
            //get the minimum / maximum value of user and item
            std::pair<double, double> item_minmax_pair = MinMaxValue(data_item);
            std::pair<double, double> user_minmax_pair = MinMaxValue(user);
            //get the distance of user / item
            const double user_dist = (user_minmax_pair.second - user_minmax_pair.first) / n_partition_;
            const double item_dist = (item_minmax_pair.second - item_minmax_pair.first) / n_partition_;
            //compute and assign the IP bound of grid
            for (int user_partID = 0; user_partID < n_partition_; user_partID++) {
                const double user_lb = user_minmax_pair.first + user_dist * user_partID;
                const double user_ub = user_minmax_pair.first + user_dist * (user_partID + 1);
                for (int item_partID = 0; item_partID < n_partition_; item_partID++) {
                    const double item_lb = item_minmax_pair.first + item_dist * item_partID;
                    const double item_ub = item_minmax_pair.first + item_dist * (item_partID + 1);
                    const double ip_lb = user_lb * item_lb;
                    const double ip_ub = user_ub * item_ub;
                    grid_[user_partID * n_partition_ + item_partID] = std::make_pair(ip_lb, ip_ub);
                }
            }
            //encode user and item
            for (int userID = 0; userID < n_user_; userID++) {
                const double *user_vecs = user.getVector(userID);
                for (int dim = 0; dim < vec_dim_; dim++) {
                    if (user_vecs[dim] <= 0) {
                        user_codeword_[userID * vec_dim_ + dim] = 127;
                    } else { // user_vecs[dim] > 0
                        unsigned char bktID = std::floor((user_vecs[dim] - user_minmax_pair.first) / user_dist);
                        assert(0 <= bktID && bktID < n_partition_);
                        user_codeword_[userID * vec_dim_ + dim] = bktID;
                    }
                }
            }

            for (int itemID = 0; itemID < n_data_item_; itemID++) {
                const double *item_vecs = data_item.getVector(itemID);
                for (int dim = 0; dim < vec_dim_; dim++) {
                    if (item_vecs[dim] <= 0) {
                        item_codeword_[itemID * vec_dim_ + dim] = 127;
                    } else { // item_vecs[dim] > 0
                        unsigned char bktID = std::floor((item_vecs[dim] - item_minmax_pair.first) / item_dist);
                        assert(0 <= bktID && bktID < n_partition_);
                        item_codeword_[itemID * vec_dim_ + dim] = bktID;
                    }
                }
            }

        }

        int GetRank(const double &queryIP, const int &min_rank, const int &userID, const double *user_vecs,
                    const VectorMatrix &item) {
            int rank = 1;
            n_cand_ = 0;
            for (int itemID = 0; itemID < n_data_item_; itemID++) {
                const double *item_vecs = item.getVector(itemID);
                const double IP_ub = IPUpperBound(user_vecs, userID, item_vecs, itemID);
                if (IP_ub < queryIP) {
                    rank++;
                    if (rank > min_rank) {
                        return -1;
                    }
                } else {
                    const double IP_lb = IPLowerBound(user_vecs, userID, item_vecs, itemID);
                    if (IP_lb <= queryIP && queryIP <= IP_ub) {
                        cand_l_[n_cand_] = itemID;
                        n_cand_++;
                    }
                }
            }

            for (int candID = 0; candID < n_cand_; candID++) {
                const int itemID = cand_l_[candID];
                const double *item_vecs = item.getVector(itemID);
                const double IP = InnerProduct(user_vecs, item_vecs, vec_dim_);
                if (IP > queryIP) {
                    rank++;
                    if (rank > min_rank) {
                        return -1;
                    }
                }
            }
            if (rank > min_rank) {
                return -1;
            }
            return rank;
        }

        double IPUpperBound(const double *user_vecs, const int &userID, const double *item_vecs, const int &itemID) {
            const unsigned char *user_code_ptr = user_codeword_.get() + userID * vec_dim_;
            const unsigned char *item_code_ptr = item_codeword_.get() + itemID * vec_dim_;
            double IP_ub = 0;
            for (int dim = 0; dim < vec_dim_; dim++) {
                if (user_code_ptr[dim] == 127 || item_code_ptr[dim] == 127) {
                    double IP = user_vecs[dim] * item_vecs[dim];
                    IP_ub += IP;
                } else {
                    const unsigned char user_code = user_code_ptr[dim];
                    const unsigned char item_code = item_code_ptr[dim];
                    const std::pair<double, double> bound_pair = grid_[user_code * n_partition_ + item_code];
                    IP_ub += bound_pair.second;
                }
            }
            return IP_ub;
        }

        double IPLowerBound(const double *user_vecs, const int &userID, const double *item_vecs, const int &itemID) {
            const unsigned char *user_code_ptr = user_codeword_.get() + userID * vec_dim_;
            const unsigned char *item_code_ptr = item_codeword_.get() + itemID * vec_dim_;
            double IP_lb = 0;
            for (int dim = 0; dim < vec_dim_; dim++) {
                if (user_code_ptr[dim] == 127 || item_code_ptr[dim] == 127) {
                    double IP = user_vecs[dim] * item_vecs[dim];
                    IP_lb += IP;
                } else {
                    const unsigned char user_code = user_code_ptr[dim];
                    const unsigned char item_code = item_code_ptr[dim];
                    const std::pair<double, double> bound_pair = grid_[user_code * n_partition_ + item_code];
                    IP_lb += bound_pair.first;
                }
            }
            return IP_lb;
        }

    };

    class Index : public BaseIndex {
        void ResetTimer() {
            inner_product_time_ = 0;
            inner_product_bound_time_ = 0;
            inner_product_prune_ratio_ = 0;
        }


        Grid grid_;
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
            this->grid_ = std::move(grid);
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

            std::vector<std::vector<UserRankElement>> query_heap_l(n_query_item, std::vector<UserRankElement>());

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
                std::vector<UserRankElement> &rank_max_heap = query_heap_l[queryID];
                for (int userID = 0; userID < topk; userID++) {
                    const double *user_vecs = user_.getVector(userID);
                    int rank = grid_.GetRank(queryIP_l_[userID], n_data_item_, userID, user_vecs, data_item_);
                    assert(rank != -1);
                    rank_max_heap[userID] = UserRankElement(userID, rank, queryIP_l_[userID]);
                }

                std::make_heap(rank_max_heap.begin(), rank_max_heap.end(), std::less());

                UserRankElement heap_ele = rank_max_heap.front();
                for (int userID = topk; userID < n_user_; userID++) {
                    const double *user_vecs = user_.getVector(userID);
                    int rank = grid_.GetRank(queryIP_l_[userID], heap_ele.rank_, userID, user_vecs, data_item_);
                    if (rank == -1) {
                        continue;
                    }

                    UserRankElement element(userID, rank, queryIP_l_[userID]);
                    if (heap_ele > element) {
                        std::pop_heap(rank_max_heap.begin(), rank_max_heap.end(), std::less());
                        rank_max_heap.pop_back();
                        rank_max_heap.push_back(element);
                        std::push_heap(rank_max_heap.begin(), rank_max_heap.end(), std::less());
                        heap_ele = rank_max_heap.front();
                    }

                }
                std::make_heap(rank_max_heap.begin(), rank_max_heap.end(), std::less());
                std::sort_heap(rank_max_heap.begin(), rank_max_heap.end(), std::less());

                inner_product_bound_time_ += inner_product_bound_record_.get_elapsed_time_second();
            }

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
