//
// Created by BianZheng on 2022/2/25.
//

#include <iostream>
#include <vector>
#include <queue>
#include <fstream>
#include <spdlog/spdlog.h>

using namespace std;

int main(int argc, char **argv) {
    const char* index_path = "../index/fake.index";
    ifstream in_stream_ = std::ifstream(index_path, std::ios::binary | std::ios::in);
    if (!in_stream_) {
        spdlog::error("error in writing index");
        exit(-1);
    }
    double arr[10];
    in_stream_.seekg(4446440, ios::beg);
    in_stream_.read((char*)arr, 10 * sizeof(double));
    for(int i=0;i<10;i++){
        printf("%.3f ", arr[i]);
    }
    printf("\n");
    in_stream_.seekg(4446448, ios::beg);
    in_stream_.read((char*)arr, 10 * sizeof(double));
    for(int i=0;i<10;i++){
        printf("%.3f ", arr[i]);
    }
    printf("\n");
    return 0;
}