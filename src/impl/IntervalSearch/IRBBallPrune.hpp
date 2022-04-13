//
// Created by BianZheng on 2022/4/12.
//

#ifndef REVERSE_KRANKS_IRBBALLPRUNE_HPP
#define REVERSE_KRANKS_IRBBALLPRUNE_HPP

#include "alg/Prune/IPbound/BallPrune.hpp"
#include "alg/Prune/IntervalSearch.hpp"
#include "alg/Prune/RankSearch.hpp"
#include "alg/Prune/PruneCandidateByBound.hpp"
#include "alg/SpaceInnerProduct.hpp"
#include "alg/SVD.hpp"
#include "struct/VectorMatrix.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/MethodBase.hpp"
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

namespace ReverseMIPS::IntervalRankBound {

    class RetrievalResult : public RetrievalResultBase {
    public:
        //unit: second
        //double total_time, read_disk_time, inner_product_time,
        //          coarse_binary_search_time, fine_binary_search_time, interval_search_time;
        //double interval_prune_ratio, binary_search_prune_ratio;
        //double second_per_query;
        //int topk;

        inline RetrievalResult() = default;

        void AddPreprocess(double build_index_time) {
            char buff[1024];
            sprintf(buff, "build index time %.3f", build_index_time);
            std::string str(buff);
            this->config_l.emplace_back(str);
        }

        std::string AddResultConfig(const int &topk,
                                    const double &total_time, const double &interval_search_time,
                                    const double &inner_product_time,
                                    const double &coarse_binary_search_time, const double &read_disk_time,
                                    const double &fine_binary_search_time,
                                    const double &interval_prune_ratio,
                                    const double &rank_search_prune_ratio,
                                    const double &second_per_query) {
            char buff[1024];
            sprintf(buff,
                    "top%d retrieval time:\n\ttotal %.3fs, interval search %.3fs, inner product %.3fs\n\tcoarse binary search %.3fs, read disk time %.3f, fine binary search %.3fs\n\tinterval prune ratio %.4f, rank search prune ratio %.4f\n\tmillion second per query %.3fms",
                    topk,
                    total_time, interval_search_time, inner_product_time,
                    coarse_binary_search_time, read_disk_time, fine_binary_search_time,
                    interval_prune_ratio, rank_search_prune_ratio,
                    second_per_query);
            std::string str(buff);
            this->config_l.emplace_back(str);
            return str;
        }

    };

    class Index : public BaseIndex {
        void ResetTimer() {
            read_disk_time_ = 0;
            inner_product_time_ = 0;
            coarse_binary_search_time_ = 0;
            fine_binary_search_time_ = 0;
            interval_search_time_ = 0;
            interval_prune_ratio_ = 0;
            rank_search_prune_ratio_ = 0;
        }

    public:
        //for interval search, store in memory
        IntervalSearch interval_ins_;
        //interval search bound
        SVD svd_ins_;
        BallPrune interval_prune_;

        //for rank search, store in memory
        RankSearch rank_ins_;

        //read index on disk
        const char *index_path_;

        VectorMatrix user_;
        int vec_dim_, n_data_item_, n_user_;
        double interval_search_time_, inner_product_time_, coarse_binary_search_time_, read_disk_time_, fine_binary_search_time_;
        TimeRecord read_disk_record_, inner_product_record_, coarse_binary_search_record_, fine_binary_search_record_, interval_search_record_;
        double interval_prune_ratio_, rank_search_prune_ratio_;

        //temporary retrieval variable
        std::unique_ptr<double[]> query_ptr_;
        std::vector<double> disk_cache_;
        std::vector<bool> prune_l_;
        std::vector<std::pair<double, double>> ip_bound_l_;
        std::vector<double> queryIP_l_;
        std::vector<int> rank_lb_l_;
        std::vector<int> rank_ub_l_;

        Index(
                //interval search
                IntervalSearch &interval_ins,
                //interval search bound
                SVD &svd_ins, BallPrune &interval_prune,
                // rank search
                RankSearch &rank_ins,
                //general retrieval
                VectorMatrix &user, const int &n_data_item, const char *index_path) {
            //interval search
            this->interval_ins_ = std::move(interval_ins);
            //interval search bound
            this->svd_ins_ = std::move(svd_ins);
            this->interval_prune_ = std::move(interval_prune);
            //rank search
            this->rank_ins_ = std::move(rank_ins);
            //general retrieval
            this->n_user_ = user.n_vector_;
            this->vec_dim_ = user.vec_dim_;
            this->user_ = std::move(user);
            this->n_data_item_ = n_data_item;
            this->index_path_ = index_path;
            assert(0 < this->user_.vec_dim_);

            //retrieval variable
            this->query_ptr_ = std::make_unique<double[]>(vec_dim_);
            this->disk_cache_.resize(this->rank_ins_.n_max_disk_read_);
            this->prune_l_.resize(n_user_);
            this->ip_bound_l_.resize(n_user_);
            this->queryIP_l_.resize(n_user_);
            this->rank_lb_l_.resize(n_user_);
            this->rank_ub_l_.resize(n_user_);
        }

        std::vector<std::vector<UserRankElement>> Retrieval(VectorMatrix &query_item, const int &topk) override {
            ResetTimer();
            std::ifstream index_stream_ = std::ifstream(this->index_path_, std::ios::binary | std::ios::in);
            if (!index_stream_) {
                spdlog::error("error in writing index");
            }

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
            std::vector<UserRankElement> user_topk_cache_l(n_user_);
            std::vector<int> rank_topk_max_heap(topk);
            for (int queryID = 0; queryID < n_query_item; queryID++) {
                prune_l_.assign(n_user_, false);
                rank_lb_l_.assign(n_user_, n_data_item_);
                rank_ub_l_.assign(n_user_, 0);


                double *query_vecs = query_ptr_.get();
                svd_ins_.TransferQuery(query_item.getVector(queryID), vec_dim_, query_vecs);

                interval_search_record_.reset();
                //get the ip bound
                interval_prune_.IPBound(query_vecs, user_, prune_l_, ip_bound_l_);
                //count rank bound
                interval_ins_.RankBound(ip_bound_l_, prune_l_, topk, rank_lb_l_, rank_ub_l_);
                //prune the bound
                PruneCandidateByBound(rank_lb_l_, rank_ub_l_,
                                      n_user_, topk,
                                      prune_l_, rank_topk_max_heap);

                this->interval_search_time_ += interval_search_record_.get_elapsed_time_second();
                int n_candidate = 0;
                for (int userID = 0; userID < n_user_; userID++) {
                    if (!prune_l_[userID]) {
                        n_candidate++;
                    }
                }
                assert(n_candidate >= topk);
                interval_prune_ratio_ += 1.0 * (n_user_ - n_candidate) / n_user_;

                //calculate the exact IP
                inner_product_record_.reset();
                for (int userID = 0; userID < n_user_; userID++) {
                    if (prune_l_[userID]) {
                        continue;
                    }
                    queryIP_l_[userID] = InnerProduct(user_.getVector(userID), query_vecs, vec_dim_);
                }
                this->inner_product_time_ += inner_product_record_.get_elapsed_time_second();

                //coarse binary search
                coarse_binary_search_record_.reset();
                rank_ins_.RankBound(queryIP_l_, topk, rank_lb_l_, rank_ub_l_, prune_l_, rank_topk_max_heap, queryID);
                PruneCandidateByBound(rank_lb_l_, rank_ub_l_,
                                      n_user_, topk,
                                      prune_l_, rank_topk_max_heap);
                coarse_binary_search_time_ += coarse_binary_search_record_.get_elapsed_time_second();
                n_candidate = 0;
                for (int userID = 0; userID < n_user_; userID++) {
                    if (!prune_l_[userID]) {
                        n_candidate++;
                    }
                }
                assert(n_candidate >= topk);
                rank_search_prune_ratio_ += 1.0 * (n_user_ - n_candidate) / n_user_;

                //read disk and fine binary search
                n_candidate = 0;
                for (int userID = 0; userID < n_user_; userID++) {
                    if (prune_l_[userID]) {
                        continue;
                    }
                    int end_idx = rank_lb_l_[userID];
                    int start_idx = rank_ub_l_[userID];
                    double queryIP = queryIP_l_[userID];
                    int &base_rank = start_idx;
                    int read_count = end_idx - start_idx;

                    assert(0 <= read_count && read_count <= disk_cache_.size());

                    assert(start_idx <= end_idx);
                    read_disk_record_.reset();
                    int64_t offset = (int64_t) userID * n_data_item_ + start_idx;
                    offset *= sizeof(double);
                    index_stream_.seekg(offset, std::ios::beg);
                    index_stream_.read((char *) disk_cache_.data(), read_count * sizeof(double));
                    read_disk_time_ += read_disk_record_.get_elapsed_time_second();

                    fine_binary_search_record_.reset();
                    int rank = FineBinarySearch(disk_cache_, queryIP, userID, base_rank, read_count);
                    fine_binary_search_time_ += fine_binary_search_record_.get_elapsed_time_second();

                    user_topk_cache_l[n_candidate] = UserRankElement(userID, rank, queryIP);
                    n_candidate++;
                }


                std::sort(user_topk_cache_l.begin(), user_topk_cache_l.begin() + n_candidate,
                          std::less());
                for (int candID = 0; candID < topk; candID++) {
                    query_heap_l[queryID][candID] = user_topk_cache_l[candID];
                }
                assert(query_heap_l[queryID].size() == topk);
            }

            interval_prune_ratio_ /= n_query_item;
            rank_search_prune_ratio_ /= n_query_item;
            return query_heap_l;
        }

        inline int FineBinarySearch(const std::vector<double> &disk_cache, const double &queryIP, const int &userID,
                                    const int &base_rank,
                                    const int &read_count) {
            if (read_count == 0) {
                return base_rank + 1;
            }
            auto iter_begin = disk_cache.begin();
            auto iter_end = disk_cache.begin() + read_count;

            auto lb_ptr = std::lower_bound(iter_begin, iter_end, queryIP,
                                           [](const double &arrIP, double queryIP) {
                                               return arrIP > queryIP;
                                           });
            return (int) (lb_ptr - iter_begin) + base_rank + 1;
        }

    };

    const int write_every_ = 1000;
    const int report_batch_every_ = 100;

    /*
     * bruteforce index
     * shape: n_user * n_data_item, type: double, the distance pair for each user
     */

    Index &BuildIndex(VectorMatrix &user, VectorMatrix &data_item, const char *index_path) {
        const int n_data_item = data_item.n_vector_;
        const int vec_dim = data_item.vec_dim_;
        const int n_user = user.n_vector_;

        user.vectorNormalize();

        const double SIGMA = 0.7;
        const double scale = 100;
        SVD svd_ins;
        int check_dim = svd_ins.Preprocess(user, data_item, SIGMA);

        const int node_threshold = 300;
        BallPrune interval_prune;
        interval_prune.Preprocess(user, node_threshold);

        //interval search
        const int n_interval = std::min(n_data_item / 10, 5000);
        IntervalSearch interval_ins(n_interval, n_user, n_data_item);

        //rank search
        const int cache_bound_every = 500;
        RankSearch rank_ins(cache_bound_every, n_data_item, n_user);

        //build and write index
        std::ofstream out(index_path, std::ios::binary | std::ios::out);
        if (!out) {
            spdlog::error("error in write result");
        }

        std::vector<double> write_distance_cache(write_every_ * n_data_item);
        const int n_batch = n_user / write_every_;
        const int n_remain = n_user % write_every_;
        spdlog::info("write_every_ {}, n_batch {}, n_remain {}", write_every_, n_batch, n_remain);

        TimeRecord batch_report_record;
        batch_report_record.reset();
        for (int i = 0; i < n_batch; i++) {
#pragma omp parallel for default(none) shared(i, data_item, user, write_distance_cache, rank_ins, check_dim, interval_ins) shared(write_every_, n_data_item, vec_dim, n_interval)
            for (int cacheID = 0; cacheID < write_every_; cacheID++) {
                int userID = write_every_ * i + cacheID;
                for (int itemID = 0; itemID < n_data_item; itemID++) {
                    double ip = InnerProduct(data_item.getVector(itemID), user.getVector(userID), vec_dim);
                    write_distance_cache[cacheID * n_data_item + itemID] = ip;
                }
                std::sort(write_distance_cache.begin() + cacheID * n_data_item,
                          write_distance_cache.begin() + (cacheID + 1) * n_data_item, std::greater());

                //interval search
                double upper_bound = write_distance_cache[cacheID * n_data_item] + 0.01;
                double lower_bound = write_distance_cache[(cacheID + 1) * n_data_item - 1] - 0.01;
                std::pair<double, double> bound_pair = std::make_pair(lower_bound, upper_bound);
                const double *distance_ptr = write_distance_cache.data() + cacheID * n_data_item;
                interval_ins.LoopPreprocess(bound_pair, distance_ptr, userID);

                //rank search
                rank_ins.LoopPreprocess(distance_ptr, userID);
            }
            out.write((char *) write_distance_cache.data(), write_distance_cache.size() * sizeof(double));

            if (i % report_batch_every_ == 0) {
                std::cout << "preprocessed " << i / (0.01 * n_batch) << " %, "
                          << batch_report_record.get_elapsed_time_second() << " s/iter" << " Mem: "
                          << get_current_RSS() / 1000000 << " Mb \n";
                batch_report_record.reset();
            }

        }

        for (int cacheID = 0; cacheID < n_remain; cacheID++) {
            int userID = write_every_ * n_batch + cacheID;
            for (int itemID = 0; itemID < n_data_item; itemID++) {
                double ip = InnerProduct(data_item.getVector(itemID), user.getVector(userID), vec_dim);
                write_distance_cache[cacheID * n_data_item + itemID] = ip;
            }

            std::sort(write_distance_cache.begin() + cacheID * n_data_item,
                      write_distance_cache.begin() + (cacheID + 1) * n_data_item, std::greater<double>());

            //interval search
            double upper_bound = write_distance_cache[cacheID * n_data_item] + 0.01;
            double lower_bound = write_distance_cache[(cacheID + 1) * n_data_item - 1] - 0.01;
            std::pair<double, double> bound_pair = std::make_pair(lower_bound, upper_bound);
            const double *distance_ptr = write_distance_cache.data() + cacheID * n_data_item;
            interval_ins.LoopPreprocess(bound_pair, distance_ptr, userID);

            //rank search
            rank_ins.LoopPreprocess(distance_ptr, userID);
        }

        out.write((char *) write_distance_cache.data(),
                  n_remain * data_item.n_vector_ * sizeof(double));
        static Index index(
                //interval search
                interval_ins,
                //interval search bound
                svd_ins, interval_prune,
                //rank search
                rank_ins,
                //general retrieval
                user, n_data_item, index_path);
        return index;
    }

}
#endif //REVERSE_KRANKS_IRBBALLPRUNE_HPP
