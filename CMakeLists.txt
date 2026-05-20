cmake_minimum_required(VERSION 3.21)
cmake_policy(SET CMP0135 NEW)
project(SatelliteInstrumentSim C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build GoogleTest suite" ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_library(sat_core STATIC
    src/AtmosphericModel.cpp
    src/TelemetryFrame.cpp
    src/TelemetryPipe.cpp
    src/AtmosphericSimulator.cpp
)
target_include_directories(sat_core PUBLIC include)
target_compile_options(sat_core PRIVATE -Wall -Wextra)

add_executable(satellite_sim src/main.cpp)
target_link_libraries(satellite_sim PRIVATE sat_core)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
