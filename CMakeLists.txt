cmake_minimum_required(VERSION 3.20)
project(gitx VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Collect all source files
file(GLOB_RECURSE GITX_SOURCES src/*.cpp)

add_executable(gitx ${GITX_SOURCES})
target_include_directories(gitx PRIVATE include)

include(CTest)
if(BUILD_TESTING)
    add_executable(tree_serialization_test
        tests/tree_serialization_test.cpp
        src/tree.cpp
    )
    target_include_directories(tree_serialization_test PRIVATE include)
    add_test(NAME tree_serialization_test COMMAND tree_serialization_test)

    file(GLOB_RECURSE GITX_LIBRARY_SOURCES src/*.cpp)
    list(REMOVE_ITEM GITX_LIBRARY_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
    add_executable(revert_test tests/revert_test.cpp ${GITX_LIBRARY_SOURCES})
    target_include_directories(revert_test PRIVATE include)
    if(MSVC)
        target_compile_options(revert_test PRIVATE /FS)
    endif()
    add_test(NAME revert_test COMMAND revert_test)
endif()
