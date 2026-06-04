cmake_minimum_required(VERSION 3.13)

project(CFCSS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_library(CFCSSPass MODULE
    CFCSS.cpp
)

llvm_map_components_to_libnames(
    LLVM_LIBS
    Core
    Support
)

target_link_libraries(
    CFCSSPass
    PRIVATE
    ${LLVM_LIBS}
)