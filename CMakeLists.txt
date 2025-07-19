cmake_minimum_required(VERSION 3.14)
project(intrusive_list_tests LANGUAGES CXX)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add the intrusive_list header-only library
add_subdirectory(intrusive_list)

# Enable testing
enable_testing()

# Include FetchContent module
include(FetchContent)

# Fetch Google Test
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Make googletest available
FetchContent_MakeAvailable(googletest)

# Create the test executable
add_executable(
  test_intrusive_list
  test_intrusive_list.cpp
)

# Link against the intrusive_list library and Google Test
target_link_libraries(
  test_intrusive_list
  PRIVATE
    intrusive_list
    gtest_main
    gtest
)

# Add compiler warnings
if(MSVC)
  target_compile_options(test_intrusive_list PRIVATE /W4)
else()
  target_compile_options(test_intrusive_list PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT test_intrusive_list)

# Register the test with CTest
include(GoogleTest)
gtest_discover_tests(test_intrusive_list)