set(TARGET_NAME ocloptimg)

include_directories(${CMAKE_SOURCE_DIR}/backend/passes)

link_directories(${LLVM_LIBRARY_DIR})

add_executable(${TARGET_NAME}
    ocl_opt_img.cpp
)

# Backend passes
set(OCL_BE_PASSES
    BuiltInFuncImport
    ShuffleCallToInst
)

# Dependencies of the passes
set(OCL_BE_PASSES_DEPS
    name_mangle
    OclBackendUtils
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

if( CMAKE_CROSSCOMPILING )
    add_custom_target( ocloptimg_native ALL
      COMMAND make ocloptimg
      DEPENDS ConfigureNativeTargets
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/native
      COMMENT "Cross-Compile: Building Native ocl_opt_img tool"
    )
endif( CMAKE_CROSSCOMPILING )

install( TARGETS ${TARGET_NAME} RUNTIME DESTINATION bin)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER "Backend" )
