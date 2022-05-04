//
// Created by BianZheng on 2022/4/13.
//

#ifndef REVERSE_KRANKS_MERGEVECTOR_HPP
#define REVERSE_KRANKS_MERGEVECTOR_HPP

#include "alg/SpaceInnerProduct.hpp"
#include "alg/KMeans/KMeansParallel.hpp"
#include "alg/DiskIndex/RankFromCandidate/CandidateBruteForce.hpp"
#include "struct/DistancePair.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/UserRankBound.hpp"
#include "util/TimeMemory.hpp"
#include <set>
#include <cfloat>
#include <memory>
#include <spdlog/spdlog.h>
#include <armadillo>

namespace ReverseMIPS {

    class MergeRankBound {
    public:
        //index variable
        int n_user_, n_data_item_, vec_dim_, n_merge_user_;
        //n_cache_rank_: stores how many intervals for each merged user
        std::vector<uint32_t> merge_label_l_; // n_user, stores which cluster the user belons to
        CandidateBruteForce exact_rank_ins_;
        const char *index_path_;

        //record time memory
        TimeRecord read_disk_record_, fine_binary_search_record_;
        double read_disk_time_, fine_binary_search_time_;

        //variable in build index
        std::ofstream out_stream_;
        //n_data_item, stores the UserRankBound in the disk, used for build index and retrieval
        std::vector<UserRankBound> disk_cache_l_;

        //variable in retrieval
        std::ifstream index_stream_;
        int n_candidate_;
        std::vector<bool> is_compute_l_;
        std::vector<UserRankElement> user_topk_cache_l_; //n_user, used for sort the element to return the top-k

        inline MergeRankBound() {}

        inline MergeRankBound(const CandidateBruteForce &exact_rank_ins, const VectorMatrix &user,
                              const int &n_data_item, const char *index_path,
                              const int &n_merge_user) {
            this->n_user_ = user.n_vector_;
            this->n_data_item_ = n_data_item;
            this->vec_dim_ = user.vec_dim_;
            this->index_path_ = index_path;
            this->n_merge_user_ = n_merge_user;
            exact_rank_ins_ = exact_rank_ins;

            spdlog::info("n_merge_user {}", n_merge_user_);

            this->merge_label_l_.resize(n_user_);
            this->disk_cache_l_.resize(n_data_item);
            for (int itemID = 0; itemID < n_data_item; itemID++) {
                this->disk_cache_l_[itemID].Reset();
            }
            this->is_compute_l_.resize(n_merge_user_);
            this->is_compute_l_.assign(n_merge_user_, false);
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

        void BuildIndexLoop(const std::vector<DistancePair> &distance_pair_l, const int &userID) {
            for (int rank = 0; rank < n_data_item_; rank++) {
                int itemID = distance_pair_l[rank].ID_;
                disk_cache_l_[itemID].Merge(rank);
            }
        }

        void WriteIndex() {
            //get the number of users in each bucket, assign into the cache_bkt_vec
            assert(disk_cache_l_.size() == n_data_item_);

            for (int itemID = 0; itemID < n_data_item_; itemID++) {
                assert(disk_cache_l_[itemID].rank_lb_ != -1 && disk_cache_l_[itemID].rank_ub_ != -1);
                out_stream_.write((char *) disk_cache_l_.data(),
                                  (std::streamsize) (n_data_item_ * sizeof(UserRankBound)));
            }

            for (int itemID = 0; itemID < n_data_item_; itemID++) {
                disk_cache_l_[itemID].Reset();
            }

        }

        void FinishWrite() {
            out_stream_.close();
        }

        inline void RetrievalPreprocess() {
            read_disk_time_ = 0;
            fine_binary_search_time_ = 0;
            index_stream_ = std::ifstream(this->index_path_, std::ios::binary | std::ios::in);
            if (!index_stream_) {
                spdlog::error("error in writing index");
            }
        }

        void GetRank(const std::vector<double> &queryIP_l,
                     const std::vector<int> &rank_lb_l, const std::vector<int> &rank_ub_l,
                     const std::vector<std::pair<double, double>> &IPbound_l,
                     const std::vector<bool> &prune_l, const VectorMatrix &user, const VectorMatrix &item) {
            //read disk and fine binary search
            n_candidate_ = 0;
            for (int iter_userID = 0; iter_userID < n_user_; iter_userID++) {
                if (prune_l[iter_userID]) {
                    continue;
                }
                int iter_labelID = (int) merge_label_l_[iter_userID];
                if (is_compute_l_[iter_labelID]) {
                    continue;
                }
                read_disk_record_.reset();
                ReadDisk(iter_labelID);
                read_disk_time_ += read_disk_record_.get_elapsed_time_second();
                for (int userID = iter_userID; userID < n_user_; userID++) {
                    int user_labelID = (int) merge_label_l_[userID];
                    if (user_labelID != iter_labelID) {
                        continue;
                    }
                    assert(0 <= rank_ub_l[userID] && rank_ub_l[userID] <= rank_lb_l[userID] &&
                           rank_lb_l[userID] <= n_data_item_);
                    fine_binary_search_record_.reset();
                    double queryIP = queryIP_l[userID];
                    int base_rank = rank_ub_l[userID];
                    int loc_rk = exact_rank_ins_.QueryRankByCandidate(user, item, queryIP, disk_cache_l_,
                                                                      userID, IPbound_l[userID],
                                                                      std::make_pair(rank_lb_l[userID],
                                                                                     rank_ub_l[userID]));
                    int rank = base_rank + loc_rk + 1;
                    fine_binary_search_time_ += fine_binary_search_record_.get_elapsed_time_second();

                    user_topk_cache_l_[n_candidate_] = UserRankElement(userID, rank, queryIP);
                    n_candidate_++;

                }
                is_compute_l_[iter_labelID] = true;
            }

            is_compute_l_.assign(n_merge_user_, false);

            std::sort(user_topk_cache_l_.begin(), user_topk_cache_l_.begin() + n_candidate_,
                      std::less());

        }

        inline void ReadDisk(const int &labelID) {
            uint64_t offset = n_data_item_ * labelID;
            std::basic_istream<char>::off_type offset_byte = offset * sizeof(UserRankBound);
            index_stream_.seekg(offset_byte, std::ios::beg);
            index_stream_.read((char *) disk_cache_l_.data(), n_data_item_ * sizeof(UserRankBound));
        }

        void FinishRetrieval() {
            index_stream_.close();
        }

    };
}
#endif //REVERSE_KRANKS_MERGEVECTOR_HPP
