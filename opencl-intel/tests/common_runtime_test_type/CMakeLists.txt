set(TARGET common_runtime_test_type)

include_directories(
  ${LLVM_EXTERNAL_LLVM_SPIRV_SOURCE_DIR}/lib ${CL_API_HEADERS}
  ${CMAKE_CURRENT_SOURCE_DIR}/common ${OCL_SOURCE_DIR}/tests/test_utils/export)

find_sources(${IGNORE_FILES})
calculate_target_sources()

file(
  GLOB CRT_KERNELS
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  kernels/[^.]*)

# Place kernels under Kernels folder in Visual Studio
if(WIN32)
  source_group("Kernels" FILES ${CRT_KERNELS})
  list(APPEND TARGET_SOURCES ${CRT_KERNELS})
endif(WIN32)

add_executable(${TARGET} ${TARGET_SOURCES})

target_link_libraries(${TARGET} OpenCL googletest)
if(NOT WIN32)
  target_link_libraries(${TARGET} pthread)
endif(NOT WIN32)

set(CREATE_DIR_NAME tests/common_runtime_test_type)

if(BUILD_X64)
  file(
    GLOB CRT_SPIRV
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    spv64/[^.]*)
else(BUILD_X64)
  file(
    GLOB CRT_SPIRV
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    spv32/[^.]*)
endif(BUILD_X64)

file(
  GLOB_RECURSE CP_FILES
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  *.py *.xml *.xsd)
# Copy *.py *.xml and *.xsd from current directory to appropriate test directory
# in 'Release' area.
install(FILES ${CP_FILES} DESTINATION ${CREATE_DIR_NAME})
# Copy *.py from ../GeneralScripts directory to appropriate test directory in
# 'Release' area.
install(FILES ../GeneralScripts/cmk_test_type_runner.py
              ../GeneralScripts/cmk_xml_entities.py
        DESTINATION ${CREATE_DIR_NAME})
# Copy generated executable to appropriate test directory in 'Release' area.
install(TARGETS ${TARGET} DESTINATION ${CREATE_DIR_NAME})

set_target_properties(${TARGET} PROPERTIES FOLDER "validation")

install(FILES ${CRT_KERNELS} DESTINATION tests/common_runtime_test_type)
# NOTE! The SPIRV files were generated from cl files with the same names that
# are placed in kernels dir. EXAMPLE: spirv-gen kernels/simpler_kernel.cl -o
# spv64/simple_kernel.spv
install(FILES ${CRT_SPIRV} DESTINATION tests/common_runtime_test_type)
install(FILES common_runtime_test_type.pm
        DESTINATION tests/common_runtime_test_type)
