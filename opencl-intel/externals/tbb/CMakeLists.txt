
GEN_RULES_FOR_COPY_SOURCE_BIN_FILES( "Prebuilt TBB DLLs" )

add_custom_target( tbb_dlls_copy ALL
                   DEPENDS ${COPIED_FILES}
                   COMMENT "Copy TBB DLLs"
                   VERBATIM )

set_target_properties( tbb_dlls_copy PROPERTIES FOLDER ${EXTERNAL_FOLDER_NAME} )

