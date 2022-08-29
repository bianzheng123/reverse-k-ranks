//
// Created by BianZheng on 2022/7/27.
//

#ifndef REVERSE_KRANKS_COMPUTEITEMIDSCORETABLE_HPP
#define REVERSE_KRANKS_COMPUTEITEMIDSCORETABLE_HPP

//#define USE_GPU

#include "struct/DistancePair.hpp"
#include "struct/VectorMatrix.hpp"
#include "util/TimeMemory.hpp"

#include <boost/sort/sort.hpp>
#include <vector>
#include <parallel/algorithm>

#ifdef USE_GPU

#include "score_computation/GPUScoreTable.hpp"
#include "score_computation/GPUSort.hpp"
//#include "score_computation/GPUScoreTableOrigin.hpp"

#else

#include "score_computation/CPUScoreTable.hpp"

#endif


namespace ReverseMIPS {
    class ComputeItemIDScoreTable {
        TimeRecord record_;
        uint64_t n_data_item_;
        std::vector<double> ip_cache_l_;

#ifdef USE_GPU
        GPUScoreTable gpu;
        GPUSort gpu_sort;
#else
        CPUScoreTable cpu;
#endif
    public:
        const int report_every_ = 100000;

        double compute_time_, sort_time_;

        ComputeItemIDScoreTable() = default;

        inline ComputeItemIDScoreTable(const VectorMatrix &user, const VectorMatrix &data_item) {
            const double *user_vecs = user.getRawData();
            const double *item_vecs = data_item.getRawData();
            const uint64_t n_user = user.n_vector_;
            const uint64_t n_data_item = data_item.n_vector_;
            const uint64_t vec_dim = user.vec_dim_;
            assert(user.vec_dim_ == data_item.vec_dim_);
            this->n_data_item_ = n_data_item;
            this->ip_cache_l_.resize(n_data_item);
#ifdef USE_GPU
            gpu = GPUScoreTable(user_vecs, item_vecs, n_user, n_data_item, vec_dim);
            gpu_sort = GPUSort(n_data_item);
#else
            cpu = CPUScoreTable(user_vecs, item_vecs, n_user, n_data_item, vec_dim);
#endif

        }

        void ComputeItems(const int &userID, double *distance_l) {
            record_.reset();
#ifdef USE_GPU
            gpu.ComputeList(userID, distance_l);
#else
            cpu.ComputeList(userID, distance_l);
#endif
            compute_time_ += record_.get_elapsed_time_second();
        }

        void SortItems(const int &userID, double *distance_l) {
            record_.reset();
#ifdef USE_GPU
            gpu_sort.SortList(distance_l);
#else
//                        __gnu_parallel::sort(distance_l, distance_l + n_data_item_, std::greater());
            boost::sort::block_indirect_sort(distance_l, distance_l + n_data_item_, std::greater(),
                                             std::thread::hardware_concurrency());
#endif


            sort_time_ += record_.get_elapsed_time_second();
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
#endif //REVERSE_KRANKS_COMPUTEITEMIDSCORETABLE_HPP