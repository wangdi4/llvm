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
	set (OCL_RECORDER_RESOURCE_FILES
		ocl_recorder.rc
	)
else (WIN32)
	set (OCL_RECORDER_RESOURCE_FILES
	)
endif (WIN32)

# Add include directories
include_directories(${VALIDATIONS_ROOT}/DataVersion)
include_directories(${OCL_RECORDER_IMPORT_DIR} ${OCL_RECORDER_EXPORT_DIR} )
include_directories(${OCL_BACKEND_IMPORTS_DIR} ${OCL_BACKEND_EXPORTS_DIR} )
include_directories(${OCL_EXTERNAL_DIR}/tinyxml ${OCL_DATAMANAGER_DIR} ${OCL_NEAT_DIR})
include_directories(${CMAKE_SOURCE_DIR}/backend/plugin_manager)

link_directories (
    ${LLVM_LIBRARY_DIR}
) 

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

# fix for CSSD100015940 (OCLRecorder doesn't work with ViennaCL workload.)
# not clear why this it helps, OCLRecorder should not use pthread
# the CSSD100017106 has been opened 
if (NOT WIN32)
  if(NOT ANDROID)
    target_link_libraries(${TARGET_NAME} pthread dl)
  endif(NOT ANDROID)
endif (NOT WIN32)

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

