cmake_minimum_required(VERSION 2.8.4)
cmake_policy(SET CMP0001 NEW)
cmake_policy(SET CMP0011 OLD)
use_eh(TRUE)

set (TARGET_NAME "ValidationTests")
set (TARGET_LIB_NAME "ValidationTestsBase")

set(TARGET_LIB_SOURCE_FILES
  DMTest.cpp
  DMImageTest.cpp
  CMPTest.cpp
  OpenCLKernelArgumentsParserTest.cpp
  DGTest.cpp
  Float16Test.cpp
  NEATALUUtils.cpp
  NEATALUTest.cpp
  NEATALUCommonTest.cpp
  NEATALUConversionTest.cpp
  NEATALUVLoadStoreTest.cpp
  NEATValueTest.cpp
  NEATMathTest.cpp
  OCLBuiltinParserTest.cpp
  NEATALUImageTest.cpp
)

set(TARGET_SOURCE_FILES
    SourceRecorder.cpp
    ClangSA.cpp
    OCLKernelDataGeneratorTest.cpp
)  

include_directories(${BACKEND_ROOT_DIR}/../externals/gtest/include)
include_directories(${BACKEND_ROOT_DIR}/../cl_api)
include_directories(${BACKEND_ROOT_DIR}/external/tinyxml)
include_directories(${BACKEND_ROOT_DIR}/plugin_manager)
include_directories(${BACKEND_ROOT_DIR}/dynamic_lib)
include_directories(${BACKEND_ROOT_DIR}/ocl_cpu_backend/export)
include_directories(${VALIDATIONS_ROOT}/Common)
include_directories(${VALIDATIONS_ROOT}/DataManager)
include_directories(${VALIDATIONS_ROOT}/Comparator)
include_directories(${VALIDATIONS_ROOT}/ImathLibd)
include_directories(${VALIDATIONS_ROOT}/NEAT)
include_directories(${VALIDATIONS_ROOT}/REFALU)
include_directories(${VALIDATIONS_ROOT}/DataGenerator)
include_directories(${VALIDATIONS_ROOT}/OCLBuiltins)
include_directories(${VALIDATIONS_ROOT}/PlugInNEAT)
include_directories(${VALIDATIONS_ROOT}/OCLBuilder)
include_directories(${VALIDATIONS_ROOT}/OCLRecorder)
include_directories(${VALIDATIONS_ROOT}/OCLKernelDataGenerator)
include_directories(${VALIDATIONS_ROOT}/OpenCLKernelArgumentsParser)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Common )
 
add_library( ${TARGET_LIB_NAME} STATIC
    ${TARGET_LIB_SOURCE_FILES}
)
 
link_directories(
    ${LLVM_LIBRARY_DIR}
    ${OCL_BINARY_DIR}/lib/${CMAKE_CFG_INTDIR}
)

add_executable(${TARGET_NAME}
  ${TARGET_SOURCE_FILES}
)

target_link_libraries(${TARGET_NAME}
    ${TARGET_LIB_NAME}
    DataManager 
    tinyxml_STL
    Comparator
    DataGenerator
    NEAT 
    REFALU
    PlugInNEAT 
    OCLBuiltins
    googletest
    ImathLibd
    OclPluginManager
    dynamic_load
    OCLRecorderStat
    OCLBuilder
    OCLKernelDataGenerator
    OpenCLKernelArgumentsParser
    name_mangle
    ${LLVM_MODULE_LIBS}
)

if( NOT WIN32)
    target_link_libraries(${TARGET_NAME}
        pthread
    )
    add_dependencies(${TARGET_NAME}
        ImathLibd_wrapper
    )
endif ()

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Tests")

install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION bin)

add_subdirectory(fe_plugin_mock)
