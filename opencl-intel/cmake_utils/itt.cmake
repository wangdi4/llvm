####################################################################
# This file adds ITT (former GPA) definitions to your cmake project.
#

# ITT libraries directory suffix
if( WIN32 )
    if (BUILD_X64)
        set(GPA_LIB_DIR_SUFFIX x64)
    else()
        set(GPA_LIB_DIR_SUFFIX x86)
    endif()

    add_library(imp_gpasdk STATIC IMPORTED)
    set_property(TARGET imp_gpasdk PROPERTY IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/externals/gpa/libs/${GPA_LIB_DIR_SUFFIX}/gpasdk_s.lib )

    list( APPEND LINK_LIBS imp_gpasdk )
else()
    list( APPEND TARGET_SOURCES ${CMAKE_SOURCE_DIR}/externals/itt/ittnotify/ittnotify_static.c )
endif(  )

