//
// Created by BianZheng on 2022/7/3.
//

#ifndef REVERSE_K_RANKS_MERGERANKBYBITMAP_HPP
#define REVERSE_K_RANKS_MERGERANKBYBITMAP_HPP

#include "alg/SpaceInnerProduct.hpp"
//#include "alg/Cluster/KMeansParallel.hpp"
#include "alg/Cluster/GreedyMergeMinClusterSize.hpp"
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

    class Bitmap {
    public:
        std::unique_ptr<unsigned char[]> bit_l_;
        int bitmap_size_;

        Bitmap() = default;

        Bitmap(const int &n_data_item) {
            int bitmap_size = n_data_item / 8;
            bitmap_size += n_data_item % 8 == 0 ? 0 : 1;
            this->bitmap_size_ = bitmap_size;
            bit_l_ = std::make_unique<unsigned char[]>(bitmap_size_);
            std::memset(bit_l_.get(), 0, bitmap_size_ * sizeof(unsigned char));
        }

        bool Find(const int &itemID) const {
            const int num_offset = itemID / 8;
            const int bit_offset = itemID % 8;
            assert(0 <= num_offset && num_offset <= bitmap_size_);
            assert(0 <= bit_offset && bit_offset <= 8 * sizeof(unsigned char));
            if ((bit_l_[num_offset]) & (1 << bit_offset)) {//find
                return true;
            } else {
                return false;
            }
        }

        void AssignVector(const int &n_data_item, std::vector<bool> candidate_l) {
            assert(candidate_l.size() <= bitmap_size_ * 8);
            assert(n_data_item == candidate_l.size());
            char num_offset = 0;
            int bit_offset = 0;
            for (int itemID = 0; itemID < n_data_item; itemID++) {
                const bool is_appear = (bit_l_[num_offset]) & (1 << bit_offset);
                candidate_l[itemID] = is_appear;
                num_offset++;
                if (num_offset % 8 == 0) {
                    num_offset = 0;
                    bit_offset++;
                }
            }
        }

    };

    class BitmapArr {
    private:
        std::unique_ptr<unsigned char[]> bit_l_;
        int n_data_item_, topt_, bitmap_size_;
        uint64_t bitmap_size_byte_, user_size_byte_;
    public:

        BitmapArr() = default;

        BitmapArr(const int &n_data_item, const int &topt) {
            this->n_data_item_ = n_data_item;
            this->topt_ = topt;
            int bitmap_size = n_data_item / 8;
            bitmap_size += n_data_item % 8 == 0 ? 0 : 1;
            this->bitmap_size_ = bitmap_size;
            this->user_size_byte_ = bitmap_size_ * topt_ * sizeof(unsigned char);
            this->bitmap_size_byte_ = bitmap_size_ * sizeof(unsigned char);
            bit_l_ = std::make_unique<unsigned char[]>(bitmap_size_ * topt_);
            std::memset(bit_l_.get(), 0, user_size_byte_);
        }

        void Add(const int &itemID, const int rank_offset) {
            assert(rank_offset <= topt_);
            assert(itemID < n_data_item_);
            const int num_offset = itemID / 8;
            const int bit_offset = itemID % 8;
            assert(0 <= num_offset && num_offset <= bitmap_size_byte_);
            assert(0 <= bit_offset && bit_offset <= 8 * sizeof(unsigned char));
            bit_l_[rank_offset * bitmap_size_ + num_offset] |= (1 << bit_offset);
        }

        void WriteDisk(std::ofstream &out_stream) {
            out_stream.write((char *) bit_l_.get(),
                             (std::streamsize) (user_size_byte_));
            std::memset(bit_l_.get(), 0, user_size_byte_);
        }

        inline void ReadDisk(std::ifstream &index_stream, const int &labelID,
                             const int &start_idx, const int &read_count, Bitmap &bitmap) {
            assert(start_idx + read_count < topt_);
            std::basic_istream<char>::off_type offset_byte = user_size_byte_ * labelID +
                                                             bitmap_size_byte_ * start_idx;
            index_stream.seekg(offset_byte, std::ios::beg);
            index_stream.read((char *) bit_l_.get(),
                              (std::streamsize) (bitmap_size_byte_ * read_count));

            std::memset(bitmap.bit_l_.get(), 0, bitmap.bitmap_size_ * sizeof(unsigned char));
            assert(bitmap_size_ == bitmap.bitmap_size_);
            for (int rank = 0; rank < read_count; rank++) {
                int bitmap_offset = rank * bitmap_size_;
                for (int bitmapID = 0; bitmapID < bitmap_size_; bitmapID++) {
                    bitmap.bit_l_[bitmapID] |= bit_l_[bitmap_offset + bitmapID];
                }
            }
        }

    };

    class MergeRankByBitmap {
    public:
        //index variable
        int n_user_, n_data_item_, vec_dim_, n_interval_;
        int topt_, n_merge_user_, bitmap_size_byte_;
        //n_cache_rank_: stores how many intervals for each merged user
        std::vector<uint32_t> merge_label_l_; // n_user, stores which cluster the user belons to
        CandidateBruteForce exact_rank_ins_;
        const char *index_path_;

        //record time memory
        TimeRecord read_disk_record_, exact_rank_refinement_record_;
        double read_disk_time_, exact_rank_refinement_time_;

        //variable in build index
        std::ofstream out_stream_;
        BitmapArr bitmap_cache_;
        Bitmap bitmap_;

        //variable in retrieval
        std::ifstream index_stream_;
        int n_candidate_;
        std::vector<UserRankElement> user_topk_cache_l_; //n_user, used for sort the element to return the top-k
        std::vector<bool> item_cand_l_;

        inline MergeRankByBitmap() {}

        inline MergeRankByBitmap(const CandidateBruteForce &exact_rank_ins, const VectorMatrix &user,
                                 const char *index_path, const int &n_data_item, const int &n_interval,
                                 const int &topt, const int &n_merge_user, const int &bitmap_size_byte) {
            this->n_user_ = user.n_vector_;
            this->vec_dim_ = user.vec_dim_;
            this->index_path_ = index_path;
            this->n_data_item_ = n_data_item;

            this->n_interval_ = n_interval;
            this->topt_ = topt;
            this->n_merge_user_ = n_merge_user;
            this->bitmap_size_byte_ = bitmap_size_byte;
            this->exact_rank_ins_ = exact_rank_ins;
            assert(bitmap_size_byte_ == (n_data_item / 8 + (n_data_item % 8 == 0 ? 0 : 1)));

            this->merge_label_l_.resize(n_user_);
            this->bitmap_cache_ = BitmapArr(n_data_item_, topt_);
            this->bitmap_ = Bitmap(n_data_item_);
            this->user_topk_cache_l_.resize(n_user_);
            this->item_cand_l_.resize(n_data_item_);

            BuildIndexPreprocess(user);
        }

        void
        BuildIndexPreprocess(const VectorMatrix &user) {
            merge_label_l_ = GreedyMergeMinClusterSize::ClusterLabel(user, n_merge_user_);

//            printf("cluster size\n");
//            for (int mergeID = 0; mergeID < n_merge_user_; mergeID++) {
//                int count = 0;
//                for (int userID = 0; userID < n_user_; userID++) {
//                    if (merge_label_l_[userID] == mergeID) {
//                        count++;
//                    }
//                }
//                printf("%d ", count);
//            }
//            printf("\n");

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

        void BuildIndexLoop(const DistancePair *distance_ptr, const int &userID) {
            for (int rank = 0; rank < topt_; rank++) {
                int itemID = distance_ptr[rank].ID_;
                bitmap_cache_.Add(itemID, rank);
            }
        }

        void WriteIndex() {
            bitmap_cache_.WriteDisk(out_stream_);
        }

        void FinishWrite() {
            out_stream_.close();
            std::ifstream index_stream(index_path_, std::ios::binary);;
            index_stream.seekg(0, std::ios::end);
            std::ios::pos_type ss = index_stream.tellg();
            auto fsize = (size_t) ss;
            assert(fsize == n_merge_user_ * topt_ * bitmap_size_byte_ * sizeof(unsigned char));
            index_stream.close();
        }

        inline void RetrievalPreprocess() {
            read_disk_time_ = 0;
            exact_rank_refinement_time_ = 0;
            index_stream_ = std::ifstream(this->index_path_, std::ios::binary | std::ios::in);
            if (!index_stream_) {
                spdlog::error("error in writing index");
            }
        }

        void GetRank(const std::vector<double> &queryIP_l,
                     const std::vector<int> &rank_lb_l, const std::vector<int> &rank_ub_l,
                     const std::vector<std::pair<double, double>> &queryIPbound_l,
                     const std::vector<bool> &prune_l,
                     const VectorMatrix &user, const VectorMatrix &item) {
            int n_below = 0;
            int n_between = 0;
            int n_above = 0;
            n_candidate_ = 0;
            //read disk and fine binary search
            for (int userID = 0; userID < n_user_; userID++) {
                if (prune_l[userID]) {
                    continue;
                }
                const int base_rank = rank_ub_l[userID];
                const double queryIP = queryIP_l[userID];
                int loc_rk;
                if (rank_lb_l[userID] == rank_ub_l[userID]) {
                    loc_rk = 0;
                } else {
                    const int rank_ub = rank_ub_l[userID];
                    const int rank_lb = rank_lb_l[userID];

                    const double *user_vecs = user.getVector(userID);
                    if (rank_lb < topt_) {
                        TimeRecord record;
                        record.reset();
                        //retrieval the top-t like before
                        loc_rk = BelowTopt(item, user_vecs, queryIP,
                                           rank_lb, rank_ub, queryIPbound_l[userID],
                                           userID);
                        n_below++;
                        double below_time = record.get_elapsed_time_second();
                    } else if (rank_ub <= topt_ && topt_ <= rank_lb) {
                        loc_rk = BetweenTopt(item, user_vecs, queryIP,
                                             rank_lb, rank_ub, queryIPbound_l[userID],
                                             userID);
                        n_between++;
                    } else if (topt_ < rank_ub) {
                        loc_rk = AboveTopt(item, user_vecs, queryIP, rank_ub);
                        n_above++;
                    } else {
                        spdlog::error("have bug in get rank, topt ID IP");
                    }

                }

                int rank = base_rank + loc_rk + 1;

                user_topk_cache_l_[n_candidate_] = UserRankElement(userID, rank, queryIP);
                n_candidate_++;
            }

            std::sort(user_topk_cache_l_.begin(), user_topk_cache_l_.begin() + n_candidate_,
                      std::less());

        }

        int AboveTopt(const VectorMatrix &item, const double *user_vecs, const double &queryIP, const int &rank_ub) {
            const int base_rank = rank_ub;
            exact_rank_refinement_record_.reset();
            const int rank = exact_rank_ins_.QueryRankByCandidate(user_vecs, item, queryIP);
            exact_rank_refinement_time_ += exact_rank_refinement_record_.get_elapsed_time_second();
            const int loc_rk = rank - base_rank;
            return loc_rk;
        }

        int BetweenTopt(const VectorMatrix &item, const double *user_vecs, const double &queryIP,
                        const int &rank_lb, const int &rank_ub, const std::pair<double, double> &queryIP_bound_pair,
                        const int &userID) {
            const int user_labelID = (int) merge_label_l_[userID];
            const int read_count = topt_ - rank_ub;
            const int base_rank = rank_ub;
            assert(0 <= user_labelID && user_labelID < n_merge_user_);

            read_disk_record_.reset();
            bitmap_cache_.ReadDisk(index_stream_, user_labelID, base_rank, read_count, bitmap_);
            read_disk_time_ += read_disk_record_.get_elapsed_time_second();

            item_cand_l_.assign(n_data_item_, false);
            assert(0 <= rank_ub && rank_ub <= rank_lb && rank_lb < topt_);
            bitmap_.AssignVector(n_data_item_, item_cand_l_);

            exact_rank_refinement_record_.reset();
            int loc_rk = exact_rank_ins_.QueryRankByCandidate(user_vecs, item,
                                                              queryIP, queryIP_bound_pair,
                                                              item_cand_l_);
            exact_rank_refinement_time_ += exact_rank_refinement_record_.get_elapsed_time_second();

            if (loc_rk == read_count) {
                exact_rank_refinement_record_.reset();
                const int rank = exact_rank_ins_.QueryRankByCandidate(user_vecs, item, queryIP);
                exact_rank_refinement_time_ += exact_rank_refinement_record_.get_elapsed_time_second();
                loc_rk = rank - base_rank;
            }
            return loc_rk;
        }

        int BelowTopt(const VectorMatrix &item, const double *user_vecs, const double &queryIP,
                      const int &rank_lb, const int &rank_ub, const std::pair<double, double> &queryIP_bound_pair,
                      const int &userID) {
            const int user_labelID = (int) merge_label_l_[userID];
            const int read_count = rank_lb - rank_ub;
            const int base_rank = rank_ub;
            assert(0 <= user_labelID && user_labelID < n_merge_user_);

            read_disk_record_.reset();
            bitmap_cache_.ReadDisk(index_stream_, user_labelID, base_rank, read_count, bitmap_);
            read_disk_time_ += read_disk_record_.get_elapsed_time_second();

            item_cand_l_.assign(n_data_item_, false);
            assert(0 <= rank_ub && rank_ub <= rank_lb && rank_lb < topt_);
            bitmap_.AssignVector(n_data_item_, item_cand_l_);

            exact_rank_refinement_record_.reset();
            const int loc_rk = exact_rank_ins_.QueryRankByCandidate(user_vecs, item,
                                                                    queryIP, queryIP_bound_pair,
                                                                    item_cand_l_);
            exact_rank_refinement_time_ += exact_rank_refinement_record_.get_elapsed_time_second();
            return loc_rk;
        }

        void FinishRetrieval() {
            index_stream_.close();
        }

    };
}
#endif //REVERSE_K_RANKS_MERGERANKBYBITMAP_HPP
