//
// Created by BianZheng on 2022/2/25.
//

#ifndef REVERSE_KRANKS_RANKBOUND_HPP
#define REVERSE_KRANKS_RANKBOUND_HPP

#include "alg/PruneCandidateByBound.hpp"
#include "alg/SpaceInnerProduct.hpp"
#include "struct/VectorMatrix.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/MethodBase.hpp"
#include "util/TimeMemory.hpp"
#include "util/VectorIO.hpp"
#include "alg/RankSearch.hpp"
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

        std::string AddResultConfig(const int &topk,
                                    const double &total_time, const double &read_disk_time,
                                    const double &inner_product_time,
                                    const double &coarse_binary_search_time, const double &fine_binary_search_time,
                                    const double &rank_prune_ratio,
                                    const double &second_per_query) {
            char buff[1024];

            sprintf(buff,
                    "top%d retrieval time:\n\ttotal %.3fs, read disk %.3fs\n\tinner product %.3fs, coarse binary search %.3fs, fine binary search %.3fs\n\trank prune ratio %.7f, million second per query %.3fms",
                    topk, total_time, read_disk_time, inner_product_time, coarse_binary_search_time,
                    fine_binary_search_time, rank_prune_ratio, second_per_query);
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
            rank_prune_ratio_ = 0;
        }

    public:
        //rank search
        RankSearch rank_ins_;

        //read index on disk
        const char *index_path_;

        VectorMatrix user_;
        int vec_dim_, n_data_item_, n_user_;
        double read_disk_time_, inner_product_time_, coarse_binary_search_time_, fine_binary_search_time_;
        TimeRecord read_disk_record_, inner_product_record_, coarse_binary_search_record_, fine_binary_search_record_;
        double rank_prune_ratio_;

        //temporary retrieval variable
        // store queryIP
        std::vector<double> queryIP_l_;
        std::vector<int> rank_lb_l_;
        std::vector<int> rank_ub_l_;
        std::vector<bool> prune_l_;
        std::vector<double> disk_cache_;

        Index(//rank search
                RankSearch &rank_ins,
                //general retrieval
                VectorMatrix &user, const int &n_data_item, const char *index_path
        ) {
            this->rank_ins_ = std::move(rank_ins);

            this->user_ = std::move(user);
            this->vec_dim_ = this->user_.vec_dim_;
            this->n_user_ = this->user_.n_vector_;
            this->n_data_item_ = n_data_item;
            this->index_path_ = index_path;

            //retrieval variable
            queryIP_l_.resize(n_user_);
            rank_lb_l_.resize(n_user_);
            rank_ub_l_.resize(n_user_);
            prune_l_.resize(n_user_);
            this->disk_cache_.resize(this->rank_ins_.n_max_disk_read_);
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

            //coarse binary search
            const int n_query_item = query_item.n_vector_;

            std::vector<std::vector<UserRankElement>> query_heap_l(n_query_item);
            for (int qID = 0; qID < n_query_item; qID++) {
                query_heap_l[qID].reserve(topk);
            }

            // for binary search, check the number
            std::vector<int> rank_topk_max_heap(topk);
            std::vector<UserRankElement> user_topk_cache_l(n_user_);
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

//                if (queryID == 10) {
//                    int userID = 243879;
//                    printf("after inner product queryID %d, userID %d, queryIP %.3f\n\t", queryID, userID,
//                           queryIP_l_[userID]);
//                    printf("rank lb %d, rank ub %d\n\t", rank_lb_l_[userID], rank_ub_l_[userID]);
//                    std::cout << std::boolalpha << "is prune: " << prune_l_[userID] << std::endl;
//                }

                //rank search
                coarse_binary_search_record_.reset();
                rank_ins_.RankBound(queryIP_l_, topk, rank_lb_l_, rank_ub_l_, prune_l_, rank_topk_max_heap, queryID);
//                if (queryID == 10) {
//                    int userID = 243879;
//                    printf("after rank search queryID %d, userID %d, queryIP %.3f\n\t", queryID, userID,
//                           queryIP_l_[userID]);
//                    printf("rank lb %d, rank ub %d\n\t", rank_lb_l_[userID], rank_ub_l_[userID]);
//                    std::cout << std::boolalpha << "is prune: " << prune_l_[userID] << std::endl;
//                }
                PruneCandidateByBound(rank_lb_l_, rank_ub_l_,
                                      n_user_, topk,
                                      prune_l_, rank_topk_max_heap);

//                if (queryID == 861) {
//                    int userID = 2;
//                    printf("after prune candidate queryID %d, userID %d, queryIP %.3f\n\t", queryID, userID,
//                           queryIP_l_[userID]);
//                    printf("rank lb %d, rank ub %d\n\t", rank_lb_l_[userID], rank_ub_l_[userID]);
//                    std::cout << std::boolalpha << "is prune: " << prune_l_[userID] << std::endl;
//                }

                coarse_binary_search_time_ += coarse_binary_search_record_.get_elapsed_time_second();
                int n_candidate = 0;
                for (int userID = 0; userID < n_user_; userID++) {
                    if (!prune_l_[userID]) {
                        n_candidate++;
                    }
                }
                assert(n_candidate >= topk);
                rank_prune_ratio_ += 1.0 * (n_user_ - n_candidate) / n_user_;

                //read disk and fine binary search
                n_candidate = 0;
                for (int userID = 0; userID < n_user_; userID++) {
                    if (prune_l_[userID]) {
                        continue;
                    }

                    int end_idx = rank_lb_l_[userID];
                    int start_idx = rank_ub_l_[userID];
                    assert(0 <= start_idx && start_idx <= end_idx && end_idx <= n_data_item_);
                    double queryIP = queryIP_l_[userID];
                    int base_rank = start_idx;
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
                    query_heap_l[queryID].emplace_back(user_topk_cache_l[candID]);
                }
                assert(query_heap_l[queryID].size() == topk);
            }
            rank_prune_ratio_ /= n_query_item;

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

    const int write_every_ = 100;
    const int report_batch_every_ = 100;

    /*
     * bruteforce index
     * shape: n_user * n_data_item, type: double, the distance pair for each user
     */

    Index &
    BuildIndex(VectorMatrix &data_item, VectorMatrix &user, const char *index_path, const int &cache_bound_every) {
        std::ofstream out(index_path, std::ios::binary | std::ios::out);
        if (!out) {
            spdlog::error("error in write result");
            exit(-1);
        }
        const int n_user = user.n_vector_;
        const int n_data_item = data_item.n_vector_;
        std::vector<double> write_distance_cache(write_every_ * n_data_item);
        const int vec_dim = data_item.vec_dim_;
        const int n_batch = user.n_vector_ / write_every_;
        const int n_remain = user.n_vector_ % write_every_;

        user.vectorNormalize();

        //rank search
        RankSearch rank_ins(cache_bound_every, n_data_item, n_user);

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
            out.write((char *) write_distance_cache.data(), write_every_ * n_data_item * sizeof(double));

            if (i % report_batch_every_ == 0) {
                std::cout << "preprocessed " << i / (0.01 * n_batch) << " %, "
                          << batch_report_record.get_elapsed_time_second() << " s/iter" << " Mem: "
                          << get_current_RSS() / 1000000 << " Mb \n";
                batch_report_record.reset();
            }

        }

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

        out.write((char *) write_distance_cache.data(),
                  n_remain * data_item.n_vector_ * sizeof(double));
        static Index index(rank_ins, user, n_data_item, index_path);
        return index;
    }

}

#endif //REVERSE_KRANKS_RANKBOUND_HPP
