# Ensure we are using the frontend LLVM tools
if( NOT DEFINED LLVM_PATH_FE )
    message( FATAL_ERROR "LLVM_PATH_FE is not specified. Please specify LLVM library location for fronted using LLVM_PATH_FE parameter to CMAKE" )
endif()

set(LLVM_PATH ${LLVM_PATH_FE})
find_package(LLVM REQUIRED)

include (${OCL_SOURCE_DIR}/cmake_utils/CMakeFuncs.txt)

set( TARGET clang_compiler_test_type )

add_definitions( ${LLVM_DEFINITIONS} )

# Add include directories
include_directories(
    ${LLVM_INCLUDE_DIRS}
    ${LLVM_INCLUDE_DIRS}/cclang
    ${OCL_SOURCE_DIR}/clang_compiler
    ${OCL_SOURCE_DIR}/externals/gtest/include
    ${OCL_SOURCE_DIR}/tests/test_common
    ${OCL_SOURCE_DIR}/externals/khronos/spirv/headers/include
    ${CL_API_HEADERS} )

set ( CLANG_COMPILER_TEST_TYPE_SOURCE_FILES
    ClangCompilerTestTypeFixture.cpp
    clang_compiler_test_type.cpp )

set ( CLANG_COMPILER_TEST_TYPE_HEADER_FILES
    ClangCompilerTestTypeFixture.h )

add_executable( ${TARGET} ${CLANG_COMPILER_TEST_TYPE_HEADER_FILES} ${CLANG_COMPILER_TEST_TYPE_SOURCE_FILES} )

if (NOT WIN32)
    if( ANDROID )
        target_link_libraries( ${TARGET} c++ )
        set_property(TARGET ${TARGET} APPEND_STRING PROPERTY LINK_FLAGS " -pie")
    endif( ANDROID )
    target_link_libraries( ${TARGET} clang_compiler${BUILD_PLATFORM} googletest ${PTHREAD_LIB})
else (NOT WIN32)
    target_link_libraries( ${TARGET} clang_compiler${BUILD_PLATFORM} googletest )
endif (NOT WIN32)

set( CREATE_DIR_NAME tests/clang_compiler_test_type)

# Generate SPIR-V input for tests

if( CMAKE_CROSSCOMPILING OR LLVM_USE_NATIVE OR ANDROID )
   set(TOOLS_BINARY_DIR "${LLVM_BINARY_DIR}/../native/bin")
   set(SPIRV_CONVERTER  "${LLVM_PATH_BE}/native/bin/llvm-spirv")
else()
   set(TOOLS_BINARY_DIR "${LLVM_BINARY_DIR}" )
   set(SPIRV_CONVERTER  "${LLVM_PATH_BE}/bin/llvm-spirv")
endif()

set( CLANG "${TOOLS_BINARY_DIR}/clang" )

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
  -include opencl-c.h -O0 -o ${CMAKE_CURRENT_BINARY_DIR}/${SPIR_OUTPUT_FILE}
  MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
  DEPENDS ${INCLUDE_HEADERS_PATH}/opencl-c.h ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
  COMMENT "Generate spir binary file for clang_compiler_test_type: ${SPIR_OUTPUT_FILE}"
  VERBATIM
)

add_custom_command( TARGET ${TARGET}
  POST_BUILD
  COMMAND ${SPIRV_CONVERTER} -o ${SPIRV_OUTPUT_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${SPIR_OUTPUT_FILE}
  DEPENDS ${SPIR_OUTPUT_FILE}
  COMMENT "Generate spirv binary file for clang_compiler_test_type: ${SPIRV_OUTPUT_FILE}"
  VERBATIM
)

# Copy build artifacts to 'Release' area.
install( TARGETS ${TARGET} DESTINATION ${CREATE_DIR_NAME} )
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/${SPIRV_OUTPUT_FILE} DESTINATION ${CREATE_DIR_NAME} )
install( FILES clang_compiler_test_type.pm DESTINATION ${CREATE_DIR_NAME} )

set_target_properties( ${TARGET} PROPERTIES FOLDER "validation/workloads" )
add_dependencies( ${TARGET} clang_compiler${BUILD_PLATFORM} )
