set(TARGET_NAME ocloptimg)

include_directories(${LLVM_INCLUDE_DIRS}
                    ${BACKEND_ROOT_DIR}/passes)
link_directories(${LLVM_LIBRARY_DIRS})

add_executable(${TARGET_NAME}
    ocl_opt_img.cpp
)

# Backend passes
set(OCL_BE_PASSES
    BuiltInFuncImport
    BuiltinCallToInst
    BuiltinLibInfo
    GenericAddressResolution
)

# Dependencies of the passes
set(OCL_BE_PASSES_DEPS
    OclBackendUtils
    metadataapi
    name_mangle
    reflection_module
)

target_link_libraries(${TARGET_NAME}
    ${OCL_BE_PASSES}
    ${OCL_BE_PASSES_DEPS}
    LLVMOptEngine
    ${LLVM_MODULE_LIBS}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME})

install( TARGETS ${TARGET_NAME} RUNTIME DESTINATION tests/LITTests)
