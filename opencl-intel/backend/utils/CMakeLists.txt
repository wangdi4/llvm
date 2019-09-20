set(TARGET_NAME OclBackendUtils)
set_opencl_version()

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
  BlockUtils.h
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
  RefcountThreadSafe.h
  Atomics.h
  Logger.h
  OclTune.h
  ${TARGET_EXTRA_INCLUDE_FILES}
)

set (TARGET_SOURCE_FILES
  BlockUtils.cpp
  CompilationUtils.cpp
  CPUDetect.cpp
  debuggingservicetype.cpp
  FunctionArgument.cpp
  ImplicitArgsUtils.cpp
  TypeAlignment.cpp
  TypeConversion.cpp
  VectorizerUtils.cpp
  Atomics.cpp
  OclTune.cpp
  ${TARGET_EXTRA_SOURCE_FILES}
)

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${CL_API_HEADERS}
  ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export
  ${BACKEND_ROOT_DIR}/name_mangling
  ${BACKEND_ROOT_DIR}/reflection_module
  ${BACKEND_ROOT_DIR}/metadata_api
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend
  ${CMAKE_BINARY_DIR}/include
  ${CMAKE_BINARY_DIR}
  ${OCL_BINARY_DIR}
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

target_link_libraries(${TARGET_NAME} name_mangle LLVMSupport)

if (OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} intrinsics_gen)
endif()

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME} )
