set(TARGET_NAME ref-ocl-tblgen)

use_eh(TRUE)

add_definitions(
  -D CCLANG_INCLUDE_PATH=${CCLANG_DEV_INCLUDE_DIRS}
  -D CLANG_BIN_PATH=${CLANG_BIN_PATH}
)

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${CL_API_HEADERS}
  ${OCL_SOURCE_DIR}/backend/dynamic_lib
  ${OCL_SOURCE_DIR}/backend/validations/OCLBuilder/
  ${OCL_SOURCE_DIR}/backend/validations/Common/
  ${OCL_SOURCE_DIR}/backend/name_mangling/
  ${OCL_SOURCE_DIR}/cl_api
)

add_opencl_executable(
  ${TARGET_NAME}
  EXCLUDE_FROM_ALL_BUILD

  OclBuiltinEmitter.cpp
  OclBuiltinsHeaderGen.cpp
  TableGen.cpp
  CodeFormatter.cpp
  ConversionParser.cpp
  ClangUtils.cpp
)

if (OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} intrinsics_gen)
  # ocl-tblgen executes clang, so clang has to be available
  # before we can launch ocl-tblgen. Instead of adding
  # the clang dependece to all target invoking ocl-tblgen,
  # add clang dependency here.
  add_dependencies(${TARGET_NAME} clang)
endif()

target_link_libraries(${TARGET_NAME}
  ${LLVM_MODULE_LIBS}
  LLVMTableGen
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER "Backend" )
