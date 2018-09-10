add_library(lightswitch INTERFACE)

target_include_directories(
    lightswitch
    INTERFACE
    ${CMAKE_CURRENT_LIST_DIR})

find_package(Threads REQUIRED)
target_link_libraries(lightswitch INTERFACE Threads::Threads)
