set(TARGET_NAME oclopt)

include_directories(${CMAKE_SOURCE_DIR}/backend/passes)

link_directories(${LLVM_LIBRARY_DIR})

add_executable(${TARGET_NAME}
    ocl_opt.cpp
    ${CMAKE_SOURCE_DIR}/backend/passes/InitializePasses.h
    ${CMAKE_SOURCE_DIR}/backend/passes/OCLPassSupport.h
)

# Backend passes
set(OCL_BE_PASSES
    BuiltInFuncImport
    InstToFuncCall
    GenericAddressResolution
    PreventDivCrashes
    RelaxedPass
    ShiftZeroUpperBits
    ShuffleCallToInst
    Prefetch
    LinearIdResolver
    LLVMBarrier
    LLVMVectorizer
    Materialize 
    PrepareKernelArgs
    AddImplicitArgs
    ModuleCleanup
    BackendPassesWrappers
    LocalBuffers
    LocalBuffAnalysis
    DetectFunctionPtrCalls
    ResolveWICall
    CloneBlockInvokeFuncToKernel
)

# Dependencies of the passes
set(OCL_BE_PASSES_DEPS
    OclBackendUtils
    reflection_module
    name_mangle
    metadataapi
)

target_link_libraries(${TARGET_NAME}
    ${OCL_BE_PASSES}
    ${OCL_BE_PASSES_DEPS}
    LLVMOptEngine
    ${LLVM_MODULE_LIBS}
)

if(NOT WIN32)
    if(NOT ANDROID)
        target_link_libraries(${TARGET_NAME} pthread)
    endif(NOT ANDROID)
    target_link_libraries(${TARGET_NAME} dl)
endif(NOT WIN32)

add_dependencies(${TARGET_NAME} metadataapi)

if( CMAKE_CROSSCOMPILING )
    add_custom_target( oclopt_native ALL
      COMMAND make oclopt
      DEPENDS ConfigureNativeTargets
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/native
      COMMENT "Cross-Compile: Building Native ocl_opt tool"
    )
endif( CMAKE_CROSSCOMPILING )

install( TARGETS ${TARGET_NAME} RUNTIME DESTINATION bin)

