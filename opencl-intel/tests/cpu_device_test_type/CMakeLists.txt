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

link_directories(
        ${CMAKE_CURRENT_BINARY_DIR}/../../externals/llvm_wrap/lib/${INSTALL_SUBDIR}
        ${TBB_LIB_DIR}
)

if(WIN32)
    link_directories(
        ${CMAKE_CURRENT_BINARY_DIR}/../../externals/llvm_wrap/LLVM/lib/${INSTALL_SUBDIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../backend/ocl_cpu_backend/lib/Win32
    )
else (WIN32)
    link_directories(
        ${CMAKE_CURRENT_BINARY_DIR}/../../externals/llvm_wrap/LLVM/lib/${INSTALL_SUBDIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../backend/ocl_cpu_backend/lib/Linux64
    )
endif(WIN32)

set( TARGET cpu_device_test_type )

include_directories( AFTER
            ../../utils/cl_sys_utils/export
            ../../externals/gtest/include
            )

set( IGNORE_FILES cpu_device_test_type.cpp )

FIND_SOURCES( ${IGNORE_FILES} )
CALCULATE_TARGET_SOURCES()

add_executable( ${TARGET} ${TARGET_SOURCES} )

if (NOT WIN32)
  if( ANDROID )
    target_link_libraries( ${TARGET} c++ )
    set_property(TARGET ${TARGET} APPEND_STRING PROPERTY LINK_FLAGS " -pie")
  endif( ANDROID )
  target_link_libraries( ${TARGET}  cl_sys_utils cpu_device${BUILD_PLATFORM} task_executor${BUILD_PLATFORM} googletest ${PTHREAD_LIB})
else (NOT WIN32)
  target_link_libraries( ${TARGET}  cl_sys_utils cpu_device${BUILD_PLATFORM} task_executor${BUILD_PLATFORM} googletest)
endif (NOT WIN32)
set( CREATE_DIR_NAME tests/cpu_device_test_type)

file(GLOB_RECURSE CP_FILES RELATIVE  ${CMAKE_CURRENT_SOURCE_DIR} *.py *.xml *.xsd )
# Copy *.py *.xml and *.xsd from current directory to appropriate test directory in 'Release' area.
install(FILES ${CP_FILES} DESTINATION ${CREATE_DIR_NAME})
# Copy *.py from ../GeneralScripts directory to appropriate test directory in 'Release' area.
install(FILES ../GeneralScripts/cmk_test_type_runner.py ../GeneralScripts/cmk_xml_entities.py DESTINATION ${CREATE_DIR_NAME})
# Copy generated executable to appropriate test directory in 'Release' area.
install(TARGETS ${TARGET} DESTINATION ${CREATE_DIR_NAME})
# Copy *.xml cfg files which defines the options of the test to appropriate test directory in 'Release' area.
install(FILES cfg_win.xml cfg_linux.xml DESTINATION ${CREATE_DIR_NAME}/cpu_device_test)

set( INCLUDE_HEADERS_PATH ${LLVM_INCLUDE_DIRS}/cclang/cl_headers)

set( INPUT_FILE test.cl )
set( OUTPUT_FILE test.bc )

add_custom_command( TARGET ${TARGET}
                    POST_BUILD
                    COMMAND ${CLANG} -cc1 -x cl ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE} 
                    -I ${INCLUDE_HEADERS_PATH} -emit-llvm-bc -include opencl_.h -O3 -o ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_FILE}
                    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
                    DEPENDS ${INCLUDE_HEADERS_PATH}/opencl_.h
                    COMMENT "Generate optimized ${OUTPUT_FILE}"
                    VERBATIM
                   )

set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_FILE} PROPERTIES GENERATED TRUE)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_FILE} DESTINATION ${CREATE_DIR_NAME})
install(FILES cpu_device_test_type.pm DESTINATION ${CREATE_DIR_NAME})

set_target_properties( ${TARGET} PROPERTIES FOLDER "validation/workloads" )

add_dependencies( ${TARGET} clang_compiler${BUILD_PLATFORM} )
