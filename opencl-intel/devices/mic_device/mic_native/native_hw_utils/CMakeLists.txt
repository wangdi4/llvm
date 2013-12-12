# add cl_hw_utils

set ( TARGET_LIB_NAME        native_hw_utils)

set( TARGET_SOURCES )
FIND_SOURCES_EX( ${HW_UTILS_DIR}/linuxImp )
CALCULATE_TARGET_SOURCES()
set( HW_UTILS_TARGET_SOURCES ${TARGET_SOURCES} )

message ("Building project for ${TARGET_LIB_NAME}")

# Add static library archive
add_device_library ( ${TARGET_LIB_NAME} ${TARGET_SOURCES} )
