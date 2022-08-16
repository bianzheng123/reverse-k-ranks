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

        void AddExecuteQuery(const int &n_execute_query) {
            char buff[128];
            sprintf(buff, "number of query item %d", n_execute_query);
            std::string str(buff);
            this->config_l.emplace_back(str);
        }

        void AddQueryInfo(const int n_eval_query) {
            char buff[128];
            sprintf(buff, "number of evaluate query %d", n_eval_query);
            std::string str(buff);
            this->config_l.emplace_back(str);
        }

        void WritePerformance(const char *dataset_name, const char *method_name,
                              const char *other_name) {
            char resPath[256];
            if (strcmp(other_name, "") == 0) {
                std::sprintf(resPath, "../../result/vis_performance/%s-%s-config.txt",
                             dataset_name, method_name);
            } else {
                std::sprintf(resPath, "../../result/vis_performance/%s-%s-%s-config.txt",
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
        }
    };

    void WritePerformanceMetric(const std::vector<size_t> &n_remain_candidate_l,
                                const std::vector<size_t> &io_cost_l,
                                const std::vector<size_t> &ip_cost_l,
                                const char *dataset_name, const char *method_name,
                                const int &topk, const int &n_sample, const int &index_size_gb,
                                const int &n_query_item) {
        assert(n_remain_candidate_l.size() == n_query_item);
        assert(io_cost_l.size() == n_query_item);
        assert(ip_cost_l.size() == n_query_item);
        char resPath[256];
        std::sprintf(resPath,
                     "../../result/attribution/PerformanceMetricRelationship/%s-%s-top%d-n_sample_%d-index_size_gb_%d.txt",
                     dataset_name, method_name, topk, n_sample, index_size_gb);
        std::ofstream file(resPath);
        if (!file) {
            spdlog::error("error in write result");
        }
        file.precision(3);
        file.width(10);
        file << "prune_ratio, io_cost, ip_cost" << std::endl;
        char buff[256];
        for (int queryID = 0; queryID < n_query_item; queryID++) {
            sprintf(buff, "%10ld, %10ld, %10ld\n", n_remain_candidate_l[queryID], io_cost_l[queryID], ip_cost_l[queryID]);
            file << buff;
        }
        file.close();
    }

}