#exectuable name
set(TARGET_NAME "OclRecorderTest")

#recorder_test has to use deprecated OpenCL API functions.
if(WIN32)
    add_definitions(
        -wd4996 # Suppress 'function': was declared deprecated'
    )
endif(WIN32)

#setting the path to the recorder we test
if(WIN32)
  set (DORDER_INSTALL "")
  string( REPLACE  "\${BUILD_TYPE}"  ${CMAKE_CFG_INTDIR}  DORDER_INSTALL  ${CMAKE_INSTALL_PREFIX} )
  add_definitions(-DRECORDER=${DORDER_INSTALL}/bin/OclRecorder.dll)
else(WIN32)
  add_definitions(-DRECORDER=${CMAKE_INSTALL_PREFIX}/bin/libOclRecorder.so)
endif(WIN32)

#include dirs
include_directories( ${OCL_SOURCE_DIR}/externals/gtest/googletest/include
  ${OCL_SOURCE_DIR}/cl_api
  ${CL_API_HEADERS}
  ${OCL_SOURCE_DIR}/tests/framework_test_type/
  ${OCL_SOURCE_DIR}/tests/test_utils/export
  ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export
)

#adding the executable
add_executable(
  ${TARGET_NAME}
  recorder_test.cpp
  ${OCL_SOURCE_DIR}/utils/cl_sys_utils/cl_utils.cpp
  ${OCL_SOURCE_DIR}/tests/framework_test_type/test_utils.cpp
)

#linking
link_directories( ${TBB_LIB_DIR} ${CMAKE_INSTALL_PREFIX}/bin/ )

target_link_libraries( ${TARGET_NAME} gtest cl_logger OpenCL )

if (NOT WIN32)
  target_link_libraries(${TARGET_NAME} ${PTHREAD_LIB})
else (NOT WIN32)
  target_link_libraries( ${TARGET_NAME}  delayimp)
  set_target_properties(${TARGET_NAME} PROPERTIES
    LINK_FLAGS_RELEASE  "${LINK_FLAGS_RELEASE} /DELAYLOAD:tbb${TBB_BINARIES_POSTFIX}.dll"
    LINK_FLAGS_DEBUG    "${LINK_FLAGS_DEBUG}   /DELAYLOAD:tbb_debug${TBB_BINARIES_POSTFIX}.dll"
  )
endif (NOT WIN32)

set( CMAKE_CXX_FLAGS_DEBUG "-D OCLFRONTEND_PLUGINS ${CMAKE_CXX_FLAGS_DEBUG}" )

#adding dependency to the test subject
add_dependencies(${TARGET_NAME} OclRecorder)

#install the executable
install(FILES OclRecorderTest.pm DESTINATION tests/recorder)
install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION tests/recorder)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER "Backend" )
