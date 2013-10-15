set(TARGET_NAME OclBackendUtils)

add_subdirectory(LoopUtils)
if(APPLE)
set(TARGET_EXTRA_SOURCE_FILES 
)
set(TARGET_EXTRA_INCLUDE_FILES 
)
else(APPLE)
set(TARGET_EXTRA_SOURCE_FILES 
  SystemInfo.cpp
)
set(TARGET_EXTRA_INCLUDE_FILES
  exceptions.h
  ExecutionContext.h 
  ExplicitArgument.h
  ExplicitGlobalMemArgument.h
  ExplicitBlockLiteralArgument.h
  SystemInfo.h
)
endif(APPLE)

set (TARGET_INCLUDE_FILES
  CompilationUtils.h
  CPUDetect.h
  debuggingservicetype.h
  FunctionArgument.h
  IArgument.h
  ImplicitArgProperties.h
  ImplicitArgsUtils.h
  ImplicitArgument.h
  OCLAddressSpace.h
  TypeAlignment.h
  TypeConversion.h
  VectorizerUtils.h
  BlockUtils.h
  RefcountThreadSafe.h
  Atomics.h
  ${TARGET_EXTRA_INCLUDE_FILES}
)

set (TARGET_SOURCE_FILES
  CompilationUtils.cpp
  CPUDetect.cpp
  debuggingservicetype.cpp
  FunctionArgument.cpp
  ImplicitArgsUtils.cpp
  TypeAlignment.cpp
  TypeConversion.cpp
  VectorizerUtils.cpp
  BlockUtils.cpp
  Atomics.cpp
  ${TARGET_EXTRA_SOURCE_FILES}
)

include_directories(
  ${BACKEND_ROOT_DIR}/../cl_api
  ${BACKEND_ROOT_DIR}/name_mangling
  ${BACKEND_ROOT_DIR}/metadata_api
  ${BACKEND_ROOT_DIR}/metadata_api/gen
  ${BACKEND_ROOT_DIR}/utils
)

if (APPLE)
include_directories(
    ${BACKEND_ROOT_DIR}/arch_headers/Apple
)
else (APPLE)
include_directories(
    ${BACKEND_ROOT_DIR}/arch_headers
)
endif (APPLE)

# Include the ASM sources
if(BUILD_X64)
    if (WIN32)
        set (UTILS_ASM_SOURCES ../ocl_cpu_backend/kernel_execute_64.asm)
    elseif (APPLE)
        set (UTILS_ASM_SOURCES mac_utils.s)
    else ()
        set (UTILS_ASM_SOURCES ../ocl_cpu_backend/linux64_utils.s)
    endif ()
else(BUILD_X64)
    if (APPLE)
        set (UTILS_ASM_SOURCES mac_utils.s)
    endif (APPLE)
endif(BUILD_X64)

if(APPLE)
    set( UTILS_ASM_ADD_TO_SOURCES_LIST ${UTILS_ASM_SOURCES} )
else(APPLE)
    CREATE_ASM_RULES( UTILS_ASM_ADD_TO_SOURCES_LIST ${UTILS_ASM_SOURCES} )
endif(APPLE)

add_library(${TARGET_NAME} STATIC
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
    ${UTILS_ASM_ADD_TO_SOURCES_LIST}
)

add_dependencies(${TARGET_NAME} metadataapi)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME} )

if(APPLE)
  file(RELATIVE_PATH SOURCE_RELATIVE_DIR ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
  message(STATUS "Utils relative dir:" ${SOURCE_RELATIVE_DIR})
  install( FILES ${TARGET_INCLUDE_FILES} ${TARGET_SOURCE_FILES} ${UTILS_ASM_ADD_TO_SOURCES_LIST}
           DESTINATION src/${SOURCE_RELATIVE_DIR}
         )
endif(APPLE)
