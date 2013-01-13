set(TARGET_NAME x86_plugin)

set(TARGET_SOURCE_FILES  
    x86_plugin.cpp
    x86_macho_link.cpp
    x86_archive.cpp
)

set(TARGET_INCLUDE_FILES
    x86_archive.h
    x86_cvms.h
    Trie.h
)

find_package(OpenGL REQUIRED)
find_package(OpenCL REQUIRED)

#include_directories( SYSTEM
#     ${OPENGL_INCLUDE_DIR}
#)

include_directories(
    ${BACKEND_ROOT_DIR}/optimizer
    ${BACKEND_ROOT_DIR}/utils
    ${BACKEND_ROOT_DIR}/metadata_api
    ${BACKEND_ROOT_DIR}/metadata_api/gen
    ${BACKEND_ROOT_DIR}/arch_headers
    ${BACKEND_ROOT_DIR}/vectorizer/AppleWrapper
    ${BACKEND_ROOT_DIR}/../cl_api
    ${CVMS_INCLUDE_DIR}
)

# link_directories should be before add_library
link_directories(
	${OPENCL_LIBRARIES}/Libraries 
	${OPENGL_LIBRARIES}/Libraries
)

add_library(${TARGET_NAME} SHARED 
    ${TARGET_SOURCE_FILES} 
    ${TARGET_INCLUDE_FILES}
)

set (PASSES_LIBS
    # Backend passes
    AddImplicitArgs
    BuiltInFuncImport
    ClangCompatFixer
    InstToFuncCall
    LocalBuffers
    LocalBuffAnalysis
    ModuleCleanup
    PrepareKernelArgs
    PreventDivCrashes
    PrintIRPass
    RelaxedPass
    ResolveWICall
    ShiftZeroUpperBits
    ShuffleCallToInst
    UndefExternalFuncs
    KernelInfo
    LLVMVectorizer
    LLVMBarrier
    )

target_link_libraries(${TARGET_NAME}
    OclOptimizer
    name_mangle
    metadataapi
    OclUtils
    "-framework Foundation"
    "-framework CoreFoundation"
    cl2module
  
    # Backend passes
    ${PASSES_LIBS}
    
    # LLVM libs
    LLVMContainer
    )

install( TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION bin)
    
file(RELATIVE_PATH SOURCE_RELATIVE_DIR ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

install( FILES ${TARGET_INCLUDE_FILES} ${TARGET_SOURCE_FILES}
         DESTINATION src/${SOURCE_RELATIVE_DIR}
       )
    