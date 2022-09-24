#define _GNU_SOURCE

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define BLK_SIZE 512

int main() {
    int fd = open("test.txt", O_DIRECT | O_SYNC | O_CREAT | O_WRONLY, 0660);
    if (fd == -1) {
        perror("open");
    }
    printf("fd = %d\n", fd);
    double array[10] = {1., 2., 3., 4., 5., 6., 7., 8., 9., 10.};
    char* write_buf = aligned_alloc(BLK_SIZE, BLK_SIZE);
    printf("write_buf = %p\n", write_buf);
    memset(write_buf, 0, BLK_SIZE);
    memcpy(write_buf, array, sizeof(array));
    ssize_t written = write(fd, write_buf, BLK_SIZE);
    if (written == -1) {
        perror("write");
    }
    printf("written = %d\n", (int)(written));
    assert(written == BLK_SIZE);
    free(write_buf);
}