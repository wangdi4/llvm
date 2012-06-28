#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME cpu_device )
if (NOT WIN32)
    set(HW_UTILS_HEADERS ../../utils/cl_hw_utils/export)
    set(HW_UTILS_PROJ_NAME cl_hw_utils)
endif (NOT WIN32)

include_directories( BEFORE ../../backend/ocl_cpu_backend/export
                            ../../fe_compilers/clang_compiler
                            ${HW_UTILS_HEADERS} 
                            ${OCL_BINARY_DIR}
					)
							
include( ../../cmake_utils/itt.cmake )

if(WIN32)
	include_directories(
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
else ()
	add_definitions( -D_LINUX )
 	message( WARNING " Task Analizer DLLs for Linux are not ready yet" )
endif (WIN32)
				
# Add MKL library integration	
if (INCLUDE_MKL)
	message( "Including MKL library" )
	add_definitions( -D__INCLUDE_MKL__ )
	
	if (WIN32)
		set( MKL_DIR $ENV{ICPP_COMPILER12}/mkl )
	else(WIN32)
		set( MKL_DIR "/opt/intel/composerxe" )
	endif(WIN32)
		
	include_directories(
		${MKL_DIR}/include
	)
	if (ADDR EQUAL 64)
		add_definitions(-DMKL_ILP64 )
		set( MKL_LIB_DIR ${MKL_DIR}/lib/intel64 )
		list(APPEND LINK_LIBS ${MKL_LIB_DIR}/${IMPLIB_PREFIX}mkl_intel_ilp64${IMPLIB_STATIC_SUFFIX})
	else(ADDR EQUAL 64)
		set( MKL_LIB_DIR ${MKL_DIR}/lib/ia32 )
	endif(ADDR EQUAL 64)

	#Required for both 32 and 64 bit
	list(APPEND LINK_LIBS ${MKL_LIB_DIR}/${IMPLIB_PREFIX}mkl_core${IMPLIB_STATIC_SUFFIX} ${MKL_LIB_DIR}/${IMPLIB_PREFIX}mkl_sequential${IMPLIB_STATIC_SUFFIX})
endif(INCLUDE_MKL)

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()
				
#add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} SHARED ${TARGET_SOURCES})

if(WIN32)
	list(APPEND LINK_LIBS delayimp.lib)
endif(WIN32)

add_dependencies( ${TARGET_NAME} clang )

install( TARGETS ${TARGET_NAME}     
			RUNTIME DESTINATION bin
			ARCHIVE DESTINATION lib
			LIBRARY DESTINATION bin)

#COPY_GENERATED_FILES_POST_BUILD( ${TARGET_NAME} ${FILES_TO_COPY} )

target_link_libraries( ${TARGET_NAME} ${HW_UTILS_PROJ_NAME}
                                      cl_sys_utils
                                      cl_logger
                                      task_executor 
                                      ${LINK_LIBS} )
									  
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${DEVICES_FOLDER_NAME} )

if (WIN32)
    file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${TARGET_NAME}_stripped.pdb PDB_NAME)
    if (${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2008 	
        set_target_properties( ${TARGET_NAME} PROPERTIES
		    LINK_FLAGS_RELEASE	"${LINK_FLAGS_RELEASE} /DELAYLOAD:tbb.dll /DELAYLOAD:task_executor.dll"
		    LINK_FLAGS_DEBUG	"${LINK_FLAGS_DEBUG} /NODEFAULTLIB:MSVCRT /DELAYLOAD:tbb_debug.dll /DELAYLOAD:task_executor.dll"
	    )
	    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS	"${LINK_FLAGS} /PDBSTRIPPED:${PDB_NAME}")
    else(${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2010 (assumed if not Visual Studio 2008)
        # This is a fix due to a bug in CMake, Does not add the flag /DEBUG to the linker flags in Release mode.
        # The /DEBUG flag is required in order to create stripped pdbs.	
	    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG	"${LINK_FLAGS_DEBUG} /NODEFAULTLIB:MSVCRT /DELAYLOAD:tbb_debug.dll /DELAYLOAD:task_executor.dll /PDBSTRIPPED:${PDB_NAME}")
        set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE	"${LINK_FLAGS_RELEASE} /DELAYLOAD:tbb.dll /DELAYLOAD:task_executor.dll /DEBUG /PDBSTRIPPED:${PDB_NAME}")
    endif (${MSVC_VERSION} EQUAL 1500)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/\${BUILD_TYPE}/${TARGET_NAME}_stripped.pdb DESTINATION bin)
    
    if (INSTALL_PDBS)    
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/\${BUILD_TYPE}/${TARGET_NAME}.pdb DESTINATION bin)
    endif(INSTALL_PDBS)
    
endif(WIN32)

