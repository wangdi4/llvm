# remove the restriction of warning as error for this component
if (NOT WIN32)
remove_definitions( -Werror )
endif (NOT WIN32)

# set that name of the main output file as a target name
set( TARGET_NAME CallAllApi )


FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

file(GLOB TARGET_SOURCES *.cpp *.h )

# add a target named ${TARGET_NAME}
add_executable ( ${TARGET_NAME} ${TARGET_SOURCES} )
target_link_libraries( ${TARGET_NAME} OpenCL cl_sys_utils )

include_directories( 
                    ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export
                    )

if(WIN32)
    if(BUILD_X64)
        SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES STATIC_LIBRARY_FLAGS  "/MACHINE:X64")
    else(BUILD_X64)
        SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES STATIC_LIBRARY_FLAGS  "/MACHINE:X86")
    endif(BUILD_X64)
endif(WIN32)

set( CREATE_DIR_NAME tests/analyze_package_test_type)
set( TARGET_DIR_NAME tests/analyze_package_test_type)

# Copy generated executable to appropriate test directory in 'Release' area.
install( TARGETS ${TARGET_NAME} DESTINATION ${CREATE_DIR_NAME} )

#copy cl files that tests use
FILE(GLOB CRT_KERNELS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} kernels/[^.]*)
install(FILES ${CRT_KERNELS} DESTINATION ${TARGET_DIR_NAME})

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER "validation/workloads" )


