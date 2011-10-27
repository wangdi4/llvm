include_directories(${CMAKE_SOURCE_DIR}/../cl_api .)

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

add_llvm_library(REFALU STATIC
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
