#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include "struct/UserRankElement.hpp"
#include <vector>
#include <ostream>
#include <string>
#include <map>
#include <iomanip>

namespace ReverseMIPS {
    class RetrievalResultBase {
    public:
        std::vector<std::string> config_l;

        void writePerformance(const char *problem_name, const char *dataset_name, const char *method_name,
                              const char *other_name = nullptr) {
            char resPath[256];
            if (other_name == nullptr) {
                std::sprintf(resPath, "../result/performance/%s-%s-%s-config.txt",
                             problem_name, dataset_name, method_name);
            } else {
                std::sprintf(resPath, "../result/performance/%s-%s-%s-%s-config.txt",
                             problem_name, dataset_name, method_name, other_name);
            }
            std::ofstream file(resPath);
            if (!file) {
                std::printf("error in write result\n");
            }
            int config_size = (int) config_l.size();
            for (int i = config_size - 1; i >= 0; i--) {
                file << config_l[i] << std::endl;
            }

            file.close();
        }
    };

    void
    writeRkRankResult(const std::vector<std::vector<UserRankElement>> &result, const char *dataset_name,
                      const char *method_name,
                      const char *other_name = nullptr) {
        int n_query_item = (int) result.size();
        int topk = (int) result[0].size();

        char resPath[256];
        if (other_name == nullptr) {
            std::sprintf(resPath, "../result/rank-RkRank/%s-%s-top%d-userID.csv", dataset_name, method_name, topk);
        } else {
            std::sprintf(resPath, "../result/rank-RkRank/%s-%s-top%d-%s-userID.csv", dataset_name, method_name, topk,
                         other_name);
        }
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

        if (other_name == nullptr) {
            std::sprintf(resPath, "../result/rank-RkRank/%s-%s-top%d-rank.csv", dataset_name, method_name, topk);
        } else {
            std::sprintf(resPath, "../result/rank-RkRank/%s-%s-top%d-%s-rank.csv", dataset_name, method_name, topk,
                         other_name);
        }
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

        if (other_name == nullptr) {
            std::sprintf(resPath, "../result/rank-RkRank/%s-%s-top%d-IP.csv", dataset_name, method_name, topk);
        } else {
            std::sprintf(resPath, "../result/rank-RkRank/%s-%s-top%d-%s-IP.csv", dataset_name, method_name, topk,
                         other_name);
        }
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

    void
    writeRTopkResult(std::vector<std::vector<UserRankElement>> &result, const int &topk, const char *dataset_name,
                     const char *method_name, const char *other_name = nullptr) {
        int n_query_item = (int) result.size();

        for (int qID = 0; qID < n_query_item; qID++) {
            std::sort(result[qID].begin(), result[qID].end(), UserRankElement::UserIDMinFirst);
        }

        char resPath[256];
        if (other_name == nullptr) {
            std::sprintf(resPath, "../result/rank-RTopk/%s-%s-top%d-userID.csv", dataset_name, method_name, topk);
        } else {
            std::sprintf(resPath, "../result/rank-RTopk/%s-%s-top%d-%s-userID.csv", dataset_name, method_name, topk,
                         other_name);
        }
        std::ofstream file(resPath);
        if (!file) {
            std::printf("error in write result\n");
        }

        for (int qID = 0; qID < n_query_item; qID++) {
            int size = result[qID].size();
            for (int j = 0; j < size - 1; j++) {
                file << result[qID][j].userID_ << ",";
            }
            if (size != 0) {
                file << result[qID][size - 1].userID_ << std::endl;
            }
        }
        file.close();

        if (other_name == nullptr) {
            std::sprintf(resPath, "../result/rank-RTopk/%s-%s-top%d-IP.csv", dataset_name, method_name, topk);
        } else {
            std::sprintf(resPath, "../result/rank-RTopk/%s-%s-top%d-%s-IP.csv", dataset_name, method_name, topk,
                         other_name);
        }
        file.open(resPath);
        if (!file) {
            std::printf("error in write result\n");
        }

        for (int qID = 0; qID < n_query_item; qID++) {
            int size = result[qID].size();
            for (int j = 0; j < size - 1; j++) {
                file << result[qID][j].queryIP_ << ",";
            }
            if (size != 0) {
                file << result[qID][size - 1].queryIP_ << std::endl;
            }
        }
        file.close();
    }

}