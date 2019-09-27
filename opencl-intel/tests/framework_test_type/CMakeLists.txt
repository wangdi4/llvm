if (INCLUDE_MKL)
    add_definitions( -D__INCLUDE_MKL__ )
endif(INCLUDE_MKL)

set( TARGET framework_test_type )

include_directories( ${OCL_SOURCE_DIR}/framework/export
                     ${OCL_SOURCE_DIR}/framework/Tracing
                     ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export )

if (NOT WIN32)
    # silence warning coming from a gtest header.
    add_definitions ( -Wno-sign-compare )
endif (NOT WIN32)

set( IGNORE_FILES clBuildCubinProgram|clBuildProgram.cpp|clCreateCudaKernel|clCreateKernelCpuGpu|clCreateQueue )

FIND_SOURCES( ${IGNORE_FILES} )
CALCULATE_TARGET_SOURCES()

set( TARGET_LIBS test_utils )

link_directories( ${TBB_LIB_DIR} )

if (WIN32)
  set( TARGET_LIBS ${TARGET_LIBS} delayimp )
endif (WIN32)

set( TARGET_LIBS ${TARGET_LIBS} intelocl${BUILD_PLATFORM} )

# In test with local memory we allocate 7MB for array. And allocate additional stack size with size 8MB = 8388608
set_source_files_properties( cl_device_local_mem_size.cpp PROPERTIES COMPILE_DEFINITIONS STACK_SIZE=8388608 )
# In test with private memory we allocate 1MB for array. But due to vectorization stack size depends on width of SIMD.
# Our compiler can works with maximum width equals to 16. So we have to allocate additional stack size equals to 16MB.
set_source_files_properties( cl_device_private_mem_size.cpp PROPERTIES COMPILE_DEFINITIONS STACK_SIZE=16777216 )

add_ocl_unittest( ${TARGET} SOURCE_FILES ${TARGET_SOURCES}
    LINK_LIBRARIES ${TARGET_LIBS} )

if (WIN32)
  set_target_properties(${TARGET} PROPERTIES
    LINK_FLAGS_RELEASE "${LINK_FLAGS_RELEASE} /DELAYLOAD:tbb${TBB_BINARIES_POSTFIX}.dll"
    LINK_FLAGS_DEBUG "${LINK_FLAGS_DEBUG} /DELAYLOAD:tbb_debug${TBB_BINARIES_POSTFIX}.dll"
  )

  # set 8 MB of stack size
  set_target_properties( ${TARGET} PROPERTIES LINK_FLAGS /STACK:"16777216" )
endif (WIN32)


set( INCLUDE_HEADERS_PATH ${CCLANG_USER_INCLUDE_DIRS})

set( INPUT_FILE test.cl )
set( SPIR_OUTPUT_FILE test.bc )
set( SPIRV_OUTPUT_FILE test.spv )
set( INV_SPIR_OUTPUT_FILE inv_test.bc )

if (BUILD_X64)
set (
  SPIR_OPT
  -triple spir64-unknown-unknown -cl-kernel-arg-info -D __x86_64__
)
set (
  INV_SPIR_OPT
  -triple spir-unknown-unknown -cl-kernel-arg-info -D __i386__
)
set( REQD_NUM_SG_INPUT_FILE reqd_num_sub_groups_64.bc )
else (BUILD_X64)
set (
  SPIR_OPT
  -triple spir-unknown-unknown -cl-kernel-arg-info -D __i386__
)
set (
  INV_SPIR_OPT
  -triple spir64-unknown-unknown -cl-kernel-arg-info -D __x86_64__
)
set( REQD_NUM_SG_INPUT_FILE reqd_num_sub_groups_32.bc )
endif(BUILD_X64)

add_custom_command( TARGET ${TARGET}
  POST_BUILD
  COMMAND ${CLANG} -cc1 -x cl ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
  -I ${INCLUDE_HEADERS_PATH} -emit-llvm-bc
  ${SPIR_OPT} -disable-llvm-passes -O0 -disable-intel-proprietary-opts
  -include opencl-c.h -o ${CMAKE_CURRENT_BINARY_DIR}/${SPIR_OUTPUT_FILE}
  COMMAND ${CLANG} -cc1 -x cl ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
  -I ${INCLUDE_HEADERS_PATH} -emit-llvm-bc
  ${INV_SPIR_OPT} -disable-llvm-passes -O0 -disable-intel-proprietary-opts
  -include opencl-c.h -o ${CMAKE_CURRENT_BINARY_DIR}/${INV_SPIR_OUTPUT_FILE}
  MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
  DEPENDS ${INCLUDE_HEADERS_PATH}/opencl-c.h
  BYPRODUCTS ${SPIR_OUTPUT_FILE} ${INV_SPIR_OUTPUT_FILE}
  COMMENT "Generate spir binary files for framework_test_type:  ${SPIR_OUTPUT_FILE}, ${INV_SPIR_OUTPUT_FILE}"
  VERBATIM
)

add_custom_command( TARGET ${TARGET}
  POST_BUILD
  COMMAND ${SPIRV_CONVERTER} -o ${SPIRV_OUTPUT_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${SPIR_OUTPUT_FILE}
  DEPENDS ${SPIR_OUTPUT_FILE}
  BYPRODUCTS ${SPIRV_OUTPUT_FILE}
  COMMENT "Generate spirv binary files for framework_test_type:  ${SPIRV_OUTPUT_FILE}"
  VERBATIM
)

add_ocl_unittest_artifacts(
  FILES
    cached_binary_without_section.bin
    cached_binary_with_wrong_version.bin
)

add_dependencies( ${TARGET} clang_compiler${BUILD_PLATFORM} )
