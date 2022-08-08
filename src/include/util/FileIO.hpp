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
    class RedundantRetrievalResult {
    public:
        std::string performance_metric_name;
        std::vector<std::vector<std::string>> config_l_l;

        void WritePerformance(const char *dataset_name, const char *method_name,
                              const char *other_name, const std::vector<int> &topk_l) {
            char resPath[256];
            const int n_topk = (int) topk_l.size();
            for (int topkID = 0; topkID < n_topk; topkID++) {
                const int topk = topk_l[topkID];
                if (strcmp(other_name, "") == 0) {
                    std::sprintf(resPath, "../result/performance/%s-%s-top%d-variance-config.csv",
                                 dataset_name, method_name, topk);
                } else {
                    std::sprintf(resPath, "../result/performance/%s-%s-%s-top%d-variance-config.csv",
                                 dataset_name, method_name, other_name, topk);
                }
                std::ofstream file(resPath);
                if (!file) {
                    spdlog::error("error in write result");
                }
                file << performance_metric_name << std::endl;
                const std::vector<std::string> &config_l = config_l_l[topkID];
                const int &n_query = (int) config_l_l[topkID].size();
                for (int queryID = 0; queryID < n_query; queryID++) {
                    file << config_l[queryID] << std::endl;
                }

                file.close();
            }
        }
    };

    class PlotPerformance {
    public:
        double retrieval_time_, ms_per_query_;
        int topk_;

        PlotPerformance() = default;

        PlotPerformance(const int &topk, const double &retrieval_time, const double &ms_per_query) {
            this->topk_ = topk;
            this->retrieval_time_ = retrieval_time;
            this->ms_per_query_ = ms_per_query;
        }
    };

    class RetrievalResult {
        std::vector<std::string> config_l;
        std::vector<PlotPerformance> performance_l;
    public:

        std::string GetConfig(const int ID) {
            return config_l[ID];
        }

        void AddBuildIndexInfo(const std::string &str) {
            this->config_l.emplace_back(str);
        }

        void AddRetrievalInfo(const std::string &str, const int &topk, const double &retrieval_time,
                              const double &ms_per_query) {
            this->config_l.emplace_back(str);
            performance_l.emplace_back(topk, retrieval_time, ms_per_query);
        }

        void AddBuildIndexTime(const double &build_index_time) {
            char buff[128];
            sprintf(buff, "build index time %.3fs", build_index_time);
            std::string str(buff);
            this->config_l.emplace_back(str);
        }

        void AddExecuteQuery(const int& n_execute_query){
            char buff[128];
            sprintf(buff, "number of query item %d", n_execute_query);
            std::string str(buff);
            this->config_l.emplace_back(str);
        }

        void AddQueryInfo(const int n_eval_query){
            char buff[128];
            sprintf(buff, "number of evaluate query %d", n_eval_query);
            std::string str(buff);
            this->config_l.emplace_back(str);
        }

        void WritePerformance(const char *dataset_name, const char *method_name,
                              const char *other_name) {
            char resPath[256];
            if (strcmp(other_name, "") == 0) {
                std::sprintf(resPath, "../result/vis_performance/%s-%s-config.txt",
                             dataset_name, method_name);
            } else {
                std::sprintf(resPath, "../result/vis_performance/%s-%s-%s-config.txt",
                             dataset_name, method_name, other_name);
            }
            std::ofstream file(resPath);
            if (!file) {
                spdlog::error("error in write result");
            }
            int config_size = (int) config_l.size();
            for (int i = config_size - 1; i >= 0; i--) {
                file << config_l[i] << std::endl;
            }
            file.close();


            if (strcmp(other_name, "") == 0) {
                std::sprintf(resPath, "../result/plot_performance/%s-%s-config.csv",
                             dataset_name, method_name);
            } else {
                std::sprintf(resPath, "../result/plot_performance/%s-%s-%s-config.csv",
                             dataset_name, method_name, other_name);
            }
            file.open(resPath);
            if (!file) {
                spdlog::error("error in write result");
            }
            int n_topk = (int) performance_l.size();
            file << "topk,retrieval_time,million_second_per_query" << std::endl;
            for (int i = n_topk - 1; i >= 0; i--) {
                file << performance_l[i].topk_ << "," << performance_l[i].retrieval_time_ << ","
                     << performance_l[i].ms_per_query_ << std::endl;
            }

            file.close();
        }
    };

    void
    WriteRankResult(const std::vector<std::vector<UserRankElement>> &result, const char *dataset_name,
                    const char *method_name,
                    const char *other_name) {
        int n_query_item = (int) result.size();
        int topk = (int) result[0].size();

        char resPath[256];
        if (strcmp(other_name, "") == 0) {
            std::sprintf(resPath, "../result/rank/%s-%s-top%d-userID.csv", dataset_name, method_name, topk);
        } else {
            std::sprintf(resPath, "../result/rank/%s-%s-top%d-%s-userID.csv", dataset_name, method_name, topk,
                         other_name);
        }
        std::ofstream file(resPath);
        if (!file) {
            spdlog::error("error in write result");
        }

        for (int i = 0; i < n_query_item; i++) {
            for (int j = 0; j < topk - 1; j++) {
                file << result[i][j].userID_ << ",";
            }
            file << result[i][topk - 1].userID_ << std::endl;
        }
        file.close();

        if (strcmp(other_name, "") == 0) {
            std::sprintf(resPath, "../result/rank/%s-%s-top%d-rank.csv", dataset_name, method_name, topk);
        } else {
            std::sprintf(resPath, "../result/rank/%s-%s-top%d-%s-rank.csv", dataset_name, method_name, topk,
                         other_name);
        }
        file.open(resPath);
        if (!file) {
            spdlog::error("error in write result");
        }

        for (int i = 0; i < n_query_item; i++) {
            for (int j = 0; j < topk - 1; j++) {
                file << result[i][j].rank_ << ",";
            }
            file << result[i][topk - 1].rank_ << std::endl;
        }
        file.close();

        if (strcmp(other_name, "") == 0) {
            std::sprintf(resPath, "../result/rank/%s-%s-top%d-IP.csv", dataset_name, method_name, topk);
        } else {
            std::sprintf(resPath, "../result/rank/%s-%s-top%d-%s-IP.csv", dataset_name, method_name, topk,
                         other_name);
        }
        file.open(resPath);
        if (!file) {
            spdlog::error("error in write result");
        }

        for (int i = 0; i < n_query_item; i++) {
            for (int j = 0; j < topk - 1; j++) {
                file << result[i][j].queryIP_ << ",";
            }
            file << result[i][topk - 1].queryIP_ << std::endl;
        }
        file.close();
    }

}