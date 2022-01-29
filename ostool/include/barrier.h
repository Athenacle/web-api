#ifndef OSTOOL_BARRIER_H_
#define OSTOOL_BARRIER_H_

#include <condition_variable>
#include <cstdint>
#include <mutex>

namespace ostool
{
namespace barrier
{
class Barrier
{
    std::mutex mutex_;
    std::condition_variable cv_;
    uint32_t number_;
    uint32_t total_;

    Barrier() = delete;
    Barrier(const Barrier &) = delete;
    Barrier(Barrier &&) = delete;
    Barrier &operator=(const Barrier &) = delete;
    Barrier &operator=(Barrier &&) = delete;

  public:
    explicit Barrier(uint32_t n)
    {
        total_ = number_ = n;
    }

    ~Barrier() {}

    void wait()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (number_ <= total_) {
            number_--;

            cv_.wait(lock, [this]() { return this->number_ == 0; });
            cv_.notify_all();
        }
    }
};
}  // namespace barrier
}  // namespace ostool
#endif