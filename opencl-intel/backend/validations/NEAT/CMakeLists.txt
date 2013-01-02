cmake_minimum_required(VERSION 2.8.4)
cmake_policy(SET CMP0001 NEW)
cmake_policy(SET CMP0011 OLD)
use_eh(TRUE)

set (TARGET_NAME "NEAT")

include_directories(
    ${BACKEND_ROOT_DIR}/../cl_api
    ${CMAKE_CURRENT_SOURCE_DIR}/../REFALU
    ${CMAKE_CURRENT_SOURCE_DIR}/../ImathLibd
    ${CMAKE_CURRENT_SOURCE_DIR}/../Common
    )

add_library(${TARGET_NAME} STATIC
    NEATALU.cpp
    NEATValue.cpp
    NEATVector.cpp
    NEAT_WRAP.cpp
    )
