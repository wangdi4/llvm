cmake_minimum_required(VERSION 2.8.7)
cmake_policy(SET CMP0001 NEW)
cmake_policy(SET CMP0011 OLD)
use_eh(TRUE)

set (TARGET_NAME "ValidationTests")

# to be shure that ICC compiler uses the correct presentation of 80-bit long doubles
# we have to set proper compiler options, for win32 only, for linux gcc(g++) is used and 
# we dont need to set such options
if (WIN32)
set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:source /fp:strict /Qlong-double /Qpc80 /Qprec-div" )
set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /fp:source /fp:strict /Qlong-double /Qpc80 /Qprec-div" ) 
endif(WIN32)


set(TARGET_SOURCE_FILES
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
include_directories(${CMAKE_SOURCE_DIR}/utils/cl_sys_utils/export)
 
 
link_directories(
    ${LLVM_LIBRARY_DIR}
    ${OCL_BINARY_DIR}/lib/${CMAKE_CFG_INTDIR}
)

add_executable(${TARGET_NAME}
  ${TARGET_SOURCE_FILES}
)

target_link_libraries(${TARGET_NAME}
    DataManager 
    Comparator
    DataGenerator
    NEAT 
    REFALU
    PlugInNEAT 
    OCLBuiltins
    googletest
    ImathLibd
    OclPluginManager
    OCLRecorderStat
    OCLBuilder
    OCLKernelDataGenerator
    OpenCLKernelArgumentsParser
    name_mangle
    dynamic_load
    cl_sys_utils
    tinyxml_STL
    ${LLVM_MODULE_LIBS}
)

if(NOT WIN32)
    target_link_libraries(${TARGET_NAME} pthread)
    add_dependencies(${TARGET_NAME} ImathLibd_wrapper)
endif(NOT WIN32)

add_dependencies( ${TARGET_NAME} cl_sys_utils)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Tests")

install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION bin)

add_subdirectory(fe_plugin_mock)
