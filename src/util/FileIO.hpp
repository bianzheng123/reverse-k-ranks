#pragma once

#include <fstream>
#include <iostream>
#include "../struct/RankElement.hpp"
#include <vector>
#include <ostream>


namespace ReverseMIPS {

    void writeRank(std::vector<std::vector<RankElement>> result, const char *dataset_name) {
        int n_query_item = result.size();
        int topk = result[0].size();

        char resPath[256];
        std::sprintf(resPath, "../result/%s-index.csv", dataset_name);
        std::ofstream file(resPath);
        if (!file) {
            std::printf("error in write result\n");
        }

        for (int i = 0; i < n_query_item; i++) {
            for (int j = 0; j < topk - 1; j++) {
                file << result[i][j].index << ",";
            }
            file << result[i][topk - 1].index << std::endl;
        }
        file.close();

        std::sprintf(resPath, "../result/%s-rank.csv", dataset_name);
        file.open(resPath);
        if (!file) {
            std::printf("error in write result\n");
        }

        for (int i = 0; i < n_query_item; i++) {
            for (int j = 0; j < topk - 1; j++) {
                file << result[i][j].rank << ",";
            }
            file << result[i][topk - 1].rank << std::endl;
        }
        file.close();
    }
}