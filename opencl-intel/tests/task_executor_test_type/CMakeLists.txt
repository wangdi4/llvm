link_directories(${TBB_LIB_DIR})

set(TARGET task_executor_test_type)

include_directories(AFTER ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export
                    ${OCL_SOURCE_DIR}/utils/task_executor)

find_sources()
calculate_target_sources()

set(TARGET_LIBS
  LLVMSupport
  LLVMIntelJITEvents
  cl_hw_utils
  cl_logger
  cl_sys_utils
  task_executor)

if(NOT WIN32)
  if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    list(APPEND TARGET_LIBS
      ${IMPLIB_PREFIX}tbb_debug${IMPLIB_SUFFIX}
      ${IMPLIB_PREFIX}tbbmalloc_debug${IMPLIB_SUFFIX})
  else()
    list(APPEND TARGET_LIBS
      ${IMPLIB_PREFIX}tbb${IMPLIB_SUFFIX}
      ${IMPLIB_PREFIX}tbbmalloc${IMPLIB_SUFFIX})
  endif(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
endif()

add_ocl_unittest(${TARGET} SOURCE_FILES ${TARGET_SOURCES} LINK_LIBRARIES
                 ${TARGET_LIBS})

link_target_with_tbb_library(${TARGET})

