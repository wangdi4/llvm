set(TARGET_NAME OclOptimizer)

set(TARGET_INCLUDE_FILES
    Optimizer.h
)

set(TARGET_SOURCE_FILES
    Optimizer.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/ocl_cpu_backend
    ${BACKEND_ROOT_DIR}/utils
    ${BACKEND_ROOT_DIR}/vectorizer/Utils
    ${BACKEND_ROOT_DIR}/../cl_api
    ${BACKEND_ROOT_DIR}/metadata_api
    ${BACKEND_ROOT_DIR}/metadata_api/gen
)

if (APPLE)
include_directories(
    ${BACKEND_ROOT_DIR}/arch_headers/Apple
)
else (APPLE)
include_directories(
    ${BACKEND_ROOT_DIR}/arch_headers
    ${BACKEND_ROOT_DIR}/passes/PrintIRPass
)
endif (APPLE)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME} )

add_dependencies(${TARGET_NAME} metadataapi)

if(APPLE)
  file(RELATIVE_PATH SOURCE_RELATIVE_DIR ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
  install( FILES ${TARGET_INCLUDE_FILES} ${TARGET_SOURCE_FILES}
           DESTINATION src/${SOURCE_RELATIVE_DIR}
         )
endif(APPLE)
