# add cl_logger 

set ( TARGET_LIB_NAME        native_logger)

message ("Building project for ${TARGET_LIB_NAME}")

set( TARGET_SOURCES )
FIND_SOURCES_EX( ${LOGGER_DIR}  )
CALCULATE_TARGET_SOURCES()

# Add static library archive
add_device_library ( ${TARGET_LIB_NAME} ${TARGET_SOURCES} )
