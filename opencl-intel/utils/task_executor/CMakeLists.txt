#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME task_executor${BUILD_PLATFORM}${OPENCL_BINARIES_POSTFIX} )

# Embed RPATH/RUNPATH for libraries which dependencies aren't in the standard system library directories
include(../../cmake_utils/embed.rpath.txt)


include( ${CMAKE_SOURCE_DIR}/cmake_utils/itt.cmake )

if (WIN32 AND NOT BUILD_X64)
    # the /Ob1 flag is set instead of /Ob2 flag in order to prevent aggressive inlining
    # optimizations in Visual Studio due to a bug discovered in device fission: CSSD100013570
    string( REPLACE /Ob2  "/Ob1" CMAKE_C_FLAGS_RELEASE   ${CMAKE_CXX_FLAGS_RELEASE} )
    string( REPLACE /Ob2  "/Ob1" CMAKE_CXX_FLAGS_RELEASE   ${CMAKE_CXX_FLAGS_RELEASE} )
endif (WIN32 AND NOT BUILD_X64)

if (WIN32)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        add_definitions( -DRESOURCE_X64 )
    endif (CMAKE_SIZEOF_VOID_P EQUAL 8) 
endif (WIN32)

link_directories( ${TBB_LIB_DIR} )

# [LLVM 3.6 UPGRADE] Disable Linux warning about unused function
# this warning occur when processing externals/itt/include/ittnotify.h
# since it contain __itt_id __itt_id_make(void*, long long unsigned int)
# function that is not used in every build target.
if (NOT WIN32)
    add_definitions( "-Wno-unused-function" )
endif(NOT WIN32)

add_definitions(-DTASK_EXECUTOR_EXPORTS
                -D__TBB_EXECUTOR__
                -D_USRDLL
                -DTBB_PREVIEW_LOCAL_OBSERVER=1
                -DTBB_PREVIEW_TASK_ARENA=1
                -D__TBB_NO_IMPLICIT_LINKAGE=1
                -D__TBB_ALLOW_MUTABLE_FUNCTORS=1)

SET_UNICODE_OFF()

# only tbb executor is supported on Linux
if (NOT WIN32)
    set( IGNORE_FILES thread_executor|xn_ )
endif(NOT WIN32)

FIND_SOURCES( ${IGNORE_FILES} )
CALCULATE_TARGET_SOURCES()

# add a target named ${TARGET_NAME}
add_opencl_library (${TARGET_NAME} SHARED
    ${TARGET_SOURCES}

    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR} # keep it here for windows
    ${CL_API_HEADERS}
    ${OCL_BINARY_DIR})

if (WIN32)
    list (APPEND LINK_LIBS delayimp.lib legacy_stdio_definitions.lib)
else (WIN32)
    if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        list (APPEND LINK_LIBS ${IMPLIB_PREFIX}tbb_debug${IMPLIB_SUFFIX})
        list (APPEND LINK_LIBS ${IMPLIB_PREFIX}tbbmalloc_debug${IMPLIB_SUFFIX})
    else ()
        list (APPEND LINK_LIBS ${IMPLIB_PREFIX}tbb${IMPLIB_SUFFIX})
        list (APPEND LINK_LIBS ${IMPLIB_PREFIX}tbbmalloc${IMPLIB_SUFFIX})
    endif (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
endif(WIN32)

target_link_libraries (${TARGET_NAME} ${LINK_LIBS} cl_logger${OPENCL_BINARIES_POSTFIX} cl_sys_utils cl_hw_utils)
set_target_properties (${TARGET_NAME} PROPERTIES FOLDER ${UTILS_FOLDER_NAME})

if (WIN32)
    get_target_property(LK_FLAGS_DEBUG ${TARGET_NAME} LINK_FLAGS_DEBUG)
    get_target_property(LK_FLAGS_RELEASE ${TARGET_NAME} LINK_FLAGS_RELEASE)
    set_target_properties (${TARGET_NAME} PROPERTIES
        LINK_FLAGS_DEBUG "${LK_FLAGS_DEBUG} tbbmalloc_debug.lib tbb_debug${TBB_BINARIES_POSTFIX}.lib /DELAYLOAD:tbbmalloc_debug.dll /DELAYLOAD:tbb_debug${TBB_BINARIES_POSTFIX}.dll"
        LINK_FLAGS_RELEASE "${LK_FLAGS_RELEASE} tbbmalloc.lib tbb${TBB_BINARIES_POSTFIX}.lib /DELAYLOAD:tbbmalloc.dll  /DELAYLOAD:tbb${TBB_BINARIES_POSTFIX}.dll")
else ()
    SET_LINUX_EXPORTS_FILE (${TARGET_NAME} task_executor_linux_exports.txt)
endif(WIN32)
