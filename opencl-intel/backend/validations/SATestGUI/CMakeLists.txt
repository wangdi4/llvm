cmake_minimum_required(VERSION 2.8)

set(TARGET_NAME SATestGUI)

use_rtti(FALSE)
add_definitions ( -DCOI_LIBRARY_VERSION=2 )
if (WIN32)
    set (ICC_WIN32_PATH $ENV{ICPP_COMPILER12})

    if (CMAKE_SIZEOF_VOID_P EQUAL 4)
      set (ICC_PLATFORM ia32)
    else ()
      set (ICC_PLATFORM intel64)
    endif ()

    link_directories( "${ICC_WIN32_PATH}/compiler/lib/${ICC_PLATFORM}" )
endif(WIN32)

SET(QT_USE_QTXML TRUE)
file(GLOB SATESTGUI_SRCS *.cpp)
file(GLOB SATESTGUI_HDRS *.h)
file(GLOB SATESTGUI_FORMS *.ui)
SET( SATESTGUI_RCS resources.qrc )
QT4_ADD_RESOURCES( SATESTGUI_RC_SRCS ${SATESTGUI_RCS} )
QT4_WRAP_CPP(SATESTGUI_HEADERS_MOC ${SATESTGUI_HDRS})
QT4_WRAP_UI(SATESTGUI_FORMS_HEADERS ${SATESTGUI_FORMS})
INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_EXECUTABLE(${TARGET_NAME} ${SATESTGUI_SRCS}
    ${SATESTGUI_HEADERS_MOC}
    ${SATESTGUI_FORMS_HEADERS}
        ${SATESTGUI_RC_SRCS})

link_directories(
    ${LLVM_LIBRARY_DIR}
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${VALIDATIONS_ROOT}/SATest
    ${VALIDATIONS_ROOT}/DataManager
    ${VALIDATIONS_ROOT}/NEAT
    ${VALIDATIONS_ROOT}/REFALU
    ${VALIDATIONS_ROOT}/Comparator
    ${VALIDATIONS_ROOT}/Common
    ${VALIDATIONS_ROOT}/OCLBuiltins
    ${VALIDATIONS_ROOT}/../external/tinyxml
    ${BACKEND_ROOT_DIR}/../cl_api
    ${VALIDATIONS_ROOT}/../ocl_cpu_backend
    ${VALIDATIONS_ROOT}/../ocl_cpu_backend/export
    ${BACKEND_ROOT_DIR}/utils
    ${BACKEND_ROOT_DIR}/arch_headers
    ${BACKEND_ROOT_DIR}/dynamic_lib
    ${BACKEND_ROOT_DIR}/name_mangling
    ${LLVM_ROOT}/lib/ExecutionEngine/Interpreter
    ${VALIDATIONS_ROOT}/InterpreterPluggable
    ${VALIDATIONS_ROOT}/PlugInNEAT
    ${VALIDATIONS_ROOT}/OCLBuilder
    ${VALIDATIONS_ROOT}/OpenCLKernelArgumentsParser
    ${VALIDATIONS_ROOT}/OCLKernelDataGenerator
    ${CMAKE_SOURCE_DIR}/utils/cl_sys_utils/export
    ${VALIDATIONS_ROOT}/ProgramConfiguration
)

target_link_libraries(
${TARGET_NAME}
    OCLBuilder
    PlugInNEAT
    DataManager
    OpenCLKernelArgumentsParser
    OCLKernelDataGenerator
    Comparator
    LLVMInterpreterPluggable
    OclBackendUtils
    LLVMInterpreter
    OCLBuiltins
    NEAT
    REFALU
    name_mangle
    dynamic_load
    tinyxml_STL
    ImathLibd
    cl_sys_utils
    ProgramConfiguration
    ${LLVM_MODULE_LIBS}
    ${QT_LIBRARIES}
)

add_dependencies( ${TARGET_NAME}
    cl_sys_utils
    ProgramConfiguration
)

install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION bin)

