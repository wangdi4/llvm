####################################################################
# This file adds ITT (former GPA) definitions to your cmake project.
#

# ITT/VTune integration
add_definitions( -DUSE_ITT )

# From CMake documentation:
# If the SYSTEM option is given, the compiler will be
# told the directories are meant as system include directories on some
# platforms. Signalling this setting might achieve effects such as the compiler
# skipping warnings, or these fixed-install system files not being considered in
# dependency calculations - see compiler docs.
set(EXTERNALS_DIR ${CMAKE_SOURCE_DIR}/externals)
if (OPENCL_INTREE_BUILD)
  set(EXTERNALS_DIR ${PROJECT_SOURCE_DIR}/externals)
endif()
include_directories( SYSTEM ${EXTERNALS_DIR}/itt/include
                     ${EXTERNALS_DIR}/itt/ittnotify/ )

# ITT libraries directory suffix
if( USE_GPA )
    if (BUILD_X64)
        set(GPA_LIB_DIR_SUFFIX x64)
    else()
        set(GPA_LIB_DIR_SUFFIX x86)
    endif()

    add_library(imp_gpasdk STATIC IMPORTED)
    set_property(TARGET imp_gpasdk PROPERTY IMPORTED_LOCATION ${EXTERNALS_DIR}/gpa/libs/${GPA_LIB_DIR_SUFFIX}/gpasdk_s.lib )

    list( APPEND LINK_LIBS imp_gpasdk )
else()
    list( APPEND TARGET_SOURCES ${EXTERNALS_DIR}/itt/ittnotify/ittnotify_static.c )
endif(  )
