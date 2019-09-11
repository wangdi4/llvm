use_eh(TRUE)

if( NOT DEFINED LLVM_PATH_FE )
    message( FATAL_ERROR "LLVM_PATH_FE is not specified. Please specify LLVM library location for backend using LLVM_PATH_FE parameter to CMAKE" )
endif()

set(LLVM_PATH ${LLVM_PATH_FE})
find_package(LLVM REQUIRED)

set(TARGET_NAME ocl-tblgen)
set(CLANG_BIN_PATH "${LLVM_BINARY_DIR}/clang")

add_definitions(
  -D CCLANG_INCLUDE_PATH=${CCLANG_DEV_INCLUDE_DIRS}
  -D CLANG_BIN_PATH=${CLANG_BIN_PATH}
  -D CL_BUILTIN_SOURCE_DIR=${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins
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


add_opencl_executable(${TARGET_NAME}
  OclBuiltinEmitter.cpp
  OclBuiltinsHeaderGen.cpp
  VectorizerTableGen.cpp
  TableGen.cpp
  CodeFormatter.cpp
  ConversionParser.cpp
  ClangUtils.cpp

  LINK_LIBS
  name_mangle
  ${LLVM_MODULE_LIBS}
)

if (OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} intrinsics_gen)
  # ocl-tblgen executes clang, so clang has to be available
  # before we can launch ocl-tblgen. Instead of adding
  # the clang dependece to all target invoking ocl-tblgen,
  # add clang dependency here.
  add_dependencies(${TARGET_NAME} clang)
endif()

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME} )
