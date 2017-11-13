set(TARGET_NAME OCLBuilder)

project(${TARGET_NAME})

include_directories(
  ${BACKEND_ROOT_DIR}/dynamic_lib
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend/export
  ${BACKEND_ROOT_DIR}/plugin_manager
  ${CMAKE_SOURCE_DIR}/cl_api
  ${CL_API_HEADERS}
  ${CMAKE_SOURCE_DIR}/FrontendDriver
  ${CMAKE_SOURCE_DIR}/utils/cl_sys_utils/export
)

add_opencl_library( ${TARGET_NAME} STATIC
  OCLBuilder.h
  OCLBuilder.cpp
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/plugins )
