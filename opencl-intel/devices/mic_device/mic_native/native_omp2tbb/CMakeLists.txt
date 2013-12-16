# add cl_sys_utils 

set ( TARGET_LIB_NAME        native_omp2tbb)

message ("Building project for ${TARGET_LIB_NAME}")

set( TARGET_SOURCES )
FIND_SOURCES_EX( ${DEVICE_COMMON_DIR}/omp2tbb )
CALCULATE_TARGET_SOURCES()

list(APPEND TARGET_SOURCES ${DEVICE_COMMON_DIR}/omp2tbb/z_Linux_asm.s)

# Add static library archive
add_device_library ( ${TARGET_LIB_NAME} ${TARGET_SOURCES} )
