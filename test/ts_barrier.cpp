#include "lightswitch/barrier.hpp"
#include <gtest/gtest.h>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

constexpr std::size_t size = 3;

class ts_barrier : public testing::Test {
public:
    explicit ts_barrier() : barrier_(size) {}
protected:
    lightswitch::barrier barrier_;
    std::function<void()> thread_fn_;
    std::size_t unlock_counter_;
    std::size_t true_counter_;
    std::mutex mutex_;

    void SetUp() {
        unlock_counter_ = 0;
        true_counter_ = 0;
        thread_fn_ = [&]() {
            bool ret = barrier_.wait();
            std::lock_guard<std::mutex> lock(mutex_);
            unlock_counter_++;
            if (ret) true_counter_++;
        };
    }

    void TearDown() {
        auto curr_count = barrier_.count();
        if (curr_count == size) return;
        // Must free all waiting threads
        while (curr_count > 0) {
            std::thread t([&](){barrier_.wait();});
            t.detach();
            curr_count--;
        }
        // Sleep reasonable amount of time
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1s);
    }
};

TEST_F(ts_barrier, count) {
    using namespace std::chrono_literals;
    ASSERT_EQ(barrier_.count(), size);
    std::thread t1(thread_fn_);
    t1.detach();
    // Sleep reasonable amount of time
    std::this_thread::sleep_for(1s);
    ASSERT_EQ(barrier_.count(), size - 1);
    std::thread t2(thread_fn_);
    t2.detach();
    // Sleep reasonable amount of time
    std::this_thread::sleep_for(1s);
    ASSERT_EQ(barrier_.count(), size - 2);
}

TEST_F(ts_barrier, would_block_true) {
    ASSERT_TRUE(barrier_.would_block());
}

TEST_F(ts_barrier, would_block_false) {
    using namespace std::chrono_literals;
    for (auto i=0; i < size - 1; ++i) {
        std::thread t(thread_fn_);
        t.detach();
    }
    // Sleep reasonable amount of time
    std::this_thread::sleep_for(1s);
    ASSERT_FALSE(barrier_.would_block());
}

TEST_F(ts_barrier, wait_blocked) {
    using namespace std::chrono_literals;
    for (auto i=0; i < size - 1; ++i) {
        std::thread t(thread_fn_);
        t.detach();
    }
    // Sleep reasonable amount of time
    std::this_thread::sleep_for(1s);
    ASSERT_EQ(unlock_counter_, 0);
}

TEST_F(ts_barrier, wait_released) {
    using namespace std::chrono_literals;
    for (auto i=0; i < size; ++i) {
        std::thread t(thread_fn_);
        t.detach();
    }
    // Sleep reasonable amount of time
    std::this_thread::sleep_for(1s);
    ASSERT_EQ(unlock_counter_, size);
}

TEST_F(ts_barrier, wait_return_value) {
    using namespace std::chrono_literals;
    for (auto i=0; i < size; ++i) {
        std::thread t(thread_fn_);
        t.detach();
    }
    // Sleep reasonable amount of time
    std::this_thread::sleep_for(1s);
    ASSERT_EQ(true_counter_, 1);
}

TEST_F(ts_barrier, reset) {
    using namespace std::chrono_literals;
    for (auto i=0; i < size; ++i) {
        std::thread t(thread_fn_);
        t.detach();
    }
    // Sleep reasonable amount of time
    std::this_thread::sleep_for(1s);
    ASSERT_EQ(barrier_.count(), size);
}
