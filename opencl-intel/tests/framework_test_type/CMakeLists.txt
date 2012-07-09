# remove the restriction of warning as error for this component
if (NOT WIN32)
remove_definitions( -Werror )
endif (NOT WIN32)

if (INCLUDE_MIC_DEVICE)
  add_definitions( -DINCLUDE_MIC_DEVICE )
endif (INCLUDE_MIC_DEVICE)

#include helping functions of cmake.
include (../../cmake_utils/CMakeFuncs.txt)

set( TARGET framework_test_type )

include_directories( ../../framework/export 
					 ../../externals/gtest/include )
                
set( IGNORE_FILES clBuildCubinProgram|clBuildProgram.cpp|clCreateCudaKernel|clCreateKernelCpuGpu|clCreateQueue )

FIND_SOURCES( ${IGNORE_FILES} )
CALCULATE_TARGET_SOURCES()

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

add_executable( ${TARGET} ${TARGET_SOURCES} )

if (NOT WIN32)
	target_link_libraries( ${TARGET} intelocl googletest pthread)
else (NOT WIN32)
	target_link_libraries( ${TARGET} intelocl googletest delayimp)
	set_target_properties(${TARGET} PROPERTIES
		LINK_FLAGS_RELEASE	"${LINK_FLAGS_RELEASE} /DELAYLOAD:tbb.dll"
		LINK_FLAGS_DEBUG	"${LINK_FLAGS_DEBUG} /DELAYLOAD:tbb_debug.dll"
	)

endif (NOT WIN32)



set( CREATE_DIR_NAME bin/validation/framework_test_type)

file(GLOB_RECURSE CP_FILES RELATIVE  ${CMAKE_CURRENT_SOURCE_DIR} *.py *.xml *.xsd )
# Copy *.py *.xml and *.xsd from current directory to appropriate test directory in 'Release' area.
install(FILES ${CP_FILES} DESTINATION ${CREATE_DIR_NAME})
# Copy *.py from ../GeneralScripts directory to appropriate test directory in 'Release' area.
install(FILES ../GeneralScripts/cmk_test_type_runner.py ../GeneralScripts/cmk_xml_entities.py DESTINATION ${CREATE_DIR_NAME})
# Copy generated executable to appropriate test directory in 'Release' area.
install(TARGETS ${TARGET} DESTINATION ${CREATE_DIR_NAME})
# Copy *.xml cfg files which defines the options of the test to appropriate test directory in 'Release' area.
install(FILES cfg_win.xml cfg_linux.xml DESTINATION ${CREATE_DIR_NAME}/framework_test)

set( INCLUDE_HEADERS_PATH ${OCL_CLANG_HEADERS})

set( INPUT_FILE test.cl )
set( OUTPUT_FILE test.bc )

if ( INCLUDE_MIC_DEVICE ) 
set( MIC_OUTPUT_FILE mic_test.bc )

set (MIC_OPTIONS
  -cc1 -emit-llvm-bc -x cl -I ${INCLUDE_HEADERS_PATH} -include opencl_.h -o 
  ${CMAKE_CURRENT_BINARY_DIR}/${MIC_OUTPUT_FILE} 
  ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
)

add_custom_command( TARGET ${TARGET}
  POST_BUILD
  COMMAND clang ${MIC_OPTIONS}
  MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
  DEPENDS ${INCLUDE_HEADERS_PATH}/opencl_.h clang 
  COMMENT "Generate optimized ${MIC_OUTPUT_FILE}"
  VERBATIM
)

endif ( INCLUDE_MIC_DEVICE )

add_custom_command( TARGET ${TARGET}
					POST_BUILD
                    COMMAND clang -cc1 -x cl ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE} 
                    -I ${INCLUDE_HEADERS_PATH} -emit-llvm-bc -include opencl_.h -O3 -o ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_FILE}
                    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
                    DEPENDS ${INCLUDE_HEADERS_PATH}/opencl_.h clang 
                    COMMENT "Generate optimized ${OUTPUT_FILE}"
                    VERBATIM
                   )

set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_FILE} PROPERTIES GENERATED TRUE)
if ( INCLUDE_MIC_DEVICE )
  set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${MIC_OUTPUT_FILE} PROPERTIES GENERATED TRUE)
endif ( INCLUDE_MIC_DEVICE )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_FILE} DESTINATION ${CREATE_DIR_NAME})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_FILE} DESTINATION bin)
if ( INCLUDE_MIC_DEVICE )
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${MIC_OUTPUT_FILE} DESTINATION ${CREATE_DIR_NAME})
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${MIC_OUTPUT_FILE} DESTINATION bin)
endif ( INCLUDE_MIC_DEVICE )

#add_custom_target( framework_test_type_kernel ALL
#                    DEPENDS ${OCL_INSTALL_DIR}/${CREATE_DIR_NAME}/${OUTPUT_FILE} ${TARGET_NAME}
#                    COMMENT "framework_test_type_kernel: Generate optimized ${OUTPUT_FILE}"
#                    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
#                    VERBATIM)


set_target_properties( ${TARGET} PROPERTIES FOLDER "validation/workloads" )
#set_target_properties( framework_test_type_kernel PROPERTIES FOLDER "validation/workloads" )

add_dependencies( ${TARGET} clang clang_compiler )
#add_dependencies( framework_test_type_kernel clang_compiler clang)

#copy external files that tests use
file(GLOB_RECURSE CP_FILES RELATIVE  ${CMAKE_CURRENT_SOURCE_DIR} externals/*.cl )
install(FILES ${CP_FILES} DESTINATION bin)
install(FILES ${CP_FILES} DESTINATION ${CREATE_DIR_NAME})

