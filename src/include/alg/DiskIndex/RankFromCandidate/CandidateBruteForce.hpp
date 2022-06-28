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

        template<typename T>
        int QueryRankByCandidate
                (const double *user_vecs, const VectorMatrix &item,
                 const double &queryIP, const std::pair<double, double> &queryIPbound_pair,
                 const std::vector<std::pair<T, T>> &item_itvID_bound_l,
                 const std::pair<int, int> &query_itvID_bound_pair
                ) {

            //calculate all the IP, then get the lower bound
            //make different situation by the information
            int avail_n_cand = 0;
            const double &queryIP_lb = queryIPbound_pair.first;
            const double &queryIP_ub = queryIPbound_pair.second;

            //set bound for compatible
            const int query_itvID_lb = query_itvID_bound_pair.first;
            const int query_itvID_ub = query_itvID_bound_pair.second;
            assert(query_itvID_ub <= query_itvID_lb);

            assert(queryIP_lb <= queryIP && queryIP <= queryIP_ub);
            for (int itemID = 0; itemID < n_data_item_; itemID++) {
                const T item_itvID_lb = item_itvID_bound_l[itemID].first;
                const T item_itvID_ub = item_itvID_bound_l[itemID].second;
                assert(0 <= item_itvID_ub && item_itvID_ub <= item_itvID_lb &&
                       item_itvID_lb <= n_data_item_);
                bool bottom_query = item_itvID_lb < query_itvID_ub;
                bool top_query = query_itvID_lb < item_itvID_ub;
                if (bottom_query || top_query) {
                    continue;
                }
                double ip = InnerProduct(item.getVector(itemID), user_vecs, vec_dim_);
                if (queryIP_lb <= ip && ip <= queryIP_ub) {
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
