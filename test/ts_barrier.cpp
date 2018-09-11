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

#include "lightswitch/barrier.hpp"
#include "catch2/catch.hpp"
#include <array>
#include <cstdint>
#include <mutex>
#include <thread>

constexpr std::size_t barrier_size = 5;

SCENARIO("barriers block threads until N-th thread arrive", "[barrier]") {
    GIVEN("A barrier of size N") {
        lightswitch::barrier barrier(barrier_size);
        std::mutex mutex;
        std::uint8_t counter = 0;
        std::uint8_t true_counter = 0;
        auto thread_body = [&counter, &true_counter, &barrier, &mutex]() {
            bool ret = barrier.wait();
            std::unique_lock<std::mutex> lock(mutex);
            counter++;
            if (ret) true_counter++;
        };

        WHEN("N-1 threads arrive") {
            std::thread t1(thread_body);
            std::thread t2(thread_body);
            std::thread t3(thread_body);
            std::thread t4(thread_body);
            THEN("all are blocked") {
                REQUIRE(counter == 0);
            }
            THEN("would block returns false") {
                REQUIRE_FALSE(barrier.would_block());
            }
            WHEN("the last thread arrives") {
                std::thread t5(thread_body);
                THEN("all are unblocked") {
                    t1.join();
                    t2.join();
                    t3.join();
                    t4.join();
                    t5.join();
                    REQUIRE(counter == 5);
                }
                THEN("only one returns true") {
                    REQUIRE(true_counter == 1);
                }
            }
        }
    }
}

SCENARIO("barriers can be reused", "[barrier]") {
    lightswitch::barrier barrier(barrier_size);
    REQUIRE(barrier.count() == barrier_size);
    auto thread_body = [&barrier]() {
        barrier.wait();
    };
    GIVEN("A barrier of size N") {
        WHEN("All threads arrive") {
            std::thread t1(thread_body);
            std::thread t2(thread_body);
            std::thread t3(thread_body);
            std::thread t4(thread_body);
            std::thread t5(thread_body);
            t1.join();
            t2.join();
            t3.join();
            t4.join();
            t5.join();
            THEN("count is N again") {
                REQUIRE(barrier.count() == barrier_size);
            }
        }
    }
}
