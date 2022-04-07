set(TARGET_NAME UndefExternalFuncs)

set(TARGET_INCLUDE_FILES
    UndefExternalFuncs.h
)

set(TARGET_SOURCE_FILES
    UndefExternalFuncs.cpp
)

add_opencl_library(${TARGET_NAME} STATIC
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

if (OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} intrinsics_gen)
endif()

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
