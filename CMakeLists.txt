cmake_minimum_required(VERSION 3.8)
project(TI_TP2)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES gzip.cpp huffman.cpp cmake-build-debug/FAQ.txt.gz helper.cpp helper.h)
add_executable(TI_TP2 ${SOURCE_FILES})