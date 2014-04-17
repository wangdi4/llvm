add_subdirectory(OCLSamplePlugin)

set(TARGET_NAME BackEndTests)

include_directories(${BACKEND_ROOT_DIR}/../externals/gtest/include)
include_directories(${BACKEND_ROOT_DIR}/../cl_api)
include_directories(${BACKEND_ROOT_DIR}/utils)
include_directories(${BACKEND_ROOT_DIR}/ocl_cpu_backend)
include_directories(${BACKEND_ROOT_DIR}/validations/Common)      #for auto-pointer template
include_directories(${BACKEND_ROOT_DIR}/dynamic_lib)             #for DLL loading
include_directories(${BACKEND_ROOT_DIR}/ocl_cpu_backend/export ) #for plugins interface
include_directories(${BACKEND_ROOT_DIR}/arch_headers)            #including CPUDetect headers
include_directories(${BACKEND_ROOT_DIR}/plugin_manager)
include_directories(${CMAKE_SOURCE_DIR}/utils/cl_sys_utils/export)

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

link_directories (
  ${LLVM_LIBRARY_DIR}
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

if(NOT WIN32)
  target_link_libraries(${TARGET_NAME}
    pthread
    dl
  )
endif(NOT WIN32)

set( CMAKE_CXX_FLAGS_DEBUG "-DOCL_DEV_BACKEND_PLUGINS ${CMAKE_CXX_FLAGS_DEBUG}")
add_dependencies( ${TARGET_NAME} cl_sys_utils)

# install the bitcode files that the project loads
install(FILES Bitcode/bitcodeNoise.bc Bitcode/bitcodeNoKernels.bc Bitcode/bitcodeWithKernels.bc DESTINATION bin)

install( TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION bin)
