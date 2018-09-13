#include "lightswitch/latch.hpp"
#include <gtest/gtest.h>
#include <chrono>
#include <thread>

constexpr std::size_t size = 2;

class ts_latch : public testing::Test {
public:
    explicit ts_latch() : latch_(size) {}
protected:
    lightswitch::latch latch_;

    void TearDown() {
        // Must unblock all waiting threads
        for (auto i=latch_.count(); i>0; --i) {
            latch_.count_down();
        }
    }
};

TEST_F(ts_latch, count_init) {
    ASSERT_EQ(latch_.count(), size);
}

TEST_F(ts_latch, count_down_decrements) {
    for (auto i=0; i<size; ++i) {
        latch_.count_down();
        ASSERT_EQ(latch_.count(), size - (i + 1));
    }
}

TEST_F(ts_latch, count_down_at_zero) {
    for (auto i=0; i<size; ++i) {
        latch_.count_down();
    }
    ASSERT_EQ(latch_.count(), 0);
    latch_.count_down();
    ASSERT_EQ(latch_.count(), 0);
}

TEST_F(ts_latch, reset_default) {
    latch_.count_down();
    latch_.count_down();
    latch_.reset();
    ASSERT_EQ(latch_.count(), size);
}

TEST_F(ts_latch, reset_value) {
    latch_.count_down();
    latch_.count_down();
    latch_.reset(size * 2);
    ASSERT_EQ(latch_.count(), size * 2);
}

TEST_F(ts_latch, reset_throws) {
    using namespace std::chrono_literals;
    std::thread t([&](){latch_.wait();});
    t.detach();
    std::this_thread::sleep_for(1s);
    ASSERT_THROW(latch_.reset(), std::runtime_error);
    ASSERT_THROW(latch_.reset(size + 1), std::runtime_error);
}

TEST_F(ts_latch, would_block_true) {
    ASSERT_TRUE(latch_.would_block());
}

TEST_F(ts_latch, would_block_false) {
    latch_.reset(0);
    ASSERT_FALSE(latch_.would_block());
}

TEST_F(ts_latch, wait) {
    using namespace std::chrono_literals;
    unsigned c = 0;
    std::thread t([&](){latch_.wait();c++;});
    t.detach();
    std::this_thread::sleep_for(1s);
    for (auto i=0; i<size; ++i) {
        ASSERT_EQ(c, 0);
        latch_.count_down();
    }
    std::this_thread::sleep_for(1s);
    ASSERT_EQ(c, 1);
}

