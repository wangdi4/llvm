set(TARGET_NAME FMAtest)

# FusedMAtest has to use deprecated OpenCL API functions.
if (WIN32)
    add_definitions(
        -wd4996 # Suppress 'function': was declared deprecated'
    )
endif (WIN32)

include_directories(
  ${CL_API_HEADERS}
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend/utils
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend
  ${BACKEND_ROOT_DIR}/common_runtime/icd
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend
  ${OCL_SOURCE_DIR}/externals/gtest/googletest/include
  ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export
  )
add_executable(${TARGET_NAME}
   FusedMAtest.cpp
)

if (NOT WIN32)
  set(LIBRARIES OclBackendUtils ${OpenCL_LIBRARY} gtest pthread)
else (NOT WIN32)
  set(LIBRARIES OclBackendUtils ${OpenCL_LIBRARY} gtest)
endif (NOT WIN32)

target_link_libraries(${TARGET_NAME}
   ${LIBRARIES}
)

install( TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION tests/FMATest
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION tests/FMATest
)
