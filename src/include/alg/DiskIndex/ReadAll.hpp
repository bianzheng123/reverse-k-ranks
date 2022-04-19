//
// Created by BianZheng on 2022/4/12.
//

#ifndef REVERSE_K_RANKS_READALL_HPP
#define REVERSE_K_RANKS_READALL_HPP

#include <memory>
#include <spdlog/spdlog.h>

namespace ReverseMIPS {

    class ReadAll {
        int n_data_item_, n_user_;
        const char *index_path_;
        int n_max_disk_read_;

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

        void
        BuildIndexPreprocess() {
            out_stream_ = std::ofstream(index_path_, std::ios::binary | std::ios::out);
            if (!out_stream_) {
                spdlog::error("error in write result");
                exit(-1);
            }
        }

    public:

        TimeRecord read_disk_record_, fine_binary_search_record_;
        double read_disk_time_, fine_binary_search_time_;

        //variable in build index
        std::ofstream out_stream_;

        //variable in retrieval
        std::ifstream index_stream_;
        std::unique_ptr<double[]> disk_cache_;
        int n_candidate_;
        std::vector<UserRankElement> user_topk_cache_l_;


        inline ReadAll() {}

        inline ReadAll(const int &n_user, const int &n_data_item, const char *index_path, const int n_max_disk_read) {
            this->n_user_ = n_user;
            this->n_data_item_ = n_data_item;
            this->index_path_ = index_path;
            this->n_max_disk_read_ = n_max_disk_read;
            this->disk_cache_ = std::make_unique<double[]>(n_max_disk_read);
            this->user_topk_cache_l_.resize(n_user);

            BuildIndexPreprocess();
        }

        void BuildIndexLoop(const std::vector<double> &distance_cache, const int &n_write) {
            // distance_cache: write_every * n_data_item_, n_write <= write_every
            assert(distance_cache.size() >= n_write * n_data_item_);
            out_stream_.write((char *) distance_cache.data(), n_write * n_data_item_ * sizeof(double));
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

        void FinishRetrieval() {
            index_stream_.close();
        }

    };
}
#endif //REVERSE_K_RANKS_READALL_HPP
