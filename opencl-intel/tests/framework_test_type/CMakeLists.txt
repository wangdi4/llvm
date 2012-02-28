# remove the restriction of warning as error for this component
if (NOT WIN32)
remove_definitions( -Werror )
endif (NOT WIN32)

#include helping functions of cmake.
include (../../cmake_utils/CMakeFuncs.txt)

set( TARGET framework_test_type )

include_directories( ../../framework/export 
					 ../../externals/gtest/include )
                
set( IGNORE_FILES clBuildCubinProgram|clBuildProgram.cpp|clCreateCudaKernel|clCreateKernelCpuGpu|clCreateQueue )

FIND_SOURCES( ${IGNORE_FILES} )
CALCULATE_TARGET_SOURCES()

link_directories( ${OCL_SOURCE_DIR}/externals/tbb/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR} )

add_executable( ${TARGET} ${TARGET_SOURCES} )

if (NOT WIN32)
target_link_libraries( ${TARGET} intelocl gtest pthread)
else (NOT WIN32)
target_link_libraries( ${TARGET} intelocl gtest)
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

add_custom_command( TARGET ${TARGET}
					POST_BUILD
                    COMMAND clang -cc1 -x cl ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE} 
                    -I ${INCLUDE_HEADERS_PATH} -emit-llvm-bc -cl-kernel-arg-info -include opencl_.h -O3 -o ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_FILE}
                    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
                    DEPENDS ${INCLUDE_HEADERS_PATH}/opencl_.h clang 
                    COMMENT "Generate optimized ${OUTPUT_FILE}"
                    VERBATIM
                   )

set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_FILE} PROPERTIES GENERATED TRUE)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_FILE} DESTINATION ${CREATE_DIR_NAME})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_FILE} DESTINATION bin)

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
