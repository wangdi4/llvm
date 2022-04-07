set(TARGET_NAME RemoveAtExit)

set(TARGET_INCLUDE_FILES
    RemoveAtexit.h
)

set(TARGET_SOURCE_FILES
    RemoveAtexit.cpp
)

add_opencl_library(${TARGET_NAME} STATIC
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

include_directories(
  ${BACKEND_ROOT_DIR}/metadata_api
)

if (OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} intrinsics_gen)
endif()

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
