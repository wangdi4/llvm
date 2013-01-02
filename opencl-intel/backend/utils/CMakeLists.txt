set(TARGET_NAME OclBackendUtils)

set (OCL_UTILS_INCLUDE_FILES
    CompilationUtils.h
    CPUDetect.h
    debuggingservicetype.h
    exceptions.h
    ExecutionContext.h
    ExplicitArgument.h
    ExecutionContext.h
    ExplicitLocalMemArgument.h
    ExplicitGlobalMemArgument.h
    FunctionArgument.h
    IArgument.h
    ImplicitArgProperties.h
    ImplicitArgsUtils.h
    ImplicitArgument.h
    SystemInfo.h
    TLLVMKernelInfo.h
    TypeAlignment.h
    ProcessorDetect.h
)

set (OCL_UTILS_SOURCE_FILES
    CompilationUtils.cpp
    CPUDetect.cpp
    debuggingservicetype.cpp
    FunctionArgument.cpp
    ImplicitArgsUtils.cpp
    SystemInfo.cpp
    TypeAlignment.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/../cl_api
    ${BACKEND_ROOT_DIR}/arch_headers
)

# Include the ASM sources
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (WIN32)
        set (UTILS_ASM_SOURCES ../ocl_cpu_backend/kernel_execute_64.asm)
    elseif (APPLE)
        set (UTILS_ASM_SOURCES mac_utils.s)
    else ()
        set (UTILS_ASM_SOURCES ../ocl_cpu_backend/linux64_utils.s)
    endif ()
    CREATE_ASM_RULES( UTILS_ASM_ADD_TO_SOURCES_LIST ${UTILS_ASM_SOURCES} )
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

add_library(${TARGET_NAME} STATIC
    ${OCL_UTILS_INCLUDE_FILES}
    ${OCL_UTILS_SOURCE_FILES}
    ${UTILS_ASM_ADD_TO_SOURCES_LIST}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME} )
