#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

int main(int argc, char **argv) {
    const char *index_path1 = "../index/test.index";
    const char *index_path2 = "../index/test.index2";
    const char *index_path3 = "../index/test.index3";
    const char *index_path4 = "../index/test.index4";

    ofstream out_stream1 = std::ofstream(index_path1, std::ios::binary | std::ios::out);
    ofstream out_stream2 = std::ofstream(index_path2, std::ios::binary | std::ios::out);
    ofstream out_stream3 = std::ofstream(index_path3, std::ios::binary | std::ios::out);
    ofstream out_stream4 = std::ofstream(index_path4, std::ios::binary | std::ios::out);

    if (!out_stream1 || !out_stream2 || !out_stream3 || !out_stream4) {
        printf("error in write result");
        exit(-1);
    }
    std::vector<int> data_l(100000, 1);
    while (true) {
        out_stream1.write((char *) data_l.data(), sizeof(int));
        out_stream2.write((char *) data_l.data(), sizeof(int));
        out_stream3.write((char *) data_l.data(), sizeof(int));
        out_stream4.write((char *) data_l.data(), sizeof(int));
    }
    return 0;
}
