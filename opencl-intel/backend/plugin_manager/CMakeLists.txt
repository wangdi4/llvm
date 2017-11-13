set(TARGET_NAME OclPluginManager)

add_opencl_library( ${TARGET_NAME} STATIC
  plugin_manager.h
  plugin_manager.cpp
  source_file.h
  source_file.cpp
  compile_data.h
  compile_data.cpp

  INCLUDE_DIRS
  ${LLVM_INCLUDE_DIRS}
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend/export
  ${BACKEND_ROOT_DIR}/dynamic_lib
  ${BACKEND_ROOT_DIR}/../cl_api
  ${CL_API_HEADERS}
  ${CMAKE_SOURCE_DIR}/utils/cl_sys_utils/export
  ${CMAKE_SOURCE_DIR}/utils/cl_hw_utils/export
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/plugins )
