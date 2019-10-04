set(TARGET_NAME OclFunctionAttrs)

set(TARGET_INCLUDE_FILES
    OclFunctionAttrs.h
)

set(TARGET_SOURCE_FILES
    OclFunctionAttrs.cpp
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

if (OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} intrinsics_gen)
endif()

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
