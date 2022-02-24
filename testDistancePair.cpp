//
// Created by BianZheng on 2022/2/24.
//

#include "src/include/struct/DistancePair.hpp"
#include "src/include/util/TimeMemory.hpp"
#include "vector"
#include "fstream"

using namespace std;
using namespace ReverseMIPS;

int main(int argc, char **argv) {
    int n = 100000000;
    vector<DistancePair> vecs(n);
    for (int i = 0; i < n; i++) {
        vecs[i].ID_ = i;
        vecs[i].dist_ = i - 0.1;
    }
    std::ofstream out("distancepair.index", std::ios::binary | std::ios::out);
    if (!out) {
        std::printf("error in write result\n");
    }
    out.write((char *) vecs.data(), n * sizeof(DistancePair));
    out.close();
    vecs.clear();

    vector<DistancePair> new_nvecs(n);
    std::ifstream index_stream_ = std::ifstream("distancepair.index", std::ios::binary | std::ios::in);
    if (!index_stream_) {
        std::printf("error in writing index\n");
    }

    TimeRecord record;
    record.reset();
    index_stream_.read((char *) new_nvecs.data(), n * sizeof(DistancePair));
    double read_time = record.get_elapsed_time_second();

    double sum = 0;
    for(DistancePair item: new_nvecs){
        sum += item.dist_;
    }
    printf("%.3f\n", sum);
    printf("read time %.3f\n", read_time);
    return 0;
}