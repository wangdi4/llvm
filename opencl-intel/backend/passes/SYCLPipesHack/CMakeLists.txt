set(TARGET_NAME SYCLPipesHack)

set(TARGET_INCLUDE_FILES
    SYCLPipesHack.h
)

set(TARGET_SOURCE_FILES
    SYCLPipesHack.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/metadata_api
    ${BACKEND_ROOT_DIR}/passes
    ${BACKEND_ROOT_DIR}/passes/BuiltinLibInfo
    ${BACKEND_ROOT_DIR}/utils
    ${OCL_SOURCE_DIR}/cl_api
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

if (OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} intrinsics_gen)
endif()

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
