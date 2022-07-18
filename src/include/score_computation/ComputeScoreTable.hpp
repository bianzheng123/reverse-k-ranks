//
// Created by BianZheng on 2022/7/13.
//

#ifndef REVERSE_KRANKS_COMPUTESCORETABLE_HPP
#define REVERSE_KRANKS_COMPUTESCORETABLE_HPP

#define USE_GPU

#include "struct/DistancePair.hpp"
#include "struct/VectorMatrix.hpp"

#include <boost/sort/sort.hpp>
#include <vector>

#ifdef USE_GPU

//#include "score_computation/GPUScoreTable.hpp"
#include "score_computation/GPUScoreTableOrigin.hpp"

#else

#include "score_computation/CPUScoreTable.hpp"

#endif


namespace ReverseMIPS {
    class ComputeScoreTable {
        int n_data_item_;
        std::vector<double> ip_cache_l_;

#ifdef USE_GPU
        GPUScoreTableOrigin gpu;
#else
        CPUScoreTable cpu;
#endif
    public:
        const int report_every_ = 10000;

        ComputeScoreTable() = default;

        inline ComputeScoreTable(const VectorMatrix &user, const VectorMatrix &data_item) {
            const double *user_vecs = user.getRawData();
            const double *item_vecs = data_item.getRawData();
            const int n_user = user.n_vector_;
            const int n_data_item = data_item.n_vector_;
            const int vec_dim = user.vec_dim_;
            this->n_data_item_ = n_data_item;
            this->ip_cache_l_.resize(n_data_item);
#ifdef USE_GPU
            gpu = GPUScoreTableOrigin(user_vecs, item_vecs, n_user, n_data_item, vec_dim);
#else
            cpu = CPUScoreTable(user_vecs, item_vecs, n_user, n_data_item, vec_dim);
#endif

        }

        void ComputeSortItems(const int &userID, double *distance_l) {
#ifdef USE_GPU
            gpu.ComputeList(userID, distance_l);
#else
            cpu.ComputeList(userID, distance_l);
#endif

            boost::sort::parallel_stable_sort(distance_l, distance_l + n_data_item_, std::greater());
        }

        void ComputeSortItems(const int &userID, DistancePair *distance_l) {
#ifdef USE_GPU
            gpu.ComputeList(userID, ip_cache_l_.data());
#else
            cpu.ComputeList(userID, ip_cache_l_.data());
#endif
            for (int itemID = 0; itemID < n_data_item_; itemID++) {
                distance_l[itemID] = DistancePair(ip_cache_l_[itemID], itemID);
            }
            boost::sort::parallel_stable_sort(distance_l, distance_l + n_data_item_, std::greater());
        }

        void FinishCompute() {
#ifdef USE_GPU
            gpu.FinishCompute();
#else
            cpu.FinishCompute();
#endif
        }
    };

}
#endif //REVERSE_KRANKS_COMPUTESCORETABLE_HPP
