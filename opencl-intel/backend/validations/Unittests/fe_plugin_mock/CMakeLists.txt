set(TARGET_NAME "FePluginMock")

add_opencl_library(
  ${TARGET_NAME}
  SHARED
  FeMock.cpp
  INCLUDE_DIRS
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend/export
  ${BACKEND_ROOT_DIR}/plugin_manager
  LINK_LIBS
  OclPluginManager
  dynamic_load
  cl_sys_utils)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER
                                                ${BACKEND_TESTS_FOLDER_NAME})
