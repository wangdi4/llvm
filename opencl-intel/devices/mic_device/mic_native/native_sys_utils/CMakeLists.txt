# add cl_sys_utils 

set ( TARGET_LIB_NAME        native_sys_utils)

message ("Building project for ${TARGET_LIB_NAME}")

set( TARGET_SOURCES )
set( SYS_UTILS_EXCEPTIONS NumericListParser|oclNotifier|cl_socket|oclRetainers )
FIND_SOURCES_EX( ${SYS_UTILS_DIR}  ${SYS_UTILS_EXCEPTIONS} )
CALCULATE_TARGET_SOURCES()

# Add static library archive
add_device_library ( ${TARGET_LIB_NAME} ${TARGET_SOURCES} )
