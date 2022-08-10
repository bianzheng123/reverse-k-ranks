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
    WriteFrequency(const std::vector<int> &user_freq_l, const int n_user,
                   const char *dataset_name, const char *method_name) {

        char resPath[256];
        std::sprintf(resPath, "../../result/attribution/UserQueryRelationship/%s-%s.csv", dataset_name, method_name);
        std::ofstream file(resPath);
        if (!file) {
            std::printf("error in write result\n");
        }

        assert(user_freq_l.size() == n_user);

        for (int userID = 0; userID < n_user; userID++) {
            file << user_freq_l[userID] << std::endl;
        }

        file.close();
    }

    void
    WriteQueryDistribution(const std::vector<UserRankElement> &user_freq_l,
                           const std::vector<int> &sample_itemID_l,
                           const int &n_sample_item, const int &topk,
                           const char *dataset_name) {

        assert(user_freq_l.size() == n_sample_item * topk);
        assert(sample_itemID_l.size() >= n_sample_item);

        {
            char resPath[256];
            std::sprintf(resPath, "../../index/%s-query-distribution-n_sample_query_%d-topk_%d.index",
                         dataset_name, n_sample_item, topk);

            std::ofstream out_stream = std::ofstream(resPath, std::ios::binary | std::ios::out);
            if (!out_stream) {
                spdlog::error("error in write result");
                exit(-1);
            }

            out_stream.write((char *) user_freq_l.data(),
                             (std::streamsize) (n_sample_item * topk * sizeof(UserRankElement)));

            out_stream.close();
        }

        {
            char resPath[256];
            std::sprintf(resPath, "../../index/%s-sample-itemID-n_sample_query_%d.index",
                         dataset_name, n_sample_item);

            std::ofstream out_stream = std::ofstream(resPath, std::ios::binary | std::ios::out);
            if (!out_stream) {
                spdlog::error("error in write result");
                exit(-1);
            }

            out_stream.write((char *) sample_itemID_l.data(),
                             (std::streamsize) (n_sample_item * sizeof(int)));

            out_stream.close();
        }
    }

}
#endif //REVERSE_KRANKS_FILEIO_HPP
