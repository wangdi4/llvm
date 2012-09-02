set(TARGET_NAME OclUtils)

set (OCL_UTILS_INCLUDE_FILES
    CompilationUtils.h
    CPUDetect.h
    exceptions.h
    ExplicitArgument.h
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
    FunctionArgument.cpp
    ImplicitArgsUtils.cpp
    SystemInfo.cpp
    TypeAlignment.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/../cl_api
    ${BACKEND_ROOT_DIR}/ocl_cpu_backend
    ${BACKEND_ROOT_DIR}/arch_headers
)

# Include the ASM sources
if ( NOT APPLE )
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (WIN32)
        set (UTILS_ASM_SOURCES ../ocl_cpu_backend/kernel_execute_64.asm)
    else (WIN32)
        set (UTILS_ASM_SOURCES ../ocl_cpu_backend/linux64_utils.s)
    endif (WIN32)
    CREATE_ASM_RULES( UTILS_ASM_ADD_TO_SOURCES_LIST ${UTILS_ASM_SOURCES} )
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)
endif (NOT APPLE)

add_library(${TARGET_NAME}
    ${OCL_UTILS_INCLUDE_FILES}
    ${OCL_UTILS_SOURCE_FILES}
    ${UTILS_ASM_ADD_TO_SOURCES_LIST}
)
