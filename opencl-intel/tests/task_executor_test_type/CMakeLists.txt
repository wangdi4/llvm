# remove the restriction of warning as error for this component
if (NOT WIN32)
remove_definitions( -Werror )
endif (NOT WIN32)

#include helping functions of cmake.
include (../../cmake_utils/CMakeFuncs.txt)
include( ../../cmake_utils/itt.cmake )

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

set( TARGET task_executor_test_type )

include_directories( AFTER
	../../utils/cl_sys_utils/export
	../../externals/gtest/include
	../../utils/task_executor
            )
               
FIND_SOURCES( )
CALCULATE_TARGET_SOURCES()

add_executable( ${TARGET} ${TARGET_SOURCES} )

if (NOT WIN32)
  if( ANDROID )
    target_link_libraries( ${TARGET} c++ )
    set_property(TARGET ${TARGET} APPEND_STRING PROPERTY LINK_FLAGS " -pie")
  endif( ANDROID )
  target_link_libraries( ${TARGET}  cl_sys_utils task_executor${BUILD_PLATFORM} googletest ${PTHREAD_LIB})
else (NOT WIN32)
  target_link_libraries( ${TARGET}  cl_sys_utils task_executor${BUILD_PLATFORM} googletest)
endif(NOT WIN32)
target_link_libraries( ${TARGET}  cl_sys_utils task_executor${BUILD_PLATFORM} googletest)

set( CREATE_DIR_NAME tests/task_executor_test_type )

file(GLOB_RECURSE CP_FILES RELATIVE  ${CMAKE_CURRENT_SOURCE_DIR} *.py *.xml *.xsd )
# Copy *.py *.xml and *.xsd from current directory to appropriate test directory in 'Release' area.
install(FILES ${CP_FILES} DESTINATION ${CREATE_DIR_NAME})
# Copy *.py from ../GeneralScripts directory to appropriate test directory in 'Release' area.
install(FILES ../GeneralScripts/cmk_test_type_runner.py ../GeneralScripts/cmk_xml_entities.py DESTINATION ${CREATE_DIR_NAME})
# Copy generated executable to appropriate test directory in 'Release' area.

install(TARGETS ${TARGET} DESTINATION ${CREATE_DIR_NAME})
install(FILES task_executor_test_type.pm DESTINATION ${CREATE_DIR_NAME})


set( INCLUDE_HEADERS_PATH ${OCL_CLANG_HEADERS})

set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_FILE} PROPERTIES GENERATED TRUE)
set_target_properties( ${TARGET} PROPERTIES FOLDER "validation/workloads" )
