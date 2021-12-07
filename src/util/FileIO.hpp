#pragma once

#include <fstream>
#include <iostream>
#include "../struct/RankElement.hpp"
#include <vector>
#include <ostream>
#include <iomanip>


namespace ReverseMIPS {

    void writeRank(std::vector<std::vector<RankElement>> result, const char *dataset_name, const char *method_name) {
        int n_query_item = result.size();
        int topk = result[0].size();

        char resPath[256];
        std::sprintf(resPath, "../result/%s-%s-index.csv", dataset_name, method_name);
        std::ofstream file(resPath);
        if (!file) {
            std::printf("error in write result\n");
        }

        for (int i = 0; i < n_query_item; i++) {
            for (int j = 0; j < topk - 1; j++) {
                file << result[i][j].index_ << ",";
            }
            file << result[i][topk - 1].index_ << std::endl;
        }
        file.close();

        std::sprintf(resPath, "../result/%s-%s-rank.csv", dataset_name, method_name);
        file.open(resPath);
        if (!file) {
            std::printf("error in write result\n");
        }

        for (int i = 0; i < n_query_item; i++) {
            for (int j = 0; j < topk - 1; j++) {
                file << result[i][j].rank_ << ",";
            }
            file << result[i][topk - 1].rank_ << std::endl;
        }
        file.close();
    }

    void writeConfig(const char *dataset_name, const char *method_name, float preprocess_time, float retrieval_time) {
        char resPath[256];
        std::sprintf(resPath, "../result/%s-%s-config.txt", dataset_name, method_name);
        std::ofstream file(resPath);
        if (!file) {
            std::printf("error in write result\n");
        }

        file << "preprocess time: " << std::fixed << std::setprecision(5) << preprocess_time << "s" << std::endl;
        file << "retrieval time: " << std::fixed << std::setprecision(5) << retrieval_time << "s" << std::endl;

        file.close();
    }
}