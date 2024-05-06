cmake_minimum_required(VERSION 3.28)
project(query_lang)

set(CMAKE_CXX_STANDARD 20)

add_executable(query_lang main.cpp
        expected.h)
