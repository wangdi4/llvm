cmake_minimum_required(VERSION 2.8.7)
cmake_policy(SET CMP0001 NEW)
cmake_policy(SET CMP0011 OLD)
use_eh(TRUE)

if( NOT DEFINED LLVM_PATH_FE )
    message( FATAL_ERROR "LLVM_PATH_FE is not specified. Please specify LLVM library location for backend using LLVM_PATH_FE parameter to CMAKE" )
endif()
set (LLVM_FE_INCLUDE_DIRS ${LLVM_PATH_FE}/include)

set (TARGET_NAME ValidationTests)

# to be shure that ICC compiler uses the correct presentation of 80-bit long doubles
# we have to set proper compiler options, for win32 only, for linux gcc(g++) is used and 
# we dont need to set such options
if (WIN32)
set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:source /fp:strict /Qlong-double /Qpc80 /Qprec-div" )
set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /fp:source /fp:strict /Qlong-double /Qpc80 /Qprec-div" ) 
endif(WIN32)


set(TARGET_SOURCE_FILES
    DVTest.cpp
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
    SourceRecorder.cpp
    ClangSA.cpp
    OCLKernelDataGeneratorTest.cpp
)

include_directories(
    ${LLVM_INCLUDE_DIRS}
    ${LLVM_FE_INCLUDE_DIRS}/cclang
    ${BACKEND_ROOT_DIR}/../externals/gtest/include
    ${BACKEND_ROOT_DIR}/../cl_api
    ${BACKEND_ROOT_DIR}/external/tinyxml
    ${BACKEND_ROOT_DIR}/plugin_manager
    ${BACKEND_ROOT_DIR}/dynamic_lib
    ${BACKEND_ROOT_DIR}/ocl_cpu_backend/export
    ${VALIDATIONS_ROOT}/Common
    ${VALIDATIONS_ROOT}/DataVersion
    ${VALIDATIONS_ROOT}/DataManager
    ${VALIDATIONS_ROOT}/Comparator
    ${VALIDATIONS_ROOT}/ImathLibd
    ${VALIDATIONS_ROOT}/NEAT
    ${VALIDATIONS_ROOT}/REFALU
    ${VALIDATIONS_ROOT}/DataGenerator
    ${VALIDATIONS_ROOT}/OCLBuiltins
    ${VALIDATIONS_ROOT}/PlugInNEAT
    ${VALIDATIONS_ROOT}/OCLBuilder
    ${VALIDATIONS_ROOT}/OCLRecorder
    ${VALIDATIONS_ROOT}/OCLKernelDataGenerator
    ${VALIDATIONS_ROOT}/OpenCLKernelArgumentsParser
    ${CMAKE_CURRENT_SOURCE_DIR}/Common 
    ${CMAKE_SOURCE_DIR}/utils/cl_sys_utils/export
)

link_directories(
    ${OCL_BINARY_DIR}/lib/${CMAKE_CFG_INTDIR}
)

add_executable(${TARGET_NAME}
  ${TARGET_SOURCE_FILES}
)

target_link_libraries(${TARGET_NAME}
    OclPluginManager
    OCLRecorderStat
    OCLBuilder
    OCLKernelDataGenerator
    OpenCLKernelArgumentsParser
    DataManager 
    Comparator
    DataGenerator
    NEAT 
    REFALU
    PlugInNEAT 
    OCLBuiltins
    googletest
    ImathLibd
    name_mangle
    dynamic_load
    cl_sys_utils
    tinyxml_STL
    ${LLVM_MODULE_LIBS}
)

if(NOT WIN32)
    add_dependencies(${TARGET_NAME} ImathLibd_wrapper)
endif(NOT WIN32)

add_dependencies( ${TARGET_NAME} cl_sys_utils)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Tests")

install(FILES ValidationTests.pm DESTINATION tests/LITTests)
install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION tests/LITTests)

add_subdirectory(fe_plugin_mock)
