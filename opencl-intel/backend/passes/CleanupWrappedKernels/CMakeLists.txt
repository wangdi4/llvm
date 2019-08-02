set(TARGET_NAME CleanupWrappedKernels)

include_directories(
    ${BACKEND_ROOT_DIR}/metadata_api
)

set(TARGET_SOURCE_FILES
  CleanupWrappedKernels.cpp
)

add_library(${TARGET_NAME}
    ${TARGET_SOURCE_FILES}
)

if (OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} intrinsics_gen)
endif()

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
