#
#    Copyright (C) 2018 Intel Corporation. All rights reserved.
#
#    The information and source code contained herein is the exclusive
#    property of Intel Corporation. and may not be disclosed, examined
#    or reproduced in whole or in part without explicit written authorization
#    from the company.
#
#    Source file: CMakeLists.txt
#    ------------
#    Configures build for project.
#

set(DTRANS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(DTRANS_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(DTRANS_MAIN_INCLUDE_DIR ${DTRANS_SOURCE_DIR}/include/Intel_DTrans)

add_subdirectory(lib)
add_subdirectory(test)
