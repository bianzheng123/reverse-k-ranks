#include <random>
#include <limits>
#include <cstdio>

int main() {
    std::random_device rd;
    std::mt19937_64 e(rd());
    std::uniform_int_distribution<uint64_t> dis;
    auto fp = fopen("random_file", "w");
    for (size_t i = 0; i < 1000000000; ++i) {
        auto r = dis(e);
        fwrite(&r, sizeof(r), 1, fp);
    }
    fclose(fp);
}