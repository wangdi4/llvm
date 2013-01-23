set(TARGET_NAME x86_plugin)

set(TARGET_SOURCE_FILES  
    x86_plugin.cpp
)

set(TARGET_INCLUDE_FILES
    x86_archive.h
    x86_cvms.h
)

find_package(OpenGL REQUIRED)
find_package(OpenCL REQUIRED)

include_directories( SYSTEM
     ${OPENGL_INCLUDE_DIR}
)

include_directories(
    ${BACKEND_ROOT_DIR}/optimizer
    ${BACKEND_ROOT_DIR}/utils
    ${BACKEND_ROOT_DIR}/metadata_api
    ${BACKEND_ROOT_DIR}/metadata_api/gen
    ${BACKEND_ROOT_DIR}/arch_headers
    ${BACKEND_ROOT_DIR}/vectorizer
)

# link_directories should be before add_library
link_directories(
	${OPENCL_LIBRARIES} 
	${LLVM_LIBRARY_DIR}
)

add_library(${TARGET_NAME} SHARED 
    ${TARGET_SOURCE_FILES} 
    ${TARGET_INCLUDE_FILES}
)

set (PASSES_LIBS
    # Backend passes
    AddImplicitArgs
    BuiltInFuncImport
    DebugInfoPass
    InstToFuncCall
    LocalBuffers
    LocalBuffAnalysis
    ModuleCleanup
    PrepareKernelArgs
    PreventDivCrashes
    PrintIRPass
    ProfilingInfoPass
    RelaxedPass
    ResolveWICall
    ShiftZeroUpperBits
    ShuffleCallToInst
    UndefExternalFuncs
    LLVMVectorizer
    LLVMBarrier
    )

target_link_libraries(${TARGET_NAME}
    OclOptimizer
    name_mangle
    metadataapi
    OclBackendUtils
    "-framework Foundation"
    "-framework CoreFoundation"
    cl2module
  
    # Backend passes
    ${PASSES_LIBS}
    
    # LLVM libs
    ${LLVM_MODULE_LIBS}
    )

install( TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION bin)