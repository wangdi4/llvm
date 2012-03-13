#include helping functions of cmake.
include (../../cmake_utils/CMakeFuncs.txt)

set(TARGET debugger_test_type)

if (NOT WIN32)
  # remove the restriction of warning as error for this component
  remove_definitions(-Werror)
endif (NOT WIN32)

include_directories(../../framework/export 
                    ../../externals/gtest/include)
                
FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

link_directories( 
  ${OCL_SOURCE_DIR}/externals/tbb/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR} 
)

add_executable(${TARGET} ${TARGET_SOURCES})

if (NOT WIN32)
  target_link_libraries(${TARGET} intelocl googletest pthread)
else (NOT WIN32)
  target_link_libraries(${TARGET} intelocl googletest)
endif (NOT WIN32)

#
##
### Post-build events to copy stuff appropriately to the output directory
##
#

# This is where things will be copied to
#
set(DTT_DIR_NAME bin/validation/debugger_test_type)


# Copy generated executable to DTT_DIR_NAME
# Since COPY_GENERATED_FILES_POST_BUILD needs OCL_INSTALL_DIR set, we save it
# and restore it after the command.
#
install(TARGETS ${TARGET} DESTINATION ${DTT_DIR_NAME})

# Copy all .py files from the main directory
#
file(GLOB MAIN_PY_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.py)
install(FILES ${MAIN_PY_FILES} DESTINATION ${DTT_DIR_NAME})

# Copy all .zip files from the main directory
#
file(GLOB MAIN_ZIP_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.zip)
install(FILES ${MAIN_ZIP_FILES} DESTINATION ${DTT_DIR_NAME})

# Create directory for CL files and copy them
#
set(DTT_KERNELS_DIR_NAME ${DTT_DIR_NAME}/cl_kernels)

file(GLOB_RECURSE CL_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} cl_kernels/*.cl)
install(FILES ${CL_FILES} DESTINATION ${DTT_KERNELS_DIR_NAME})

# Create directory for testcases and copy them
#
set(DTT_TESTCASES_DIR_NAME ${DTT_DIR_NAME}/testcases)

file(GLOB_RECURSE TESTCASE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} testcases/*.py)
install(FILES ${TESTCASE_FILES} DESTINATION ${DTT_TESTCASES_DIR_NAME})

# Copy all code in the testlib/ dir
#
set(DTT_TESTLIB_DIR_NAME ${DTT_DIR_NAME}/testlib)

file(GLOB_RECURSE TESTLIB_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} testlib/*.py)
install( FILES ${TESTLIB_FILES} DESTINATION ${DTT_TESTLIB_DIR_NAME})

# Copy .xml config files for test runner
install( FILES cfg_win.xml cfg_linux.xml
         DESTINATION ${DTT_DIR_NAME}/debugger_test)

set_target_properties( ${TARGET} PROPERTIES FOLDER "validation" )

