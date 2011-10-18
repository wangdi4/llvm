include_directories ( BEFORE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/export
        ${CMAKE_CURRENT_SOURCE_DIR}/icd
        ${CMAKE_CURRENT_SOURCE_DIR}/gl_sharing
        ${CMAKE_CURRENT_SOURCE_DIR}/Platform
        ${CMAKE_CURRENT_SOURCE_DIR}/Context
        ${CMAKE_CURRENT_SOURCE_DIR}/Execution
	${CMAKE_CURRENT_SOURCE_DIR}/MemoryAllocator
    )

if (DEFINED ENV{DXSDK_DIR})
	include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/d3d9_sharing)
endif (DEFINED ENV{DXSDK_DIR})

if (DEFINED ENV{DXSDK_DIR})
	if (${BUILD_X64} STREQUAL "ON")
		link_directories($ENV{DXSDK_DIR}/Lib/x64)
	else (${BUILD_X64} STREQUAL "ON")
		link_directories($ENV{DXSDK_DIR}/Lib/x86)
	endif(${BUILD_X64} STREQUAL "ON")
add_definitions (-DDX9_SHARING)
endif (DEFINED ENV{DXSDK_DIR})

if (INCLUDE_MIC_DEVICE)
    add_definitions( -D INCLUDE_ACCELERATOR_SUPPORT )
endif(INCLUDE_MIC_DEVICE)

add_subdirectory( icd )
add_subdirectory( Platform )

if(WIN32)
	include_directories(
						${CMAKE_CURRENT_SOURCE_DIR}/../externals/gpa/include
				    )
	link_directories(
					${CMAKE_CURRENT_SOURCE_DIR}/../externals/gpa/libs/${TAL_LIB_DIR_SUFFIX}
				)
    add_definitions( -DUSE_GPA )
endif(WIN32)

# these sub-directories must be added here in order to recieve the GPA definitions
add_subdirectory( Context )
add_subdirectory( Execution )
add_subdirectory( MemoryAllocator )

if (WIN32)
    add_subdirectory( gl_sharing )
    list( APPEND LINK_LIBS gl_sharing )
	if (DEFINED ENV{DXSDK_DIR})
		add_subdirectory( d3d9_sharing )
		list( APPEND LINK_LIBS d3d9_sharing )
	endif (DEFINED ENV{DXSDK_DIR})
else()
    message( WARNING " ERROR: OpenGL Sharing is not ported to Linux!")
endif(WIN32)

# set that name of the main output file as a target name
set( TARGET_NAME intelocl )

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

set (CONFIG_FILES cl.cfg)

if (WIN32)
    list( APPEND CONFIG_FILES register_vendor.reg unregister_vendor.reg)

    source_group( "API" FILES cl_framework.cpp cl_framework.h framework_proxy.cpp framework_proxy.h)

    source_group( "Resource Files" FILES ${CONFIG_FILES} )
    list( APPEND TARGET_SOURCES ${CONFIG_FILES} )

    list( APPEND LINK_LIBS PowrProf.lib )
endif (WIN32)

if (DEFINED CMAKE_IMPORT_LIBRARY_PREFIX OR DEFINED CMAKE_IMPORT_LIBRARY_SUFFIX)
    set( IMPORT_LIB ${CMAKE_IMPORT_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_IMPORT_LIBRARY_SUFFIX} )
endif (DEFINED CMAKE_IMPORT_LIBRARY_PREFIX OR DEFINED CMAKE_IMPORT_LIBRARY_SUFFIX)

set(FILES_TO_COPY ${CMAKE_SHARED_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}
                  ${IMPORT_LIB} )

link_directories( ${OCL_SOURCE_DIR}/externals/tbb/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR} )

# add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} SHARED ${TARGET_SOURCES})

COPY_GENERATED_FILES_POST_BUILD( ${TARGET_NAME} ${FILES_TO_COPY} )
COPY_SOURCE_FILES_POST_BUILD( ${TARGET_NAME} ${CONFIG_FILES} )

if (WIN32)
	set_target_properties(${TARGET_NAME} PROPERTIES
		LINK_FLAGS_RELEASE	"${LINK_FLAGS_RELEASE} /DELAYLOAD:tbb.dll /DELAYLOAD:task_executor.dll"
		LINK_FLAGS_DEBUG	"${LINK_FLAGS_DEBUG} /NODEFAULTLIB:MSVCRT /DELAYLOAD:tbb_debug.dll /DELAYLOAD:task_executor.dll"
	)

	list( APPEND LINK_LIBS delayimp.lib )
endif(WIN32)

target_link_libraries( ${TARGET_NAME} Context
                                      Execution
                                      Platform
									  MemoryAllocator
                                      cl_logger
                                      cl_sys_utils
                                      task_executor
                                      ${LINK_LIBS} )

set_target_properties( ${TARGET_NAME}  PROPERTIES FOLDER ${RUNTIME_FOLDER_NAME} )
