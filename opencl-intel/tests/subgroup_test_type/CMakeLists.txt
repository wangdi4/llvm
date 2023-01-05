use_rtti(FALSE)

include_directories(${OCL_SOURCE_DIR}/tests/framework_test_type
                    ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export)

link_directories(${TBB_LIB_DIR})

set(TARGET_LIBS test_utils intelocl${BUILD_PLATFORM})
set_source_files_properties(SGEmulationTest.cpp PROPERTIES COMPILE_FLAGS -w)

find_sources()
calculate_target_sources()

add_ocl_unittest(
  subgroup_test_type
  SOURCE_FILES
  ${TARGET_SOURCES}
  ../framework_test_type/CL_BASE.cpp
  ../framework_test_type/TestsHelpClasses.cpp
  LINK_LIBRARIES
  ${TARGET_LIBS})
