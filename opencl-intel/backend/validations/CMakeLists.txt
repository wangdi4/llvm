#
# INTEL CONFIDENTIAL
#
# Copyright (C) 2022 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may
# not use, modify, copy, publish, distribute, disclose or transmit this software
# or the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express or
# implied warranties, other than those that are expressly stated in the License.
#

# Variables for LLVM libraries
set(VALIDATIONS_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  # 32 bit
  set(PLATFORM 32)
else()
  # 64 bit
  set(PLATFORM 64)
endif()

# icc environment variables
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(ICC_PLATFORM ia32)
else()
  set(ICC_PLATFORM intel64)
endif()

# To be sure that ICC compiler uses the correct presentation of 80-bit long
# doubles we have to set proper compiler options:
if(WIN32)
  set(ICC_AUXILIARY_CXX_FLAGS
      "/fp:source /Qlong-double /fp:strict /Qpc80 /Qprec-div")
  set(ICC_AUXILIARY_C_FLAGS
      "/fp:source /Qlong-double /fp:strict /Qpc80 /Qprec-div")
else()
  set(ICC_AUXILIARY_CXX_FLAGS "-fp-model extended -pc80 -prec-div")
  set(ICC_AUXILIARY_C_FLAGS "-fp-model extended -pc80 -prec-div")
endif()

include_directories(${VALIDATIONS_ROOT}/Common)

add_subdirectory(ImathLibd)
add_subdirectory(DataVersion)
add_subdirectory(DataManager)
add_subdirectory(DataGenerator)
add_subdirectory(Comparator)
add_subdirectory(NEAT)
add_subdirectory(REFALU)
add_subdirectory(SATest)
add_subdirectory(NEATCheck)
add_subdirectory(OCLRecorder)
add_subdirectory(OCLBuiltins)
add_subdirectory(InterpreterPluggable)
add_subdirectory(PlugInNEAT)
add_subdirectory(Unittests)
add_subdirectory(OCLBuilder)
add_subdirectory(OpenCLKernelArgumentsParser)
add_subdirectory(OCLKernelDataGenerator)
add_subdirectory(FMATest)
add_subdirectory(TableGen)
add_subdirectory(ProgramConfiguration)
if(BUILD_QTGUI)
  if(WIN32)
    add_subdirectory(SATestGUI)
  endif(WIN32)
endif(BUILD_QTGUI)
