set( TARGET_NAME dynamic_load )

include_directories(${BACKEND_ROOT_DIR}/../cl_api
                    ${BACKEND_ROOT_DIR}/dynamic_lib
                    ${CMAKE_SOURCE_DIR}/utils/cl_sys_utils/export)

add_library( ${TARGET_NAME} STATIC
  BE_DynamicLib.cpp
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME} )
