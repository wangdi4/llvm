#
# INTEL CONFIDENTIAL
#
# Copyright (C) 2022 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may
# not use, modify, copy, publish, distribute, disclose or transmit this software
# or the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express or
# implied warranties, other than those that are expressly stated in the License.
#

# QAGUI
set(QT_VERSION "4.8.5")
if(DEFINED QT_REVISION) # QT_REVISION is a variable that can be defined in
                        # Build.py. It is not defined by default.
  set(QT_VERSION ${QT_REVISION})
endif()

# install the QT and boost libraries themselfs
if(WIN32)
  # special processing for QT library on windows: the problem is that QT doesn't
  # naturally supports installing both 32 and 64 bits libraries on the same
  # machine (there is only a single registry settings for QT library per machine
  # ) this makes it difficult to rely on default QT Cmake support to find the
  # appropriate version. The trick is to set the correct installation directory
  # on the fly by changing the registry information and removing the default
  # paths
  remove_path("c:\\Qt\\4.8.1\\VS2010-64\\bin")
  remove_path("c:\\Qt\\4.8.1\\VS2010\\bin")
  execute_process(COMMAND "reg" add HKCU\\Software\\Trolltech\\Versions /v
                          DefaultQtVersion /d ${QT_VERSION} /f)
  if(BUILD_X64)
    execute_process(
      COMMAND "reg" add HKCU\\Software\\Trolltech\\Versions\\${QT_VERSION} /v
              InstallDir /d c:\\qt\\${QT_VERSION}\\vs2012-64 /f)
  else(BUILD_X64)
    execute_process(
      COMMAND "reg" add HKCU\\Software\\Trolltech\\Versions\\${QT_VERSION} /v
              InstallDir /d c:\\qt\\${QT_VERSION}\\vs2012 /f)
  endif(BUILD_X64)
else(WIN32)
  add_path("/usr/local/DigiaQtCommercialSDK/Desktop/${QT_VERSION}/gcc-64/bin")
endif(WIN32)

find_package(Qt4 REQUIRED)

if(WIN32)
  set(SHARED_DEBUG_LIBS
      ${QT_BINARY_DIR}/QtGuid4.dll ${QT_BINARY_DIR}/QtCored4.dll
      ${QT_BINARY_DIR}/Qt3Supportd4.dll ${QT_BINARY_DIR}/QtNetworkd4.dll
      ${QT_BINARY_DIR}/QtXmld4.dll ${QT_BINARY_DIR}/QtSqld4.dll)
  set(SHARED_RELEASE_LIBS
      ${QT_BINARY_DIR}/QtGui4.dll ${QT_BINARY_DIR}/QtCore4.dll
      ${QT_BINARY_DIR}/Qt3Support4.dll ${QT_BINARY_DIR}/QtNetwork4.dll
      ${QT_BINARY_DIR}/QtXml4.dll ${QT_BINARY_DIR}/QtSql4.dll)
  set(SHARED_DEBUG_LIBS ${SHARED_DEBUG_LIBS}
                        ${QT_BINARY_DIR}/QtCommercialChartd.dll)
  set(SHARED_RELEASE_LIBS ${SHARED_RELEASE_LIBS}
                          ${QT_BINARY_DIR}/QtCommercialChart.dll)
  install(
    FILES ${SHARED_DEBUG_LIBS}
    DESTINATION bin
    CONFIGURATIONS Debug)
  install(
    FILES ${SHARED_RELEASE_LIBS}
    DESTINATION bin
    CONFIGURATIONS Release)
else(WIN32)
  set(QT_VERSION ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH})
  set(SHARED_LIBS
      ${QT_LIBRARY_DIR}/libQtCore.so.4 ${QT_LIBRARY_DIR}/libQtGui.so.4
      ${QT_LIBRARY_DIR}/libQtCore.so.${QT_VERSION}
      ${QT_LIBRARY_DIR}/libQtGui.so.${QT_VERSION})
  set(SQUISH_LIBS
      ${QT_LIBRARY_DIR}/libQt3Support.so.4
      ${QT_LIBRARY_DIR}/libQtNetwork.so.4
      ${QT_LIBRARY_DIR}/libQtSql.so.4
      ${QT_LIBRARY_DIR}/libQtXml.so.4
      ${QT_LIBRARY_DIR}/libQt3Support.so.${QT_VERSION}
      ${QT_LIBRARY_DIR}/libQtNetwork.so.${QT_VERSION}
      ${QT_LIBRARY_DIR}/libQtSql.so.${QT_VERSION}
      ${QT_LIBRARY_DIR}/libQtXml.so.${QT_VERSION})
  install(FILES ${SHARED_LIBS} DESTINATION bin)
  install(FILES ${SQUISH_LIBS} DESTINATION bin)
endif(WIN32)

set(TARGET_NAME SATestGUI)

use_rtti(FALSE)
add_definitions(-DCOI_LIBRARY_VERSION=2)
if(WIN32)
  set(ICC_WIN32_PATH $ENV{ICPP_COMPILER12})

  if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(ICC_PLATFORM ia32)
  else()
    set(ICC_PLATFORM intel64)
  endif()

  link_directories("${ICC_WIN32_PATH}/compiler/lib/${ICC_PLATFORM}")
endif(WIN32)

if(NOT OPENCL_INTREE_BUILD)
  link_directories(${LLVM_LIBRARY_DIRS})
endif()

set(QT_USE_QTXML TRUE)
file(GLOB SATESTGUI_SRCS *.cpp)
file(GLOB SATESTGUI_HDRS *.h)
file(GLOB SATESTGUI_FORMS *.ui)
set(SATESTGUI_RCS resources.qrc)
qt4_add_resources(SATESTGUI_RC_SRCS ${SATESTGUI_RCS})
qt4_wrap_cpp(SATESTGUI_HEADERS_MOC ${SATESTGUI_HDRS})
qt4_wrap_ui(SATESTGUI_FORMS_HEADERS ${SATESTGUI_FORMS})
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

add_executable(${TARGET_NAME} ${SATESTGUI_SRCS} ${SATESTGUI_HEADERS_MOC}
                              ${SATESTGUI_FORMS_HEADERS} ${SATESTGUI_RC_SRCS})

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${LLVM_ROOT}/lib/ExecutionEngine/Interpreter
  ${BACKEND_ROOT_DIR}/arch_headers
  ${BACKEND_ROOT_DIR}/dynamic_lib
  ${BACKEND_ROOT_DIR}/external/tinyxml
  ${BACKEND_ROOT_DIR}/name_mangling
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend/export
  ${CMAKE_CURRENT_BINARY_DIR}
  ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export
  ${VALIDATIONS_ROOT}/DataVersion
  ${VALIDATIONS_ROOT}/SATest
  ${VALIDATIONS_ROOT}/DataManager
  ${VALIDATIONS_ROOT}/NEAT
  ${VALIDATIONS_ROOT}/REFALU
  ${VALIDATIONS_ROOT}/Comparator
  ${VALIDATIONS_ROOT}/Common
  ${VALIDATIONS_ROOT}/OCLBuiltins
  ${VALIDATIONS_ROOT}/InterpreterPluggable
  ${VALIDATIONS_ROOT}/PlugInNEAT
  ${VALIDATIONS_ROOT}/OCLBuilder
  ${VALIDATIONS_ROOT}/OpenCLKernelArgumentsParser
  ${VALIDATIONS_ROOT}/OCLKernelDataGenerator
  ${VALIDATIONS_ROOT}/ProgramConfiguration)

target_link_libraries(
  ${TARGET_NAME}
  OCLBuilder
  PlugInNEAT
  DataManager
  OpenCLKernelArgumentsParser
  OCLKernelDataGenerator
  Comparator
  OclBackendUtils
  OCLBuiltins
  NEAT
  REFALU
  dynamic_load
  tinyxml_STL
  ImathLibd
  cl_sys_utils
  ProgramConfiguration
  ${LLVM_MODULE_LIBS}
  LLVMInterpreter
  LLVMInterpreterPluggable
  ${QT_LIBRARIES})

add_dependencies(${TARGET_NAME} cl_sys_utils ProgramConfiguration)

install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION tests/SATestGUI)
