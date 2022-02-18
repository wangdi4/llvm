set( TARGET FrontendDriver_test_type )

add_definitions( ${LLVM_DEFINITIONS} )

# Add include directories
include_directories(
    ${LLVM_INCLUDE_DIRS}
    # common_clang.h is in CCLANG_DEV_INCLUDE_DIRS
    ${CCLANG_DEV_INCLUDE_DIRS}
    ${OCL_SOURCE_DIR}/FrontendDriver
    ${LLVM_EXTERNAL_LLVM_SPIRV_SOURCE_DIR}/lib )

# Generate SPIR-V input for tests
set( INPUT_FILE test.cl )
set( SPIRV_OUTPUT_FILE test.spv )
set(TEST_SPV_FILE ${TARGET}_test_spv)
compile_spv(${INPUT_FILE} ${SPIRV_OUTPUT_FILE}
  EXTRA_CLANG_FLAGS -cl-std=CL2.0 -O0)
add_custom_target(${TEST_SPV_FILE} DEPENDS ${SPIRV_OUTPUT_FILE})
add_definitions(-DSPIRV_TEST_FILE="${SPIRV_OUTPUT_FILE}")

# TODO: remove from runtime unittest, see CORC-2398
add_ocl_unittest( ${TARGET}
    SOURCE_FILES
      FrontendDriverFixture.cpp
      FrontendDriver_test_type.cpp
    LINK_LIBRARIES
      clang_compiler${BUILD_PLATFORM}
      test_utils
      LLVMCore
      LLVMSupport
)

add_dependencies(${TARGET} ${TEST_SPV_FILE})
