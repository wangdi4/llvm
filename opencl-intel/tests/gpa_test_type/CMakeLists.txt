# remove the restriction of warning as error for this component
if (NOT WIN32)
remove_definitions( -Werror )
endif (NOT WIN32)

#include helping functions of cmake.
include (../../cmake_utils/CMakeFuncs.txt)

set( TARGET gpa_test_type )

include_directories( ../../externals/gtest/include
 ${CMAKE_CURRENT_SOURCE_DIR}/common
 )
                
#set( IGNORE_FILES clBuildCubinProgram|clBuildProgram.cpp|clCreateCudaKernel|clCreateKernelCpuGpu|clCreateQueue )

FIND_SOURCES( ${IGNORE_FILES} )
CALCULATE_TARGET_SOURCES()

#link_directories( ${OCL_SOURCE_DIR}/externals/tbb/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR} )

add_executable( ${TARGET} ${TARGET_SOURCES} )

if (NOT WIN32)
target_link_libraries( ${TARGET} OpenCL gtest pthread)
else (NOT WIN32)
target_link_libraries( ${TARGET} OpenCL gtest)
endif (NOT WIN32)



set( CREATE_DIR_NAME /validation/gpa_test_type )

file(GLOB_RECURSE CP_FILES RELATIVE  ${CMAKE_CURRENT_SOURCE_DIR} *.py *.xml *.xsd )
# Copy *.py *.xml and *.xsd from current directory to appropriate test directory in 'Release' area.
COPY_FILES_POST_BUILD_EX( ${TARGET} ${CMAKE_CURRENT_SOURCE_DIR} ${OCL_INSTALL_DIR}/${CREATE_DIR_NAME} ${CP_FILES} )
# Copy *.py from ../GeneralScripts directory to appropriate test directory in 'Release' area.
COPY_FILES_POST_BUILD_EX( ${TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../GeneralScripts ${OCL_INSTALL_DIR}/${CREATE_DIR_NAME} cmk_test_type_runner.py cmk_xml_entities.py )
set( TEMP ${OCL_INSTALL_DIR} )
set( OCL_INSTALL_DIR ${OCL_INSTALL_DIR}/${CREATE_DIR_NAME} )
# Copy generated executable to appropriate test directory in 'Release' area.
COPY_GENERATED_FILES_POST_BUILD( ${TARGET} ${TARGET}${CMAKE_EXECUTABLE_SUFFIX} )
set( OCL_INSTALL_DIR ${TEMP} )
# Copy *.xml cfg files which defines the options of the test to appropriate test directory in 'Release' area.
COPY_FILES_POST_BUILD_EX( ${TARGET} ${CMAKE_CURRENT_SOURCE_DIR} ${OCL_INSTALL_DIR}/${CREATE_DIR_NAME}/gpa_test )

#set( INCLUDE_HEADERS_PATH ${OCL_INSTALL_DIR}/fe_include )

#set( INPUT_FILE test.cl )
#set( OUTPUT_FILE test.bc )

#add_custom_command( TARGET ${TARGET}
#					POST_BUILD
#					COMMAND ${CMAKE_COMMAND} -E  make_directory ${OCL_INSTALL_DIR}/${CREATE_DIR_NAME}					
#                    COMMAND clang -x cl ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE} 
#                    -I ${INCLUDE_HEADERS_PATH} -emit-llvm -c -include opencl_.h -O3 -o ${OCL_INSTALL_DIR}/${CREATE_DIR_NAME}/${OUTPUT_FILE}
#                    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
#                    DEPENDS ${INCLUDE_HEADERS_PATH}/opencl_.h clang 
#                   COMMENT "Generate optimized ${OUTPUT_FILE}"
#                    VERBATIM
#                   )

set_source_files_properties(${OCL_INSTALL_DIR}/${CREATE_DIR_NAME}/${OUTPUT_FILE} PROPERTIES GENERATED TRUE)


#add_custom_target( framework_test_type_kernel ALL
#                    DEPENDS ${OCL_INSTALL_DIR}/${CREATE_DIR_NAME}/${OUTPUT_FILE} ${TARGET_NAME}
#                    COMMENT "framework_test_type_kernel: Generate optimized ${OUTPUT_FILE}"
#                    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
#                    VERBATIM)


set_target_properties( ${TARGET} PROPERTIES FOLDER "validation/workloads" )
#set_target_properties( framework_test_type_kernel PROPERTIES FOLDER "validation/workloads" )

#add_dependencies( ${TARGET} clang clang_compiler )
#add_dependencies( framework_test_type_kernel clang_compiler clang)
