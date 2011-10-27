#
# Settings for Linux compilation
#
if (NOT WIN32)

# Compiler switches that CANNOT be modified during makefile generation
set (ADD_COMMON_C_FLAGS  "-msse3 -mssse3 -fPIC -fdiagnostics-show-option -funsigned-bitfields")

set (ADD_C_FLAGS         "${ADD_COMMON_C_FLAGS} -std=gnu99")
set (ADD_CXX_FLAGS       "${ADD_COMMON_C_FLAGS} ")

set (ADD_C_FLAGS_DEBUG   "-O0 -ggdb -D _DEBUG")
set (ADD_C_FLAGS_RELEASE "-O2 -g0 ")

# C switches
set( CMAKE_C_FLAGS         "${CMAKE_C_FLAGS}         ${ADD_C_FLAGS}")
set( CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}   ${ADD_C_FLAGS_DEBUG}")
set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${ADD_C_FLAGS_RELEASE}")

# C++ switches
set( CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         ${ADD_CXX_FLAGS}")
set( CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   ${ADD_C_FLAGS_DEBUG}")
set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${ADD_C_FLAGS_RELEASE}")

endif (NOT WIN32)

add_llvm_library(OCLBuiltins
  IBLTMapFiller.h
  BLTCommon.cpp
  BLTMath.cpp
  BLTImages.cpp
  BLTInteger.cpp
  BLTGeometric.cpp
  BLTConversion.cpp
  BLTRelational.cpp
  BLTAsyncCopiesAndPrefetch.cpp
  OCLBuiltinParser.cpp
  OCLBuiltins.cpp
  VLoadStore.cpp
  Helpers.cpp
  )

include_directories (
  ${CMAKE_SOURCE_DIR}/../cl_api
  ${CMAKE_SOURCE_DIR}/../validations/Common
  ${CMAKE_SOURCE_DIR}/../validations/REFALU
  )

target_link_libraries(OCLBuiltins
  DataManager
  LLVMSupport
  REFALU
  )

add_dependencies(OCLBuiltins
  DataManager
  LLVMSupport
  REFALU
  )

