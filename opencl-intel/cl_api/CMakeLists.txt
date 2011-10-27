
file(GLOB EXTERNAL_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CL/*.h*)

GEN_RULES_FOR_COPY_SOURCE_FILES_WITH_SUBDIR( ${OCL_INSTALL_DIR} ${EXTERNAL_HEADERS} )

add_custom_target( external_headers_copy ALL
                   DEPENDS ${EXTERNAL_HEADERS} ${COPIED_FILES}
                   COMMENT "Copy Khronos OpenCL Headers" )

set_target_properties( external_headers_copy PROPERTIES FOLDER ${API_FOLDER_NAME} )

