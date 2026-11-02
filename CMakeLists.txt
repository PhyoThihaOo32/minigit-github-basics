cmake_minimum_required(VERSION 3.20)
project(MiniGitWeek1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(minigit_paths src/PathLab.cpp)
target_include_directories(minigit_paths PUBLIC include)

include(CTest)
if(BUILD_TESTING)
    add_executable(path_lab_tests tests/PathLabTests.cpp)
    target_link_libraries(path_lab_tests PRIVATE minigit_paths)
    add_test(NAME path_lab_tests COMMAND path_lab_tests)

    add_executable(read_failure_tests tests/ReadFailureTests.cpp)
    target_link_libraries(read_failure_tests PRIVATE minigit_paths)
    target_include_directories(read_failure_tests PRIVATE src)
    add_test(NAME read_failure_tests COMMAND read_failure_tests)
endif()
