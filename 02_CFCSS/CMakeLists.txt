cmake_minimum_required(VERSION 3.13)

project(CFCSS)

find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_library(CFCSSPass MODULE
    CFCSS.cpp
)