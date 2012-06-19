####################################################################
# This file adds ITT (former GPA) definitions to your cmake project.
#

# ITT libraries directory suffix
if(WIN32)
    add_definitions( -DUSE_GPA )

    include_directories( ${CMAKE_SOURCE_DIR}/externals/gpa/include )
    if (BUILD_X64)
        set(GPA_LIB_DIR_SUFFIX x64)
    else (BUILD_X64)
        set(GPA_LIB_DIR_SUFFIX x86)
    endif (BUILD_X64)
    
    add_library(imp_gpasdk STATIC IMPORTED)
    set_property(TARGET imp_gpasdk PROPERTY IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/externals/gpa/libs/${GPA_LIB_DIR_SUFFIX}/gpasdk_s.lib )

    list( APPEND LINK_LIBS imp_gpasdk )

else()
    add_definitions( -DUSE_ITT )

    include_directories( ${CMAKE_SOURCE_DIR}/externals/itt/include )
    if (OCL_BUILD32)
        set(ITT_LIB_DIR_SUFFIX lin32)
    else (OCL_BUILD32)
        set(ITT_LIB_DIR_SUFFIX lin64)
    endif (OCL_BUILD32)

    add_library(imp_ittnotify STATIC IMPORTED)
    set_property(TARGET imp_ittnotify PROPERTY IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/externals/itt/libs/${ITT_LIB_DIR_SUFFIX}/libittnotify.a )

    list( APPEND LINK_LIBS imp_ittnotify )

endif(WIN32)


#link_directories( ${CMAKE_SOURCE_DIR}/externals/itt/libs/${ITT_LIB_DIR_SUFFIX} )

