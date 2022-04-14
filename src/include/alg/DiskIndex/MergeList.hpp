//
// Created by BianZheng on 2022/4/13.
//

#ifndef REVERSE_KRANKS_MERGEVECTOR_HPP
#define REVERSE_KRANKS_MERGEVECTOR_HPP

#include "alg/KMeans/KMeansParallel.hpp"
#include <memory>
#include <spdlog/spdlog.h>

namespace ReverseMIPS {

    class MergeList {
    public:

        int n_user_, n_data_item_, n_merge_user_, compress_rank_every_, n_cache_rank_;
        //n_cache_rank_: stores how many intervals for each merged user
        //compress_rank_every_: how many ranks should sample for a merged user
        std::vector<uint32_t> merge_label_l_; // n_merge_user, stores which cluster the user belons to
        std::vector<int> index_size_l;//n_merge_user * n_cache_rank, stores the
        const char *index_path_;
        int n_max_disk_read_;

        TimeRecord read_disk_record_, fine_binary_search_record_;
        double read_disk_time_, fine_binary_search_time_;

        //variable in build index
        std::ofstream out_stream_;

        //variable in retrieval
        std::ifstream index_stream_;
        std::unique_ptr<double[]> disk_cache_;
        int n_candidate_;
        std::vector<UserRankElement> user_topk_cache_l_;

        inline MergeList() {}

        inline MergeList(const VectorMatrix &user, const int &n_data_item, const char *index_path,
                         const int &n_merge_user, const int &compress_rank_every) {
            this->n_user_ = user.n_vector_;
            this->n_data_item_ = n_data_item;
            this->index_path_ = index_path;
            this->n_merge_user_ = n_merge_user;
            this->user_topk_cache_l_.resize(n_user_);

            this->compress_rank_every_ = compress_rank_every;
            const int n_cache_rank = n_data_item_ / compress_rank_every_;
            this->n_cache_rank_ = n_cache_rank;


            BuildIndexPreprocess(user);
        }

        void
        BuildIndexPreprocess(const VectorMatrix &user) {
            known_rank_idx_l_ = std::make_unique<int[]>(n_cache_rank_);
            for (int known_rank_idx = cache_bound_every_ - 1, idx = 0;
                 known_rank_idx < n_data_item_; known_rank_idx += cache_bound_every_, idx++) {
                known_rank_idx_l_[idx] = known_rank_idx;
            }

            //build kmeans on user
            std::vector<std::vector<double>> user_vecs_l(n_user_, std::vector<double>(user.vec_dim_));
            for (int userID = 0; userID < n_user_; userID++) {
                memcpy(user_vecs_l[userID].data(), user.getVector(userID), user.vec_dim_);
            }
            ReverseMIPS::clustering_parameters<double> para(n_merge_user_);
            para.set_random_seed(0);
            para.set_max_iteration(200);

            std::tuple<std::vector<std::vector<double>>, std::vector<uint32_t>> cluster_data =
                    kmeans_lloyd_parallel(user_vecs_l, para);

            merge_label_l_ = std::get<1>(cluster_data);

            out_stream_ = std::ofstream(index_path_, std::ios::binary | std::ios::out);
            if (!out_stream_) {
                spdlog::error("error in write result");
                exit(-1);
            }
        }

        std::vector<std::vector<int>> &EvalVector() {
            static std::vector<std::vector<int>> eval_seq_l(n_merge_user_);
            for (int labelID = 0; labelID < n_merge_user_; labelID++) {
                std::vector<int> &eval_l = eval_seq_l[labelID];
                for (int userID = 0; userID < n_user_; userID++) {
                    if (merge_label_l_[userID] == labelID) {
                        eval_l.push_back(userID);
                    }
                }
            }
            return eval_seq_l;
        }

        void BuildIndexLoop(const std::vector<double> &distance_cache, const int &n_write) {
            // distance_cache: write_every * n_data_item_, n_write <= write_every
            assert(distance_cache.size() >= n_write * n_data_item_);
            out_stream_.write((char *) distance_cache.data(), n_write * n_data_item_ * sizeof(double));
        }

        inline int FineBinarySearch(const double &queryIP, const int &userID,
                                    const int &base_rank,
                                    const int &read_count) const {
            if (read_count == 0) {
                return base_rank + 1;
            }
            const double *cache_ptr = disk_cache_.get();
            auto iter_begin = cache_ptr;
            auto iter_end = cache_ptr + read_count;

            auto lb_ptr = std::lower_bound(iter_begin, iter_end, queryIP,
                                           [](const double &arrIP, double queryIP) {
                                               return arrIP > queryIP;
                                           });
            return (int) (lb_ptr - iter_begin) + base_rank + 1;
        }

        inline void ReadDisk(const int &userID, const int &start_idx, const int &read_count) {
            int64_t offset = (int64_t) userID * n_data_item_ + start_idx;
            offset *= sizeof(double);
            index_stream_.seekg(offset, std::ios::beg);
            index_stream_.read((char *) disk_cache_.get(), read_count * sizeof(double));
        }

        void RetrievalPreprocess() {
            read_disk_time_ = 0;
            fine_binary_search_time_ = 0;
            index_stream_ = std::ifstream(this->index_path_, std::ios::binary | std::ios::in);
            if (!index_stream_) {
                spdlog::error("error in writing index");
            }
        }

        void GetRank(const std::vector<double> &queryIP_l,
                     std::vector<int> &rank_lb_l, std::vector<int> &rank_ub_l,
                     std::vector<bool> &prune_l) {

            //read disk and fine binary search
            n_candidate_ = 0;
            for (int userID = 0; userID < n_user_; userID++) {
                if (prune_l[userID]) {
                    continue;
                }

                int end_idx = rank_lb_l[userID];
                int start_idx = rank_ub_l[userID];
                assert(0 <= start_idx && start_idx <= end_idx && end_idx <= n_data_item_);

                double queryIP = queryIP_l[userID];
                int base_rank = start_idx;
                int read_count = end_idx - start_idx;

                assert(0 <= read_count && read_count <= n_max_disk_read_);

                assert(start_idx <= end_idx);
                read_disk_record_.reset();
                ReadDisk(userID, start_idx, read_count);
                read_disk_time_ += read_disk_record_.get_elapsed_time_second();
                fine_binary_search_record_.reset();
                int rank = FineBinarySearch(queryIP, userID, base_rank, read_count);
                fine_binary_search_time_ += fine_binary_search_record_.get_elapsed_time_second();

                user_topk_cache_l_[n_candidate_] = UserRankElement(userID, rank, queryIP);
                n_candidate_++;
            }

            std::sort(user_topk_cache_l_.begin(), user_topk_cache_l_.begin() + n_candidate_,
                      std::less());

        }

    };
}
#endif //REVERSE_KRANKS_MERGEVECTOR_HPP
