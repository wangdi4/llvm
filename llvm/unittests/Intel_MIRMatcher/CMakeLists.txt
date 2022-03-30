#
#    Copyright (C) 2018 Intel Corporation. All rights reserved.
#
#    The information and source code contained herein is the exclusive
#    property of Intel Corporation. and may not be disclosed, examined
#    or reproduced in whole or in part without explicit written authorization
#    from the company.
#

set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  CodeGen
  Core
  MC
  Support
  Target
  )

if(";${LLVM_TARGETS_TO_BUILD};" MATCHES ";X86;")
include_directories(
  ${CMAKE_BINARY_DIR}/lib/Target/X86/
)

  add_llvm_unittest(Intel_MIRMatcherTests
    Intel_MIRMatcherTest.cpp
    )
endif()

if (HOST_LINUX AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|IntelLLVM" )
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++17-extensions")
endif()
