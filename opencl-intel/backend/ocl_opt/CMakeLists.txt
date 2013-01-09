set(TARGET oclopt)

include_directories(${CMAKE_SOURCE_DIR}/backend/passes)

link_directories(${LLVM_LIBRARY_DIR})

add_executable(${TARGET}
    ocl_opt.cpp
)

# Backend passes
set(OCL_BE_PASSES
    BuiltInFuncImport
    InstToFuncCall
    PreventDivCrashes
    RelaxedPass
    ShiftZeroUpperBits
    ShuffleCallToInst
    Prefetch
    LLVMBarrier
    LLVMVectorizer
    ModuleCleanup
)

# Dependencies of the passes
set(OCL_BE_PASSES_DEPS
    name_mangle
    OclBackendUtils
    BackendPassesWrappers
)

target_link_libraries(${TARGET}
    ${OCL_BE_PASSES}
    ${OCL_BE_PASSES_DEPS}
    ${LLVM_MODULE_LIBS}
    optEngine
)
