
# HW12 M2 - MiniGit executable integration tests
# Test 1: Initialize a new repository

if(NOT DEFINED MINIGIT_EXE OR NOT EXISTS "${MINIGIT_EXE}")
    message(FATAL_ERROR "Could not find the minigit executable")
endif()


# Keep track of temporary folders created by these tests
set(test_folders)

# Clean up temporary folders before reporting a failure
macro(fail_test error_message)
    foreach(folder IN LISTS test_folders)
        file(REMOVE_RECURSE "${folder}")
    endforeach()
    message(FATAL_ERROR "${error_message}")
endmacro()


# Create a unique temporary project folder
string(RANDOM LENGTH 12 ALPHABET 0123456789abcdef test_id)
set(test_folder "${CMAKE_CURRENT_BINARY_DIR}/minigit-integration-${test_id}")

if(EXISTS "${test_folder}")
    message(FATAL_ERROR "Temporary folder already exists")
endif()

file(MAKE_DIRECTORY "${test_folder}")
list(APPEND test_folders "${test_folder}")

# Run the actual MiniGit executable
execute_process(
    COMMAND "${MINIGIT_EXE}" init
    WORKING_DIRECTORY "${test_folder}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error_output
)

# Check the exit code
if(NOT "${result}" STREQUAL "0")
    message(FATAL_ERROR "Test 1 failed: init should return 0. ${error_output}")
endif()

# Check that init printed the success message
if(NOT output MATCHES "Initialized empty MiniGit repository")
    message(FATAL_ERROR "Test 1 failed: missing success message")
endif()

# Check that there are no errors
if(NOT error_output STREQUAL "")
    message(FATAL_ERROR "Test 1 failed: unexpected error output")
endif()

# Check the required directories
set(meta "${test_folder}/.minigit")

foreach(folder IN ITEMS
    "${meta}"
    "${meta}/stage"
    "${meta}/commits"
)
    if(NOT IS_DIRECTORY "${folder}")
        message(FATAL_ERROR "Test 1 failed: missing directory ${folder}")
    endif()
endforeach()

# Check the required files
foreach(filename IN ITEMS HEAD index.txt next-id.txt)
    if(NOT EXISTS "${meta}/${filename}" OR IS_DIRECTORY "${meta}/${filename}")
        message(FATAL_ERROR "Test 1 failed: missing file ${filename}")
    endif()
endforeach()

# HEAD and index.txt must be empty
file(SIZE "${meta}/HEAD" head_size)
file(SIZE "${meta}/index.txt" index_size)

if(NOT head_size EQUAL 0 OR NOT index_size EQUAL 0)
    message(FATAL_ERROR "Test 1 failed: HEAD and index.txt must be empty")
endif()

# The first available commit ID must be 1
file(READ "${meta}/next-id.txt" next_id)

if(NOT next_id STREQUAL "1\n")
    message(FATAL_ERROR "Test 1 failed: next-id.txt should contain 1 followed by a newline")
endif()

# stage and commits must be empty
foreach(folder IN ITEMS stage commits)
    file(GLOB contents "${meta}/${folder}/*")
    if(contents)
        message(FATAL_ERROR "Test 1 failed: ${folder} should be empty")
    endif()
endforeach()


# Test 2: Repeated initialization

# Create a separate folder for Test 2
string(RANDOM LENGTH 12 ALPHABET 0123456789abcdef repeat_id)
set(repeat_folder "${CMAKE_CURRENT_BINARY_DIR}/minigit-repeat-${repeat_id}")
file(MAKE_DIRECTORY "${repeat_folder}")
list(APPEND test_folders "${repeat_folder}")

set(repeat_meta "${repeat_folder}/.minigit")

# Initialize this repository before testing repeated init
execute_process(
    COMMAND "${MINIGIT_EXE}" init
    WORKING_DIRECTORY "${repeat_folder}"
    RESULT_VARIABLE setup_result
    OUTPUT_VARIABLE setup_output
    ERROR_VARIABLE setup_error
)

if(NOT "${setup_result}" STREQUAL "0")
    fail_test("Test 2 setup failed: ${setup_error}")
endif()


# Create a file to check that existing data is preserved
set(sentinel "${repeat_meta}/keep.txt")
file(WRITE "${sentinel}" "keep this data")

# Save original metadata before running init again
file(READ "${repeat_meta}/HEAD" original_head)
file(READ "${repeat_meta}/index.txt" original_index)
file(READ "${repeat_meta}/next-id.txt" original_next_id)

# Run minigit init for the second time
execute_process(
    COMMAND "${MINIGIT_EXE}" init
    WORKING_DIRECTORY "${repeat_folder}"
    RESULT_VARIABLE second_result
    OUTPUT_VARIABLE second_output
    ERROR_VARIABLE second_error
)

# Check that the second init succeeds

if(NOT "${second_result}" STREQUAL "0")
    fail_test(
        "Test 2 failed: repeated init should return 0\nActual result: ${second_result}\nstdout: ${second_output}\nstderr: ${second_error}"
    )
endif()


# Check the message
if(NOT second_output MATCHES "already initialized")
    message(FATAL_ERROR "Test 2 failed: missing already initialized message")
endif()

# Check there are no errors
if(NOT second_error STREQUAL "")
    message(FATAL_ERROR "Test 2 failed: unexpected error output")
endif()

# Check that existing data was not overwritten or deleted
if(NOT EXISTS "${sentinel}")
    message(FATAL_ERROR "Test 2 failed: existing file was deleted")
endif()

file(READ "${sentinel}" sentinel_content)

if(NOT sentinel_content STREQUAL "keep this data")
    message(FATAL_ERROR "Test 2 failed: existing file was modified")
endif()


# Verify metadata was preserved
file(READ "${repeat_meta}/HEAD" current_head)
file(READ "${repeat_meta}/index.txt" current_index)
file(READ "${repeat_meta}/next-id.txt" current_next_id)

if(NOT "${current_head}" STREQUAL "${original_head}")
    message(FATAL_ERROR "Test 2 failed: HEAD was modified")
endif()

if(NOT "${current_index}" STREQUAL "${original_index}")
    message(FATAL_ERROR "Test 2 failed: index.txt was modified")
endif()

if(NOT "${current_next_id}" STREQUAL "${original_next_id}")
    message(FATAL_ERROR "Test 2 failed: next-id.txt was modified")
endif()


message(STATUS "Test 2 passed: repeated initialization")
# Remove only Test 2's temporary repository
file(REMOVE_RECURSE "${repeat_folder}")


# Test 3: Damaged repository (missing HEAD)

# Create a separate folder for Test 3
string(RANDOM LENGTH 12 ALPHABET 0123456789abcdef damaged_id)
set(damaged_folder "${CMAKE_CURRENT_BINARY_DIR}/minigit-damaged-${damaged_id}")
file(MAKE_DIRECTORY "${damaged_folder}")
list(APPEND test_folders "${damaged_folder}")

set(damaged_meta "${damaged_folder}/.minigit")

# Initialize the repository first
execute_process(
    COMMAND "${MINIGIT_EXE}" init
    WORKING_DIRECTORY "${damaged_folder}"
    RESULT_VARIABLE damaged_setup_result
    ERROR_VARIABLE damaged_setup_error
)

if(NOT "${damaged_setup_result}" STREQUAL "0")
    fail_test("Test 3 setup failed: ${damaged_setup_error}")
endif()

# Add a file to check whether existing data is preserved
set(damaged_sentinel "${damaged_meta}/keep.txt")
file(WRITE "${damaged_sentinel}" "keep this data")

# Save existing metadata before damaging the repository
file(READ "${damaged_meta}/index.txt" original_damaged_index)
file(READ "${damaged_meta}/next-id.txt" original_damaged_id)

# Remove HEAD to simulate damaged metadata
file(REMOVE "${damaged_meta}/HEAD")

# Run init on the damaged repository
execute_process(
    COMMAND "${MINIGIT_EXE}" init
    WORKING_DIRECTORY "${damaged_folder}"
    RESULT_VARIABLE damaged_result
    OUTPUT_VARIABLE damaged_output
    ERROR_VARIABLE damaged_error
)

# The command must fail
if(NOT "${damaged_result}" STREQUAL "1")
    fail_test("Test 3 failed: damaged repository should return 1")
endif()

# The error must begin with Error:
if(NOT damaged_error MATCHES "^Error:")
    message(FATAL_ERROR "Test 3 failed: expected Error: on stderr")
endif()

# It must not print a success message
if(NOT damaged_output STREQUAL "")
    message(FATAL_ERROR "Test 3 failed: damaged init should not print success")
endif()

# init must not repair HEAD automatically
if(EXISTS "${damaged_meta}/HEAD")
    message(FATAL_ERROR "Test 3 failed: missing HEAD should not be recreated")
endif()

# Verify that our existing sentinel was preserved
file(READ "${damaged_sentinel}" sentinel_after_damage)

if(NOT sentinel_after_damage STREQUAL "keep this data")
    message(FATAL_ERROR "Test 3 failed: existing data was modified")
endif()


# Verify existing metadata still exists
foreach(filename IN ITEMS index.txt next-id.txt)
    if(NOT EXISTS "${damaged_meta}/${filename}")
        fail_test("Test 3 failed: ${filename} was deleted")
    endif()
endforeach()

# Compare metadata before and after the failed init
file(READ "${damaged_meta}/index.txt" current_damaged_index)
file(READ "${damaged_meta}/next-id.txt" current_damaged_id)

if(NOT "${current_damaged_index}" STREQUAL "${original_damaged_index}")
    fail_test("Test 3 failed: index.txt was modified")
endif()

if(NOT "${current_damaged_id}" STREQUAL "${original_damaged_id}")
    fail_test("Test 3 failed: next-id.txt was modified")
endif()

# Verify the existing directories were preserved
if(NOT IS_DIRECTORY "${damaged_meta}/stage"
   OR NOT IS_DIRECTORY "${damaged_meta}/commits")
    fail_test("Test 3 failed: repository directories were removed")
endif()


message(STATUS "Test 3 passed: damaged repository rejected")
file(REMOVE_RECURSE "${damaged_folder}")

# Test 4: Invalid and unsupported commands

foreach(args IN ITEMS "status" "unknown" "init extra" "")

    # Create a separate temporary folder for each command
    string(RANDOM LENGTH 12 ALPHABET 0123456789abcdef case_id)
    set(case_folder "${CMAKE_CURRENT_BINARY_DIR}/minigit-invalid-${case_id}")
    file(MAKE_DIRECTORY "${case_folder}")

    # Convert the argument string into a list
    if(args STREQUAL "")
        set(command_args)
    else()
        separate_arguments(command_args NATIVE_COMMAND "${args}")
    endif()

    # Run the actual executable
    execute_process(
        COMMAND "${MINIGIT_EXE}" ${command_args}
        WORKING_DIRECTORY "${case_folder}"
        RESULT_VARIABLE invalid_result
        OUTPUT_VARIABLE invalid_output
        ERROR_VARIABLE invalid_error
    )

    if(NOT "${invalid_result}" STREQUAL "1")
        message(FATAL_ERROR "Test 4 failed: '${args}' should return 1")
    endif()

    if(NOT invalid_error MATCHES "^Error:")
        message(FATAL_ERROR "Test 4 failed: '${args}' should print Error: on stderr")
    endif()

    if(NOT invalid_output STREQUAL "")
        message(FATAL_ERROR "Test 4 failed: '${args}' should not print success")
    endif()

    # An invalid command should not create repository metadata
    if(EXISTS "${case_folder}/.minigit")
        message(FATAL_ERROR "Test 4 failed: '${args}' created metadata")
    endif()

    file(REMOVE_RECURSE "${case_folder}")
    list(APPEND test_folders "${case_folder}")
endforeach()

message(STATUS "Test 4 passed: invalid commands rejected")


# Test 5: .minigit exists as a regular file

# Create a separate folder for this test
string(RANDOM LENGTH 12 ALPHABET 0123456789abcdef file_case_id)
set(file_case_folder "${CMAKE_CURRENT_BINARY_DIR}/minigit-file-${file_case_id}")
file(MAKE_DIRECTORY "${file_case_folder}")
list(APPEND test_folders "${file_case_folder}")

# Create .minigit as a file, not a directory
set(metadata_file "${file_case_folder}/.minigit")
file(WRITE "${metadata_file}" "important existing data")

# Run minigit init
execute_process(
    COMMAND "${MINIGIT_EXE}" init
    WORKING_DIRECTORY "${file_case_folder}"
    RESULT_VARIABLE file_result
    OUTPUT_VARIABLE file_output
    ERROR_VARIABLE file_error
)

# The command must fail
if(NOT "${file_result}" STREQUAL "1")
    message(FATAL_ERROR "Test 5 failed: .minigit as a file should return 1")
endif()

# Check for the error message
if(NOT file_error MATCHES "^Error:")
    message(FATAL_ERROR "Test 5 failed: expected Error: on stderr")
endif()

# No success message should be printed
if(NOT file_output STREQUAL "")
    message(FATAL_ERROR "Test 5 failed: unexpected success output")
endif()

# .minigit must still be a regular file
if(IS_DIRECTORY "${metadata_file}" OR NOT EXISTS "${metadata_file}")
    message(FATAL_ERROR "Test 5 failed: original .minigit file was replaced")
endif()

# Check the original contents were preserved
file(READ "${metadata_file}" file_content)

if(NOT file_content STREQUAL "important existing data")
    message(FATAL_ERROR "Test 5 failed: original .minigit file was modified")
endif()

# Clean up this test's folder
file(REMOVE_RECURSE "${file_case_folder}")

message(STATUS "Test 5 passed: .minigit regular file preserved")


# Clean up only our test folder
file(REMOVE_RECURSE "${test_folder}")

message(STATUS "Test 1 passed: fresh initialization")
