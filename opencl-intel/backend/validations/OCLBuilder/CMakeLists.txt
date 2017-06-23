set(TARGET_NAME OCLBuilder)

project(${TARGET_NAME})

cmake_minimum_required(VERSION 2.8.7)

include_directories(
  ${BACKEND_ROOT_DIR}/dynamic_lib
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend/export
  ${BACKEND_ROOT_DIR}/plugin_manager
  ${CMAKE_SOURCE_DIR}/cl_api
  ${CL_API_HEADERS}
  ${CMAKE_SOURCE_DIR}/clang_compiler
  ${CMAKE_SOURCE_DIR}/utils/cl_sys_utils/export
)

add_library( ${TARGET_NAME} STATIC
  OCLBuilder.h
  OCLBuilder.cpp
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/plugins )

install(TARGETS ${TARGET_NAME} ${TARGET_NAME}
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION bin
            ARCHIVE DESTINATION bin)
