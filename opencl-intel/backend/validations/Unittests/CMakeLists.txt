use_eh(TRUE)

if (NOT OPENCL_INTREE_BUILD)
  if( NOT DEFINED LLVM_PATH_FE )
    message( FATAL_ERROR "LLVM_PATH_FE is not specified. Please specify LLVM library location for backend using LLVM_PATH_FE parameter to CMAKE" )
  endif()
  set (LLVM_FE_INCLUDE_DIRS ${LLVM_PATH_FE}/include)
endif()

set (TARGET_NAME ValidationTests)

# to be shure that ICC compiler uses the correct presentation of 80-bit long doubles
# we have to set proper compiler options, for win32 only, for linux gcc(g++) is used and
# we dont need to set such options
if (WIN32)
    set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ICC_AUXILIARY_CXX_FLAGS}" )
    set ( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${ICC_AUXILIARY_C_FLAGS}" )
    # some unittests have to use deprecated OpenCL API functions.
    add_definitions(
        -wd4996 # Suppress 'function': was declared deprecated'
    )
else(WIN32)
    # remove the restriction of warning as error for this component
    remove_definitions( -Werror )
endif(WIN32)

include_directories(
    ${LLVM_INCLUDE_DIRS}
    ${CL_API_HEADERS}
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
    ${OCL_SOURCE_DIR}/externals/gtest/googletest/include
    ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export
)

if (OPENCL_INTREE_BUILD)
  include_directories(${CCLANG_DEV_INCLUDE_DIRS})
else()
  include_directories(${LLVM_FE_INCLUDE_DIRS}/cclang)
endif()

link_directories(
    ${OCL_OUTPUT_LIBRARY_DIR}
)

add_opencl_executable(${TARGET_NAME}
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
    OCLBuiltinsRef
    gtest
    ImathLibd
    name_mangle
    dynamic_load
    cl_sys_utils
    tinyxml_STL
    ${LLVM_MODULE_LIBS}
)

add_dependencies( ${TARGET_NAME} cl_sys_utils)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Tests")

add_subdirectory(fe_plugin_mock)
