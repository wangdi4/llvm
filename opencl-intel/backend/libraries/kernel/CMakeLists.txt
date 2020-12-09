set (TARGET_NAME_PREFIX ${OCL_LIBRARY_KERNEL_TARGET_NAME})
project(${TARGET_NAME_PREFIX})

set(RTL_TARGET_NAME ${TARGET_NAME_PREFIX})
set(TMP_RTL_TARGET_NAME ${TARGET_NAME_PREFIX}_tmp)
set(INTERMEDIATE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
# target directory for OCL .rtl and .o files
set (LIBRARY_OUTPUT_DIR ${OCL_OUTPUT_LIBRARY_DIR})

function(add_ocl_kernel_tmp_rtl CLANG_FLAGS INPUT_CL TMP_DIR OUTPUT_RTL)
  set(TMP_RTL_FILE ${TMP_RTL_TARGET_NAME}${OCL_OUTPUT_EXTENSION})
  set(OUTPUT_FILE ${TMP_DIR}/${TMP_RTL_FILE})
  get_filename_component(FILENAME ${INPUT_CL} NAME)
  set(OUTPUT_BC_FILE ${FILENAME}.bc)
  add_custom_command(OUTPUT ${OUTPUT_FILE}
    DEPENDS ${CLANG} common_clang${BUILD_PLATFORM} ${OPT} ${INPUT_CL}
    COMMAND ${CLANG} ${CLANG_FLAGS} ${INPUT_CL}
            -o ${TMP_DIR}/${OUTPUT_BC_FILE}
    COMMAND ${OPT} ${TMP_DIR}/${OUTPUT_BC_FILE} -verify -llvm-equalizer
            -kernel-analysis -cl-loop-bound -deduce-max-dim -cl-loop-creator
            -loop-idiom -loop-deletion -add-implicit-args -resolve-wi-call
            -prepare-kernel-args -inline -dce -globaldce -instcombine
            -cleanup-wrapped-kernels
            -o ${OUTPUT_FILE}
    COMMENT "Compiling ${TMP_RTL_FILE}"
  )

  add_custom_target(${TMP_RTL_TARGET_NAME} ALL
    DEPENDS ${OUTPUT_FILE}
    VERBATIM
    SOURCES ${INPUT_CL}
  )

  set_target_properties(${TMP_RTL_TARGET_NAME} PROPERTIES
                        FOLDER ${BACKEND_FOLDER_NAME}/cl_library_kernel)

  set(${OUTPUT_RTL} ${OUTPUT_FILE} PARENT_SCOPE)
endfunction()

function(create_ocl_kernel_rtl_target INPUT_RTL OUTPUT_DIR)
  set(RTL_FILE ${RTL_TARGET_NAME}${OCL_OUTPUT_EXTENSION})
  set(OUTPUT_FILE ${OUTPUT_DIR}/${RTL_FILE})
  add_custom_command(OUTPUT ${OUTPUT_FILE}
    DEPENDS  ${OPT} ${TMP_RTL_TARGET_NAME} ${INPUT_RTL}
    COMMAND ${OPT} ${INPUT_RTL} -strip-intel-ip -o ${OUTPUT_FILE}
    COMMENT "Compiling ${RTL_FILE}"
  )

  add_custom_target(${RTL_TARGET_NAME} ALL
    DEPENDS ${OUTPUT_FILE}
    VERBATIM
    SOURCES ${INPUT_RTL}
  )

  set_target_properties(${RTL_TARGET_NAME} PROPERTIES
                        FOLDER ${BACKEND_FOLDER_NAME}/cl_library_kernel)

  install_to(${OUTPUT_FILE} DESTINATION lib/${OUTPUT_ARCH_SUFF}
             COMPONENT ocl-library-kernel)
  install_to(${OUTPUT_FILE} DESTINATION lib/${OUTPUT_EMU_SUFF}
             COMPONENT ocl-library-kernel)
endfunction()

function(create_ocl_kernel_target INPUT_RTL PROC ARCH OUTPUT_DIR)
  set(TARGET_NAME ${TARGET_NAME_PREFIX}${ARCH})
  set(OUTPUT_OBJ
      ${OUTPUT_DIR}/${TARGET_NAME}${OCL_PRECOMPILED_OUTPUT_EXTENSION})

  add_custom_command(OUTPUT ${OUTPUT_OBJ}
    DEPENDS ${LLC} ${TMP_RTL_TARGET_NAME} ${INPUT_RTL}
    COMMAND ${LLC} ${LLC_PARAMS} -mcpu=${PROC} ${INPUT_RTL} -o ${OUTPUT_OBJ}
    COMMENT "Building object ${TARGET_NAME}${OCL_PRECOMPILED_OUTPUT_EXTENSION}"
  )

  add_custom_target(${TARGET_NAME} ALL
    DEPENDS ${OUTPUT_OBJ}
    VERBATIM
    SOURCES ${INPUT_RTL}
  )

  if (OPENCL_INTREE_BUILD)
    add_dependencies(${TARGET_NAME} common_clang${BUILD_PLATFORM})
  endif()

  set_target_properties(${TARGET_NAME} PROPERTIES
                        FOLDER ${BACKEND_FOLDER_NAME}/cl_library_kernel)

  install_to(${OUTPUT_OBJ} DESTINATION lib/${OUTPUT_ARCH_SUFF}
             COMPONENT ocl-library-kernel)
  install_to(${OUTPUT_OBJ} DESTINATION lib/${OUTPUT_EMU_SUFF}
             COMPONENT ocl-library-kernel)
endfunction()

set(SOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/mem_copy_set.cl)
set(CLANG_CL_COMMON_COMPILER_FLAGS ${CLANG_CL_COMMON_FLAGS} -cl-kernel-arg-info)

add_ocl_kernel_tmp_rtl("${CLANG_CL_COMMON_COMPILER_FLAGS}" ${SOURCE_FILE}
                       ${INTERMEDIATE_OUTPUT_DIR} TMP_RTL)
create_ocl_kernel_rtl_target(${TMP_RTL} ${LIBRARY_OUTPUT_DIR})

create_ocl_kernel_target(${TMP_RTL} skx        ${PrefixAVX512}
                         ${LIBRARY_OUTPUT_DIR})
create_ocl_kernel_target(${TMP_RTL} core-avx2  ${PrefixAVX2}
                         ${LIBRARY_OUTPUT_DIR})
create_ocl_kernel_target(${TMP_RTL} corei7-avx ${PrefixAVX}
                         ${LIBRARY_OUTPUT_DIR})
create_ocl_kernel_target(${TMP_RTL} corei7     ${PrefixSSE}
                         ${LIBRARY_OUTPUT_DIR})
