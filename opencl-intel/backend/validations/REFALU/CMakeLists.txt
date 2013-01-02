cmake_minimum_required(VERSION 2.8.4)
cmake_policy(SET CMP0001 NEW)
cmake_policy(SET CMP0011 OLD)
use_eh(TRUE)

set (TARGET_NAME "REFALU")

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
