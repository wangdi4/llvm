# exectuable name
set(TARGET_NAME OclRecorderTest)

# setting the path to the recorder we test
if(WIN32)
  set(DORDER_INSTALL "")
  string(REPLACE "\${BUILD_TYPE}" ${CMAKE_CFG_INTDIR} DORDER_INSTALL
                 ${CMAKE_INSTALL_PREFIX})
  add_definitions(-DRECORDER=${DORDER_INSTALL}/bin/OclRecorder.dll)
else(WIN32)
  add_definitions(-DRECORDER=${CMAKE_INSTALL_PREFIX}/bin/libOclRecorder.so)
endif(WIN32)

# include dirs
include_directories(
  ${OCL_SOURCE_DIR}/cl_api
  ${CL_API_HEADERS}
  ${OCL_SOURCE_DIR}/tests/framework_test_type/
  ${OCL_SOURCE_DIR}/tests/test_utils/export
  ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export)

if(NOT WIN32)
  set(LINK_LIBRARIES ${PTHREAD_LIB})
else(NOT WIN32)
  set(LINK_LIBRARIES delayimp)
endif(NOT WIN32)

# linking
link_directories(${CMAKE_INSTALL_PREFIX}/bin/)

# adding the executable
add_opencl_executable(
  ${TARGET_NAME}
  EXCLUDE_FROM_ALL_BUILD
  recorder_test.cpp
  ${OCL_SOURCE_DIR}/utils/cl_sys_utils/cl_utils.cpp
  ${OCL_SOURCE_DIR}/tests/test_utils/test_utils.cpp
  LINK_LIBS
  LLVMSupport
  LLVMTargetParser
  llvm_gtest
  cl_logger
  cl_hw_utils
  cl_sys_utils
  OpenCL-ICD
  ${LINK_LIBRARIES}
  )

link_target_with_tbb_library(${TARGET_NAME})

set(CMAKE_CXX_FLAGS_DEBUG "-D OCLFRONTEND_PLUGINS ${CMAKE_CXX_FLAGS_DEBUG}")

# adding dependency to the test subject
add_dependencies(${TARGET_NAME} OclRecorder)

# install the executable
install(FILES OclRecorderTest.pm DESTINATION tests/recorder)
install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION tests/recorder)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Backend")
