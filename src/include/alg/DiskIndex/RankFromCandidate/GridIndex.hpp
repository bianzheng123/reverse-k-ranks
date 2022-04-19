//
// Created by BianZheng on 2022/4/19.
//

#ifndef REVERSE_KRANKS_GRIDINDEX_HPP
#define REVERSE_KRANKS_GRIDINDEX_HPP

#include "struct/VectorMatrix.hpp"
#include "struct/UserRankBound.hpp"

namespace ReverseMIPS {

    class GridIndex {
    public:
        int n_data_item_, vec_dim_, n_codeword_;
        double max_error_;
        std::unique_ptr<double[]> codebook_; // (n_codeword + 1) * (n_codeword + 1), each cell stores the bound of index

        //in retrieval
        std::vector<double> IPcandidate_l_;

        inline GridIndex() = default;

        inline void Preprocess(const VectorMatrix &user, const VectorMatrix &item, const int &n_codeword) {
            assert(user.vec_dim_ == item.vec_dim_);
            this->n_data_item_ = item.n_vector_;
            this->vec_dim_ = user.vec_dim_;
            this->n_codeword_ = n_codeword;

            IPcandidate_l_.resize(n_data_item_);
            BuildGrid(user, item);
        }

        void BuildGrid(const VectorMatrix &user, const VectorMatrix &item) {
            //get the maximum and minimum absolute value of user and item
            //partition it with different size and split the inner product

        }

        int RankByCandidate
                (const VectorMatrix &user, const VectorMatrix &item, const double &queryIP,
                 const std::vector<UserRankBound> &item_rank_bound_l, const int &userID,
                 const std::pair<double, double> &IPbound_pair, const std::pair<int, int> &rank_bound_pair) {

            //judge negative or positive
            //get the IP bound
            //prune

            //calculate all the IP, then get the lower bound
            //make different situation by the information
            int avail_n_cand = 0;
            const double &IP_lb = IPbound_pair.first;
            const double &IP_ub = IPbound_pair.second;
            const int query_rank_lb = rank_bound_pair.first;
            const int query_rank_ub = rank_bound_pair.second;
            assert(0 <= query_rank_ub && query_rank_ub <= query_rank_lb && query_rank_lb <= n_data_item_);
            assert(IP_lb <= queryIP && queryIP <= IP_ub);
            for (int itemID = 0; itemID < n_data_item_; itemID++) {
                UserRankBound element = item_rank_bound_l[itemID];
                assert(0 <= element.rank_ub_ && element.rank_ub_ <= element.rank_lb_ &&
                       element.rank_lb_ <= n_data_item_);
                bool ub_intersect = element.rank_ub_ <= query_rank_ub && query_rank_ub <= element.rank_lb_;
                bool overlap = query_rank_ub <= element.rank_ub_ && element.rank_lb_ <= query_rank_lb;
                bool lb_intersect = element.rank_ub_ <= query_rank_lb && query_rank_lb <= element.rank_lb_;
                if (ub_intersect || overlap || lb_intersect) {
                    continue;
                }
                double ip = InnerProduct(item.getVector(itemID), user.getVector(userID), vec_dim_);
                if (IP_lb <= ip && ip <= IP_ub) {
                    IPcandidate_l_[avail_n_cand] = ip;
                    avail_n_cand++;
                }
            }

            std::sort(IPcandidate_l_.begin(), IPcandidate_l_.begin() + avail_n_cand, std::greater());

            auto lb_ptr = std::lower_bound(IPcandidate_l_.begin(), IPcandidate_l_.begin() + avail_n_cand,
                                           queryIP,
                                           [](const double &info, double value) {
                                               return info > value;
                                           });
            int loc_rk = (int) (lb_ptr - IPcandidate_l_.begin());

            return loc_rk;
        }
    };
}
#endif //REVERSE_KRANKS_GRIDINDEX_HPP
