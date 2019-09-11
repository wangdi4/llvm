set( TARGET_NAME Platform )

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
)

# [LLVM 3.6 UPGRADE] Disable Linux warning about unused function
# this warning occur when processing externals/itt/include/ittnotify.h
# since it contain __itt_id __itt_id_make(void*, long long unsigned int)
# function that is not used in every build target.
if (NOT WIN32)
    add_definitions( "-Wno-unused-function" )
endif(NOT WIN32)

include_directories(${CCLANG_DEV_INCLUDE_DIRS})

add_library (${TARGET_NAME} STATIC 
             ${TARGET_SOURCE_FILES}
             ${TARGET_INCLUDE_FILES})

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${RUNTIME_FOLDER_NAME} )
