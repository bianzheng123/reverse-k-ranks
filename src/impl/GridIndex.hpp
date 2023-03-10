//
// Created by BianZheng on 2022/5/11.
//

#ifndef REVERSE_K_RANKS_GRIDINDEX_HPP
#define REVERSE_K_RANKS_GRIDINDEX_HPP

#include "alg/Grid.hpp"
#include "alg/SpaceInnerProduct.hpp"
#include "alg/SVD.hpp"
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

    class Index : public BaseIndex {
        void ResetTimer() {
            total_retrieval_time_ = 0;
            inner_product_time_ = 0;
            inner_product_bound_time_ = 0;
            early_prune_ratio_ = 0;
            total_ip_cost_ = 0;
            total_appr_ip_cost_ = 0;
        }

        std::unique_ptr<Grid> ip_bound_ins_;

        VectorMatrix user_, data_item_;
        int vec_dim_, n_data_item_, n_user_;
        size_t stop_time_;
        double total_retrieval_time_, inner_product_time_, inner_product_bound_time_;
        TimeRecord total_retrieval_record_, inner_product_record_, inner_product_bound_record_;
        size_t total_ip_cost_, total_appr_ip_cost_;
        double early_prune_ratio_;
    public:

        //temporary retrieval variable
        std::vector<double> queryIP_l_;
        std::vector<int> item_cand_l_;
        std::unique_ptr<double[]> query_ptr_;

        Index(
                //ip_bound_ins
                std::unique_ptr<Grid> &ip_bound_ins,
                //general retrieval
                VectorMatrix &data_item,
                VectorMatrix &user,
                const size_t &stop_time
        ) {
            this->ip_bound_ins_ = std::move(ip_bound_ins);

            this->data_item_ = std::move(data_item);
            this->n_data_item_ = this->data_item_.n_vector_;
            this->user_ = std::move(user);
            this->vec_dim_ = this->user_.vec_dim_;
            this->n_user_ = this->user_.n_vector_;
            this->stop_time_ = stop_time;

            //retrieval variable
            queryIP_l_.resize(n_user_);
            item_cand_l_.resize(n_data_item_);
            query_ptr_ = std::make_unique<double[]>(vec_dim_);
        }

        std::vector<std::vector<UserRankElement>>
        Retrieval(const VectorMatrix &query_item, const int &topk, const int &n_execute_query,
                  std::vector<SingleQueryPerformance> &query_performance_l) override {
            ResetTimer();

            if (n_execute_query > query_item.n_vector_) {
                spdlog::error("n_execute_query larger than n_query_item, program exit");
                exit(-1);
            }

            if (topk > user_.n_vector_) {
                spdlog::error("top-k is too large, program exit");
                exit(-1);
            }

            const int report_every = 5000;
            //coarse binary search
            const int n_query_item = n_execute_query;

            std::vector<std::vector<UserRankElement>> query_heap_l(n_query_item, std::vector<UserRankElement>(topk));
            int n_proc_query = 0;

            for (int queryID = 0; queryID < n_query_item; queryID++) {
                n_proc_query++;
                total_retrieval_record_.reset();
                double *query_vecs = query_ptr_.get();
                ip_bound_ins_->PreprocessQuery(query_item.getVector(queryID), vec_dim_, query_vecs);

                size_t this_ip_cost = 0;
                size_t this_appr_ip_cost = 0;
                //calculate IP
                inner_product_record_.reset();
                for (int userID = 0; userID < n_user_; userID++) {
                    double *user_vec = user_.getVector(userID);
                    double queryIP = InnerProduct(query_vecs, user_vec, vec_dim_);
                    queryIP_l_[userID] = queryIP;
                }
                inner_product_time_ += inner_product_record_.get_elapsed_time_second();
                this_ip_cost += n_user_;

                //rank search
                inner_product_bound_record_.reset();
                int early_prune_candidate = 0;
                std::vector<UserRankElement> &rank_max_heap = query_heap_l[queryID];
                for (int userID = 0; userID < topk; userID++) {
                    const double *user_vecs = user_.getVector(userID);
                    int rank = GetRank(queryIP_l_[userID], n_data_item_ + 1, userID, user_vecs, data_item_,
                                       this_ip_cost, this_appr_ip_cost);

                    assert(rank != -1);
                    rank_max_heap[userID] = UserRankElement(userID, rank, queryIP_l_[userID]);
                }

                std::make_heap(rank_max_heap.begin(), rank_max_heap.end(), std::less());

                UserRankElement heap_ele = rank_max_heap.front();
                for (int userID = topk; userID < n_user_; userID++) {
                    const double *user_vecs = user_.getVector(userID);
                    int rank = GetRank(queryIP_l_[userID], heap_ele.rank_, userID, user_vecs, data_item_,
                                       this_ip_cost, this_appr_ip_cost);

                    if (userID % report_every == 0) {
                        spdlog::info(
                                "queryID {}, userID {}, query_ip_cost {}, query_appr_ip_cost {}, time_used {:.2f}s",
                                queryID, userID, this_ip_cost, this_appr_ip_cost,
                                inner_product_bound_record_.get_elapsed_time_second());
                    }

                    if (rank == -1) {
                        early_prune_candidate++;
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
                const double this_query_inner_product_time = inner_product_bound_record_.get_elapsed_time_second();
                inner_product_bound_time_ += this_query_inner_product_time;
                const double this_query_retrieval_time = total_retrieval_record_.get_elapsed_time_second();
                total_retrieval_time_ += this_query_retrieval_time;
                const double this_query_prune_ratio = early_prune_candidate * 1.0 / n_user_;
                early_prune_ratio_ += this_query_prune_ratio;

                total_ip_cost_ += this_ip_cost;
                total_appr_ip_cost_ += this_appr_ip_cost;

                spdlog::info(
                        "queryID {}, query_time {:.2f}s, total_retrieval_time {:.2f}s, early_prune_ratio {:.2f}, total_ip_cost {}, total_appr_ip_cost {}",
                        queryID, this_query_inner_product_time, this_query_retrieval_time,
                        this_query_prune_ratio,
                        total_ip_cost_, total_appr_ip_cost_);
                if (total_retrieval_time_ > (double) stop_time_) {
                    spdlog::info("total retrieval time larger than stop time, retrieval exit");
                    break;
                }
            }

            early_prune_ratio_ /= n_proc_query;

            return query_heap_l;
        }

        int GetRank(const double &queryIP, const int &min_rank, const int &userID, const double *user_vecs,
                    const VectorMatrix &item, size_t &total_ip_cost, size_t &total_appr_ip_cost) {
            int rank = 1;
            int n_cand_ = 0;
            for (int itemID = 0; itemID < n_data_item_; itemID++) {
                const double *item_vecs = item.getVector(itemID);
                const double IP_lb = ip_bound_ins_->IPLowerBound(user_vecs, userID, item_vecs, itemID);
                total_appr_ip_cost++;
                assert(IP_lb <= InnerProduct(user_vecs, item_vecs, vec_dim_));
                if (queryIP < IP_lb) {
                    rank++;
                    if (rank > min_rank) {
                        return -1;
                    }
                } else {
                    const double IP_ub = ip_bound_ins_->IPUpperBound(user_vecs, userID, item_vecs, itemID);
                    total_appr_ip_cost++;
                    assert(InnerProduct(user_vecs, item_vecs, vec_dim_) <= IP_ub);
                    if (IP_lb <= queryIP && queryIP <= IP_ub) {
                        item_cand_l_[n_cand_] = itemID;
                        n_cand_++;
                    }
                }
            }

            for (int candID = 0; candID < n_cand_; candID++) {
                const int itemID = item_cand_l_[candID];
                const double *item_vecs = item.getVector(itemID);
                const double IP = InnerProduct(user_vecs, item_vecs, vec_dim_);
                total_ip_cost++;
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

        std::string
        PerformanceStatistics(const int &topk) override {
            // int topk;
            //double total_time,
            //          inner_product_time, inner_product_bound_time
            //double early_prune_ratio_
            //unit: second

            char buff[1024];

            sprintf(buff,
                    "top%d retrieval time: total %.3fs\n\tinner product time %.3fs, inner product bound time %.3fs\n\tearly prune ratio %.4f, total IP cost %ld, total appr IP cost %ld",
                    topk, total_retrieval_time_,
                    inner_product_time_, inner_product_bound_time_,
                    early_prune_ratio_, total_ip_cost_, total_appr_ip_cost_);
            std::string str(buff);
            return str;
        }

        uint64_t IndexSizeByte() override {
            return ip_bound_ins_->IndexSize();
        }


    };

    /*
     * bruteforce index
     * shape: n_user * n_data_item, type: double, the distance pair for each user
     */

    std::unique_ptr<Index>
    BuildIndex(VectorMatrix &data_item, VectorMatrix &user, const size_t &stop_time) {
        user.vectorNormalize();
        assert(user.vec_dim_ == data_item.vec_dim_);

        std::unique_ptr<Grid> IPbound_ptr;

        int n_user = user.n_vector_;
        int n_data_item = data_item.n_vector_;
        int vec_dim = user.vec_dim_;

        const int min_codeword = std::floor(std::sqrt(1.0 * 80 * std::sqrt(3 * user.vec_dim_)));
        int n_codeword = 1;
        while (n_codeword < min_codeword) {
            n_codeword = n_codeword << 1;
        }
        spdlog::info("GridIndex min_codeword {}, codeword {}", min_codeword, n_codeword);

        IPbound_ptr = std::make_unique<Grid>(n_user, n_data_item, vec_dim, n_codeword);
        IPbound_ptr->Preprocess(user, data_item);

        std::unique_ptr<Index> index_ptr = std::make_unique<Index>(IPbound_ptr, data_item, user, stop_time);
        return index_ptr;
    }

}

#endif //REVERSE_K_RANKS_GRIDINDEX_HPP
