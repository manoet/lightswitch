add_executable(
    ts_lightswitch
    ts_lightswitch.cpp)

target_link_libraries(
    ts_lightswitch
    PRIVATE
    catch2
    lightswitch)
