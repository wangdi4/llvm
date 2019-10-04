set(TARGET_NAME OCLAliasAnalysis)

set(TARGET_INCLUDE_FILES
    OCLAliasAnalysis.h
)

set(TARGET_SOURCE_FILES
    OCLAliasAnalysis.cpp
)

include_directories(
    ${CMAKE_BINARY_DIR}/include
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

if (OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} intrinsics_gen)
endif()

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
