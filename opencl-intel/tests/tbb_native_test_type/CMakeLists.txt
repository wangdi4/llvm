# remove the restriction of warning as error for this component
if (NOT WIN32)
remove_definitions( -Werror )
endif (NOT WIN32)

set( TARGET tbb_native_test_type )

include_directories( BEFORE ${OCL_SOURCE_DIR}/externals/tbb/enhancements)

add_definitions( -DTBB_PREVIEW_LOCAL_OBSERVER=1 -DTBB_PREVIEW_TASK_ARENA=1 -D__TBB_NO_IMPLICIT_LINKAGE=1)

FIND_SOURCES( ${IGNORE_FILES} )
CALCULATE_TARGET_SOURCES()

link_directories( ${TBB_LIB_DIR} )

add_executable( ${TARGET} ${TARGET_SOURCES} )

set_target_properties(${TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OCL_BINARY_DIR}/tests/tbb_native_test_type
    LIBRARY_OUTPUT_DIRECTORY ${OCL_BINARY_DIR}/tests/tbb_native_test_type
    ARCHIVE_OUTPUT_DIRECTORY ${OCL_BINARY_DIR}/tests/tbb_native_test_type)

if (WIN32)
    # This is a fix due to a bug in CMake, Does not add the flag /DEBUG to the linker flags in Release mode.
    # The /DEBUG flag is required in order to create stripped pdbs.
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set_target_properties(${TARGET} PROPERTIES LINK_FLAGS_DEBUG	"${LINK_FLAGS_DEBUG} ocltbbmalloc64_debug.lib ocltbb64_debug${TBB_BINARIES_POSTFIX}.lib /DELAYLOAD:ocltbb64_debug${TBB_BINARIES_POSTFIX}.dll")
        set_target_properties(${TARGET} PROPERTIES LINK_FLAGS_RELEASE	"${LINK_FLAGS_RELEASE} ocltbbmalloc64.lib ocltbb64${TBB_BINARIES_POSTFIX}.lib /DELAYLOAD:ocltbb64${TBB_BINARIES_POSTFIX}.dll /DEBUG")
    else(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set_target_properties(${TARGET} PROPERTIES LINK_FLAGS_DEBUG	"${LINK_FLAGS_DEBUG} ocltbbmalloc32_debug.lib ocltbb32_debug${TBB_BINARIES_POSTFIX}.lib /DELAYLOAD:ocltbb32_debug${TBB_BINARIES_POSTFIX}.dll")
        set_target_properties(${TARGET} PROPERTIES LINK_FLAGS_RELEASE	"${LINK_FLAGS_RELEASE} ocltbbmalloc32.lib ocltbb32${TBB_BINARIES_POSTFIX}.lib /DELAYLOAD:ocltbb32${TBB_BINARIES_POSTFIX}.dll /DEBUG")
    endif (CMAKE_SIZEOF_VOID_P EQUAL 8)
    list(APPEND LINK_LIBS delayimp.lib)
else (WIN32)
    if ( ${CMAKE_BUILD_TYPE} STREQUAL "Debug" )
        list(APPEND LINK_LIBS ${IMPLIB_PREFIX}tbb_debug${IMPLIB_SUFFIX} )
        list(APPEND LINK_LIBS ${IMPLIB_PREFIX}tbbmalloc_debug${IMPLIB_SUFFIX} )
        set_property(TARGET ${TARGET} APPEND_STRING PROPERTY LINK_FLAGS " -pie")
    else ()
        list(APPEND LINK_LIBS ${IMPLIB_PREFIX}tbb${IMPLIB_SUFFIX} )
        list(APPEND LINK_LIBS ${IMPLIB_PREFIX}tbbmalloc${IMPLIB_SUFFIX} )
    endif ( ${CMAKE_BUILD_TYPE} STREQUAL "Debug" )
endif (WIN32)
link_directories( ${TBB_LIB_DIR} )

# Additional link libraries
target_link_libraries( ${TARGET} ${LINK_LIBS} )

set_target_properties( ${TARGET} PROPERTIES FOLDER "validation" )

