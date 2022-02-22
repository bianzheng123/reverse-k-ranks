#include <spdlog/spdlog.h>

int main(int argc, char **argv) {
    char* arr = "arr";
    if(arr == "arr"){
        printf("hello\n");
    }
    spdlog::info("processing cache {} of total {}", 1, 2);
    return 0;
}