set(TARGET_NAME SubGroupAdaptation)

set(TARGET_INCLUDE_FILES
    SubGroupAdaptation.h
)

set(TARGET_SOURCE_FILES
    SubGroupAdaptation.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/passes
    ${BACKEND_ROOT_DIR}/utils
    ${BACKEND_ROOT_DIR}/name_mangling
    ${BACKEND_ROOT_DIR}/../cl_api
    ${BACKEND_ROOT_DIR}/reflection_module
    ${CMAKE_BINARY_DIR}/include
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})