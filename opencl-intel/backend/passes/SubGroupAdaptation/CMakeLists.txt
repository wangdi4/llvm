set(TARGET_NAME SubGroupAdaptation)

set(TARGET_INCLUDE_FILES
    SubGroupAdaptation.h
)

set(TARGET_SOURCE_FILES
    SubGroupAdaptation.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/name_mangling
    ${BACKEND_ROOT_DIR}/reflection_module
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
