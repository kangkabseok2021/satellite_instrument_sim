add_executable(sat_tests
    test_atmospheric.cpp
    test_frame.cpp
    test_simulator.cpp
)
target_link_libraries(sat_tests PRIVATE sat_core GTest::gtest_main)
target_compile_options(sat_tests PRIVATE -Wall -Wextra)
include(GoogleTest)
gtest_discover_tests(sat_tests)
