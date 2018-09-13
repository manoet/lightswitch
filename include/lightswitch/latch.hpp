/* Copyright (c) 2018 Marco Giglio
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <condition_variable>
#include <cstdint>
#include <mutex>

namespace lightswitch {

/**
 * A latch is a synchronization mechanism with an internal counter.
 *
 * One or more threads can wait for the counter to become zero, other threads
 * can decrement the counter.
 * This class implements a reusable latch, as the value of the counter can be
 * reset when no thread is blocked.
 */
class latch {
public:
    /**
     * Initialize the latch to the given value
     *
     * \param[in] count: counter initial value
     */
    inline explicit latch(std::size_t count) noexcept;

    // Latches can't be copy constructed
    latch(const latch &o) = delete;

    // Latches can't be moved
    latch(latch &&o) = delete;

    //! Return the current value of the counter
    inline std::size_t count() noexcept;

    /**
     * Decrement the counter by 1
     *
     * If the counter value is already zero, nothing happens.
     * If the counter value becomes zero after the decrement, then all awaiting
     * threads are notified.
     */
    inline void count_down();

    /**
     * Decrement the counter and block the thread if it doesn't reach 0
     *
     * If the counter is already zero, nothing happens.
     * If the counter reaches 0 after the decrement, all awaiting thread are
     * notified and the current thread is not blocked; otherwise no thread is
     * notified and the current thread becomes blocked.
     */
    inline void count_down_and_wait();

    /**
     * Reset the counter to its initial value
     *
     * If no thread is waiting on the latch, then the internal counter is reset
     * to its initial value. If there are threads waiting on the latch, then
     * an exception is generated.
     *
     *\throws runtime_error: if there are threads waiting on the latch
     */
    inline void reset();

    /**
     * Reset the counter to the given value.
     *
     * If no thread is waiting on the latch, then the internal counter is reset
     * to its initial value. If there are threads waiting on the latch, then
     * an exception is generated.
     * Mind that invoking this function doesn't change the default reset value.
     *
     * \param[in] value: new counter value
     * \throws runtime_error: if there are threads waiting on the latch
     */
    inline void reset(std::size_t value);

    /**
     * Block the current thread if the counter value is not 0
     *
     * The calling thread is blocked if the counter value is different from 0.
     * The function terminates immediately if the counter is already 0.
     */
    inline void wait();

    //! Return true if wait() would block, false otherwise
    inline bool would_block() noexcept;
private:
    inline void inner_count_down();
    inline void inner_wait(std::unique_lock<std::mutex> &lock);

    std::size_t count_;
    std::size_t reset_;
    std::size_t waiting_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

inline latch::latch(std::size_t count) noexcept :
  count_(count), reset_(count), waiting_(0)
{}

inline std::size_t latch::count() noexcept {
    // No real need to acquire the mutex here
    return count_;
}

inline void latch::inner_count_down() {
    if (count_ == 0) return;
    count_--;
    if (count_ == 0) {
        cv_.notify_all();
    }
}

inline void latch::count_down() {
    std::lock_guard<std::mutex> lock(mutex_);
    inner_count_down();
}

inline void latch::count_down_and_wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    inner_count_down();
    if (count_ != 0) inner_wait(lock);
}

inline void latch::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (waiting_ != 0) {
        throw std::runtime_error("reset() forbidden while threads are waiting");
    }
    count_ = reset_;
}

inline void latch::reset(std::size_t value) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (waiting_ != 0) {
        throw std::runtime_error("reset() forbidden while threads are waiting");
    }
    count_ = value;
}

inline void latch::inner_wait(std::unique_lock<std::mutex> &lock) {
    waiting_++;
    while(count_ != 0) {
        cv_.wait(lock);
    }
    waiting_--;
}

inline void latch::wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    inner_wait(lock);
}

inline bool latch::would_block() noexcept {
    // No real need to acquire the mutex here
    return count_ != 0;
}

} // namespace
