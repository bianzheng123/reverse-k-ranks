//
// Created by BianZheng on 2022/7/27.
//

#ifndef REVERSE_KRANKS_FILEIO_HPP
#define REVERSE_KRANKS_FILEIO_HPP

#include "struct/VectorMatrix.hpp"
#include "struct/UserRankElement.hpp"

#include <spdlog/spdlog.h>
#include <vector>
#include <fstream>
#include <algorithm>
#include <random>

namespace ReverseMIPS {

    void WriteDistributionBelowTopk(const std::vector<int> &sample_rank_l,
                                    const std::vector<int> &kth_rank_l,
                                    const std::vector<int> &sample_itemID_l,
                                    const int &n_sample_item, const int &sample_topk,
                                    const char *dataset_name) {
        size_t n_sample_item_size_t = n_sample_item * n_sample_item;
        assert(sample_rank_l.size() == n_sample_item_size_t);
        assert(kth_rank_l.size() == n_sample_item);
        assert(sample_itemID_l.size() == n_sample_item);

        {
            char resPath[256];
            std::sprintf(resPath,
                         "../index/query_distribution/%s-below-topk-n_sample_query_%d-sample_topk_%d.index",
                         dataset_name, n_sample_item, sample_topk);

            std::ofstream out_stream = std::ofstream(resPath, std::ios::binary | std::ios::out);
            if (!out_stream) {
                spdlog::error("error in write result");
                exit(-1);
            }

            out_stream.write((char *) sample_rank_l.data(),
                             (std::streamsize) (sizeof(int) * n_sample_item_size_t));

            out_stream.close();
        }

        {
            char resPath[256];
            std::sprintf(resPath,
                         "../index/query_distribution/%s-kth-rank-n_sample_query_%d-sample_topk_%d.index",
                         dataset_name, n_sample_item, sample_topk);

            std::ofstream out_stream = std::ofstream(resPath, std::ios::binary | std::ios::out);
            if (!out_stream) {
                spdlog::error("error in write result");
                exit(-1);
            }

            out_stream.write((char *) kth_rank_l.data(),
                             (std::streamsize) (n_sample_item * sizeof(int)));

            out_stream.close();
        }

        {
            char resPath[256];
            std::sprintf(resPath, "../index/query_distribution/%s-sample-itemID-n_sample_query_%d-sample_topk_%d.txt",
                         dataset_name, n_sample_item, sample_topk);

            std::ofstream out_stream = std::ofstream(resPath, std::ios::out);
            if (!out_stream) {
                spdlog::error("error in write result");
                exit(-1);
            }
            for (int rank_itemID = 0; rank_itemID < n_sample_item; rank_itemID++) {
                out_stream << sample_itemID_l[rank_itemID] << std::endl;
            }
            out_stream.close();
        }

    }

}
#endif //REVERSE_KRANKS_FILEIO_HPP
