#ensure we are using the frontend LLVM tools
if( NOT DEFINED LLVM_PATH_FE )
    message( FATAL_ERROR "LLVM_PATH_FE is not specified. Please specify LLVM library location for fronted using LLVM_PATH_FE parameter to CMAKE" )
endif()

set(LLVM_PATH ${LLVM_PATH_FE})
find_package(LLVM REQUIRED)

if( CMAKE_CROSSCOMPILING OR LLVM_USE_NATIVE OR ANDROID )
   set(TOOLS_BINARY_DIR "${LLVM_BINARY_DIR}/../native/bin")
else()
   set(TOOLS_BINARY_DIR "${LLVM_BINARY_DIR}" )
endif()

set( CLANG "${TOOLS_BINARY_DIR}/clang" )

# remove the restriction of warning as error for this component
if (NOT WIN32)
remove_definitions( -Werror )
endif (NOT WIN32)

if (INCLUDE_MIC_DEVICE)
  add_definitions( -DINCLUDE_MIC_DEVICE )
endif (INCLUDE_MIC_DEVICE)

if (INCLUDE_MKL)
	add_definitions( -D__INCLUDE_MKL__ )
endif(INCLUDE_MKL)

#include helping functions of cmake.
include (../../cmake_utils/CMakeFuncs.txt)

set( TARGET framework_test_type )

include_directories( ../../framework/export 
                     ../../externals/gtest/include
                     ${OCL_SOURCE_DIR}/tests/test_common )

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
	if( ANDROID )
  		target_link_libraries( ${TARGET} gnustl_shared )
        set_property(TARGET ${TARGET} APPEND_STRING PROPERTY LINK_FLAGS " -pie")
	endif( ANDROID )
	target_link_libraries( ${TARGET} intelocl${BUILD_PLATFORM} googletest ${PTHREAD_LIB})
else (NOT WIN32)
	target_link_libraries( ${TARGET} intelocl${BUILD_PLATFORM} googletest delayimp)
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

if (BUILD_X64)
set (
  SPIR_OPT 
  -triple spir64-unkown-unkown -cl-kernel-arg-info -D __x86_64__
)
else (BUILD_X64)
set (
  SPIR_OPT
  -triple spir-unkown-unkown -cl-kernel-arg-info -D __i386__
)
endif(BUILD_X64)

if ( INCLUDE_MIC_DEVICE ) 
set( MIC_OUTPUT_FILE mic_test.bc )

set (MIC_OPTIONS
  -cc1 -emit-llvm-bc -fblocks -x cl -I ${INCLUDE_HEADERS_PATH} -include opencl_.h -o 
  ${CMAKE_CURRENT_BINARY_DIR}/${MIC_OUTPUT_FILE}
  ${SPIR_OPT}
  ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
)

add_custom_command( TARGET ${TARGET}
  POST_BUILD
  COMMAND ${CLANG} ${MIC_OPTIONS}
  MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
  DEPENDS ${INCLUDE_HEADERS_PATH}/opencl_.h  
  COMMENT "Generate spir binary: ${MIC_OUTPUT_FILE}"
  VERBATIM
)

endif ( INCLUDE_MIC_DEVICE )

add_custom_command( TARGET ${TARGET}
  POST_BUILD
  COMMAND ${CLANG} -cc1 -x cl ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE} 
  -I ${INCLUDE_HEADERS_PATH} -emit-llvm-bc
  ${SPIR_OPT}
  -include opencl_.h -O0 -o ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_FILE}
  MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
  DEPENDS ${INCLUDE_HEADERS_PATH}/opencl_.h ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
  COMMENT "Generate spir binary:  ${OUTPUT_FILE}"
  VERBATIM
)

set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_FILE} PROPERTIES GENERATED TRUE)
if ( INCLUDE_MIC_DEVICE )
  set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${MIC_OUTPUT_FILE} PROPERTIES GENERATED TRUE)
endif ( INCLUDE_MIC_DEVICE )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_FILE} DESTINATION ${CREATE_DIR_NAME})
if ( INCLUDE_MIC_DEVICE )
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${MIC_OUTPUT_FILE} DESTINATION ${CREATE_DIR_NAME})
endif ( INCLUDE_MIC_DEVICE )

set_target_properties( ${TARGET} PROPERTIES FOLDER "validation/workloads" )
#set_target_properties( framework_test_type_kernel PROPERTIES FOLDER "validation/workloads" )

add_dependencies( ${TARGET} clang_compiler${BUILD_PLATFORM} )
