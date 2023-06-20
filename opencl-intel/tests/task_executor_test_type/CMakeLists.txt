set(TARGET task_executor_test_type)

use_rtti(TRUE)

include_directories(AFTER ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export
                    ${OCL_SOURCE_DIR}/utils/task_executor)

find_sources()
calculate_target_sources()

set(TARGET_LIBS
  LLVMSupport
  LLVMTargetParser
  LLVMIntelJITEvents
  cl_hw_utils
  cl_logger
  cl_sys_utils
  task_executor)

add_ocl_unittest(${TARGET} SOURCE_FILES ${TARGET_SOURCES} LINK_LIBRARIES
                 ${TARGET_LIBS})

link_target_with_tbb_library(${TARGET})

