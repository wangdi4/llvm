set(TARGET_NAME OclPluginManager)

# ITT/VTune integration
add_definitions( -DUSE_ITT )

if (NOT WIN32)
# The C flag is only used for compiling ittnotify_static.c file
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-implicit-fallthrough -Wno-self-assign")
endif(NOT WIN32)

add_opencl_library( ${TARGET_NAME} STATIC
  plugin_manager.h
  plugin_manager.cpp
  source_file.h
  source_file.cpp
  compile_data.h
  compile_data.cpp
  ${OCL_SOURCE_DIR}/externals/itt/ittnotify/ittnotify_static.c

  INCLUDE_DIRS
  ${LLVM_INCLUDE_DIRS}
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend/export
  ${BACKEND_ROOT_DIR}/dynamic_lib
  ${CL_API_HEADERS}
  ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export
  ${OCL_SOURCE_DIR}/utils/cl_hw_utils/export
  ${OCL_SOURCE_DIR}/externals/itt/include
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/plugins )
