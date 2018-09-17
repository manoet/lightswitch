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

set_target_properties(
    ts_lightswitch
    PROPERTIES
    CXX_STANDARD 14)

add_test(ts_lightswitch ts_lightswitch)
