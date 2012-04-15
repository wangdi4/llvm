set( TARGET_NAME dynamic_load )

include_directories(${BACKEND_ROOT_DIR}/../cl_api)

add_library( ${TARGET_NAME} STATIC
  DynamicLib.cpp
)

