# remove the restriction of warning as error for this component
if (NOT WIN32)
    add_definitions( "-Wno-type-limits -Wno-missing-braces -Wno-unused-variable" )
    remove_definitions( -Werror )
endif (NOT WIN32)

#include helping functions of cmake.
include (../../cmake_utils/CMakeFuncs.txt)

set( TARGET common_runtime_test_type )

if( NOT DEFINED LLVM_PATH_SPIRV )
    message( FATAL_ERROR "LLVM_PATH_SPIRV is not specified." )
endif()

set( CLANG "${LLVM_PATH_SPIRV}/bin/clang" )

include_directories( ../../externals/gtest/include
                     ../../externals/khronos/spirv/headers/include
                     ${CL_API_HEADERS}
                     ${CMAKE_CURRENT_SOURCE_DIR}/common )

#set( IGNORE_FILES clBuildCubinProgram|clBuildProgram.cpp|clCreateCudaKernel|clCreateKernelCpuGpu|clCreateQueue )

FIND_SOURCES( ${IGNORE_FILES} )
CALCULATE_TARGET_SOURCES()

FILE(GLOB CRT_KERNELS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} kernels/[^.]*)

# Place kernels under Kernels folder in Visual Studio
if (WIN32)
    source_group( "Kernels" FILES ${CRT_KERNELS} )
    list( APPEND TARGET_SOURCES ${CRT_KERNELS} )
endif (WIN32)

add_executable( ${TARGET} ${TARGET_SOURCES} )

target_link_libraries( ${TARGET} OpenCL googletest)
if (NOT WIN32 AND NOT ANDROID)
    target_link_libraries( ${TARGET} pthread)
endif (NOT WIN32 AND NOT ANDROID)

set( CREATE_DIR_NAME tests/common_runtime_test_type )

set( COMMON_OPT -cl-kernel-arg-info -cl-std=CL2.0 -D__OPENCL_C_VERSION__=200 -D__OPENCL_VERSION__=200 )

if (BUILD_X64)
list (
  APPEND
  SPIR_OPT
  -triple spir64-unknown-unknown -D __x86_64__
)
else (BUILD_X64)
list (
  APPEND
  SPIR_OPT
  -triple spir-unknown-unknown -D __i386__
)
endif(BUILD_X64)

set( INCLUDE_HEADERS_PATH ${LLVM_PATH_SPIRV}/lib/clang/3.6.1/include/)

set( INPUT_FILE kernels/subgroups.cl )
set( SPIRV_OUTPUT_FILE subgroups.spv )

add_custom_command( TARGET ${TARGET}
  POST_BUILD
  COMMAND ${CLANG} -cc1 -x cl ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
  -I ${INCLUDE_HEADERS_PATH} -emit-spirv
  ${COMMON_OPT} ${SPIR_OPT} -Dcl_khr_subgroups
  -include opencl.h -O0 -o ${CMAKE_CURRENT_BINARY_DIR}/${SPIRV_OUTPUT_FILE}
  MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
  DEPENDS ${INCLUDE_HEADERS_PATH}/opencl.h ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
  COMMENT "Generate spirv binary file for common_runtime_test_type:  ${SPIRV_OUTPUT_FILE}"
  VERBATIM
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${SPIRV_OUTPUT_FILE} DESTINATION tests/common_runtime_test_type )

set( INPUT_FILE kernels/simple_kernels.cl )
set( SPIRV_OUTPUT_FILE simple_kernels.spv )

add_custom_command( TARGET ${TARGET}
  POST_BUILD
  COMMAND ${CLANG} -cc1 -x cl ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
  -I ${INCLUDE_HEADERS_PATH} -emit-spirv
  ${COMMON_OPT} ${SPIR_OPT}
  -include opencl.h -O0 -o ${CMAKE_CURRENT_BINARY_DIR}/${SPIRV_OUTPUT_FILE}
  MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
  DEPENDS ${INCLUDE_HEADERS_PATH}/opencl.h ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
  COMMENT "Generate spirv binary file for common_runtime_test_type:  ${SPIRV_OUTPUT_FILE}"
  VERBATIM
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${SPIRV_OUTPUT_FILE} DESTINATION tests/common_runtime_test_type )

file(GLOB_RECURSE CP_FILES RELATIVE  ${CMAKE_CURRENT_SOURCE_DIR} *.py *.xml *.xsd )
# Copy *.py *.xml and *.xsd from current directory to appropriate test directory in 'Release' area.
install(FILES ${CP_FILES} DESTINATION ${CREATE_DIR_NAME})
# Copy *.py from ../GeneralScripts directory to appropriate test directory in 'Release' area.
install(FILES ../GeneralScripts/cmk_test_type_runner.py ../GeneralScripts/cmk_xml_entities.py DESTINATION ${CREATE_DIR_NAME})
# Copy generated executable to appropriate test directory in 'Release' area.
install(TARGETS ${TARGET} DESTINATION ${CREATE_DIR_NAME})

set_target_properties( ${TARGET} PROPERTIES FOLDER "validation" )

install(FILES ${CRT_KERNELS} DESTINATION tests/common_runtime_test_type)
install(FILES common_runtime_test_type.pm DESTINATION tests/common_runtime_test_type)

