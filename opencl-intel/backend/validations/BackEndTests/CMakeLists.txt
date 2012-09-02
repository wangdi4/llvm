
add_subdirectory(OCLSamplePlugin)

set(TARGET_NAME BackEndTests)

include_directories(${LLVM_MAIN_SRC_DIR}/utils/unittest/googletest/include)
include_directories(${BACKEND_ROOT_DIR}/../cl_api)
include_directories(${BACKEND_ROOT_DIR}/utils)
include_directories(${BACKEND_ROOT_DIR}/ocl_cpu_backend)
include_directories(${BACKEND_ROOT_DIR}/validations/Common)      #for auto-pointer template
include_directories(${BACKEND_ROOT_DIR}/dynamic_lib)             #for DLL loading
include_directories(${BACKEND_ROOT_DIR}/ocl_cpu_backend/export ) #for plugins interface
include_directories(${BACKEND_ROOT_DIR}/arch_headers)            #including CPUDetect headers
include_directories(${BACKEND_ROOT_DIR}/plugin_manager)            #including CPUDetect headers

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
    )

add_executable(${TARGET_NAME}
    ${BACKEND_TESTS_INCLUDE_FILES}
    ${BACKEND_TESTS_SOURCE_FILES}
)

set(GOOGLE_TEST_LIBS
  gtest
  gtest_main
  LLVMSupport # gtest needs it for raw_ostream.
)

target_link_libraries(${TARGET_NAME} ${GOOGLE_TEST_LIBS}
  OclUtils
  dynamic_load
  OclPluginManager
)

# install the bitcode files that the project loads
install(FILES Bitcode/bitcodeNoise.bc Bitcode/bitcodeNoKernels.bc Bitcode/bitcodeWithKernels.bc DESTINATION bin)


install( TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION bin)
    
    
    
