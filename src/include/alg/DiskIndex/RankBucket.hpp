//
// Created by BianZheng on 2022/4/13.
//

#ifndef REVERSE_KRANKS_MERGEVECTOR_HPP
#define REVERSE_KRANKS_MERGEVECTOR_HPP

#include "alg/SpaceInnerProduct.hpp"
#include "alg/KMeans/KMeansParallel.hpp"
#include "struct/DistancePair.hpp"
#include <set>
#include <cfloat>
#include <memory>
#include <spdlog/spdlog.h>
#include <armadillo>

namespace ReverseMIPS {

    class RankBucket {
    public:

        int n_user_, n_data_item_, vec_dim_, n_merge_user_, compress_rank_every_, n_cache_rank_;
        //n_cache_rank_: stores how many intervals for each merged user
        //compress_rank_every_: how many ranks should sample for a merged user
        std::vector<uint32_t> merge_label_l_; // n_user, stores which cluster the user belons to
        std::vector<uint64_t> index_size_l_;//n_merge_user * n_cache_rank, stores the offset of every rank intervals
        std::vector<double> IPbound_l_;//n_user * n_cache_rank, stores the upper IP bound for each interval
        std::vector<int> rank_boundary_l_;
        //n_cache_rank_, sample a value for every compress_rank_every_ items, the last value must be n_data_item
        const char *index_path_;
        uint32_t n_max_disk_read_;

        TimeRecord read_disk_record_, fine_binary_search_record_;
        double read_disk_time_, fine_binary_search_time_;

        //variable in build index
        std::ofstream out_stream_;

        //variable in retrieval
        std::ifstream index_stream_;
        std::unique_ptr<int[]> user_disk_cache_; //n_max_disk_read_, stores the userID in the disk
        int n_candidate_;
        std::vector<UserRankElement> user_topk_cache_l_;

        inline RankBucket() {}

        inline RankBucket(const VectorMatrix &user, const int &n_data_item, const char *index_path,
                          const int &n_merge_user, const int &compress_rank_every) {
            this->n_user_ = user.n_vector_;
            this->n_data_item_ = n_data_item;
            this->vec_dim_ = user.vec_dim_;
            this->index_path_ = index_path;
            this->n_merge_user_ = n_merge_user;

            this->compress_rank_every_ = compress_rank_every;
            int offset = n_data_item_ % compress_rank_every_ == 0 ? 0 : 1;
            const int n_cache_rank = n_data_item_ / compress_rank_every_ + offset;
            this->n_cache_rank_ = n_cache_rank;
            spdlog::info("n_merge_user {}, compress_rank_every {}, n_cache_rank {}", n_merge_user_,
                         compress_rank_every_, n_cache_rank_);

            this->merge_label_l_.resize(n_user_);
            this->index_size_l_.resize(n_merge_user_ * n_cache_rank_);
            this->IPbound_l_.resize(n_user_ * n_cache_rank_);
            this->rank_boundary_l_.resize(n_cache_rank_);

            this->user_topk_cache_l_.resize(n_user_);

            BuildIndexPreprocess(user);
        }

        void
        BuildIndexPreprocess(const VectorMatrix &user) {
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
            spdlog::info("Finish KMeans clustering");

            printf("cluster size\n");
            for (int mergeID = 0; mergeID < n_merge_user_; mergeID++) {
                int count = 0;
                for (int userID = 0; userID < n_user_; userID++) {
                    if (merge_label_l_[userID] == mergeID) {
                        count++;
                    }
                }
                printf("%d ", count);
            }
            printf("\n");

            //rank boundary
            rank_boundary_l_.resize(this->n_cache_rank_);
            for (int known_rank_idx = compress_rank_every_ - 1, idx = 0;
                 known_rank_idx < n_data_item_; known_rank_idx += compress_rank_every_, idx++) {
                rank_boundary_l_[idx] = known_rank_idx;
            }
            if (n_data_item_ % compress_rank_every_ != 0) {
                rank_boundary_l_[n_cache_rank_ - 1] = n_data_item_ - 1;
            }

            printf("rank boundary_l\n");
            for (int rankID = 0; rankID < n_cache_rank_; rankID++) {
                printf("%d ", rank_boundary_l_[rankID]);
            }
            printf("\n");

            out_stream_ = std::ofstream(index_path_, std::ios::binary | std::ios::out);
            if (!out_stream_) {
                spdlog::error("error in write result");
                exit(-1);
            }
        }

        std::vector<std::vector<int>> &BuildIndexMergeUser() {
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

        void BuildIndexLoop(const std::vector<DistancePair> &distance_pair_l, const int &userID,
                            std::vector<std::set<int>> &cache_bkt_vec) {
            // cache_bkt_vec: n_cache_rank
            // merge to the cache_bkt_vec
            for (int bucketID = 0; bucketID < n_cache_rank_; bucketID++) {
                int base_idx = bucketID == 0 ? 0 : rank_boundary_l_[bucketID - 1] + 1;
                int remain_size = n_data_item_ % compress_rank_every_;
                int bucket_size = bucketID == n_cache_rank_ - 1 ? remain_size : compress_rank_every_;
                std::set<int> &bucket_s = cache_bkt_vec[bucketID];
                for (int candID = 0; candID < bucket_size; candID++) {
                    const int tmp_userID = distance_pair_l[base_idx + candID].ID_;
                    if (bucket_s.find(tmp_userID) == bucket_s.end()) { // not found
                        bucket_s.insert(tmp_userID);
                    }
                }
            }

            for (int rankID = 0; rankID < n_cache_rank_; rankID++) {
                int rank_val = rank_boundary_l_[rankID];
                IPbound_l_[userID * n_cache_rank_ + rankID] = distance_pair_l[rank_val].dist_;
            }

        }

        void WriteIndex(const int &labelID, std::vector<std::set<int>> &cache_bkt_vec) {
            //get the number of users in each bucket, assign into the cache_bkt_vec
            assert(cache_bkt_vec.size() == n_cache_rank_);

            for (int bucketID = 0; bucketID < n_cache_rank_; bucketID++) {
                std::set<int> &bkt_s = cache_bkt_vec[bucketID];
                std::vector<int> bkt_l;
                bkt_l.assign(bkt_s.begin(), bkt_s.end());

                uint32_t merge_size = bkt_l.size();
                index_size_l_[labelID * n_cache_rank_ + bucketID] = merge_size;
                out_stream_.write((char *) bkt_l.data(),
                                  merge_size * sizeof(int));
                cache_bkt_vec[bucketID].clear();

                uint32_t n_cand = bkt_l.size();
                for (int candID = 0; candID < n_cand; candID++) {
                    for (int tcandID = 0; tcandID < n_cand; tcandID++) {
                        if (candID != tcandID) {
                            assert(bkt_l[candID] != bkt_l[tcandID]);
                        }
                    }
                }

            }

        }

        void FinishWrite() {
            out_stream_.close();

            n_max_disk_read_ = 0;
            for (int labelID = 0; labelID < n_merge_user_; labelID++) {
                for (int bucketID = 0; bucketID < n_cache_rank_; bucketID++) {
                    n_max_disk_read_ = std::max(n_max_disk_read_,
                                                (uint32_t) index_size_l_[labelID * n_cache_rank_ + bucketID]);
                }
            }

            user_disk_cache_ = std::make_unique<int[]>(n_max_disk_read_);

            printf("n_max_disk_read %d\n", n_max_disk_read_);
            printf("index size not accumulate\n");
            for (int mergeID = 0; mergeID < n_merge_user_; mergeID++) {
                for (int rankID = 0; rankID < n_cache_rank_; rankID++) {
                    printf("%ld ", index_size_l_[mergeID * n_cache_rank_ + rankID]);
                }
                printf("\n");
            }

            int64_t size = n_merge_user_ * n_cache_rank_;
            for (int i = 1; i < size; i++) {
                index_size_l_[i] += index_size_l_[i - 1];
            }

        }

        inline void RetrievalPreprocess() {
            read_disk_time_ = 0;
            fine_binary_search_time_ = 0;
            index_stream_ = std::ifstream(this->index_path_, std::ios::binary | std::ios::in);
            if (!index_stream_) {
                spdlog::error("error in writing index");
            }
        }

        inline void ReadDisk(const int &userID, const uint64_t &base_user_offset, const uint64_t &read_user_count) {
            uint64_t offset = base_user_offset * sizeof(int);
            uint64_t read_count = read_user_count * sizeof(int);
            index_stream_.seekg(offset, std::ios::beg);
            index_stream_.read((char *) user_disk_cache_.get(), read_count);
        }

        void GetRank(const std::vector<double> &queryIP_l,
                     std::vector<int> &rank_lb_l, std::vector<int> &rank_ub_l,
                     std::vector<bool> &prune_l, const VectorMatrix &user, const VectorMatrix &item, int queryID) {
            //read disk and fine binary search
            n_candidate_ = 0;
            for (int userID = 0; userID < n_user_; userID++) {
                if (prune_l[userID]) {
                    continue;
                }
                assert(0 <= rank_ub_l[userID] && rank_ub_l[userID] <= rank_lb_l[userID] &&
                       rank_lb_l[userID] <= n_data_item_);

                //may have bug
                int bkt_ub_idx = (rank_ub_l[userID] + 1) / compress_rank_every_;
                int bkt_offset = (rank_lb_l[userID] + 1) % compress_rank_every_ == 0 ? 0 : 1;
                int bkt_lb_idx = (rank_lb_l[userID] + 1) / compress_rank_every_ + bkt_offset;
                assert(0 <= bkt_ub_idx && bkt_ub_idx <= bkt_lb_idx && bkt_lb_idx <= n_cache_rank_);

                double queryIP = queryIP_l[userID];
                int base_rank = bkt_ub_idx == 0 ? 0 : rank_boundary_l_[bkt_ub_idx - 1];

                uint32_t labelID = merge_label_l_[userID];
                uint64_t base_user_offset =
                        labelID == 0 && bkt_ub_idx == 0 ? 0 : index_size_l_[labelID * n_cache_rank_ + bkt_ub_idx - 1];
                uint64_t read_user_count = index_size_l_[labelID * n_cache_rank_ + bkt_lb_idx] -
                                           base_user_offset;

                if (queryID == 0 && userID == 468) {
                    printf("bkt_ub_idx %d, bkt_lb_idx %d, base_rank %d, labelID %d\n", bkt_ub_idx, bkt_lb_idx,
                           base_rank, labelID);
                    printf("base_user_offset %ld, read_user_count %ld\n", base_user_offset, read_user_count);
                }

                //TODO: have bug
                assert(0 <= read_user_count && read_user_count <= n_max_disk_read_);

                assert(bkt_ub_idx <= bkt_lb_idx);
                read_disk_record_.reset();
                ReadDisk(userID, base_user_offset, read_user_count);
                read_disk_time_ += read_disk_record_.get_elapsed_time_second();
                fine_binary_search_record_.reset();
                int loc_rk = RelativeRankInBucket(user, item, queryIP, (int) read_user_count, bkt_ub_idx, bkt_lb_idx,
                                                  userID);
                fine_binary_search_time_ += fine_binary_search_record_.get_elapsed_time_second();

                int rank = base_rank + loc_rk + 1;
                user_topk_cache_l_[n_candidate_] = UserRankElement(userID, rank, queryIP);
                n_candidate_++;
            }

            std::sort(user_topk_cache_l_.begin(), user_topk_cache_l_.begin() + n_candidate_,
                      std::less());

        }

        [[nodiscard]] int
        RelativeRankInBucket(const VectorMatrix &user, const VectorMatrix &item, const double &queryIP,
                             const int &n_candidate,
                             const int &bucket_ub, const int &bucket_lb, const int &userID) const {
            //calculate all the IP, then get the lower bound
            //make different situation by the information
            std::vector<double> dp_l;
            double bucket_upper_bound =
                    bucket_ub == 0 ? DBL_MAX : IPbound_l_[userID * n_cache_rank_ + bucket_ub - 1];
            double bucket_lower_bound =
                    bucket_lb == n_cache_rank_ ? -DBL_MAX : IPbound_l_[userID * n_cache_rank_ + bucket_lb];
            assert(bucket_lower_bound < queryIP && queryIP < bucket_upper_bound);
            for (int candID = 0; candID < n_candidate; candID++) {
                double ip = InnerProduct(item.getVector(user_disk_cache_[candID]), user.getVector(userID), vec_dim_);
                if (bucket_lower_bound < ip && ip < bucket_upper_bound) {
                    dp_l.emplace_back(ip);
                }
            }
            std::sort(dp_l.begin(), dp_l.end(), std::greater());

            auto lb_ptr = std::lower_bound(dp_l.begin(), dp_l.end(), queryIP,
                                           [](const double &info, double value) {
                                               return info > value;
                                           });
            return (int) (lb_ptr - dp_l.begin());
        }

        void FinishRetrieval() {
            index_stream_.close();
        }

    };
}
#endif //REVERSE_KRANKS_MERGEVECTOR_HPP
