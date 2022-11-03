set(TARGET_NAME test_utils)

include_directories(export ${GTEST_INCLUDE_DIR} ${CL_API_HEADERS})

find_sources()
calculate_target_sources()

add_opencl_library(${TARGET_NAME} STATIC EXCLUDE_FROM_ALL ${TARGET_SOURCES})
