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

#include "lightswitch/latch.hpp"
#include "catch2/catch.hpp"
#include <thread>

constexpr std::size_t latch_size = 3;

SCENARIO("Initialization", "[latch]") {
    lightswitch::latch latch(latch_size);
    GIVEN("a latch") {
        WHEN("latch is built") {
            THEN("count() is equal to the given count") {
                REQUIRE(latch.count() == latch_size);
            }
        }
    }
}

SCENARIO("Counting down", "[latch]") {
    lightswitch::latch latch(latch_size);
    REQUIRE(latch.count() == latch_size);
    GIVEN("a latch") {
        WHEN("would_block() is invoked") {
            THEN("it returns true") {
                REQUIRE(latch.would_block());
            }
        }
        WHEN("count_down is invoked") {
            latch.count_down();
            THEN("count() is decreased") {
                REQUIRE(latch.count() == latch_size - 1);
            }
        }
        WHEN("count reaches zero") {
            std::thread t([&latch](){latch.wait();});
            for (auto i=0; i<latch_size; ++i) {
                latch.count_down();
            }
            THEN("threads are unlocked") {
                t.join();
                REQUIRE(true);
            }
            THEN("would_block() returns false") {
                REQUIRE_FALSE(latch.would_block());
            }
            WHEN("count_down invoked again") {
                latch.count_down();
                THEN("count_down has no effect") {
                    REQUIRE(latch.count() == 0);
                }
            }
        }
    }
}

SCENARIO("Reset mechanism", "[latch]") {
    GIVEN("a latch with count to 0") {
        lightswitch::latch latch(latch_size);
        for (auto i=0; i<latch_size; ++i) {
            latch.count_down();
        }
        WHEN("reset() is invoked") {
            latch.reset();
            THEN("count is restored") {
                REQUIRE(latch.count() == latch_size);
            }
        }
        WHEN("reset(N) is invoked") {
            auto new_count = latch_size + 3;
            latch.reset(new_count);
            THEN("count is N") {
                REQUIRE(latch.count() == new_count);
            }
        }
    }

    GIVEN("a thread waiting on the latch") {
        lightswitch::latch latch(latch_size);
        std::thread t([&latch](){latch.wait();});
        WHEN("reset() is invoked") {
            THEN("an error is thrown") {
                REQUIRE_THROWS_AS(latch.reset(), std::runtime_error);
            }
        }
        WHEN("reset(N) is invoked") {
            THEN("an error is thrown") {
                REQUIRE_THROWS_AS(latch.reset(42), std::runtime_error);
            }
        }
    }
}
