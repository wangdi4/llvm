set(TARGET_NAME OclBackendUtils)

add_subdirectory(LoopUtils)
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
  Logger.h
  OclTune.h
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
  OclTune.cpp
  ${TARGET_EXTRA_SOURCE_FILES}
)

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${BACKEND_ROOT_DIR}/../cl_api
  ${CL_API_HEADERS}
  ${BACKEND_ROOT_DIR}/name_mangling
  ${BACKEND_ROOT_DIR}/reflection_module
  ${BACKEND_ROOT_DIR}/metadata_api
  ${BACKEND_ROOT_DIR}/utils
  ${CMAKE_BINARY_DIR}/include
  ${CMAKE_BINARY_DIR}
)

include_directories(
    ${BACKEND_ROOT_DIR}/arch_headers
)

# Include the ASM sources
if(BUILD_X64)
    if (WIN32)
        CREATE_ASM_RULES( UTILS_ASM_ADD_TO_SOURCES_LIST ../ocl_cpu_backend/kernel_execute_64.asm )
    else ()
        set (UTILS_ASM_ADD_TO_SOURCES_LIST ../ocl_cpu_backend/linux64_utils.s)
    endif ()
endif(BUILD_X64)

add_library(${TARGET_NAME} STATIC
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
    ${UTILS_ASM_ADD_TO_SOURCES_LIST}
)

add_dependencies(${TARGET_NAME} metadataapi)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME} )
