set(TARGET_NAME PrintIRPass)

set(TARGET_INCLUDE_FILES
    PrintIRPass.h
)

set(TARGET_SOURCE_FILES
    PrintIRPass.cpp
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

if (OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} intrinsics_gen)
endif()

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
