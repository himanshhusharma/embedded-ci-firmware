cmake_minimum_required(VERSION 3.20)
project(embedded_ci_firmware LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(FW_BUILD_TESTS "Build host unit tests" ON)
option(FW_BUILD_APP "Build host simulation app" ON)

# --- firmware library: hardware-independent, no heap, no exceptions ---------
add_library(fw_core STATIC
    src/crc8.cpp
    src/overspeed_monitor.cpp
    src/speed_frame.cpp
)
target_include_directories(fw_core PUBLIC include)
target_compile_options(fw_core PRIVATE
    -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow -Werror
)

if(CMAKE_CROSSCOMPILING)
    # Target build: just the library, sized for a Cortex-M.
    return()
endif()

if(FW_BUILD_APP)
    add_executable(speed_sim app/main.cpp)
    target_link_libraries(speed_sim PRIVATE fw_core)
    target_compile_options(speed_sim PRIVATE -Wall -Wextra -Werror)
endif()

if(FW_BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
        URL_HASH SHA256=8ad598c73ad796e0d8280b082cebd82a630d73e73cd3c70057938a6501bba5d7
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    add_executable(fw_tests
        tests/test_crc8.cpp
        tests/test_ring_buffer.cpp
        tests/test_moving_average.cpp
        tests/test_overspeed_monitor.cpp
        tests/test_speed_frame.cpp
    )
    target_link_libraries(fw_tests PRIVATE fw_core GTest::gtest_main)
    target_compile_options(fw_tests PRIVATE -Wall -Wextra -Werror)
    include(GoogleTest)
    gtest_discover_tests(fw_tests)
endif()
