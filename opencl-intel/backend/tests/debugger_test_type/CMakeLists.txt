set(TARGET debugger_test_type)

find_sources()
calculate_target_sources()
include_directories(${opencl-cpp-header_SOURCE_DIR}/include)

set(nowarning_flags "-Wno-suggest-override -Wno-cast-qual -Wno-missing-field-initializers")
check_cxx_compiler_flag("${nowarning_flags}" CXX_SUPPORTS_FLAGS)
if(CXX_SUPPORTS_FLAGS)
  set_source_files_properties(${TARGET_SOURCES} PROPERTIES COMPILE_FLAGS "${nowarning_flags}")
endif()

add_opencl_executable(
  ${TARGET}
  ${TARGET_SOURCES}
  INCLUDE_DIRS
  ${OCL_SOURCE_DIR}/framework/export
  ${OCL_SOURCE_DIR}/cl_api
  ${CL_API_HEADERS}
  ${OCL_SOURCE_DIR}/utils/cl_hw_utils/export
  ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export
  LINK_LIBS
  LLVMSupport
  LLVMTargetParser
  OpenCL-ICD
  cl_hw_utils
  cl_sys_utils)

#
# Post-build events to copy stuff appropriately to the output directory
#

# This is where things will be copied to
#
set(DTT_DIR_NAME tests/debugger_test_type)

# Copy generated executable to DTT_DIR_NAME Since
# COPY_GENERATED_FILES_POST_BUILD needs OCL_INSTALL_DIR set, we save it and
# restore it after the command.
#
install(TARGETS ${TARGET} DESTINATION ${DTT_DIR_NAME})

# Copy all .py files from the main directory
#
file(
  GLOB MAIN_PY_FILES
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  *.py)
install(FILES ${MAIN_PY_FILES} DESTINATION ${DTT_DIR_NAME})

# Copy all .pm files from the main directory
#
file(
  GLOB MAIN_PM_FILES
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  *.pm)
install(FILES ${MAIN_PM_FILES} DESTINATION ${DTT_DIR_NAME})

# Copy all .zip files from the testlib directory
#
set(DTT_TESTLIB_DIR_NAME ${DTT_DIR_NAME}/testlib)
file(
  GLOB MAIN_ZIP_FILES
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  testlib/*.zip)
install(FILES ${MAIN_ZIP_FILES} DESTINATION ${DTT_TESTLIB_DIR_NAME})

# Create directory for CL files and copy them
#
set(DTT_KERNELS_DIR_NAME ${DTT_DIR_NAME}/cl_kernels)

file(
  GLOB_RECURSE CL_FILES
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  cl_kernels/*.cl)
install(FILES ${CL_FILES} DESTINATION ${DTT_KERNELS_DIR_NAME})

# Create directory for testcases and copy them
#
set(DTT_TESTCASES_DIR_NAME ${DTT_DIR_NAME}/testcases)

file(
  GLOB_RECURSE TESTCASE_FILES
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  testcases/*.py)
install(FILES ${TESTCASE_FILES} DESTINATION ${DTT_TESTCASES_DIR_NAME})

# Copy all code in the testlib/ dir
#
file(
  GLOB_RECURSE TESTLIB_FILES
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  testlib/*.py)
install(FILES ${TESTLIB_FILES} DESTINATION ${DTT_TESTLIB_DIR_NAME})

# Copy .xml config files for test runner
install(FILES cfg_win.xml cfg_linux.xml
        DESTINATION ${DTT_DIR_NAME}/debugger_test)

set_target_properties(${TARGET} PROPERTIES FOLDER "validation")
