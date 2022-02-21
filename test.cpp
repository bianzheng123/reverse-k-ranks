#include <spdlog/spdlog.h>

int main(int argc, char **argv) {
    spdlog::info("processing cache {} of total {}", 1, 2);
    return 0;
}