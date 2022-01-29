#ifndef OSTOOL_UTILS_H
#define OSTOOL_UTILS_H

#include <cassert>
#include <cstring>
#include <string>
#include <vector>

namespace ostool
{
namespace cxxutils
{

void split(const std::string& in, std::vector<std::string>& out, const std::string& sep);

class Buffer
{
    //std::basic_string<uint8_t> b_;
    uint8_t* b_;
    uint8_t* p_;
    size_t bs_;


    void alloc(size_t size)
    {
        assert(size > bs_);

        size = size + (size >> 1);
        auto n = new uint8_t[size];
        memcpy(n, b_, this->size());
        p_ = n + this->size();

        bs_ = size;
        delete[] b_;
        b_ = n;
    }

    Buffer& operator=(const Buffer&) = delete;
    // Buffer& operator=(Buffer&&) = delete;

    Buffer(Buffer&&) = delete;

  public:
    Buffer& operator=(Buffer&& other)
    {
        p_ = other.p_;
        b_ = other.b_;
        bs_ = other.bs_;
        other.b_ = other.p_ = nullptr;
        other.bs_ = 0;
        return *this;
    }

    Buffer(const Buffer& other) : Buffer()
    {
        if (other.size() > 0) {
            alloc(other.bs_);
            memcpy(b_, other.b_, other.size());
            p_ = b_ + other.size();
        }
    }
    Buffer()
    {
        bs_ = 0;
        p_ = b_ = nullptr;
    }

    Buffer(size_t s) : Buffer()
    {
        if (s > 0) {
            bs_ = s;
            p_ = b_ = new uint8_t[bs_];
        }
    }
    ~Buffer()
    {
        delete[] b_;
    }

    void append(const Buffer& buffer)
    {
        append(buffer.c_str(), buffer.size());
    }

    void append(const uint8_t* d, size_t s)
    {
        reserve(size() + s);
        memcpy(p_, d, s);

        p_ += s;
    }

    void append(uint8_t d, size_t s)
    {
        reserve(size() + s);
        for (size_t i = 0; i < s; i++) {
            *p_ = d;
            p_++;
        }
    }

    void clear()
    {
        p_ = b_;
        memset(p_, 0, bs_);
    }


    size_t size() const
    {
        return p_ - b_;
    }

    void reserve(size_t size)
    {
        if (size > bs_) {
            alloc(size);
        }
    }

    const uint8_t* c_str() const
    {
        return b_;
    }

    std::string base64();
};


class LineIO
{
    std::string buffer_;

  public:
    void feed(const char* d, size_t);
    std::string eat();
    std::string eatAll();
};

}  // namespace cxxutils

}  // namespace ostool

#endif