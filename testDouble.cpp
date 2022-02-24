//
// Created by BianZheng on 2022/2/24.
//

#include "vector"
#include "fstream"
#include "src/include/util/TimeMemory.hpp"

using namespace std;
using namespace ReverseMIPS;

int main(int argc, char **argv) {
    int n = 100000000;
    vector<double> vecs(n);
    for (int i = 0; i < n; i++) {
        vecs[i] = i - 0.1;
    }
    std::ofstream out("double.index", std::ios::binary | std::ios::out);
    if (!out) {
        std::printf("error in write result\n");
    }
    out.write((char *) vecs.data(), n * sizeof(double));

    vector<double> new_nvecs(n);
    std::ifstream index_stream_ = std::ifstream("double.index", std::ios::binary | std::ios::in);
    if (!index_stream_) {
        std::printf("error in writing index\n");
    }

    TimeRecord record;
    record.reset();
    index_stream_.read((char *) new_nvecs.data(), n * sizeof(double));
    double read_time = record.get_elapsed_time_second();

    double sum = 0;
    for(double item: new_nvecs){
        sum += item;
    }
    printf("%.3f\n", sum);
    printf("read time %.3f\n", read_time);
    return 0;
}