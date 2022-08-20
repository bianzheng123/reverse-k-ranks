#include <stdio.h>
#include <vector>
#include "util/FileReader.hpp"

int main() {
    FileReader fileReader{};
    fileReader = FileReader("../index/fake-normal_QueryRankSearch128.index");
//    std::vector<double> vecs_l(10);
    double* data = fileReader.read<double>(0, sizeof(double) * 10);
    for(int i=0;i<10;i++){
        printf("%.3f ", data[i]);
    }
    printf("\n");
    return 0;
}