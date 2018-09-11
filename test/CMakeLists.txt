add_executable(
    ts_lightswitch
    ts_lightswitch.cpp
    ts_barrier.cpp
    ts_latch.cpp)

target_link_libraries(
    ts_lightswitch
    PRIVATE
    catch2
    lightswitch)

