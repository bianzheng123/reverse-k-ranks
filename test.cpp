//
// Created by BianZheng on 2021/12/27.
//

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <strstream>
#include <fstream>
#include <iomanip>

//预处理时不做任何动作, 在线计算全部的向量, 然后返回最大的k个rank

using namespace std;


void writePerformance(const char *dataset_name, const char *method_name, const map<string, string> &perform_m) {
    char resPath[256];
    std::sprintf(resPath, "../result/%s-%s-config.txt", dataset_name, method_name);
    std::ofstream file(resPath);
    if (!file) {
        std::printf("error in write result\n");
    }

    for (const pair<const string, string> &iter: perform_m) {
        file << iter.first << ": " << iter.second << "s" << std::endl;
    }

    file.close();
}


string double2string(double number) {
    strstream ss;
    ss.precision(5);
    ss << number;
    return ss.str();
}

int main(int argc, char **argv) {
    char *dataset_name = "test-dataset";
    map<string, string> performance_result_m;
    double total_preprocess_time = 1.54654645;
    double retrieval_time = 2.42840;
    double ip_calc_time = 3.9879802;
    double binary_search_time = 4.26494;

    performance_result_m.emplace("total_preprocess_time", double2string(total_preprocess_time));
    performance_result_m.emplace("retrieval_time", double2string(retrieval_time));

    writePerformance(dataset_name, "DiskIndexBruteForce", performance_result_m);

    return 0;
}