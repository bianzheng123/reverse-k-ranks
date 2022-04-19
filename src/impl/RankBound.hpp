//
// Created by BianZheng on 2022/2/25.
//

#ifndef REVERSE_KRANKS_RANKBOUND_HPP
#define REVERSE_KRANKS_RANKBOUND_HPP

#include "alg/DiskIndex/ReadAll.hpp"
#include "alg/Prune/PruneCandidateByBound.hpp"
#include "alg/SpaceInnerProduct.hpp"
#include "struct/VectorMatrix.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/MethodBase.hpp"
#include "util/TimeMemory.hpp"
#include "util/VectorIO.hpp"
#include "alg/Prune/RankSearch.hpp"
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <map>
#include <set>
#include <cassert>
#include <spdlog/spdlog.h>

namespace ReverseMIPS::RankBound {

    class RetrievalResult : public RetrievalResultBase {
    public:
        //unit: second
        //double total_time, read_disk_time, inner_product_time,
        //          coarse_binary_search_time, fine_binary_search_time;
        //double rank_prune_ratio;
        //double second_per_query;
        //int topk;

        inline RetrievalResult() = default;

        void AddPreprocess(double build_index_time) {
            char buff[1024];
            sprintf(buff, "build index time %.3f", build_index_time);
            std::string str(buff);
            this->config_l.emplace_back(str);
        }

        std::string AddResultConfig(const int &topk, const double &total_time,
                                    const double &inner_product_time, const double &coarse_binary_search_time,
                                    const double &rank_prune_time,
                                    const double &read_disk_time, const double &fine_binary_search_time,
                                    const double &rank_prune_ratio,
                                    const double &second_per_query) {
            char buff[1024];

            sprintf(buff,
                    "top%d retrieval time: total %.3fs\n\tinner product %.3fs, coarse binary search %.3fs\n\trank_prune_time %.3fs, read disk %.3fs, fine binary search %.3fs\n\trank prune ratio %.7f, million second per query %.3fms",
                    topk, total_time, inner_product_time, coarse_binary_search_time,
                    rank_prune_time, read_disk_time, fine_binary_search_time,
                    rank_prune_ratio, second_per_query);
            std::string str(buff);
            this->config_l.emplace_back(str);
            return str;
        }

    };

    class Index : public BaseIndex {
        void ResetTimer() {
            inner_product_time_ = 0;
            coarse_binary_search_time_ = 0;
            rank_prune_time_ = 0;
            read_disk_time_ = 0;
            fine_binary_search_time_ = 0;
            rank_prune_ratio_ = 0;
        }

    public:
        //rank search
        RankSearch rank_ins_;
        //read disk
        ReadAll disk_ins_;

        VectorMatrix user_;
        int vec_dim_, n_data_item_, n_user_;
        double inner_product_time_, coarse_binary_search_time_, rank_prune_time_, read_disk_time_, fine_binary_search_time_;
        TimeRecord inner_product_record_, coarse_binary_search_record_, rank_prune_record_;
        double rank_prune_ratio_;

        //temporary retrieval variable
        // store queryIP
        std::vector<std::pair<double, double>> IPbound_l_;
        std::vector<double> queryIP_l_;
        std::vector<int> rank_lb_l_;
        std::vector<int> rank_ub_l_;
        std::vector<bool> prune_l_;

        Index(//rank search
                RankSearch &rank_ins,
                //disk index
                ReadAll &disk_ins,
                //general retrieval
                VectorMatrix &user, const int &n_data_item
        ) {
            //rank search
            this->rank_ins_ = std::move(rank_ins);
            //read disk
            this->disk_ins_ = std::move(disk_ins);

            this->user_ = std::move(user);
            this->vec_dim_ = this->user_.vec_dim_;
            this->n_user_ = this->user_.n_vector_;
            this->n_data_item_ = n_data_item;

            //retrieval variable
            IPbound_l_.resize(n_user_);
            queryIP_l_.resize(n_user_);
            rank_lb_l_.resize(n_user_);
            rank_ub_l_.resize(n_user_);
            prune_l_.resize(n_user_);
        }

        std::vector<std::vector<UserRankElement>> Retrieval(VectorMatrix &query_item, const int &topk) override {
            ResetTimer();
            disk_ins_.RetrievalPreprocess();

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
                prune_l_.assign(n_user_, false);
                rank_lb_l_.assign(n_user_, n_data_item_);
                rank_ub_l_.assign(n_user_, 0);

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
                coarse_binary_search_record_.reset();
                rank_ins_.RankBound(queryIP_l_, topk, rank_lb_l_, rank_ub_l_, IPbound_l_, prune_l_, rank_topk_max_heap);
                coarse_binary_search_time_ += coarse_binary_search_record_.get_elapsed_time_second();

                rank_prune_record_.reset();
                PruneCandidateByBound(rank_lb_l_, rank_ub_l_,
                                      n_user_, topk,
                                      prune_l_, rank_topk_max_heap);
                rank_prune_time_ += rank_prune_record_.get_elapsed_time_second();

                int n_candidate = 0;
                for (int userID = 0; userID < n_user_; userID++) {
                    if (!prune_l_[userID]) {
                        n_candidate++;
                    }
                }
                assert(n_candidate >= topk);
                rank_prune_ratio_ += 1.0 * (n_user_ - n_candidate) / n_user_;

                //read disk and fine binary search
                disk_ins_.GetRank(queryIP_l_, rank_lb_l_, rank_ub_l_, prune_l_);

                for (int candID = 0; candID < topk; candID++) {
                    query_heap_l[queryID].emplace_back(disk_ins_.user_topk_cache_l_[candID]);
                }
                assert(query_heap_l[queryID].size() == topk);
            }
            disk_ins_.FinishRetrieval();

            read_disk_time_ = disk_ins_.read_disk_time_;
            fine_binary_search_time_ = disk_ins_.fine_binary_search_time_;

            rank_prune_ratio_ /= n_query_item;

            return query_heap_l;
        }

    };

    const int write_every_ = 100;
    const int report_batch_every_ = 100;

    /*
     * bruteforce index
     * shape: n_user * n_data_item, type: double, the distance pair for each user
     */

    Index &
    BuildIndex(VectorMatrix &data_item, VectorMatrix &user, const char *index_path, const int &cache_bound_every) {
        const int n_user = user.n_vector_;
        const int n_data_item = data_item.n_vector_;
        std::vector<double> write_distance_cache(write_every_ * n_data_item);
        const int vec_dim = data_item.vec_dim_;
        const int n_batch = user.n_vector_ / write_every_;
        const int n_remain = user.n_vector_ % write_every_;

        user.vectorNormalize();

        //rank search
        RankSearch rank_ins(cache_bound_every, n_data_item, n_user);
        //disk index
        ReadAll disk_ins(n_user, n_data_item, index_path, rank_ins.n_max_disk_read_);

        TimeRecord batch_report_record;
        batch_report_record.reset();
        for (int i = 0; i < n_batch; i++) {
#pragma omp parallel for default(none) shared(i, data_item, user, write_distance_cache, rank_ins) shared(write_every_, n_data_item, vec_dim)
            for (int cacheID = 0; cacheID < write_every_; cacheID++) {
                int userID = write_every_ * i + cacheID;
                for (int itemID = 0; itemID < n_data_item; itemID++) {
                    double ip = InnerProduct(data_item.getVector(itemID), user.getVector(userID), vec_dim);
                    write_distance_cache[cacheID * n_data_item + itemID] = ip;
                }
                std::sort(write_distance_cache.begin() + cacheID * n_data_item,
                          write_distance_cache.begin() + (cacheID + 1) * n_data_item, std::greater());

                //rank search
                const double *distance_ptr = write_distance_cache.data() + cacheID * n_data_item;
                rank_ins.LoopPreprocess(distance_ptr, userID);
            }
            disk_ins.BuildIndexLoop(write_distance_cache, write_every_);

            if (i % report_batch_every_ == 0) {
                std::cout << "preprocessed " << i / (0.01 * n_batch) << " %, "
                          << batch_report_record.get_elapsed_time_second() << " s/iter" << " Mem: "
                          << get_current_RSS() / 1000000 << " Mb \n";
                batch_report_record.reset();
            }

        }

        {
            for (int cacheID = 0; cacheID < n_remain; cacheID++) {
                int userID = write_every_ * n_batch + cacheID;
                for (int itemID = 0; itemID < data_item.n_vector_; itemID++) {
                    double ip = InnerProduct(data_item.getRawData() + itemID * vec_dim,
                                             user.getRawData() + userID * vec_dim, vec_dim);
                    write_distance_cache[cacheID * data_item.n_vector_ + itemID] = ip;
                }

                std::sort(write_distance_cache.begin() + cacheID * n_data_item,
                          write_distance_cache.begin() + (cacheID + 1) * n_data_item, std::greater());

                //rank search
                const double *distance_ptr = write_distance_cache.data() + cacheID * n_data_item;
                rank_ins.LoopPreprocess(distance_ptr, userID);
            }

            disk_ins.BuildIndexLoop(write_distance_cache, n_remain);
        }

        static Index index(rank_ins, disk_ins, user, n_data_item);
        return index;
    }

}

#endif //REVERSE_KRANKS_RANKBOUND_HPP
