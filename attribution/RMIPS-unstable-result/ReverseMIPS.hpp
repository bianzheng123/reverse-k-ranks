//
// Created by BianZheng on 2022/11/11.
//

#ifndef REVERSE_KRANKS_REVERSEMIPS_HPP
#define REVERSE_KRANKS_REVERSEMIPS_HPP

#include "alg/SpaceInnerProduct.hpp"
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

namespace ReverseMIPS::RMIPS {

    class Index {
        void ResetTimer() {
            total_retrieval_time_ = 0;
            total_ip_cost_ = 0;
        }

    public:
        SimpferIndex simpfer_index_;

        VectorMatrix user_, data_item_;
        int vec_dim_, n_data_item_, n_user_;
        double total_retrieval_time_;
        TimeRecord total_retrieval_record_;
        size_t total_ip_cost_;

        Index(
                SimpferIndex &&simpfer_index,
                //general retrieval
                VectorMatrix &user, VectorMatrix &data_item)
                : simpfer_index_(std::move(simpfer_index)) {
            //index for reverseMIPS
            //general retrieval
            this->n_user_ = user.n_vector_;
            this->vec_dim_ = user.vec_dim_;
            this->user_ = std::move(user);
            this->n_data_item_ = data_item.n_vector_;
            this->data_item_ = std::move(data_item);
            assert(0 < this->user_.vec_dim_);

        }

        std::vector<int>
        Retrieval(const VectorMatrix &query_item, const int &topk, const int &n_execute_query) {
            ResetTimer();
            if (n_execute_query > query_item.n_vector_) {
                spdlog::error("n_execute_query larger than n_query_item, program exit");
                exit(-1);
            }

            if (topk > user_.n_vector_) {
                spdlog::error("top-k is too large, program exit");
                exit(-1);
            }

            const int n_query_item = n_execute_query;
            std::vector<int> query_heap_l(n_query_item);

            std::vector<SimpferData> query_sd_l;
            TransformData(query_item, query_sd_l);

            // store queryIP
            for (int queryID = 0; queryID < n_query_item; queryID++) {
                total_retrieval_record_.reset();

                std::vector<int> result_userID_l;
                int n_block_prune = 0;
                int n_sample_prune = 0;
                int n_norm_prune = 0;
                size_t ip_count = 0;
                int result_size = 0;

                simpfer_index_.RTopKRetrieval(query_sd_l[queryID], topk,
                                              result_userID_l,
                                              n_block_prune, n_sample_prune, n_norm_prune, ip_count, result_size);


                const double query_time = total_retrieval_record_.get_elapsed_time_second();
                total_retrieval_time_ += query_time;
                total_ip_cost_ += ip_count;

                spdlog::info(
                        "queryID {}, result_size {}, ip_cost {}, query_time {:.2f}s, total_ip_cost {}, total_retrieval_time {:.2f}s",
                        queryID, result_size, ip_count, query_time, total_ip_cost_, total_retrieval_time_);
                assert(result_userID_l.size() == result_size);

                query_heap_l[queryID] = result_size;
            }

            return query_heap_l;
        }

        std::string
        PerformanceStatistics(const int &topk) {
            // int topk;
            //size_t total_ip_cost;
            //double total_time,
            //unit: second

            char buff[1024];
            sprintf(buff,
                    "top%d retrieval:\n\ttotal IP cost %ld, total time %.3fs",
                    topk, total_ip_cost_, total_retrieval_time_);
            std::string str(buff);
            return str;
        }

        uint64_t IndexSizeByte() {
            return simpfer_index_.IndexSizeByte();
        }

    };

    /*
     * bruteforce index
     * shape: n_user * n_data_item, type: double, the distance pair for each user
     */

    std::unique_ptr<Index>
    BuildIndex(VectorMatrix &data_item, VectorMatrix &user, const int &simpfer_k_max) {
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

        SimpferIndex simpfer_index(user_sd_l, data_item_sd_l, block_l,
                                   simpfer_k_max,
                                   n_user, n_data_item, vec_dim);

        std::unique_ptr<Index> index_ptr = std::make_unique<Index>(
                std::move(simpfer_index),
                //general retrieval
                user, data_item);
        return index_ptr;
    }

}
#endif //REVERSE_KRANKS_REVERSEMIPS_HPP
