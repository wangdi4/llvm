#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME task_executor${BUILD_PLATFORM} )

# Embed RPATH/RUNPATH for libraries which dependencies aren't in the standard system library directories
include(../../cmake_utils/embed.rpath.txt)


include( ${CMAKE_SOURCE_DIR}/cmake_utils/itt.cmake )

if (WIN32 AND NOT BUILD_X64)
    # the /Ob1 flag is set instead of /Ob2 flag in order to prevent aggressive inlining
    # optimizations in Visual Studio due to a bug discovered in device fission: CSSD100013570
    string( REPLACE /Ob2  "/Ob1" CMAKE_C_FLAGS_RELEASE   ${CMAKE_CXX_FLAGS_RELEASE} )
    string( REPLACE /Ob2  "/Ob1" CMAKE_CXX_FLAGS_RELEASE   ${CMAKE_CXX_FLAGS_RELEASE} )
endif (WIN32 AND NOT BUILD_X64)

include_directories( BEFORE 
        .
        ${CL_API_HEADERS}
        ${OCL_BINARY_DIR}
 )

if (WIN32)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        add_definitions( -DRESOURCE_X64 )
    endif (CMAKE_SIZEOF_VOID_P EQUAL 8) 
    
    if (${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2008
        set( TBB_LIB_DIR ${OCL_SOURCE_DIR}/externals/tbb/${IMPLIB_SUBDIR}/VS2008/${OCL_SYS_DEPENDENT_SUBDIR} )
    else (${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2010
        set( TBB_LIB_DIR ${OCL_SOURCE_DIR}/externals/tbb/${IMPLIB_SUBDIR}/VS2010${EXPERIMENTAL_POSTFIX}/${OCL_SYS_DEPENDENT_SUBDIR} )
    endif (${MSVC_VERSION} EQUAL 1500)
else (WIN32)
    set( TBB_LIB_DIR ${OCL_SOURCE_DIR}/externals/tbb/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR} )
endif (WIN32)

link_directories( ${TBB_LIB_DIR} )

# [LLVM 3.6 UPGRADE] Disable Linux warning about unused function
# this warning occur when processing externals/itt/include/ittnotify.h
# since it contain __itt_id __itt_id_make(void*, long long unsigned int)
# function that is not used in every build target.
if (NOT WIN32)
    add_definitions( "-Wno-unused-function" )
endif(NOT WIN32)

add_definitions( -DTASK_EXECUTOR_EXPORTS -D__TBB_EXECUTOR__ -D_USRDLL -DTBB_PREVIEW_LOCAL_OBSERVER=1 -DTBB_PREVIEW_TASK_ARENA=1 -D__TBB_NO_IMPLICIT_LINKAGE=1)

SET_UNICODE_OFF()


# only tbb executor is supported on Linux
if (NOT WIN32)
    set( IGNORE_FILES thread_executor|xn_ )
endif(NOT WIN32)

FIND_SOURCES( ${IGNORE_FILES} )
CALCULATE_TARGET_SOURCES()

# add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} SHARED 
        ${TARGET_SOURCES})

install(TARGETS ${TARGET_NAME} 
		RUNTIME DESTINATION bin
		ARCHIVE DESTINATION lib
		LIBRARY DESTINATION bin)

if (WIN32)
	list(APPEND LINK_LIBS delayimp.lib )
else (WIN32)
    SET_LINUX_EXPORTS_FILE( ${TARGET_NAME} task_executor_linux_exports.txt )
endif(WIN32)

if (WIN32)
    get_target_property(RT_OUTPUT_DIRECTORY ${TARGET_NAME} RUNTIME_OUTPUT_DIRECTORY)
    file(TO_NATIVE_PATH ${RT_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/${TARGET_NAME}${EXPERIMENTAL_POSTFIX}_stripped.pdb PDB_NAME)
    if (${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2008
        set_target_properties(${TARGET_NAME} PROPERTIES
		    LINK_FLAGS_RELEASE	"${LINK_FLAGS_RELEASE} /DELAYLOAD:tbb${EXPERIMENTAL_POSTFIX}.dll"
		    LINK_FLAGS_DEBUG	"${LINK_FLAGS_DEBUG} /DELAYLOAD:tbb_debug${EXPERIMENTAL_POSTFIX}.dll"
	    )        
	    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS	"${LINK_FLAGS} /PDBSTRIPPED:${PDB_NAME}")	    
    else (${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2010 (assumed if not Visual Studio 2008)
        # This is a fix due to a bug in CMake, Does not add the flag /DEBUG to the linker flags in Release mode.
        # The /DEBUG flag is required in order to create stripped pdbs.	
	    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG	"${LINK_FLAGS_DEBUG} tbbmalloc_debug.lib tbb_debug${EXPERIMENTAL_POSTFIX}.lib /DELAYLOAD:tbbmalloc_debug.dll /DELAYLOAD:tbb_debug${EXPERIMENTAL_POSTFIX}.dll /PDBSTRIPPED:${PDB_NAME}")
	    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE	"${LINK_FLAGS_RELEASE} tbbmalloc.lib tbb${EXPERIMENTAL_POSTFIX}.lib /DELAYLOAD:tbbmalloc.dll  /DELAYLOAD:tbb${EXPERIMENTAL_POSTFIX}.dll /DEBUG /PDBSTRIPPED:${PDB_NAME}")
    endif (${MSVC_VERSION} EQUAL 1500)
	install(FILES ${RT_OUTPUT_DIRECTORY}/\${BUILD_TYPE}/${TARGET_NAME}${EXPERIMENTAL_POSTFIX}_stripped.pdb DESTINATION bin)
    if (INSTALL_PDBS)
        install(FILES ${RT_OUTPUT_DIRECTORY}/\${BUILD_TYPE}/${TARGET_NAME}${EXPERIMENTAL_POSTFIX}.pdb DESTINATION bin)
    endif(INSTALL_PDBS)
else (WIN32)
	if ( ${CMAKE_BUILD_TYPE} STREQUAL "Debug" )
		list(APPEND LINK_LIBS ${IMPLIB_PREFIX}tbb_debug${IMPLIB_SUFFIX} )
		if( NOT ANDROID )
		  list(APPEND LINK_LIBS ${IMPLIB_PREFIX}tbbmalloc_debug${IMPLIB_SUFFIX} )
		endif( NOT ANDROID )
	else ()
		list(APPEND LINK_LIBS ${IMPLIB_PREFIX}tbb${IMPLIB_SUFFIX} )
		if( NOT ANDROID )
		  list(APPEND LINK_LIBS ${IMPLIB_PREFIX}tbbmalloc${IMPLIB_SUFFIX} )
		endif( NOT ANDROID )
	endif ( ${CMAKE_BUILD_TYPE} STREQUAL "Debug" )
endif (WIN32)

target_link_libraries( ${TARGET_NAME} ${LINK_LIBS} cl_logger cl_sys_utils cl_hw_utils )
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${UTILS_FOLDER_NAME} )

if ( BUILD_EXPERIMENTAL_21 )
    set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME "${TARGET_NAME}${EXPERIMENTAL_POSTFIX}")
endif ( BUILD_EXPERIMENTAL_21 )
