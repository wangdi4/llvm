set(TARGET_NAME LocalBuffersAnalysis)

set(LOCALBUFFERSANALYSIS_INCLUDE_FILES
    LocalBuffersAnalysis.h
)

set(LOCALBUFFERSANALYSIS_SOURCE_FILES
    LocalBuffersAnalysis.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/../cl_api
    ${BACKEND_ROOT_DIR}/ocl_cpu_backend/ocl_cpu_utils
)

add_library(${TARGET_NAME}
    ${LOCALBUFFERSANALYSIS_INCLUDE_FILES}
    ${LOCALBUFFERSANALYSIS_SOURCE_FILES}
)

target_link_libraries(${TARGET_NAME}
    OclUtils
)

add_dependencies(${TARGET_NAME}
    OclUtils
)

install( TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION bin)