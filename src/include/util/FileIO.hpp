#pragma once

#include <fstream>
#include <iostream>
#include "../struct/UserRankElement.hpp"
#include <vector>
#include <ostream>
#include <string>
#include <map>
#include <iomanip>

namespace ReverseMIPS {
    
    void writeRank(std::vector<std::vector<UserRankElement>> &result, const char *dataset_name, const char *method_name) {
        int n_query_item = (int) result.size();
        int topk = (int) result[0].size();

        char resPath[256];
        std::sprintf(resPath, "../result/rank/%s-%s-top%d-index.csv", dataset_name, method_name, topk);
        std::ofstream file(resPath);
        if (!file) {
            std::printf("error in write result\n");
        }

        for (int i = 0; i < n_query_item; i++) {
            for (int j = 0; j < topk - 1; j++) {
                file << result[i][j].userID_ << ",";
            }
            file << result[i][topk - 1].userID_ << std::endl;
        }
        file.close();

        std::sprintf(resPath, "../result/rank/%s-%s-top%d-rank.csv", dataset_name, method_name, topk);
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

        std::sprintf(resPath, "../result/rank/%s-%s-top%d-IP.csv", dataset_name, method_name, topk);
        file.open(resPath);
        if (!file) {
            std::printf("error in write result\n");
        }

        for (int i = 0; i < n_query_item; i++) {
            for (int j = 0; j < topk - 1; j++) {
                file << result[i][j].queryIP_ << ",";
            }
            file << result[i][topk - 1].queryIP_ << std::endl;
        }
        file.close();
    }

    void writePerformance(const char *dataset_name, const char *method_name,
                          const std::map<std::string, std::string> &perform_m) {
        char resPath[256];
        std::sprintf(resPath, "../result/performance/%s-%s-config.txt", dataset_name, method_name);
        std::ofstream file(resPath);
        if (!file) {
            std::printf("error in write result\n");
        }

        for (const std::pair<const std::string, std::string> &iter: perform_m) {
            file << iter.first << ": " << iter.second << "s" << std::endl;
        }

        file.close();
    }


    std::string double2string(double number) {
        char tmp_char[25];
        sprintf(tmp_char, "%.3f", number);
        std::string str(tmp_char);
        return str;
    }

}