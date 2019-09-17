set(TARGET_NAME InstToFuncCall)

set(TARGET_INCLUDE_FILES
    InstToFuncCall.h
)

set(TARGET_SOURCE_FILES
    InstToFuncCall.cpp
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

if (OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} intrinsics_gen)
endif()

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
