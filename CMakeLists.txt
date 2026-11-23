cmake_minimum_required(VERSION 3.20)
project(MiniGitWeek1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(minigit_paths src/PathLab.cpp src/RepositoryPaths.cpp src/FileSystem.cpp)
target_sources(minigit_paths PRIVATE
    src/ReadIndex.cpp
    src/MiniGitManager.cpp
    src/FileSystemPaths.cpp
    src/CommandLineApp.cpp)
target_include_directories(minigit_paths PUBLIC include)

add_executable(minigit src/main.cpp)
target_link_libraries(minigit PRIVATE minigit_paths)

include(CTest)
if(BUILD_TESTING)
    # Compile each future header independently, without linking unfinished methods.
    set(interface_header_sources)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/interface_headers")
    foreach(header IN ITEMS CommandLineApp MiniGitManager IndexTable WorkingTree CommitStore FileState)
        set(source "${CMAKE_CURRENT_BINARY_DIR}/interface_headers/${header}.cpp")
        file(WRITE "${source}" "#include \"${header}.hpp\"\n")
        list(APPEND interface_header_sources "${source}")
    endforeach()
    add_library(interface_headers_check OBJECT ${interface_header_sources})
    target_link_libraries(interface_headers_check PRIVATE minigit_paths)

    add_executable(path_lab_tests tests/PathLabTests.cpp)
    target_link_libraries(path_lab_tests PRIVATE minigit_paths)
    add_test(NAME path_lab_tests COMMAND path_lab_tests)

    add_executable(read_failure_tests tests/ReadFailureTests.cpp)
    target_link_libraries(read_failure_tests PRIVATE minigit_paths)
    target_include_directories(read_failure_tests PRIVATE src)
    add_test(NAME read_failure_tests COMMAND read_failure_tests)

    add_executable(read_index_tests tests/ReadIndexTests.cpp)
    target_link_libraries(read_index_tests PRIVATE minigit_paths)
    add_test(NAME read_index_tests COMMAND read_index_tests)

    # Keep the Issue 3 writing and directory-isolation regression checks.
    add_executable(issue3_tests tests/Issue3Tests.cpp)
    target_link_libraries(issue3_tests PRIVATE minigit_paths)
    add_test(NAME issue3_tests COMMAND issue3_tests)

    add_executable(repository_paths_tests tests/RepositoryPathsTests.cpp)
    target_link_libraries(repository_paths_tests PRIVATE minigit_paths)
    add_test(NAME repository_paths_tests COMMAND repository_paths_tests)

    add_executable(filesystem_tests tests/FileSystemTests.cpp)
    target_link_libraries(filesystem_tests PRIVATE minigit_paths)
    add_test(NAME filesystem_tests COMMAND filesystem_tests)

    add_executable(minigit_manager_tests tests/MiniGitManagerTests.cpp)
    target_link_libraries(minigit_manager_tests PRIVATE minigit_paths)
    add_test(NAME minigit_manager_tests COMMAND minigit_manager_tests)

    add_executable(filesystem_paths_tests tests/FileSystemPathsTests.cpp)
    target_link_libraries(filesystem_paths_tests PRIVATE minigit_paths)
    target_include_directories(filesystem_paths_tests PRIVATE src)
    add_test(NAME filesystem_paths_tests COMMAND filesystem_paths_tests)

    add_executable(command_line_app_tests tests/CommandLineAppTests.cpp)
    target_link_libraries(command_line_app_tests PRIVATE minigit_paths)
    add_test(NAME command_line_app_tests COMMAND command_line_app_tests)
    # HW12 M2: Test the actual minigit executable
    add_test(
        NAME minigit_init_integration
        COMMAND ${CMAKE_COMMAND}
            "-DMINIGIT_EXE=$<TARGET_FILE:minigit>"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/MinigitInitIntegration.cmake"
    )
endif()
