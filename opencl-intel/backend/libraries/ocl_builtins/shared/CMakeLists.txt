set (TARGET_DIR ${SHARED_RTL_NAME})

# intermediate directory for OpenCL compilation/link
set (INTERMEDIATE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})

list(APPEND CMD_ARGS -gen-ocl-bi )
# generate OCL builtin implementation
tblgenoclblt(OCL_TABLEGEN_SOURCE
  ${INTERMEDIATE_OUTPUT_DIR}
  ${TARGET_DIR}
  target-${SHARED_RTL_NAME}.td
  target-${SHARED_RTL_NAME}.cl
  ${CMD_ARGS}
)

# generate shared LL builtin implementations
set(SHARED_LL_TD_FILES
  GENERIC/ll_sub_group_block_assume_uniform.td
  shared/ll_subgroup_impl.td
)
set(SHARED_LL_IMPLS)
foreach(FILE ${SHARED_LL_TD_FILES})
  tblgenllblt(LL_OUTPUT
    ${INTERMEDIATE_OUTPUT_DIR}
    ${FILE}
  )
  list(APPEND SHARED_LL_IMPLS ${LL_OUTPUT})
endforeach(FILE)

# build .rtl file out of OCL code (by clang) and copy it to install directory
set (OUTPUT_FILE ${TARGET_NAME_SHARED}${OCL_OUTPUT_EXTENSION})
set (PROC core2)
# Compile all files and prepare a 'TMP_FILES' list of .rtl files
foreach (FILE ${OCL_TABLEGEN_SOURCE})
  compile( "${CLANG_CL_COMMON_COMPILER_FLAGS}" TMP_RTL ${PROC} ${FILE} ${INTERMEDIATE_OUTPUT_DIR})
  list (APPEND TMP_FILES ${TMP_RTL})
endforeach (FILE)

list(APPEND TMP_FILES ${CL_BUILTIN_SOURCE_DIR}/GENERIC/ll_intrinsics.ll)
add_custom_command (OUTPUT ${BUILTINS_SHARED_OUTPUT_DIR}/${OUTPUT_FILE}
  COMMENT "Linking ${INTERMEDIATE_OUTPUT_DIR}/${OUTPUT_FILE}"
  COMMAND ${LLVM_LINK} ${LLVM_LINK_PARAMS} -o ${BUILTINS_SHARED_OUTPUT_DIR}/${OUTPUT_FILE} ${TMP_FILES} ${SHARED_LL_IMPLS}
  DEPENDS ${LLVM_LINK} ${TMP_FILES} ${SHARED_LL_IMPLS}
  VERBATIM
  )

set (TARGET_DEPENDENCIES ${BUILTINS_SHARED_OUTPUT_DIR}/${OUTPUT_FILE})

# top-level target for OCL target
add_custom_target (${TARGET_NAME_SHARED} ALL
  DEPENDS ${TARGET_DEPENDENCIES}
  COMMENT "Target ${TARGET_NAME_SHARED} build completed"
  VERBATIM
  SOURCES ${OCL_TABLEGEN_SOURCE}
)

set_property(GLOBAL APPEND PROPERTY OCL_BUILTIN_LIBS ${TARGET_NAME_SHARED})

set_target_properties (${TARGET_NAME_SHARED} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/cl_builtins)

install_to (${BUILTINS_SHARED_OUTPUT_DIR}/${OUTPUT_FILE}
            DESTINATION lib
            COMPONENT ocl-builtins-shared)
