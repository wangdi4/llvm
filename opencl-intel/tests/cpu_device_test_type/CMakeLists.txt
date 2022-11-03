set(TARGET cpu_device_test_type)

include_directories(AFTER ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export)

find_sources(${IGNORE_FILES})
calculate_target_sources()

link_directories(${TBB_LIB_DIR})

set(TARGET_LIBS cl_sys_utils cpu_device${BUILD_PLATFORM}
                task_executor${BUILD_PLATFORM} test_utils)

set(INPUT_FILE test.cl)
set(OUTPUT_FILE test.bc)
compile_bitcode(${INPUT_FILE} ${OUTPUT_FILE})

set(TEST_BC_FILES ${TARGET_NAME}_test_files)
add_custom_target(${TEST_BC_FILES} DEPENDS ${OUTPUT_FILE})

add_ocl_unittest(${TARGET} SOURCE_FILES ${TARGET_SOURCES} LINK_LIBRARIES
                 ${TARGET_LIBS})

add_dependencies(${TARGET} clang_compiler${BUILD_PLATFORM} ${OCL_RT_TEST_DEPS}
                 ${TEST_BC_FILES})

if(WIN32)
  set_target_properties(
    ${TARGET}
    PROPERTIES
      LINK_FLAGS_RELEASE
      "${LINK_FLAGS_RELEASE} tbb12${TBB_BINARIES_POSTFIX}.lib /DELAYLOAD:tbb12${TBB_BINARIES_POSTFIX}.dll"
      LINK_FLAGS_DEBUG
      "${LINK_FLAGS_DEBUG} tbb12_debug${TBB_BINARIES_POSTFIX}.lib /DELAYLOAD:tbb12_debug${TBB_BINARIES_POSTFIX}.dll"
  )
endif()
