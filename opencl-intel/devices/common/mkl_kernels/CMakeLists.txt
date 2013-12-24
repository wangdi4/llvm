#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME mkl_kernels )
if (NOT WIN32)
    set(HW_UTILS_HEADERS ../../utils/cl_hw_utils/export)
    set(HW_UTILS_PROJ_NAME cl_hw_utils)
endif (NOT WIN32)

add_definitions(-DMKL_ILP64)

include_directories( AFTER
    ${MKL_DIR}/include
    ${HW_UTILS_HEADERS}
    ../builtin_kernels
)

get_directory_property(OUT_VAR LINK_DIRECTORIES)
message(STATUS "DIR: ${OUT_VAR}")

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()
                
#add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} STATIC ${TARGET_SOURCES})

install( TARGETS ${TARGET_NAME}     
            RUNTIME DESTINATION bin
            ARCHIVE DESTINATION lib
            LIBRARY DESTINATION bin)

target_link_libraries( ${TARGET_NAME} ${HW_UTILS_PROJ_NAME}
                                      cl_sys_utils
                                      cl_logger
                                      ${LINK_LIBS} )
                                      
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${DEVICES_FOLDER_NAME} )
