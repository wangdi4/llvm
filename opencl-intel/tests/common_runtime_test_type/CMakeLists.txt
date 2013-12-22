# remove the restriction of warning as error for this component
if (NOT WIN32)
    add_definitions( "-Wno-type-limits -Wno-missing-braces -Wno-unused-variable" )
    remove_definitions( -Werror )
endif (NOT WIN32)

#include helping functions of cmake.
include (../../cmake_utils/CMakeFuncs.txt)

set( TARGET common_runtime_test_type )

include_directories( ../../externals/gtest/include
 ${CMAKE_CURRENT_SOURCE_DIR}/common
 )
                
#set( IGNORE_FILES clBuildCubinProgram|clBuildProgram.cpp|clCreateCudaKernel|clCreateKernelCpuGpu|clCreateQueue )

FIND_SOURCES( ${IGNORE_FILES} )
CALCULATE_TARGET_SOURCES()

#link_directories( ${OCL_SOURCE_DIR}/externals/tbb/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR} )

add_executable( ${TARGET} ${TARGET_SOURCES} )

if (NOT WIN32)
target_link_libraries( ${TARGET} OpenCL googletest)
  if(NOT ANDROID)
    target_link_libraries( ${TARGET} pthread)
  endif()
else (NOT WIN32)
target_link_libraries( ${TARGET} OpenCL googletest)
endif (NOT WIN32)



set( CREATE_DIR_NAME bin/validation/common_runtime_test_type )

file(GLOB_RECURSE CP_FILES RELATIVE  ${CMAKE_CURRENT_SOURCE_DIR} *.py *.xml *.xsd )
# Copy *.py *.xml and *.xsd from current directory to appropriate test directory in 'Release' area.
install(FILES ${CP_FILES} DESTINATION ${CREATE_DIR_NAME})
# Copy *.py from ../GeneralScripts directory to appropriate test directory in 'Release' area.
install(FILES ../GeneralScripts/cmk_test_type_runner.py ../GeneralScripts/cmk_xml_entities.py DESTINATION ${CREATE_DIR_NAME})
# Copy generated executable to appropriate test directory in 'Release' area.
install(TARGETS ${TARGET} DESTINATION ${CREATE_DIR_NAME})

set_target_properties( ${TARGET} PROPERTIES FOLDER "validation/workloads" )

FILE(GLOB CRT_KERNELS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} kernels/[^.]*)

install(FILES ${CRT_KERNELS} DESTINATION bin/validation/common_runtime_test_type)

