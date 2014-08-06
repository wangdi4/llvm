if(WIN32)
    ocl_replace_compiler_option_to_dynamic()
endif(WIN32)

set( TARGET_NAME tinyXML2 )

# add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} STATIC tinyxml2.cpp tinyxml2.h)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${UTILS_FOLDER_NAME} )

install( TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib)

