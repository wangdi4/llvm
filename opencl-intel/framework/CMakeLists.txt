if( NOT DEFINED LLVM_PATH_FE )
    message( FATAL_ERROR "LLVM_PATH_FE is not specified. Please specify LLVM library location for backend using LLVM_PATH_FE parameter to CMAKE" )
endif()
set(LLVM_PATH ${LLVM_PATH_FE})
find_package(LLVM REQUIRED)

if (NOT WIN32)
  # disable warnings inherited from external componennts
  add_definitions(-Wno-attributes)
  add_definitions(-Wno-missing-braces)
endif (NOT WIN32)


# Embed RPATH/RUNPATH for libraries which dependencies aren't in the standard system libarary directories
include(../cmake_utils/embed.rpath.txt)

# this component has to use deprecated OpenCL API functions.
if (WIN32)
    add_definitions(
        -wd4996 # Suppress 'function': was declared deprecated'
    )
endif (WIN32)

include_directories ( BEFORE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/export
        ${CL_API_HEADERS}
        ${CMAKE_CURRENT_SOURCE_DIR}/icd
        ${CMAKE_CURRENT_SOURCE_DIR}/gl_sharing
        ${CMAKE_CURRENT_SOURCE_DIR}/Platform
        ${CMAKE_CURRENT_SOURCE_DIR}/Context
        ${CMAKE_CURRENT_SOURCE_DIR}/Execution
        ${CMAKE_CURRENT_SOURCE_DIR}/MemoryAllocator
        ${OCL_BINARY_DIR}
    )

if (WIN32 AND (DEFINED ENV{DXSDK_DIR}) )	
	include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/d3d9_sharing)

	if (${BUILD_X64} STREQUAL "ON")
		link_directories($ENV{DXSDK_DIR}/Lib/x64)
	else (${BUILD_X64} STREQUAL "ON")
		link_directories($ENV{DXSDK_DIR}/Lib/x86)
	endif(${BUILD_X64} STREQUAL "ON")

	if (WIN32 AND DEFINED ENV{DXSDK_DIR} )
		add_definitions (-DDX_MEDIA_SHARING)
		add_definitions (-DCL_DX9_MEDIA_SHARING_INTEL_EXT)
	endif ()
endif ()

# For debug only
if (USE_ITT_INTERNAL)
	add_definitions (-DUSE_ITT_INTERNAL)
endif()

add_subdirectory( icd )

# these sub-directories must be added here in order to recieve the ITT definitions
add_subdirectory( Platform )
add_subdirectory( Context )
add_subdirectory( Execution )
add_subdirectory( MemoryAllocator )

if (WIN32)
    add_subdirectory( gl_sharing )
    list( APPEND LINK_LIBS gl_sharing )
	if ( DEFINED ENV{DXSDK_DIR} )
		add_subdirectory( d3d9_sharing )
		list( APPEND LINK_LIBS d3d9_sharing )
		endif ( DEFINED ENV{DXSDK_DIR} )
else()
    message( WARNING " Warning: OpenGL Sharing is not ported to Linux!")
endif(WIN32)

# set that name of the main output file as a target name
set( TARGET_NAME intelocl${BUILD_PLATFORM} )

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

set (CONFIG_FILES cl.cfg)

if (INCLUDE_MIC_DEVICE)
  add_definitions( -DINCLUDE_MIC_DEVICE )
endif (INCLUDE_MIC_DEVICE)

if (INCLUDE_ISP_DEVICE)
  add_definitions( -DINCLUDE_ISP_DEVICE )
endif (INCLUDE_ISP_DEVICE)

if (WIN32)
    list( APPEND CONFIG_FILES register_vendor.reg unregister_vendor.reg)

    source_group( "API" FILES cl_framework.cpp cl_framework.h framework_proxy.cpp framework_proxy.h)
    
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        add_definitions( -DRESOURCE_X64 )
    endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

    source_group( "Resource Files" FILES ${CONFIG_FILES} )
    list( APPEND TARGET_SOURCES ${CONFIG_FILES} )

    list( APPEND LINK_LIBS PowrProf.lib )
endif (WIN32)

if (DEFINED CMAKE_IMPORT_LIBRARY_PREFIX OR DEFINED CMAKE_IMPORT_LIBRARY_SUFFIX)
    set( IMPORT_LIB ${CMAKE_IMPORT_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_IMPORT_LIBRARY_SUFFIX} )
endif (DEFINED CMAKE_IMPORT_LIBRARY_PREFIX OR DEFINED CMAKE_IMPORT_LIBRARY_SUFFIX)

# [LLVM 3.6 UPGRADE] Disable Linux warning about unused function
# this warning occur when processing externals/itt/include/ittnotify.h
# since it contain __itt_id __itt_id_make(void*, long long unsigned int)
# function that is not used in every build target.
# Disable warning about converting null pointer to long int.
if (NOT WIN32)
    add_definitions( "-Wno-unused-function" )
endif(NOT WIN32)

if (WIN32)
    if (${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2008
        set( TBB_LIB_DIR ${OCL_SOURCE_DIR}/externals/tbb/${IMPLIB_SUBDIR}/VS2008/${OCL_SYS_DEPENDENT_SUBDIR} )
    else (${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2010
        set( TBB_LIB_DIR ${OCL_SOURCE_DIR}/externals/tbb/${IMPLIB_SUBDIR}/VS2010/${OCL_SYS_DEPENDENT_SUBDIR} )
    endif (${MSVC_VERSION} EQUAL 1500)
else (WIN32)
    set( TBB_LIB_DIR ${OCL_SOURCE_DIR}/externals/tbb/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR} )
endif (WIN32)

link_directories( ${TBB_LIB_DIR} )

include( ../cmake_utils/itt.cmake )

# add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} SHARED ${TARGET_SOURCES})

install(TARGETS ${TARGET_NAME} 
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION bin
		ARCHIVE DESTINATION lib
		)

install(FILES ${CONFIG_FILES} DESTINATION bin )

if (WIN32)
    get_target_property(RT_OUTPUT_DIRECTORY ${TARGET_NAME} RUNTIME_OUTPUT_DIRECTORY)
	file(TO_NATIVE_PATH ${RT_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/${TARGET_NAME}_stripped.pdb PDB_NAME)	
    if (${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2008	
	    set_target_properties(${TARGET_NAME} PROPERTIES
		    LINK_FLAGS_RELEASE	"${LINK_FLAGS_RELEASE}"
		    LINK_FLAGS_DEBUG	"${LINK_FLAGS_DEBUG} /NODEFAULTLIB:MSVCRT"
		    LINK_FLAGS	"${LINK_FLAGS} /PDBSTRIPPED:${PDB_NAME}"
            )
    else(${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2010 (assumed if not Visual Studio 2008)
        # This is a fix due to a bug in CMake, Does not add the flag /DEBUG to the linker flags in Release mode.
        # The /DEBUG flag is required in order to create stripped pdbs.
        set_target_properties(${TARGET_NAME} PROPERTIES
		    LINK_FLAGS_RELEASE	"${LINK_FLAGS_RELEASE} /DEBUG /PDBSTRIPPED:${PDB_NAME}"
		    LINK_FLAGS_DEBUG	"${LINK_FLAGS_DEBUG} /NODEFAULTLIB:MSVCRT /PDBSTRIPPED:${PDB_NAME}"
            )
    endif(${MSVC_VERSION} EQUAL 1500)
	install(FILES ${RT_OUTPUT_DIRECTORY}/\${BUILD_TYPE}/${TARGET_NAME}_stripped.pdb DESTINATION bin)
    if (INSTALL_PDBS)    
        install(FILES ${RT_OUTPUT_DIRECTORY}/\${BUILD_TYPE}/${TARGET_NAME}.pdb DESTINATION bin)
    endif(INSTALL_PDBS)   
    
	list( APPEND LINK_LIBS delayimp.lib )
else (WIN32)
    SET_LINUX_EXPORTS_FILE( ${TARGET_NAME} cl_framework_linux_exports.txt )
endif (WIN32)


target_link_libraries( ${TARGET_NAME} Context
                                      Execution
                                      Platform
									  MemoryAllocator
                                      cl_logger
                                      cl_sys_utils
                                      task_executor${BUILD_PLATFORM}
                                      CLElfLib
                                      ${LINK_LIBS} )

set_target_properties( ${TARGET_NAME}  PROPERTIES FOLDER ${RUNTIME_FOLDER_NAME} )
