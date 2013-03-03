set( TARGET_NAME dynamic_load )

include_directories(${BACKEND_ROOT_DIR}/../cl_api
                    ${BACKEND_ROOT_DIR}/dynamic_lib
                    ${CMAKE_SOURCE_DIR}/utils/cl_sys_utils/export)

add_library( ${TARGET_NAME} STATIC
  BE_DynamicLib.cpp
)

link_directories(cl_sys_utils)
target_link_libraries(${TARGET_NAME} cl_sys_utils)

add_dependencies( ${TARGET_NAME} cl_sys_utils)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME} )
