//
// Created by BianZheng on 2022/4/19.
//

#ifndef REVERSE_KRANKS_CANDIDATEBRUTEFORCE_HPP
#define REVERSE_KRANKS_CANDIDATEBRUTEFORCE_HPP

#include "struct/VectorMatrix.hpp"
#include "struct/UserRankBound.hpp"

//Only for reference
//#include "alg/DiskIndex/RankFromCandidate/GridIndex.hpp"

namespace ReverseMIPS {

    class CandidateBruteForce {
        int n_data_item_, vec_dim_;
    public:
        //in retrieval
        std::vector<double> IPcandidate_l_;

        inline CandidateBruteForce() = default;

        inline CandidateBruteForce(const int &n_data_item, const int &vec_dim) {
            this->n_data_item_ = n_data_item;
            this->vec_dim_ = vec_dim;

            IPcandidate_l_.resize(n_data_item);
        }

        int QueryRankByCandidate
                (const VectorMatrix &user, const VectorMatrix &item, const double &queryIP,
                 const std::vector<UserRankBound> &item_rank_bound_l, const int &userID,
                 const std::pair<double, double> &IPbound_pair, const std::pair<int, int> &rank_bound_pair) {

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
                bool top_element = element.rank_lb_ < query_rank_ub;
                bool bottom_element = query_rank_lb < element.rank_ub_;
                if (top_element || bottom_element) {
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
#endif //REVERSE_KRANKS_CANDIDATEBRUTEFORCE_HPP
