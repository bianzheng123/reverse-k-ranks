#ifndef FILE_READER_HPP
#define FILE_READER_HPP

#include <filesystem>
#include <cassert>

#include <cassert>

#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <cstring>


class FileReader {
public:
    FileReader(const std::filesystem::path &path) noexcept
            : m_ret_val(0), m_last_call(""), m_addr(nullptr), m_length(0) {
        auto fd = open(path.c_str(), O_RDONLY);
        if (fd == -1) {
            m_last_call = "open";
            m_ret_val = -1;
        }

        struct stat st;
        auto ret = fstat(fd, &st);
        if (ret == -1) {
            m_last_call = "fstat";
            m_ret_val = -1;
            close(fd);
        }

        m_length = st.st_size;
        m_addr = (char *) mmap64(nullptr, m_length, PROT_READ, MAP_SHARED, fd, 0);
        if (m_addr == MAP_FAILED) {
            m_last_call = "mmap64";
            m_ret_val = -1;
            close(fd);
        }
        close(fd);
    }

    FileReader(FileReader &&r) noexcept
            : m_ret_val(r.m_ret_val), m_last_call(r.m_last_call), m_addr(r.m_addr), m_length(r.m_length) {
        r.m_addr = nullptr;
    }

    FileReader &operator=(FileReader &&rhs) noexcept {
        auto tmp_ret_val = rhs.m_ret_val;
        auto tmp_last_call = rhs.m_last_call;
        auto tmp_addr = rhs.m_addr;
        auto tmp_length = rhs.m_length;
        rhs.m_addr = nullptr;
        using namespace std;
        swap(this->m_ret_val, tmp_ret_val);
        swap(this->m_last_call, tmp_last_call);
        swap(this->m_addr, tmp_addr);
        swap(this->m_length, tmp_length);
        if (tmp_addr) {
            munmap(tmp_addr, tmp_length);
        }
        return *this;
    }

    FileReader(const FileReader &) = delete;

    FileReader() = default;

    FileReader &operator=(const FileReader &) = delete;

    ~FileReader() noexcept {
        if (m_addr) {
            munmap(m_addr, m_length);
        }
    }

    template<class T>
    T* read(const off64_t &offset, const size_t &size) noexcept {
        assert(offset >= 0);
        T *begin = reinterpret_cast<T *>(m_addr + offset);
        assert(m_length >= offset + size * sizeof(T));
        size_t ele_size = static_cast<size_t>(std::min(m_length, offset + size * sizeof(T)) -
                                              std::min(m_length, (size_t) offset)) / sizeof(T);
//        std::memcpy(data_ptr, begin, size * sizeof(T));
        return begin;
    }

    operator bool() noexcept {
        return m_ret_val == 0;
    }

    const char *last_call() noexcept {
        return m_last_call;
    }

private:
    int m_ret_val;
    const char *m_last_call;
    char *m_addr;
    size_t m_length;
};

#endif // FILE_READER_HPP