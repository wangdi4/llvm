set( TARGET_NAME test_utils )

include_directories( export )

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

add_opencl_library( ${TARGET_NAME} STATIC EXCLUDE_FROM_ALL ${TARGET_SOURCES} )
