set( TARGET FrontendDriver_test_type )

add_definitions( ${LLVM_DEFINITIONS} )

# Add include directories
include_directories(
    ${LLVM_INCLUDE_DIRS}
    ${CCLANG_USER_INCLUDE_DIRS}
    ${OCL_SOURCE_DIR}/FrontendDriver
    ${OCL_SOURCE_DIR}/externals/khronos/spirv/headers/include )


# TODO: remove from runtime unittest, see CORC-2398
add_ocl_unittest( ${TARGET}
    SOURCE_FILES
      FrontendDriverFixture.cpp
      FrontendDriver_test_type.cpp
    LINK_LIBRARIES
      clang_compiler${BUILD_PLATFORM}
      test_utils
)

# Generate SPIR-V input for tests

set( INPUT_FILE test.cl )
set( SPIR_OUTPUT_FILE test.bc )
set( SPIRV_OUTPUT_FILE test.spv )
add_definitions ( -DSPIRV_TEST_FILE="${SPIRV_OUTPUT_FILE}" )

if (BUILD_X64)
  set (SPIR_OPTIONS -triple spir64-unknown-unknown -D __x86_64__)
else ()
  set (SPIR_OPTIONS -triple spir-unknown-unknown -D __i386__)
endif()

set( INCLUDE_HEADERS_PATH ${CCLANG_USER_INCLUDE_DIRS} )

add_custom_command( TARGET ${TARGET}
  POST_BUILD
  COMMAND ${CLANG} -cc1 -x cl ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
  -I ${INCLUDE_HEADERS_PATH} -emit-llvm-bc
  ${SPIR_OPTIONS} -disable-llvm-passes -O0 -disable-intel-proprietary-opts
  -include opencl-c.h -o ${CMAKE_CURRENT_BINARY_DIR}/${SPIR_OUTPUT_FILE}
  MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
  DEPENDS ${INCLUDE_HEADERS_PATH}/opencl-c.h ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
  BYPRODUCTS ${SPIR_OUTPUT_FILE}
  COMMENT "Generate spir binary file for FrontendDriver_test_type: ${SPIR_OUTPUT_FILE}"
  VERBATIM
)

add_custom_command( TARGET ${TARGET}
  POST_BUILD
  COMMAND ${SPIRV_CONVERTER} -o ${SPIRV_OUTPUT_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${SPIR_OUTPUT_FILE}
  DEPENDS ${SPIR_OUTPUT_FILE}
  BYPRODUCTS ${SPIRV_OUTPUT_FILE}
  COMMENT "Generate spirv binary file for FrontendDriver_test_type: ${SPIRV_OUTPUT_FILE}"
  VERBATIM
)
