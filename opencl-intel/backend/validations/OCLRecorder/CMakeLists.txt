if( NOT DEFINED LLVM_PATH_FE )
    message( FATAL_ERROR "LLVM_PATH_FE is not specified. Please specify LLVM library location for backend using LLVM_PATH_FE parameter to CMAKE" )
endif()
set (LLVM_FE_INCLUDE_DIRS ${LLVM_PATH_FE}/include)

find_package(LLVM REQUIRED)
if(NOT WIN32)
add_definitions(-msse4.2)
endif(NOT WIN32)

set(TARGET_NAME OclRecorder)

project(${TARGET_NAME})

cmake_minimum_required(VERSION 2.8.7)

#clang include directories, injected for the OCL source recorder
add_definitions(-DCLANG_HEADERS="${BACKEND_ROOT_DIR}/clang_headers")

set(OCL_RECORDER_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(OCL_RECORDER_IMPORT_DIR ${BACKEND_ROOT_DIR}/../cl_api)
set(OCL_RECORDER_EXPORT_DIR ${OCL_RECORDER_DIR}/export)
set(OCL_RECORDER_LIB_DIR ${OCL_RECORDER_DIR}/lib/Win${PLATFORM})

# Variables for ocldevbackend project
set (OCL_BACKEND_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../ocl_cpu_backend)
set (OCL_BACKEND_EXPORTS_DIR ${OCL_BACKEND_ROOT}/export)
set (OCL_BACKEND_IMPORTS_DIR ${OCL_BACKEND_ROOT}/../cl_api)

# Other dependency directories
set (OCL_EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../external)
set (OCL_DATAMANAGER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../validations/DataManager)
set (OCL_NEAT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../validations/NEAT)

set (OCL_RECORDER_INCLUDE_FILES
  ocl_recorder.h
  ocl_source_recorder.h
  stdafx.h
  targetver.h
  ${VALIDATIONS_ROOT}/Common/md5.h
  )

set (OCL_RECORDER_SOURCE_FILES
  dllmain.cpp
  ocl_recorder.cpp
  ocl_source_recorder.cpp
  stdafx.cpp
  ${VALIDATIONS_ROOT}/Common/md5.cpp
  )

# Resources
if (WIN32)
  set (OCL_RECORDER_RESOURCE_FILES ocl_recorder.rc)
else (WIN32)
  set (OCL_RECORDER_RESOURCE_FILES)
endif (WIN32)

# Add include directories
include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${LLVM_FE_INCLUDE_DIRS}/cclang
  ${VALIDATIONS_ROOT}/DataVersion
  ${OCL_RECORDER_IMPORT_DIR}
  ${OCL_RECORDER_EXPORT_DIR}
  ${OCL_BACKEND_IMPORTS_DIR}
  ${OCL_BACKEND_EXPORTS_DIR}
  ${OCL_EXTERNAL_DIR}/tinyxml
  ${OCL_DATAMANAGER_DIR}
  ${OCL_NEAT_DIR}
  ${CL_API_HEADERS}
  ${CMAKE_SOURCE_DIR}/backend/plugin_manager
  ${OCL_BINARY_DIR}
)

message (STATUS DEBUG: LLVM_PATH_FE=${LLVM_PATH_FE})

if(WIN32)
    get_target_property( lib_dir NEAT_wrapper lib_dir )

    # Add additional lib direcories
    link_directories (
            ${OCL_RECORDER_LIB_DIR}
            ${lib_dir}
    )
endif()

# Create DLL
add_library (${TARGET_NAME} SHARED
    ${OCL_RECORDER_INCLUDE_FILES}
    ${OCL_RECORDER_SOURCE_FILES}
    ${OCL_RECORDER_RESOURCE_FILES}
    ${ASM_ADD_TO_SOURCES_LIST}
)

# Create static library for unit tests
add_library (OCLRecorderStat STATIC
    ${OCL_RECORDER_SOURCE_FILES}
)

# Link with appropriate libs
target_link_libraries (${TARGET_NAME}
    tinyxml_STL
    DataManager
    OclPluginManager
    ${LLVM_MODULE_LIBS}
    NEAT
)

if(WIN32)
    # Define the dependancy projects
    add_dependencies(${TARGET_NAME} NEAT_wrapper)
else(WIN32)
    add_dependencies(${TARGET_NAME} NEAT)
endif(WIN32)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/plugins )
set_target_properties( OCLRecorderStat PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/plugins )

install(TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION bin)

install(TARGETS OCLRecorderStat
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION bin)
