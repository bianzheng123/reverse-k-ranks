#ifndef FILE_READER_HPP
#define FILE_READER_HPP

#include <filesystem>
#include <cassert>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <spdlog/spdlog.h>

template<class T>
struct FileSpan {
    T *data;
    size_t size;
};

class FileReader {
private:
    int return_val_;
    const char *last_call_;
    char *filename_;
    size_t file_length_;
public:
    FileReader(const std::filesystem::path &path) noexcept
            : return_val_(0), last_call_(""), filename_(nullptr), file_length_(0) {

        //test whether can read
        auto fd = open(path.c_str(), O_RDONLY);
        if (fd == -1) {
            last_call_ = "open";
            return_val_ = -1;
        }

        //get the file statistic
        struct stat st;
        auto ret = fstat(fd, &st);
        if (ret == -1) {
            last_call_ = "fstat";
            return_val_ = -1;
            close(fd);
        }

        //get the length and mmap information
        file_length_ = st.st_size;
        filename_ = (char *) mmap64(nullptr, file_length_, PROT_READ, MAP_SHARED, fd, 0);
        if (filename_ == MAP_FAILED) {
            last_call_ = "mmap64";
            return_val_ = -1;
            close(fd);
        }
        close(fd);
    }

    FileReader(FileReader &&r) noexcept
            : return_val_(r.return_val_), last_call_(r.last_call_), filename_(r.filename_), file_length_(r.file_length_) {
        r.filename_ = nullptr;
    }

    FileReader &operator=(FileReader &&rhs) noexcept {
        auto tmp_ret_val = rhs.return_val_;
        auto tmp_last_call = rhs.last_call_;
        auto tmp_addr = rhs.filename_;
        auto tmp_length = rhs.file_length_;
        rhs.filename_ = nullptr;
        using namespace std;
        swap(this->return_val_, tmp_ret_val);
        swap(this->last_call_, tmp_last_call);
        swap(this->filename_, tmp_addr);
        swap(this->file_length_, tmp_length);
        if (tmp_addr) {
            munmap(tmp_addr, tmp_length);
        }
        return *this;
    }

    FileReader(const FileReader &) = delete;

    FileReader &operator=(const FileReader &) = delete;

    ~FileReader() noexcept {
        if (filename_) {
            munmap(filename_, file_length_);
        }
    }

    template<class T>
    FileSpan<T> read(off64_t offset, size_t size) noexcept {
        assert(offset >= 0);
        auto begin = reinterpret_cast<T *>(filename_ + offset);
        auto ele_size = static_cast<size_t>(std::min(file_length_, offset + size * sizeof(T)) -
                                            std::min(file_length_, (size_t) offset)) / sizeof(T);
        return FileSpan<T>{begin, ele_size};
    }

    operator bool() noexcept {
        return return_val_ == 0;
    }

    const char *last_call() noexcept {
        return last_call_;
    }

};

#endif // FILE_READER_HPP