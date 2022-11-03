set(TARGET_NAME OCLSamplePlugin)

include_directories(${OCL_SOURCE_DIR}/cl_api)
include_directories(${CL_API_HEADERS})
include_directories(${OCL_SOURCE_DIR}/backend/ocl_cpu_backend)

# for plugins interface
include_directories(${OCL_SOURCE_DIR}/backend/ocl_cpu_backend/export)

set(SAMPLEPLUGIN_INCLUDE_FILES SamplePlugin.h)

set(SAMPLEPLUGIN_SOURCE_FILES dllmain.cpp SamplePlugin.cpp)

# Create DLL
add_opencl_library(${TARGET_NAME} SHARED ${SAMPLEPLUGIN_INCLUDE_FILES}
                   ${SAMPLEPLUGIN_SOURCE_FILES})

target_link_libraries(${TARGET_NAME} OclBackendUtils)

set_target_properties(
  ${TARGET_NAME}
  PROPERTIES FOLDER Backend/validation/tests
             RUNTIME_OUTPUT_DIRECTORY ${BE_TESTS_BINARY_DIR}
             LIBRARY_OUTPUT_DIRECTORY ${BE_TESTS_BINARY_DIR}
             ARCHIVE_OUTPUT_DIRECTORY ${BE_TESTS_BINARY_DIR}
             SOVERSION "${VERSIONSTRING}")
