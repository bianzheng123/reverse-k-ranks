//#define _GNU_SOURCE

#include <cassert>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <spdlog/spdlog.h>

#define DISK_PAGE_SIZE 512

using namespace std;

class DiskData {
public:
    const double *data_;
    size_t n_ele_;

    inline DiskData(const char *data, const size_t &n_ele, const int64_t &offset_byte) {
        this->data_ = (const double *) (data + offset_byte);
        this->n_ele_ = n_ele;
    }
};

class ReadDiskIndex {
    int fileID_;
    char *disk_cache_;
    const char *index_path_;
    int64_t n_user_, n_data_item_;
    int64_t io_ele_unit_, disk_cache_capacity_;

public:
    inline ReadDiskIndex(const int64_t &n_user, const int64_t &n_data_item, const char *index_path) {
        this->n_user_ = n_user;
        this->n_data_item_ = n_data_item;
        this->index_path_ = index_path;
        this->io_ele_unit_ = DISK_PAGE_SIZE / sizeof(double);
        this->disk_cache_capacity_ = (n_data_item_ / io_ele_unit_ + 2) * io_ele_unit_;
        assert(disk_cache_capacity_ >= n_data_item);
    }

    void StartRead() {
        fileID_ = open("test.index", O_DIRECT | O_RDONLY, 0660);
        if (fileID_ == -1) {
            perror("open");
        }

        disk_cache_ = (char *) aligned_alloc(DISK_PAGE_SIZE, disk_cache_capacity_ * sizeof(double));
        assert(disk_cache_capacity_ * sizeof(double) % DISK_PAGE_SIZE == 0);
        if (!disk_cache_) {
            perror("aligned_alloc");
        }
    }

    DiskData ReadDisk(const int &userID, const int &start_idx, const int &read_count) {
        assert(0 <= start_idx && start_idx < n_data_item_);
        assert(0 <= read_count && read_count <= n_data_item_);
        assert(0 <= start_idx + read_count && start_idx + read_count <= n_data_item_);
        const int64_t ele_offset = userID * n_data_item_ + start_idx;
        //make alignment
        const int64_t ele_actual_offset = ele_offset - (ele_offset % io_ele_unit_);

        const int64_t ele_pad_read_count = ele_offset % io_ele_unit_ + read_count;
        const int64_t ele_actual_read_count =
                io_ele_unit_ * (ele_pad_read_count / io_ele_unit_ + (ele_pad_read_count % io_ele_unit_ == 0 ? 0 : 1));
        assert(ele_actual_offset <= n_data_item_ * (userID + 1));
        assert(ele_actual_offset + ele_actual_read_count >= ele_offset + read_count);
        assert(ele_actual_read_count <= disk_cache_capacity_);

        off_t seek_offset = lseek(fileID_, sizeof(double) * ele_actual_offset, SEEK_SET);
        assert(seek_offset == sizeof(double) * ele_actual_offset);
//        printf("seek_offset %ld\n", seek_offset);
        ssize_t read_chars = read(fileID_, disk_cache_, ele_actual_read_count * sizeof(double));
        assert(read_chars != -1 && (read_chars == 0 || read_count <= read_chars));
//        printf("read_chars = %d\n", (int) (read_chars));
        assert(ele_actual_read_count * sizeof(double) % DISK_PAGE_SIZE == 0);

        DiskData res(disk_cache_, read_count, ele_offset % io_ele_unit_ * sizeof(double));
        return res;
    }

    void FinishRead() {
        close(fileID_);
        free(disk_cache_);
    }

};

void TestData(const int &n_user, const int &n_data_item,
              const int &userID, const int &offset, const int &read_count,
              const DiskData &disk_data) {
    uint64_t start_num = userID * n_data_item + offset;
    assert(read_count == disk_data.n_ele_);
    printf("userID %d, offset %d, read_count %d, start_num %ld\n",
           userID, offset, read_count, start_num);
    for (int i = 0; i < read_count; i++) {
        printf("%.0f ", disk_data.data_[i]);
        assert(std::abs(disk_data.data_[i] - (start_num + i)) < 0.01);
    }
    printf("\n");
}

int main() {
//    int fd = open("test.index", O_DIRECT | O_RDONLY, 0660);
//    if (fd == -1) {
//        perror("open");
//    }
//    printf("fd = %d\n", fd);
//    char *read_array = (char *) aligned_alloc(DISK_PAGE_SIZE, 512);
//    if (!read_array) {
//        perror("aligned_alloc");
//    }
//    printf("read_array = %p\n", read_array);
//    off_t seek_offset = lseek(fd, sizeof(double) * 8192, SEEK_SET);
//    printf("seek_offset %ld\n", seek_offset);
//    ssize_t read_chars = read(fd, read_array, 512);
//    if (read_chars == -1) {
//        perror("read\n");
//    }
//    printf("read_chars = %d\n", (int) (read_chars));
//    assert(read_chars % DISK_PAGE_SIZE == 0);
//
//    double *array = (double *) (read_array);
//    for (int i = 0; i < 10; ++i) {
//        printf("%f ", array[i]);
//    }
//    printf("\n");
//    free(read_array);

    const int n_user = 100;
    const int n_data_item = 100;
    const char *index_path = "test.index";
    ReadDiskIndex rdi(n_user, n_data_item, index_path);

    rdi.StartRead();
    {
        const int userID = 10;
        const int start_idx = 50;
        const int read_count = 40;
        DiskData dd = rdi.ReadDisk(userID, start_idx, read_count);
        TestData(n_user, n_data_item,
                 userID, start_idx, read_count, dd);
    }
    {
        const int userID = 99;
        const int start_idx = 50;
        const int read_count = 50;
        DiskData dd = rdi.ReadDisk(userID, start_idx, read_count);
        TestData(n_user, n_data_item,
                 userID, start_idx, read_count, dd);
    }
    {
        const int userID = 99;
        const int start_idx = 99;
        const int read_count = 1;
        DiskData dd = rdi.ReadDisk(userID, start_idx, read_count);
        TestData(n_user, n_data_item,
                 userID, start_idx, read_count, dd);
    }
    {
        const int userID = 3;
        const int start_idx = 1;
        const int read_count = 99;
        DiskData dd = rdi.ReadDisk(userID, start_idx, read_count);
        TestData(n_user, n_data_item,
                 userID, start_idx, read_count, dd);
    }
    {
        const int userID = 99;
        const int start_idx = 0;
        const int read_count = 100;
        DiskData dd = rdi.ReadDisk(userID, start_idx, read_count);
        TestData(n_user, n_data_item,
                 userID, start_idx, read_count, dd);
    }
    {
        const int userID = 99;
        const int start_idx = 1;
        const int read_count = 99;
        DiskData dd = rdi.ReadDisk(userID, start_idx, read_count);
        TestData(n_user, n_data_item,
                 userID, start_idx, read_count, dd);
    }
    {
        const int userID = 98;
        const int start_idx = 10;
        const int read_count = 90;
        DiskData dd = rdi.ReadDisk(userID, start_idx, read_count);
        TestData(n_user, n_data_item,
                 userID, start_idx, read_count, dd);
    }
    {
        const int userID = 0;
        const int start_idx = 1;
        const int read_count = 99;
        DiskData dd = rdi.ReadDisk(userID, start_idx, read_count);
        TestData(n_user, n_data_item,
                 userID, start_idx, read_count, dd);
    }

    rdi.FinishRead();

}