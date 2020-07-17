if (INCLUDE_MKL)
    add_definitions( -D__INCLUDE_MKL__ )
endif(INCLUDE_MKL)

set( TARGET framework_test_type )

# Temporarily disable loopopt due to a LoopOpt bug, see CMPLRLLVM-11239 for details
ocl_replace_compiler_option(CMAKE_CXX_FLAGS "-mllvm -loopopt" "")
ocl_replace_compiler_option(CMAKE_C_FLAGS "-mllvm -loopopt" "")
add_flag_if_supported("-mllvm -loopopt=0" CMPLR_LLVM_LOOPOPT)

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
# In test with private memory we allocate 1MB for array. But due to vectorization stack size depends on width of SIMD
# and scalar kernel, our compiler can works with maximum width equals to 17. So we have to allocate additional stack
# size equals to 17MB.
set_source_files_properties( cl_device_private_mem_size.cpp PROPERTIES COMPILE_DEFINITIONS STACK_SIZE=17825792 )

set(INPUT_FILE test.cl)
set(SPIRV_OUTPUT_FILE test.spv)
set(INV_BC_OUTPUT_FILE inv_test.bc)
set(INV_SPIRV_OUTPUT_FILE inv_test.spv)
set(TEST_SPV_FILES ${TARGET}_test_spv)
compile_spv(${INPUT_FILE} ${SPIRV_OUTPUT_FILE}
  EXTRA_CLANG_FLAGS -cl-kernel-arg-info)
compile_spv(${INPUT_FILE} ${INV_SPIRV_OUTPUT_FILE}
  EXTRA_CLANG_FLAGS -cl-kernel-arg-info
  BC_OUTPUT_FILE ${INV_BC_OUTPUT_FILE}
  BUILD_INV)
add_custom_target(${TEST_SPV_FILES}
  DEPENDS ${SPIRV_OUTPUT_FILE} ${INV_SPIRV_OUTPUT_FILE})

add_ocl_unittest( ${TARGET}
  SOURCE_FILES ${TARGET_SOURCES}
  LINK_LIBRARIES ${TARGET_LIBS})

if (WIN32)
  set_target_properties(${TARGET} PROPERTIES
    LINK_FLAGS_RELEASE "${LINK_FLAGS_RELEASE} /DELAYLOAD:tbb${TBB_BINARIES_POSTFIX}.dll"
    LINK_FLAGS_DEBUG "${LINK_FLAGS_DEBUG} /DELAYLOAD:tbb_debug${TBB_BINARIES_POSTFIX}.dll"
  )

  # set 17 MB of stack size
  set_target_properties( ${TARGET} PROPERTIES LINK_FLAGS /STACK:"17825792" )
endif (WIN32)



add_ocl_unittest_artifacts(
  FILES
    cached_binary_current_version.bin
    cached_binary_without_section.bin
    cached_binary_with_wrong_version.bin
    data/fp-as-kernel-arg.spv
    spec_const.spv
)

add_dependencies(${TARGET}
  clang_compiler${BUILD_PLATFORM}
  OclCpuBackEnd${BUILD_PLATFORM}
  cpu_device${BUILD_PLATFORM}
  ${OCL_RT_TEST_DEPS} ${TEST_SPV_FILES})
