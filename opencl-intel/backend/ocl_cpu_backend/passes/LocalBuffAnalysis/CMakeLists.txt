set(TARGET_NAME LocalBuffAnalysis)

set(LOCALBUFFANALYSIS_INCLUDE_FILES
    LocalBuffAnalysis.h
)

set(LOCALBUFFANALYSIS_SOURCE_FILES
    LocalBuffAnalysis.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/../cl_api
    ${BACKEND_ROOT_DIR}/ocl_cpu_backend/utils
)

add_library(${TARGET_NAME}
    ${LOCALBUFFANALYSIS_INCLUDE_FILES}
    ${LOCALBUFFANALYSIS_SOURCE_FILES}
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