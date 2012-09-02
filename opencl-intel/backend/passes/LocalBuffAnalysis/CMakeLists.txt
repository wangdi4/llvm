set(TARGET_NAME LocalBuffAnalysis)

set(LOCALBUFFANALYSIS_INCLUDE_FILES
    LocalBuffAnalysis.h
)

set(LOCALBUFFANALYSIS_SOURCE_FILES
    LocalBuffAnalysis.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/../cl_api
    ${BACKEND_ROOT_DIR}/utils
)

add_library(${TARGET_NAME}
    ${LOCALBUFFANALYSIS_INCLUDE_FILES}
    ${LOCALBUFFANALYSIS_SOURCE_FILES}
)
