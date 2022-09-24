#define _GNU_SOURCE

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define BLK_SIZE 512

int main() {
    int fd = open("test.txt", O_DIRECT | O_CREAT | O_RDONLY, 0660);
    if (fd == -1) {
        perror("open");
    }
    printf("fd = %d\n", fd);
    char* read_array = aligned_alloc(BLK_SIZE, BLK_SIZE);
    if (!read_array) {
        perror("aligned_alloc");
    }
    printf("read_array = %p\n", read_array);
    ssize_t read_chars = read(fd, read_array, BLK_SIZE);
    if (read_chars == -1) {
        perror("read");
    }
    printf("read_chars = %d\n", (int)(read_chars));
    assert(read_chars == BLK_SIZE);

    double* array = (double*)(read_array);
    for (int i = 0; i < 10; ++i) {
        printf("%f ", array[i]);
    }
    printf("\n");
    free(read_array);
}