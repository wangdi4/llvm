set( TARGET_NAME Tracing )

set( TARGET_INCLUDE_FILES
     tracing_api.h
     tracing_handle.h
     tracing_notify.h
     tracing_types.h
)

set( TARGET_SOURCE_FILES
     tracing_api.cpp
)

add_library( ${TARGET_NAME} STATIC
             ${TARGET_SOURCE_FILES}
             ${TARGET_INCLUDE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${RUNTIME_FOLDER_NAME} )