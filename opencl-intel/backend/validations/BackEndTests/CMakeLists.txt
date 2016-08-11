#ensure we are using the frontend LLVM tools
if( NOT DEFINED LLVM_PATH_FE )
    message( FATAL_ERROR "LLVM_PATH_FE is not specified. Please specify LLVM library location for fronted using LLVM_PATH_FE parameter to CMAKE" )
endif()

set(LLVM_PATH ${LLVM_PATH_FE})
find_package(LLVM REQUIRED)

if( CMAKE_CROSSCOMPILING OR LLVM_USE_NATIVE OR ANDROID )
   set(TOOLS_BINARY_DIR "${LLVM_BINARY_DIR}/../native/bin")
   set(SPIRV_CONVERTER  "${LLVM_PATH_BE}/native/bin/llvm-spirv")
else()
   set(TOOLS_BINARY_DIR "${LLVM_BINARY_DIR}" )
   set(SPIRV_CONVERTER  "${LLVM_PATH_BE}/bin/llvm-spirv")
endif()

set( CLANG "${TOOLS_BINARY_DIR}/clang" )

set( INCLUDE_HEADERS_PATH ${LLVM_INCLUDE_DIRS}/cclang)

if (BUILD_X64)
set (
  SPIR_OPT
  -triple spir64-unknown-unknown -cl-kernel-arg-info -D __x86_64__
)
set ( NAME_POSTFIX _64 )
else (BUILD_X64)
set (
  SPIR_OPT
  -triple spir-unknown-unknown -cl-kernel-arg-info -D __i386__
)
set ( NAME_POSTFIX _32 )
endif(BUILD_X64)

# function to compile cl to bc.
function( compile INPUT_FILE OUTPUT_FILE_NAME)

  add_custom_command( TARGET ${TARGET_NAME}
    COMMAND ${CLANG} -cc1 -x cl ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
    -I ${INCLUDE_HEADERS_PATH} -emit-llvm-bc ${SPIR_OPT}
    -include opencl-c.h -O0 -o ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_FILE_NAME}
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
    DEPENDS ${INCLUDE_HEADERS_PATH}/opencl-c.h ${CLANG}
    COMMENT "Generate spir binary files for ${TARGET_NAME}: ${SPIR_OUTPUT_FILE}"
    VERBATIM
  )

endfunction( compile )

add_subdirectory(OCLSamplePlugin)

set(TARGET_NAME BackEndTests)

include_directories(
${LLVM_INCLUDE_DIRS}
${BACKEND_ROOT_DIR}/../externals/gtest/include
${BACKEND_ROOT_DIR}/../cl_api
${CL_API_HEADERS}
${BACKEND_ROOT_DIR}/utils
${BACKEND_ROOT_DIR}/ocl_cpu_backend
${BACKEND_ROOT_DIR}/validations/Common      
${BACKEND_ROOT_DIR}/dynamic_lib             
${BACKEND_ROOT_DIR}/ocl_cpu_backend/export  
${BACKEND_ROOT_DIR}/arch_headers            
${BACKEND_ROOT_DIR}/plugin_manager
${CMAKE_SOURCE_DIR}/utils/cl_sys_utils/export
)

set (BACKEND_TESTS_INCLUDE_FILES
  BackendWrapper.h
  BWOptions.h
)

set (BACKEND_TESTS_SOURCE_FILES
  BackendWrapper.cpp
  BWOptions.cpp
  FactoryTest.cpp
  CompilationServiceTest.cpp
  PluginsTest.cpp
  BlockLiteralTest.cpp
  KernelSubGroupInfoTest.cpp
)

add_executable(${TARGET_NAME}
  ${BACKEND_TESTS_INCLUDE_FILES}
  ${BACKEND_TESTS_SOURCE_FILES}
)

target_link_libraries(${TARGET_NAME}
  OclBackendUtils
  OclPluginManager
  googletest
  dynamic_load
  cl_sys_utils
  ${LLVM_MODULE_LIBS}
)

set( CMAKE_CXX_FLAGS_DEBUG "-DOCL_DEV_BACKEND_PLUGINS ${CMAKE_CXX_FLAGS_DEBUG}")
add_dependencies( ${TARGET_NAME} cl_sys_utils)

# compile cl files to bc
set ( NOKERNELS_OUTPUT_NAME bitcodeNoKernels${NAME_POSTFIX}.bc )
compile( Bitcode/cltestNoKernels.cl ${NOKERNELS_OUTPUT_NAME} )
set ( WITHKERNELS_OUTPUT_NAME bitcodeWithKernels${NAME_POSTFIX}.bc )
compile( Bitcode/cltestWithKernels.cl ${WITHKERNELS_OUTPUT_NAME} )

# install the bitcode files that the project loads
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${NOKERNELS_OUTPUT_NAME} DESTINATION tests/BackEndTests)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${WITHKERNELS_OUTPUT_NAME} DESTINATION tests/BackEndTests)
install(FILES Bitcode/bitcodeNoise.bc DESTINATION tests/BackEndTests)
install(FILES BackEndTests.pm DESTINATION tests/BackEndTests)
if (BUILD_X64)
    install(FILES Bitcode/reqd_num_sub_groups_64.bc DESTINATION tests/BackEndTests)
else (BUILD_X64)
    install(FILES Bitcode/reqd_num_sub_groups_32.bc DESTINATION tests/BackEndTests)
endif(BUILD_X64)

install( TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION tests/BackEndTests
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION tests/BackEndTests)
