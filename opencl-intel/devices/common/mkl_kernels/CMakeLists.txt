#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME mkl_kernels )
if (NOT WIN32)
    set(HW_UTILS_HEADERS ../../utils/cl_hw_utils/export)
    set(HW_UTILS_PROJ_NAME cl_hw_utils)
endif (NOT WIN32)

# Add MKL library integration	
#Required for both 32 and 64 bit
# Linux
# -L$(MKLROOT)/lib/intel64 -lmkl_rt -lpthread -lm
# -DMKL_ILP64 -I$(MKLROOT)/include
# Windows
# mkl_rt.lib libiomp5md.lib
# /DMKL_ILP64  -I%MKLROOT%/include

if (ADDR EQUAL 64)
	set ( ICC_ARCH_DIR /intel64 )
	add_definitions(-DMKL_ILP64 )
else(ADDR EQUAL 64)
	set( ICC_ARCH_DIR /ia32 )
endif(ADDR EQUAL 64)

if (WIN32)
	set (ICC_WIN32_PATH $ENV{ICPP_COMPILER12})
	if (NOT DEFINED ICC_WIN32_PATH)
		set (ICC_WIN32_PATH $ENV{ICPP_COMPILER13}) 
		if (NOT DEFINED ICC_WIN32_PATH)
			message (FATAL_ERROR "ICC 12 or 13 are not installed in the system")
		endif ()
	endif ()
	set( MKL_DIR "${ICC_WIN32_PATH}/mkl" )
	set( ICC_DIR "${ICC_WIN32_PATH}/compiler")
#	list(APPEND LINK_LIBS ${MKL_DIR}/lib/${ICC_ARCH_DIR}/mkl_rt.lib ${ICC_DIR}/lib/${ICC_ARCH_DIR}/libiomp5md.lib )
else(WIN32)
	set( ICC_DIR "/opt/intel/composerxe")
	set( MKL_DIR "${ICC_DIR}/mkl" )
#	list(APPEND LINK_LIBS ${MKL_DIR}/lib/${ICC_ARCH_DIR}/libmkl_rt.so pthread m )	
endif(WIN32)
	
include_directories( AFTER
	${MKL_DIR}/include
	${HW_UTILS_HEADERS} 
)

get_directory_property(OUT_VAR LINK_DIRECTORIES)
message(STATUS "DIR: ${OUT_VAR}")

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()
				
#add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} STATIC ${TARGET_SOURCES})

install( TARGETS ${TARGET_NAME}     
			RUNTIME DESTINATION bin
			ARCHIVE DESTINATION lib
			LIBRARY DESTINATION bin)

target_link_libraries( ${TARGET_NAME} ${HW_UTILS_PROJ_NAME}
                                      cl_sys_utils
                                      cl_logger
                                      ${LINK_LIBS} )
									  
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${DEVICES_FOLDER_NAME} )
