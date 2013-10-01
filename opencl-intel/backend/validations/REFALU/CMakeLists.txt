cmake_minimum_required(VERSION 2.8.7)
cmake_policy(SET CMP0001 NEW)
cmake_policy(SET CMP0011 OLD)
use_eh(TRUE)

set (TARGET_NAME "REFALU")

# to be shure that ICC compiler uses the correct presentation of 80-bit long doubles
# we have to set proper compiler options, for win32 only, for linux gcc(g++) is used and 
# we dont need to set such options
if (WIN32)
set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:source /fp:strict /Qlong-double /Qpc80 /Qprec-div" )
set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /fp:source /fp:strict /Qlong-double /Qpc80 /Qprec-div" ) 
endif(WIN32)

include_directories(${BACKEND_ROOT_DIR}/../cl_api 
                    ${CMAKE_CURRENT_SOURCE_DIR}/../ImathLibd
                    ${CMAKE_CURRENT_SOURCE_DIR}/../Common
                    ${CMAKE_CURRENT_SOURCE_DIR}/Conformance
                   )


set(REFALU_HEADERS
  ../Common/FloatOperations.h
  ../Common/dxfloat.h
  ../Common/Exception.h
  ImagesALU.h
  ./Conformance/reference_convert.h
  ./Conformance/reference_math.h
  ./Conformance/Utility.h
  ./Conformance/test_common/compat.h
  ./Conformance/test_common/rounding_mode.h
  ./Conformance/test_common/testHarness.h
  ./Conformance/test_common/threadTesting.h
)

add_library(${TARGET_NAME} STATIC
  ${REFALU_HEADERS}
  RefALU.cpp
  ImagesALU.cpp
  ./Conformance/reference_math.cpp
  ./Conformance/reference_convert.cpp
  ./Conformance/Utility.cpp
  ./Conformance/test_common/msvc9.cpp
  ./Conformance/test_common/rounding_mode.cpp
  ../Common/FloatOperations.cpp
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER "Backend" )
