cmake_minimum_required(VERSION 3.20)

if(POLICY CMP0111)
    cmake_policy(SET CMP0111 NEW)
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(practice)
