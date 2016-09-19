# remove the restriction of warning as error for this component
if (NOT WIN32)
remove_definitions( -Werror )
endif (NOT WIN32)

#include helping functions of cmake.
include (../../cmake_utils/CMakeFuncs.txt)

set( TARGET tbb_native_test_type )

include_directories( BEFORE ${OCL_SOURCE_DIR}/externals/tbb/enhancements)

add_definitions( -DTBB_PREVIEW_LOCAL_OBSERVER=1 -DTBB_PREVIEW_TASK_ARENA=1 -D__TBB_NO_IMPLICIT_LINKAGE=1)
                
FIND_SOURCES( ${IGNORE_FILES} )
CALCULATE_TARGET_SOURCES()

if (WIN32)
    set( TBB_LIB_DIR ${OCL_SOURCE_DIR}/externals/tbb/${IMPLIB_SUBDIR}/VS2010${EXPERIMENTAL_POSTFIX}/${OCL_SYS_DEPENDENT_SUBDIR} )
else (WIN32)
    set( TBB_LIB_DIR ${OCL_SOURCE_DIR}/externals/tbb/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR} )
endif (WIN32)
link_directories( ${TBB_LIB_DIR} )

add_executable( ${TARGET} ${TARGET_SOURCES} )

if (WIN32)
	# Visual Studio 2010 (assumed if not Visual Studio 2008)
	# This is a fix due to a bug in CMake, Does not add the flag /DEBUG to the linker flags in Release mode.
	# The /DEBUG flag is required in order to create stripped pdbs.	
	set_target_properties(${TARGET} PROPERTIES LINK_FLAGS_DEBUG	"${LINK_FLAGS_DEBUG} tbbmalloc_debug.lib tbb_debug${EXPERIMENTAL_POSTFIX}.lib /DELAYLOAD:tbb_debug${EXPERIMENTAL_POSTFIX}.dll")
	set_target_properties(${TARGET} PROPERTIES LINK_FLAGS_RELEASE	"${LINK_FLAGS_RELEASE} tbbmalloc.lib tbb${EXPERIMENTAL_POSTFIX}.lib /DELAYLOAD:tbb${EXPERIMENTAL_POSTFIX}.dll /DEBUG")
else (WIN32)
	if ( ${CMAKE_BUILD_TYPE} STREQUAL "Debug" )
		list(APPEND LINK_LIBS ${IMPLIB_PREFIX}tbb_debug${IMPLIB_SUFFIX} )
		if( NOT ANDROID )
		  list(APPEND LINK_LIBS ${IMPLIB_PREFIX}tbbmalloc_debug${IMPLIB_SUFFIX} )
                  set_property(TARGET ${TARGET} APPEND_STRING PROPERTY LINK_FLAGS " -pie")
		endif( NOT ANDROID )
	else ()
		list(APPEND LINK_LIBS ${IMPLIB_PREFIX}tbb${IMPLIB_SUFFIX} )
		if( NOT ANDROID )
		  list(APPEND LINK_LIBS ${IMPLIB_PREFIX}tbbmalloc${IMPLIB_SUFFIX} )
		endif( NOT ANDROID )
	endif ( ${CMAKE_BUILD_TYPE} STREQUAL "Debug" )
endif (WIN32)
link_directories( ${TBB_LIB_DIR} )

# Additional link libraries
target_link_libraries( ${TARGET} ${LINK_LIBS} )

set( CREATE_DIR_NAME tests/tbb_native_test_type)

# Copy generated executable to appropriate test directory in 'Release' area.
install(TARGETS ${TARGET} DESTINATION ${CREATE_DIR_NAME})

set_target_properties( ${TARGET} PROPERTIES FOLDER "validation/workloads" )

