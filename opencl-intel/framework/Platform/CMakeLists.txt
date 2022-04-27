set( TARGET_NAME Platform )

include(itt)

set(TARGET_INCLUDE_FILES
    Device.h
    fe_compiler.h
    iplatform.h
    observer.h
    platform_module.h
)

set(TARGET_SOURCE_FILES
    Device.cpp
    fe_compiler.cpp
    platform_module.cpp
    ${TARGET_SOURCES}
)

include_directories(${CCLANG_DEV_INCLUDE_DIRS})

add_opencl_library (${TARGET_NAME} STATIC
             ${TARGET_SOURCE_FILES}
             ${TARGET_INCLUDE_FILES})

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${RUNTIME_FOLDER_NAME} )
