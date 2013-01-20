set(TARGET_NAME oclopt)

include_directories(${CMAKE_SOURCE_DIR}/backend/passes)

link_directories(${LLVM_LIBRARY_DIR})

add_executable(${TARGET_NAME}
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

target_link_libraries(${TARGET_NAME}
    ${OCL_BE_PASSES}
    ${OCL_BE_PASSES_DEPS}
    optEngine
    ${LLVM_MODULE_LIBS}
)

if( NOT WIN32)
    target_link_libraries(${TARGET_NAME}
#        pthread
        dl
    )
endif ()

install( TARGETS ${TARGET_NAME} RUNTIME DESTINATION bin)

