set (TARGET_NAME "FePluginMock")
include_directories(${BACKEND_ROOT_DIR}/ocl_cpu_backend/export/)
include_directories(${BACKEND_ROOT_DIR}/../cl_api)
include_directories(${BACKEND_ROOT_DIR}/plugin_manager)


add_library( ${TARGET_NAME} SHARED FeMock.cpp)

target_link_libraries(${TARGET_NAME}
  OclPluginManager
)

install (TARGETS ${TARGET_NAME} ${TARGET_NAME}
  RUNTIME DESTINATION bin
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION bin
)
