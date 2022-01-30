#pragma once

#include <fstream>
#include <iostream>
#include "../struct/RankElement.hpp"
#include <vector>
#include <ostream>
#include <string>
#include <map>
#include <strstream>
#include <iomanip>

#if defined(_WIN32)

#error "Cannot make directory for windows OS."

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))

#include <sys/stat.h>
#include <sys/types.h>

#else
#error "Cannot make directory for an unknown OS."
#endif

namespace ReverseMIPS {

    void recreateFile(char *filename) {
        if (mkdir(filename, 0755) == -1) {
            printf("can not make directory, delete directory\n");
            char delete_command[256];
            std::sprintf(delete_command, "rm -rf %s", filename);
            if (system(delete_command) == 1) {
                printf("fail to delete the directory command");
            }
        }
        mkdir(filename, 0755);
    }


    void writeRank(std::vector<std::vector<RankElement>> &result, const char *dataset_name, const char *method_name) {
        int n_query_item = (int) result.size();
        int topk = (int) result[0].size();

        char resPath[256];
        std::sprintf(resPath, "../result/%s-%s-top%d-index.csv", dataset_name, method_name, topk);
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

        std::sprintf(resPath, "../result/%s-%s-top%d-rank.csv", dataset_name, method_name, topk);
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

    void writePerformance(const char *dataset_name, const char *method_name,
                          const std::map<std::string, std::string> &perform_m) {
        char resPath[256];
        std::sprintf(resPath, "../result/%s-%s-config.txt", dataset_name, method_name);
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