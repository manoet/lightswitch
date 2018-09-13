enable_testing()
find_package(GTest REQUIRED)

add_executable(
    ts_lightswitch
    ts_lightswitch.cpp
    ts_barrier.cpp
    ts_latch.cpp)

target_link_libraries(
    ts_lightswitch
    GTest::GTest
    GTest::Main
    lightswitch)

add_test(ts_lightswitch ts_lightswitch)
