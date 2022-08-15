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

    void
    WriteQueryDistribution(const std::vector<int> &topk_rank_l,
                           const std::vector<int> &sample_itemID_l,
                           const int &n_sample_item, const int &topk,
                           const char *dataset_name) {

        assert(topk_rank_l.size() == n_sample_item);
        assert(sample_itemID_l.size() == n_sample_item);

        {
            char resPath[256];
            std::sprintf(resPath,
                         "../../index/query_distribution/%s-kth-rank-n_sample_query_%d-topk_%d.index",
                         dataset_name, n_sample_item, topk);

            std::ofstream out_stream = std::ofstream(resPath, std::ios::binary | std::ios::out);
            if (!out_stream) {
                spdlog::error("error in write result");
                exit(-1);
            }

            out_stream.write((char *) topk_rank_l.data(),
                             (std::streamsize) (n_sample_item * sizeof(int)));

            out_stream.close();
        }

        {
            char resPath[256];
            std::sprintf(resPath, "../../index/query_distribution/%s-sample-itemID-n_sample_query_%d-topk_%d.txt",
                         dataset_name, n_sample_item, topk);

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

    void ReadSampleItemID(const int &n_sample_item, const int &topk, const char *dataset_name,
                          std::vector<int> &sample_itemID_l) {
        assert(sample_itemID_l.size() == n_sample_item);
        char resPath[256];
        std::sprintf(resPath, "../../index/query_distribution/%s-sample-itemID-n_sample_query_%d-topk_%d.txt",
                     dataset_name, n_sample_item, topk);

        std::ifstream in_stream = std::ifstream(resPath, std::ios::in);
        if (!in_stream.is_open()) {
            spdlog::error("error in open file");
            exit(-1);
        }

        for (int rank_itemID = 0; rank_itemID < n_sample_item; rank_itemID++) {
            in_stream >> sample_itemID_l[rank_itemID];
        }

        in_stream.close();
    }

    void ReadKthRank(const int &n_sample_item, const int &topk, const char *dataset_name,
                     std::vector<int> &topk_rank_l) {

        assert(topk_rank_l.size() == n_sample_item);

        char resPath[256];
        std::sprintf(resPath,
                     "../../index/query_distribution/%s-kth-rank-n_sample_query_%d-topk_%d.index",
                     dataset_name, n_sample_item, topk);

        std::ifstream in_stream = std::ifstream(resPath, std::ios::binary | std::ios::in);
        if (!in_stream.is_open()) {
            spdlog::error("error in open file");
            exit(-1);
        }

        in_stream.read((char *) topk_rank_l.data(), sizeof(int) * n_sample_item);
    }

    void WriteDistributionBelowTopk(const std::vector<int> &sample_rank_l,
                                    const int &n_sample_item, const int &topk,
                                    const char *dataset_name) {
        assert(sample_rank_l.size() == n_sample_item * n_sample_item);

        {
            char resPath[256];
            std::sprintf(resPath,
                         "../../index/query_distribution/%s-below-topk-n_sample_query_%d-topk_%d.index",
                         dataset_name, n_sample_item, topk);

            std::ofstream out_stream = std::ofstream(resPath, std::ios::binary | std::ios::out);
            if (!out_stream) {
                spdlog::error("error in write result");
                exit(-1);
            }

            out_stream.write((char *) sample_rank_l.data(),
                             (std::streamsize) (sizeof(int) * n_sample_item * n_sample_item));

            out_stream.close();
        }

    }

}
#endif //REVERSE_KRANKS_FILEIO_HPP
