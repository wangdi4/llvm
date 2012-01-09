# remove the restriction of warning as error for this component
if (NOT WIN32)
remove_definitions( -Werror )
endif (NOT WIN32)

#include helping functions of cmake.
include (../../cmake_utils/CMakeFuncs.txt)

link_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../../externals/tal/lib/${TAL_LIB_DIR_SUFFIX} 
		${CMAKE_CURRENT_BINARY_DIR}/../../externals/llvm_wrap/lib/${INSTALL_SUBDIR}
		${OCL_SOURCE_DIR}/externals/tbb/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR}
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

#add_dependencies( ${TARGET} cpu_device cl_sys_utils task_executor)
target_link_libraries( ${TARGET}  cl_sys_utils cpu_device task_executor gtest)

set( CREATE_DIR_NAME bin/validation/cpu_device_test_type)

file(GLOB_RECURSE CP_FILES RELATIVE  ${CMAKE_CURRENT_SOURCE_DIR} *.py *.xml *.xsd )
# Copy *.py *.xml and *.xsd from current directory to appropriate test directory in 'Release' area.
install(FILES ${CP_FILES} DESTINATION ${CREATE_DIR_NAME})
# Copy *.py from ../GeneralScripts directory to appropriate test directory in 'Release' area.
install(FILES ../GeneralScripts/cmk_test_type_runner.py ../GeneralScripts/cmk_xml_entities.py DESTINATION ${CREATE_DIR_NAME})
# Copy generated executable to appropriate test directory in 'Release' area.
install(TARGETS ${TARGET} DESTINATION ${CREATE_DIR_NAME})
# Copy *.xml cfg files which defines the options of the test to appropriate test directory in 'Release' area.
install(FILES cfg_win.xml cfg_linux.xml DESTINATION ${CREATE_DIR_NAME}/cpu_device_test)

set( INCLUDE_HEADERS_PATH ${OCL_CLANG_HEADERS})

set( INPUT_FILE test.cl )
set( OUTPUT_FILE test.bc )


add_custom_command( TARGET ${TARGET}
					POST_BUILD
                    COMMAND clang -x cl ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE} 
                    -I ${INCLUDE_HEADERS_PATH} -emit-llvm -c -include opencl_.h -O3 -o ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_FILE}
                    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
                    DEPENDS ${INCLUDE_HEADERS_PATH}/opencl_.h clang
                    COMMENT "Generate optimized ${OUTPUT_FILE}"
                    VERBATIM
                   )

set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_FILE} PROPERTIES GENERATED TRUE)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_FILE} DESTINATION ${CREATE_DIR_NAME})

#add_custom_target( cpu_device_test_type_kernel ALL
#                    DEPENDS ${OCL_INSTALL_DIR}/${CREATE_DIR_NAME}/${OUTPUT_FILE} ${TARGET_NAME} clang
#                    COMMENT "cpu_device_test_type_kernel:  Generate optimized ${OUTPUT_FILE}"
#                    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
#                    VERBATIM)

set_target_properties( ${TARGET} PROPERTIES FOLDER "validation/workloads" )
#set_target_properties( cpu_device_test_type_kernel PROPERTIES FOLDER "validation/workloads" )

add_dependencies( ${TARGET} clang clang_compiler )
#add_dependencies( cpu_device_test_type_kernel clang_compiler )
