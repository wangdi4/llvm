# add cl_sys_utils 

set ( TARGET_LIB_NAME        native_builtin_kernels)

message ("Building project for ${TARGET_LIB_NAME}")

set( TARGET_SOURCES )
FIND_SOURCES_EX( ${DEVICE_COMMON_DIR}/builtin_kernels )
CALCULATE_TARGET_SOURCES()

# Add static library archive
add_device_library ( ${TARGET_LIB_NAME} ${TARGET_SOURCES} )
