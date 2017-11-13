set (TARGET_NAME "FePluginMock")
include_directories(${BACKEND_ROOT_DIR}/ocl_cpu_backend/export/)
include_directories(${BACKEND_ROOT_DIR}/../cl_api)
include_directories(${BACKEND_ROOT_DIR}/plugin_manager)


add_opencl_library( ${TARGET_NAME} SHARED FeMock.cpp)

target_link_libraries(${TARGET_NAME}
  OclPluginManager
  dynamic_load
  cl_sys_utils
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_TESTS_FOLDER_NAME} )
