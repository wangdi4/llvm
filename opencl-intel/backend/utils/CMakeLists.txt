set(TARGET_NAME OclUtils)

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
  ExecutionContext.h 
  ExplicitArgument.h
  ExplicitLocalMemArgument.h
  ExplicitGlobalMemArgument.h
  SystemInfo.h
)
endif(APPLE)

set (TARGET_INCLUDE_FILES
  CompilationUtils.h
  CPUDetect.h
  debuggingservicetype.h
  exceptions.h
  FunctionArgument.h
  IArgument.h
  ImplicitArgProperties.h
  ImplicitArgsUtils.h
  ImplicitArgument.h
  ProcessorDetect.h
  TLLVMKernelInfo.h
  TypeAlignment.h
  ${TARGET_EXTRA_INCLUDE_FILES}
)

set (TARGET_SOURCE_FILES
  CompilationUtils.cpp
  CPUDetect.cpp
  debuggingservicetype.cpp
  FunctionArgument.cpp
  ImplicitArgsUtils.cpp
  TypeAlignment.cpp
  ${TARGET_EXTRA_SOURCE_FILES}
)

include_directories(
    ${BACKEND_ROOT_DIR}/../cl_api
    ${BACKEND_ROOT_DIR}/arch_headers
)

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

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
    ${UTILS_ASM_ADD_TO_SOURCES_LIST}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME} )

if(APPLE)
  file(RELATIVE_PATH SOURCE_RELATIVE_DIR ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
  message("Utils relative dir:" ${SOURCE_RELATIVE_DIR})
  install( FILES ${TARGET_INCLUDE_FILES} ${TARGET_SOURCE_FILES} ${UTILS_ASM_ADD_TO_SOURCES_LIST}
           DESTINATION src/${SOURCE_RELATIVE_DIR}
         )
endif(APPLE)
