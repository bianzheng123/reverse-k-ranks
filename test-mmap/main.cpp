#include "FileReader.hpp"

int main(int argc, char *argv[]) {
    auto fr = FileReader("random_file");
    if (!fr) {
        perror(fr.last_call());
        exit(EXIT_FAILURE);
    }
    uint64_t ret = 0;
    auto array = fr.read<uint64_t>(0, (sizeof(size_t) * 1000000000));
    for (size_t i = 0; i < array.size; ++i) {
        if (i % 10000000 == 0) {
            printf("%f%%\n", 1.0 * i / array.size * 100.);
        }
        ret += array.data[i];
    }
    printf("%lu\n", ret);

    // auto fr = FileReader("test.txt");
    // if (!fr) {
    //     perror(fr.last_call());
    //     exit(EXIT_FAILURE);
    // }
    // auto array = fr.read<double>(0, 10);
    // for (auto v: array) {
    //     printf("%f ", v);
    // }
    // printf("\n");
}