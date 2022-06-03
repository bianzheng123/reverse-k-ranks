//
// Created by BianZheng on 2022/4/19.
//

#ifndef REVERSE_KRANKS_IRBMERGERANKBOUND_HPP
#define REVERSE_KRANKS_IRBMERGERANKBOUND_HPP

#include "alg/DiskIndex/MergeRankBound.hpp"
#include "alg/PruneUser/HashSearch.hpp"
#include "alg/PruneUser/PruneCandidateByBound.hpp"
#include "alg/SpaceInnerProduct.hpp"
#include "alg/SVD.hpp"
#include "struct/VectorMatrix.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/MethodBase.hpp"
#include "struct/DistancePair.hpp"
#include "util/TimeMemory.hpp"
#include "util/VectorIO.hpp"
#include "util/FileIO.hpp"
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <map>
#include <set>
#include <cassert>
#include <spdlog/spdlog.h>

namespace ReverseMIPS::HRBMergeRankBound {

    class Index : public BaseIndex {
        void ResetTimer() {
            read_disk_time_ = 0;
            inner_product_time_ = 0;
            rank_bound_refinement_time_ = 0;
            exact_rank_refinement_time_ = 0;
            rank_search_prune_ratio_ = 0;
        }

    public:
        //for rank search, store in memory
        HashSearch rank_ins_;
        //read all instance
        MergeRankBound disk_ins_;

        VectorMatrix user_, data_item_;
        int vec_dim_, n_data_item_, n_user_;
        double inner_product_time_, rank_bound_refinement_time_, read_disk_time_, exact_rank_refinement_time_;
        TimeRecord inner_product_record_, rank_bound_refinement_record_;
        double rank_search_prune_ratio_;

        //temporary retrieval variable
        std::vector<bool> prune_l_;
        std::vector<std::pair<double, double>> queryIPbound_l_;
        std::vector<double> queryIP_l_;
        std::vector<int> rank_lb_l_;
        std::vector<int> rank_ub_l_;

        Index(
                // hash search
                HashSearch &rank_ins,
                //disk index
                MergeRankBound &disk_ins,
                //general retrieval
                VectorMatrix &user, VectorMatrix &data_item) {
            //hash search
            this->rank_ins_ = std::move(rank_ins);
            //read disk
            this->disk_ins_ = std::move(disk_ins);
            //general retrieval
            this->n_user_ = user.n_vector_;
            this->vec_dim_ = user.vec_dim_;
            this->user_ = std::move(user);
            this->n_data_item_ = data_item.n_vector_;
            this->data_item_ = std::move(data_item);
            assert(0 < this->user_.vec_dim_);

            //retrieval variable
            this->prune_l_.resize(n_user_);
            this->queryIPbound_l_.resize(n_user_);
            this->queryIP_l_.resize(n_user_);
            this->rank_lb_l_.resize(n_user_);
            this->rank_ub_l_.resize(n_user_);

        }

        std::vector<std::vector<UserRankElement>> Retrieval(const VectorMatrix &query_item, const int &topk) override {
            ResetTimer();
            disk_ins_.RetrievalPreprocess();

            if (topk > user_.n_vector_) {
                spdlog::error("top-k is too large, program exit");
                exit(-1);
            }

            const int n_query_item = query_item.n_vector_;
            std::vector<std::vector<UserRankElement>> query_heap_l(n_query_item);
            for (int qID = 0; qID < n_query_item; qID++) {
                query_heap_l[qID].resize(topk);
            }

            // store queryIP
            std::vector<int> rank_topk_max_heap(topk);
            for (int queryID = 0; queryID < n_query_item; queryID++) {
                prune_l_.assign(n_user_, false);
                rank_lb_l_.assign(n_user_, n_data_item_);
                rank_ub_l_.assign(n_user_, 0);

                double *query_vecs = query_item.getVector(queryID);

                //calculate the exact IP
                inner_product_record_.reset();
                for (int userID = 0; userID < n_user_; userID++) {
                    if (prune_l_[userID]) {
                        continue;
                    }
                    queryIP_l_[userID] = InnerProduct(user_.getVector(userID), query_vecs, vec_dim_);
                }
                this->inner_product_time_ += inner_product_record_.get_elapsed_time_second();

                //rank bound refinement
                rank_bound_refinement_record_.reset();
                rank_ins_.RankBound(queryIP_l_, prune_l_, rank_lb_l_, rank_ub_l_, queryIPbound_l_);
                PruneCandidateByBound(rank_lb_l_, rank_ub_l_,
                                      n_user_, topk,
                                      prune_l_, rank_topk_max_heap);
                rank_bound_refinement_time_ += rank_bound_refinement_record_.get_elapsed_time_second();
                int n_candidate = 0;
                for (int userID = 0; userID < n_user_; userID++) {
                    if (!prune_l_[userID]) {
                        n_candidate++;
                    }
                }
                assert(n_candidate >= topk);
                rank_search_prune_ratio_ += 1.0 * (n_user_ - n_candidate) / n_user_;

                //read disk and fine binary search
                disk_ins_.GetRank(queryIP_l_, rank_lb_l_, rank_ub_l_, queryIPbound_l_, prune_l_, user_, data_item_);

                for (int candID = 0; candID < topk; candID++) {
                    query_heap_l[queryID][candID] = disk_ins_.user_topk_cache_l_[candID];
                }
                assert(query_heap_l[queryID].size() == topk);
            }
            disk_ins_.FinishRetrieval();

            exact_rank_refinement_time_ = disk_ins_.exact_rank_refinement_time_;
            read_disk_time_ = disk_ins_.read_disk_time_;

            rank_search_prune_ratio_ /= n_query_item;
            return query_heap_l;
        }

        std::string
        PerformanceStatistics(const int &topk, const double &retrieval_time, const double &second_per_query) override {
            // int topk;
            //double total_time,
            //          inner_product_time, rank_bound_refinement_time_
            //          read_disk_time_, exact_rank_refinement_time_,
            //          rank_search_prune_ratio_
            //double second_per_query;
            //unit: second

            char buff[1024];
            sprintf(buff,
                    "top%d retrieval time:\n\ttotal %.3fs\n\tinner product %.3fs, coarse binary search %.3fs\n\tread disk time %.3fs, exact rank refinement %.3fs\n\trank search prune ratio %.4f\n\tmillion second per query %.3fms",
                    topk, retrieval_time,
                    inner_product_time_, rank_bound_refinement_time_,
                    read_disk_time_, exact_rank_refinement_time_,
                    rank_search_prune_ratio_,
                    second_per_query);
            std::string str(buff);
            return str;
        }

    };

    const int report_batch_every = 100;

    /*
     * bruteforce index
     * shape: n_user * n_data_item, type: double, the distance pair for each user
     */

    std::unique_ptr<Index> BuildIndex(VectorMatrix &data_item, VectorMatrix &user, const char *index_path,
                                      const int &cache_bound_every, const int &n_interval, const int &topt_perc) {
        const int n_data_item = data_item.n_vector_;
        const int vec_dim = data_item.vec_dim_;
        const int n_user = user.n_vector_;

        if (not(0 < topt_perc && topt_perc <= 100)) {
            spdlog::error("topt_perc too small, program exit");
            exit(-1);
        }
        const int n_merge_user = int(n_user * 1.0 / 100 * topt_perc);

        user.vectorNormalize();

        //rank search
        HashSearch rank_ins(n_data_item, n_user, cache_bound_every, n_interval);

        //exact rank refinement
        CandidateBruteForce exact_rank_ins(n_data_item, vec_dim);

        //disk index
        MergeRankBound disk_ins(exact_rank_ins, user, n_data_item, index_path, n_merge_user);
        std::vector<std::vector<int>> &eval_seq_l = disk_ins.BuildIndexMergeUser();
        assert(eval_seq_l.size() == n_merge_user);

        std::vector<UserRankBound> merge_user_list(n_data_item);

        std::vector<DistancePair> distance_pair_l(n_data_item);
        TimeRecord batch_report_record;
        batch_report_record.reset();
        for (int labelID = 0; labelID < n_merge_user; labelID++) {
            std::vector<int> &user_l = eval_seq_l[labelID];
            const unsigned int n_eval = user_l.size();

            for (int evalID = 0; evalID < n_eval; evalID++) {
                int userID = user_l[evalID];
#pragma omp parallel for default(none) shared(n_data_item, data_item, user, userID, vec_dim, distance_pair_l)
                for (int itemID = 0; itemID < n_data_item; itemID++) {
                    double ip = InnerProduct(data_item.getVector(itemID), user.getVector(userID), vec_dim);
                    distance_pair_l[itemID] = DistancePair(ip, itemID);
                }
                std::sort(distance_pair_l.begin(), distance_pair_l.end(), std::greater());

                //rank search
                rank_ins.LoopPreprocess(distance_pair_l.data(), userID);

                disk_ins.BuildIndexLoop(distance_pair_l, userID);
            }
            disk_ins.WriteIndex();
            if (labelID % report_batch_every == 0) {
                std::cout << "preprocessed " << labelID / (0.01 * n_merge_user) << " %, "
                          << batch_report_record.get_elapsed_time_second() << " s/iter" << " Mem: "
                          << get_current_RSS() / 1000000 << " Mb \n";
                batch_report_record.reset();
            }
        }
        disk_ins.FinishWrite();

        std::unique_ptr<Index> index_ptr = std::make_unique<Index>(
                //hash search
                rank_ins,
                //disk index
                disk_ins,
                //general retrieval
                user, data_item);
        return index_ptr;
    }

}
#endif //REVERSE_KRANKS_IRBMERGERANKBOUND_HPP
