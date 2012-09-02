set(TARGET_NAME x86_plugin)

set(ADDIMPLICITARGS_SOURCE_FILES  
    x86_plugin.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/optimizer
)

add_library(${TARGET_NAME} SHARED 
    ${ADDIMPLICITARGS_SOURCE_FILES} 
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
    )

target_link_libraries(${TARGET_NAME}
    OclOptimizer
    OclUtils
    # Backend passes
    ${PASSES_LIBS}
)

add_dependencies(${TARGET_NAME}
    OclOptimizer
    OclUtils
    # Backend passes
    ${PASSES_LIBS}
)

install( TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION bin)