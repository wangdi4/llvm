# Task Executor compilation for MIC

set ( TASK_EXECUTOR_DIR      ${UTILS_DIR}/task_executor )
set ( TARGET_LIB_NAME        native_task_executor)

message ("Building project for ${TARGET_LIB_NAME}")

# add task_executor
set( TARGET_SOURCES )
set( TASK_EXECUTOR_EXCEPTIONS thread_ )
FIND_SOURCES_EX( ${TASK_EXECUTOR_DIR}  ${TASK_EXECUTOR_EXCEPTIONS} )
CALCULATE_TARGET_SOURCES()

# Add static library archive
add_device_library ( ${TARGET_LIB_NAME} ${TARGET_SOURCES} )
