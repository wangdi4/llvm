# remove the restriction of warning as error for this component
if (NOT WIN32)
    add_definitions( "-Wno-type-limits -Wno-missing-braces -Wno-unused-variable" )
    remove_definitions( -Werror )
endif (NOT WIN32)

#include helping functions of cmake.
include (../../cmake_utils/CMakeFuncs.txt)

set( TARGET common_runtime_test_type )

include_directories( ../../externals/gtest/include
                     ../../externals/khronos/spirv/headers/include
                     ${CL_API_HEADERS}
                     ${CMAKE_CURRENT_SOURCE_DIR}/common )

FIND_SOURCES( ${IGNORE_FILES} )
CALCULATE_TARGET_SOURCES()

FILE(GLOB CRT_KERNELS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} kernels/[^.]*)

# Place kernels under Kernels folder in Visual Studio
if (WIN32)
    source_group( "Kernels" FILES ${CRT_KERNELS} )
    list( APPEND TARGET_SOURCES ${CRT_KERNELS} )
endif (WIN32)

add_executable( ${TARGET} ${TARGET_SOURCES} )

target_link_libraries( ${TARGET} OpenCL googletest)
if (NOT WIN32)
    target_link_libraries( ${TARGET} pthread)
endif (NOT WIN32)

set( CREATE_DIR_NAME tests/common_runtime_test_type )

if (BUILD_X64)
    FILE(GLOB CRT_SPIRV   RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} spv64/[^.]*)
else (BUILD_X64)
    FILE(GLOB CRT_SPIRV   RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} spv32/[^.]*)
endif(BUILD_X64)

file(GLOB_RECURSE CP_FILES RELATIVE  ${CMAKE_CURRENT_SOURCE_DIR} *.py *.xml *.xsd )
# Copy *.py *.xml and *.xsd from current directory to appropriate test directory in 'Release' area.
install(FILES ${CP_FILES} DESTINATION ${CREATE_DIR_NAME})
# Copy *.py from ../GeneralScripts directory to appropriate test directory in 'Release' area.
install(FILES ../GeneralScripts/cmk_test_type_runner.py ../GeneralScripts/cmk_xml_entities.py DESTINATION ${CREATE_DIR_NAME})
# Copy generated executable to appropriate test directory in 'Release' area.
install(TARGETS ${TARGET} DESTINATION ${CREATE_DIR_NAME})

set_target_properties( ${TARGET} PROPERTIES FOLDER "validation" )

install(FILES ${CRT_KERNELS} DESTINATION tests/common_runtime_test_type)
#NOTE! The SPIRV files were generated from cl files with the same names that
# are placed in kernels dir.
#   EXAMPLE: spirv-gen kernels/simpler_kernel.cl -o spv64/simple_kernel.spv
install(FILES ${CRT_SPIRV} DESTINATION tests/common_runtime_test_type)
install(FILES common_runtime_test_type.pm DESTINATION tests/common_runtime_test_type)
