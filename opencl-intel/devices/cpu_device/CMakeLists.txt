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

include_directories( BEFORE ../../externals/llvm_wrap/llvm/ocl_cpu_backend/export
                            ${HW_UTILS_HEADERS} 
                            ${OCL_BINARY_DIR} )
							
if(WIN32)
include_directories(
						${CMAKE_CURRENT_SOURCE_DIR}/../../externals/gpa/include )
endif(WIN32)

set(FILES_TO_COPY ${CMAKE_SHARED_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX} )

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()


link_directories( 	${CMAKE_CURRENT_SOURCE_DIR}/../../externals/llvm_wrap/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR}
					${CMAKE_CURRENT_BINARY_DIR}/../../externals/llvm_wrap/lib/${INSTALL_SUBDIR}
					${OCL_SOURCE_DIR}/externals/tbb/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR}
				  	${OCL_INSTALL_DIR}
				)

if(WIN32)
link_directories(
					${CMAKE_CURRENT_BINARY_DIR}/../../externals/llvm_wrap/LLVM_BUILD/lib/${INSTALL_SUBDIR}
					${CMAKE_CURRENT_SOURCE_DIR}/../../externals/llvm_wrap/llvm/ocl_cpu_backend/lib/Win32
					${CMAKE_CURRENT_SOURCE_DIR}/../../externals/gpa/libs/${TAL_LIB_DIR_SUFFIX} 
				)
else(WIN32)
link_directories(
					${CMAKE_CURRENT_BINARY_DIR}/../../externals/llvm_wrap/LLVM_BUILD/lib/${INSTALL_SUBDIR}
					${CMAKE_CURRENT_SOURCE_DIR}/../../externals/llvm_wrap/llvm/ocl_cpu_backend/lib/Linux64
					${CMAKE_CURRENT_SOURCE_DIR}/../../externals/gpa/libs/${TAL_LIB_DIR_SUFFIX} 
				)
endif(WIN32)

if (WIN32)
     add_definitions( -D_WINDOWS -DUSE_GPA )
else ()
	add_definitions( -D_LINUX )
 	message( WARNING " ERROR-in-CPU-device: Task Analizer DLLs for Linux are not ready yet" )
endif (WIN32)

#add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} SHARED ${TARGET_SOURCES})

if(WIN32)
	set_target_properties( ${TARGET_NAME}  PROPERTIES LINK_FLAGS_DEBUG "${LINK_FLAGS_DEBUG} /NODEFAULTLIB:MSVCRT" )
endif(WIN32)

add_dependencies( ${TARGET_NAME} clang )

COPY_GENERATED_FILES_POST_BUILD( ${TARGET_NAME} ${FILES_TO_COPY} )

target_link_libraries( ${TARGET_NAME} ${HW_UTILS_PROJ_NAME}
                                      cl_sys_utils
                                      cl_logger
                                      task_executor 
                                      ${LINK_LIBS} )
									  


set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${DEVICES_FOLDER_NAME} )

