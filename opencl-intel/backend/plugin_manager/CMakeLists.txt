set(TARGET_NAME OclPluginManager)

project(${TARGET_NAME})


cmake_minimum_required(VERSION 2.8.4)

include_directories(
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend/export
  ${BACKEND_ROOT_DIR}/dynamic_lib
  ${BACKEND_ROOT_DIR}/../cl_api
)

add_library( ${TARGET_NAME} STATIC
  plugin_manager.h
  plugin_manager.cpp
  source_file.h
  source_file.cpp
  compile_data.h
  compile_data.cpp
)

add_dependencies( ${TARGET_NAME} dynamic_load )

link_directories(${CMAKE_CURRENT_BINARY_DIR}/../lib)

target_link_libraries(${TARGET_NAME} dynamic_load)

install(TARGETS ${TARGET_NAME} ${TARGET_NAME}
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION bin
            ARCHIVE DESTINATION bin)
