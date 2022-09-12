//
// Created by BianZheng on 2022/9/9.
//

#ifndef REVERSE_KRANKS_SIMPFER_HPP
#define REVERSE_KRANKS_SIMPFER_HPP

#include "alg/SpaceInnerProduct.hpp"
#include "alg/TopkLBHeap.hpp"
#include "alg/DiskIndex/TopTIP.hpp"
#include "alg/RankBoundRefinement/PruneCandidateByBound.hpp"
#include "alg/RankBoundRefinement/RankSearch.hpp"

#include "simpfer/SimpferData.hpp"
#include "simpfer/SimpferBuildIndex.hpp"
#include "simpfer/SimpferRetrieval.hpp"

#include "fexipro/alg/SVDIntUpperBoundIncrPrune2.h"

#include "score_computation/ComputeScoreTable.hpp"
#include "struct/VectorMatrix.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/MethodBase.hpp"
#include "util/TimeMemory.hpp"
#include "util/VectorIO.hpp"
#include "util/FileIO.hpp"
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <map>
#include <set>
#include <cassert>
#include <spdlog/spdlog.h>
#include <filesystem>

namespace ReverseMIPS::Simpfer {

    class Index : public BaseIndex {
        void ResetTimer() {
        }

    public:
        SimpferIndex simpfer_index_;
        Matrix user_matrix_;

        VectorMatrix user_, data_item_;
        int vec_dim_, n_data_item_, n_user_;
        TimeRecord query_record_;

        Index(
                SimpferIndex &&simpfer_index, Matrix &&user_matrix,
                //general retrieval
                VectorMatrix &user, VectorMatrix &data_item)
                : simpfer_index_(std::move(simpfer_index)), user_matrix_(std::move(user_matrix)) {
            //index for reverseMIPS
            //general retrieval
            this->n_user_ = user.n_vector_;
            this->vec_dim_ = user.vec_dim_;
            this->user_ = std::move(user);
            this->n_data_item_ = data_item.n_vector_;
            this->data_item_ = std::move(data_item);
            assert(0 < this->user_.vec_dim_);

        }

        std::vector<std::vector<UserRankElement>>
        Retrieval(const VectorMatrix &query_item, const int &topk, const int &n_execute_query,
                  std::vector<SingleQueryPerformance> &query_performance_l) override {
            ResetTimer();
            if (n_execute_query > query_item.n_vector_) {
                spdlog::error("n_execute_query larger than n_query_item, program exit");
                exit(-1);
            }

            if (topk > user_.n_vector_) {
                spdlog::error("top-k is too large, program exit");
                exit(-1);
            }

            spdlog::info("n_query_item {}", n_execute_query);
            const int n_query_item = n_execute_query;
            std::vector<std::vector<UserRankElement>> query_heap_l(n_query_item);

            std::vector<SimpferData> query_sd_l;
            TransformData(query_item, query_sd_l);

            // store queryIP
            for (int queryID = 0; queryID < n_query_item; queryID++) {
                query_record_.reset();

                int rtk_topk = 1;
                std::vector<int> result_userID_l;
                int n_block_prune = 0;
                int n_sample_prune = 0;
                int n_norm_prune = 0;
                int64_t ip_count = 0;
                int result_size = 0;
                while (rtk_topk < n_data_item_) {
                    simpfer_index_.RTopKRetrieval(query_sd_l[queryID], user_matrix_, rtk_topk,
                                                  result_userID_l,
                                                  n_block_prune, n_sample_prune, n_norm_prune, ip_count, result_size,
                                                  queryID);

                    if (result_size >= topk) {
                        break;
                    } else {
                        rtk_topk *= 2;
                    }

                }

                const double single_query_time = query_record_.get_elapsed_time_second();
                spdlog::info("queryID {}, result_size {}, rtk_topk {}, single_query_time {:.2f}s",
                             queryID, result_size, rtk_topk, single_query_time);
                assert(result_userID_l.size() == result_size);

                for (int resultID = 0; resultID < result_size; resultID++) {
                    const int userID = result_userID_l[resultID];
                    query_heap_l[queryID].push_back(UserRankElement(userID));
                }
            }

            return query_heap_l;
        }

        std::string
        PerformanceStatistics(
                const int &topk,
                const double &retrieval_time,
                const double &ms_per_query) override {
            // int topk;
            //double total_time,
            //          inner_product_time, memory_index_search_time_
            //          read_disk_time_, exact_rank_time_,
            //          rank_bound_prune_ratio_
            //double ms_per_query;
            //unit: second

            char buff[1024];
            sprintf(buff,
                    "top%d retrieval time:\n\ttotal %.3fs\n\tmillion second per query %.3fms",
                    topk, retrieval_time, ms_per_query);
            std::string str(buff);
            return str;
        }

    };

    /*
     * bruteforce index
     * shape: n_user * n_data_item, type: double, the distance pair for each user
     */

    std::unique_ptr<Index> BuildIndex(VectorMatrix &data_item, VectorMatrix &user, const int &simpfer_k_max) {
        const int n_data_item = data_item.n_vector_;
        const int vec_dim = data_item.vec_dim_;
        const int n_user = user.n_vector_;

        user.vectorNormalize();

        std::vector<SimpferData> user_sd_l;
        TransformData(user, user_sd_l);

        std::vector<SimpferData> data_item_sd_l;
        TransformData(data_item, data_item_sd_l);

        ComputeNorm(user_sd_l);
        ComputeNorm(data_item_sd_l);

        // sort by norm in descending order
        std::sort(user_sd_l.begin(), user_sd_l.end(), std::greater());
        std::sort(data_item_sd_l.begin(), data_item_sd_l.end(), std::greater());

        ComputeLowerbound(user_sd_l, data_item_sd_l,
                          n_user, n_data_item,
                          simpfer_k_max, 2, vec_dim);

        std::vector<SimpferBlock> block_l;
        Blocking(user_sd_l, block_l,
                 n_user, simpfer_k_max);

        //transform the input
        const int scaling_value = 1000;
        const double SIGMA = 0.7;

        Matrix data_item_matrix, user_matrix;
        data_item_matrix.init(data_item);
        user_matrix.init(user);

        SIRPrune sirPrune(scaling_value, SIGMA, &data_item_matrix);

        SimpferIndex simpfer_index(user_sd_l, data_item_sd_l, block_l,
                                   std::move(sirPrune),
                                   simpfer_k_max,
                                   n_user, n_data_item, vec_dim);

        std::unique_ptr<Index> index_ptr = std::make_unique<Index>(
                std::move(simpfer_index),
                std::move(user_matrix),
                //general retrieval
                user, data_item);
        return index_ptr;
    }

}
#endif //REVERSE_KRANKS_SIMPFER_HPP
