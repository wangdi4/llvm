set(TARGET_NAME LocalBuffers)

set(LOCALBUFFERS_INCLUDE_FILES
    LocalBuffers.h
)

set(LOCALBUFFERS_SOURCE_FILES
    LocalBuffers.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/../cl_api
    ${BACKEND_ROOT_DIR}/ocl_cpu_backend/utils
    ${BACKEND_ROOT_DIR}/ocl_cpu_backend/passes/LocalBuffAnalysis
)

add_library(${TARGET_NAME}
    ${LOCALBUFFERS_INCLUDE_FILES}
    ${LOCALBUFFERS_SOURCE_FILES}
)

target_link_libraries(${TARGET_NAME}
    OclUtils
    LocalBuffAnalysis
)

add_dependencies(${TARGET_NAME}
    OclUtils
    LocalBuffAnalysis
)

install( TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION bin)