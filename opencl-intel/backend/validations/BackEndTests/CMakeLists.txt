add_subdirectory(OCLSamplePlugin)

set(TARGET_NAME BackEndTests)

include_directories(
${LLVM_INCLUDE_DIRS}
${BACKEND_ROOT_DIR}/../externals/gtest/include
${BACKEND_ROOT_DIR}/../cl_api
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

# install the bitcode files that the project loads
install(FILES Bitcode/bitcodeNoise.bc Bitcode/bitcodeNoKernels.bc Bitcode/bitcodeWithKernels.bc DESTINATION tests/BackEndTests)
install(FILES BackEndTests.pm DESTINATION tests/BackEndTests)

install( TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION tests/BackEndTests
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION tests/BackEndTests)
