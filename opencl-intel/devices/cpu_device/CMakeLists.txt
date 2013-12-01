#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME cpu_device${BUILD_PLATFORM} )
if (NOT WIN32)
    set(HW_UTILS_HEADERS ../../utils/cl_hw_utils/export)
    set(HW_UTILS_PROJ_NAME cl_hw_utils)
endif (NOT WIN32)

include_directories( BEFORE ../../backend/ocl_cpu_backend/export
                            ../../fe_compilers/clang_compiler
                            ${HW_UTILS_HEADERS} 
                            ${OCL_BINARY_DIR}
							../common/builtin_kernels
					)
							
if(WIN32)
	include_directories( AFTER
		${OCL_SOURCE_DIR}/externals/gpa/include
		)
endif(WIN32)

set(FILES_TO_COPY ${CMAKE_SHARED_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX} )

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

link_directories( 	${TBB_LIB_DIR}
					${OCL_SOURCE_DIR}/externals/gpa/libs/${TAL_LIB_DIR_SUFFIX}
				)

if (WIN32)
    add_definitions( -D_WINDOWS )
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        add_definitions( -DRESOURCE_X64 )
    endif (CMAKE_SIZEOF_VOID_P EQUAL 8) 
else ()
	add_definitions( -D_LINUX )
endif (WIN32)

#Support for builtin kernels
add_subdirectory(../common/builtin_kernels ${CMAKE_CURRENT_BINARY_DIR}/builtin_kernels)
list (APPEND LINK_LIBS builtin_kernels device_commands)

# Add MKL library integration	
if (INCLUDE_MKL)
	message(STATUS "Including MKL library" )
	add_definitions( -D__INCLUDE_MKL__ )

	add_subdirectory(../common/mkl_kernels ${CMAKE_CURRENT_BINARY_DIR}/mkl_kernels)
	include_directories( BEFORE
		../common/mkl_kernels/export
		)
	
	list (APPEND LINK_LIBS mkl_kernels)
endif(INCLUDE_MKL)

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

#include ITT notify sources
include( ../../cmake_utils/itt.cmake )
				
#add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} SHARED ${TARGET_SOURCES})

if(WIN32)
	list(APPEND LINK_LIBS delayimp.lib)
else (WIN32)
    SET_LINUX_EXPORTS_FILE( ${TARGET_NAME} cpu_device_linux_exports.txt )
endif (WIN32)

install( TARGETS ${TARGET_NAME}     
			RUNTIME DESTINATION bin
			ARCHIVE DESTINATION lib
			LIBRARY DESTINATION bin)

#COPY_GENERATED_FILES_POST_BUILD( ${TARGET_NAME} ${FILES_TO_COPY} )

target_link_libraries( ${TARGET_NAME} ${HW_UTILS_PROJ_NAME}
                                      cl_sys_utils
                                      cl_logger
				      device_commands
                                      task_executor${BUILD_PLATFORM}
									  ${LINK_LIBS} )
									  
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${DEVICES_FOLDER_NAME} )

if (WIN32)
    get_target_property(RT_OUTPUT_DIRECTORY ${TARGET_NAME} RUNTIME_OUTPUT_DIRECTORY)
    file(TO_NATIVE_PATH ${RT_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/${TARGET_NAME}_stripped.pdb PDB_NAME)
    if (${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2008 	
        set_target_properties( ${TARGET_NAME} PROPERTIES
		    LINK_FLAGS_RELEASE	"${LINK_FLAGS_RELEASE} /DELAYLOAD:tbbmalloc.dll /DELAYLOAD:task_executor${BUILD_PLATFORM}.dll"
		    LINK_FLAGS_DEBUG	"${LINK_FLAGS_DEBUG} /DELAYLOAD:tbbmalloc_debug.dll /NODEFAULTLIB:MSVCRT /DELAYLOAD:task_executor${BUILD_PLATFORM}.dll"
	    )
	    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS	"${LINK_FLAGS} /PDBSTRIPPED:${PDB_NAME}")
    else(${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2010 (assumed if not Visual Studio 2008)
        # This is a fix due to a bug in CMake, Does not add the flag /DEBUG to the linker flags in Release mode.
        # The /DEBUG flag is required in order to create stripped pdbs.	
		set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG	"${LINK_FLAGS_DEBUG} /NODEFAULTLIB:MSVCRT /DELAYLOAD:tbbmalloc_debug.dll /DELAYLOAD:task_executor${BUILD_PLATFORM}.dll /PDBSTRIPPED:${PDB_NAME}")
        set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE	"${LINK_FLAGS_RELEASE} /DELAYLOAD:tbbmalloc.dll /DELAYLOAD:task_executor${BUILD_PLATFORM}.dll /DEBUG /PDBSTRIPPED:${PDB_NAME}")
    endif (${MSVC_VERSION} EQUAL 1500)
	install(FILES ${RT_OUTPUT_DIRECTORY}/\${BUILD_TYPE}/${TARGET_NAME}_stripped.pdb DESTINATION bin)
    
    if (INSTALL_PDBS)    
        install(FILES ${RT_OUTPUT_DIRECTORY}/\${BUILD_TYPE}/${TARGET_NAME}.pdb DESTINATION bin)
    endif(INSTALL_PDBS)
    
endif(WIN32)

