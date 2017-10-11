set( TARGET clang_compiler_test_type )

add_definitions( ${LLVM_DEFINITIONS} )

# Add include directories
include_directories(
    ${LLVM_INCLUDE_DIRS}
    ${LLVM_INCLUDE_DIRS}/cclang
    ${OCL_SOURCE_DIR}/clang_compiler
    ${OCL_SOURCE_DIR}/externals/khronos/spirv/headers/include )

set( CLANG_COMPILER_TEST_TYPE_SOURCE_FILES
    ClangCompilerTestTypeFixture.cpp
    clang_compiler_test_type.cpp )

set( CLANG_COMPILER_TEST_TYPE_LIBS clang_compiler${BUILD_PLATFORM} test_utils )

# TODO: remove from runtime unittest, see CORC-2398
add_ocl_unittest( ${TARGET}
    SOURCE_FILES ${CLANG_COMPILER_TEST_TYPE_SOURCE_FILES}
    LINK_LIBRARIES ${CLANG_COMPILER_TEST_TYPE_LIBS} )

# Generate SPIR-V input for tests

set( INPUT_FILE test.cl )
set( SPIR_OUTPUT_FILE test.bc )
set( SPIRV_OUTPUT_FILE test.spv )
add_definitions ( -DSPIRV_TEST_FILE="${SPIRV_OUTPUT_FILE}" )

if (BUILD_X64)
set (
  SPIR_OPTIONS
  -triple spir64-unknown-unknown -D __x86_64__
)
else ()
set (
  SPIR_OPTIONS
  -triple spir-unknown-unknown -D __i386__
)
endif()

set( INCLUDE_HEADERS_PATH ${LLVM_INCLUDE_DIRS}/cclang )

add_custom_command( TARGET ${TARGET}
  POST_BUILD
  COMMAND ${CLANG} -cc1 -x cl ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
  -I ${INCLUDE_HEADERS_PATH} -emit-llvm-bc
  ${SPIR_OPTIONS}
  -include opencl-c.h -disable-llvm-passes -o ${CMAKE_CURRENT_BINARY_DIR}/${SPIR_OUTPUT_FILE}
  MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
  DEPENDS ${INCLUDE_HEADERS_PATH}/opencl-c.h ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
  BYPRODUCTS ${SPIR_OUTPUT_FILE}
  COMMENT "Generate spir binary file for clang_compiler_test_type: ${SPIR_OUTPUT_FILE}"
  VERBATIM
)

add_custom_command( TARGET ${TARGET}
  POST_BUILD
  COMMAND ${SPIRV_CONVERTER} -o ${SPIRV_OUTPUT_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${SPIR_OUTPUT_FILE}
  DEPENDS ${SPIR_OUTPUT_FILE}
  BYPRODUCTS ${SPIRV_OUTPUT_FILE}
  COMMENT "Generate spirv binary file for clang_compiler_test_type: ${SPIRV_OUTPUT_FILE}"
  VERBATIM
)
