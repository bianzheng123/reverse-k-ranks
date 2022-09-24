#include <memory>

#include <cassert>
#include <cstdio>
#include <cstdlib>

#include <fcntl.h>
#include <unistd.h>

#define BLK_SIZE 512

int main() {
    auto close_lambda = [](int *pfd) { close(*pfd); };
    auto free_lambda = [](void *p) { free(p); };
    int fd = open("test.txt", O_DIRECT | O_CREAT | O_RDONLY, 0660);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    auto fd_guard = std::unique_ptr<int, decltype(close_lambda)>(&fd, close_lambda);
    auto buf = std::unique_ptr<double[], decltype(free_lambda)>(static_cast<double*>(aligned_alloc(BLK_SIZE, BLK_SIZE)), free_lambda);
    if (!buf) {
        perror("aligned_alloc");
        exit(EXIT_FAILURE);
    }
    ssize_t read_chars = read(fd, buf.get(), BLK_SIZE);
    if (read_chars == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }
    assert(read_chars == BLK_SIZE);

    for (int i = 0; i < 10; ++i) {
        printf("%f ", buf[i]);
    }
    printf("\n");
}