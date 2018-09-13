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
 * A barrier is a synchronization mechanism used to synchronize a set of threads
 * so that they wait for each other to reach a certain point.
 * This class implements a reusable barrier, which allows threads to synchronize
 * on the same objects multiple times.
 */
class barrier {
public:
    /**
     * Initialize the barrier to the given value.
     *
     * \param[in] size: number of threads that will use the barrier. Must be
     *                  greater than 0 or the barrier behavior will be undefined
     */
    explicit inline barrier(std::size_t size) noexcept;

    // Barriers can't be copy constructed
    barrier(const barrier &o) = delete;

    // Barriers can't be moved
    barrier(barrier &&o) = delete;

    /**
     * Returns the current counter value
     *
     * Counter is the current number of threads that must arrive to unlock the
     * barrier. When counter reaches 0, the barrier is unlocked.
     */
    inline std::size_t count() noexcept;

    /**
     * Wait for all the threads to invoke wait()
     *
     * If size-1 threads are already waiting on the barrier, then all awaiting
     * threads are notified and the current thread is not blocked; otherwise
     * blocks the current thread until all the threads invoke wait().
     * When all the threads reach the barrier and exit from it, the barrier
     * status is automatically restored so that it is ready to be used again.
     *
     * \return: true to only one thread, false to all the others
     */
    inline bool wait();

    //! Return true if wait() would block, false otherwise
    inline bool would_block() noexcept;
private:
    std::condition_variable cv_;
    std::mutex mutex_;
    std::size_t count_;
    std::size_t reset_;
    std::size_t generation_;
};

inline barrier::barrier(std::size_t size) noexcept :
    count_(size), reset_(size), generation_(0)
{}

inline std::size_t barrier::count() noexcept {
    // No real need to acquire the mutex here
    return count_;
}

inline bool barrier::wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    if (count_ == 1) {
        count_ = reset_;
        generation_++;
        cv_.notify_all();
        return true;
    }
    count_--;
    const auto curr_generation = generation_;
    while (curr_generation == generation_) {
        cv_.wait(lock);
    }
    return false;
}

inline bool barrier::would_block() noexcept {
    // No real need to acquire the mutex here
    return count_ != 1;
}

} // namespace
