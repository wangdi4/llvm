set(TARGET_NAME OCLBuilder)

project(${TARGET_NAME})

cmake_minimum_required(VERSION 2.8.4)

include_directories(
  ${BACKEND_ROOT_DIR}/dynamic_lib
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend/export
  ${BACKEND_ROOT_DIR}/plugin_manager
  ${BACKEND_ROOT_DIR}/../cl_api
  ${BACKEND_ROOT_DIR}/../fe_compilers/clang_compiler
)

add_library( ${TARGET_NAME} STATIC
  OCLBuilder.h
  OCLBuilder.cpp
)

add_dependencies( ${TARGET_NAME} dynamic_load )

link_directories(${CMAKE_CURRENT_BINARY_DIR}/../lib)

target_link_libraries(${TARGET_NAME}
  LLVMBitReader
  LLVMAsmParser
  dynamic_load)

install(TARGETS ${TARGET_NAME} ${TARGET_NAME}
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION bin
            ARCHIVE DESTINATION bin)
