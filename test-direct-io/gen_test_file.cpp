//
// Created by BianZheng on 2022/9/24.
//

#include <fstream>
#include <iostream>
#include <vector>

#define BLK_SIZE 512

using namespace std;

int main() {

    const char *index_path = "test.index";
    std::ofstream out_stream = std::ofstream(index_path, std::ios::binary | std::ios::out);
    if (!out_stream) {
        printf("error in write result\n");
        exit(-1);
    }
    const int length = 10000;
    std::vector<double> vecs(length);
    for (int i = 0; i < length; i++) {
        vecs[i] = (double) i;
    }
    out_stream.write((char *) vecs.data(), length * sizeof(double));
    out_stream.close();
    printf("finish write\n");
    return 0;
}