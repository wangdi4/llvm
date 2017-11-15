use_eh(TRUE)

if( NOT DEFINED LLVM_PATH_FE )
    message( FATAL_ERROR "LLVM_PATH_FE is not specified. Please specify LLVM library location for backend using LLVM_PATH_FE parameter to CMAKE" )
endif()

set(LLVM_PATH ${LLVM_PATH_FE})
find_package(LLVM REQUIRED)

set(TARGET_NAME ocl-tblgen)
set(CLANG_BIN_PATH "${LLVM_BINARY_DIR}/clang")
if (NOT EXISTS ${CLANG_BIN_PATH})
  message (FATAL_ERROR "There is no clang executable in LLVM binaries: ${CLANG_BIN_PATH}")
endif()
set(CCLANG_INCLUDE_PATH "${LLVM_INCLUDE_DIRS}/cclang")
if (NOT EXISTS ${CCLANG_INCLUDE_PATH})
  message (FATAL_ERROR "Common clang include directory is missing: ${CCLANG_INCLUDE_PATH}")
endif()

add_definitions(
  -D CCLANG_INCLUDE_PATH=${CCLANG_INCLUDE_PATH}
  -D CLANG_BIN_PATH=${CLANG_BIN_PATH}
)

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/backend/validations/OCLBuilder/
  ${CMAKE_SOURCE_DIR}/backend/validations/Common/
  ${CMAKE_SOURCE_DIR}/backend/name_mangling/
  ${CMAKE_SOURCE_DIR}/cl_api
  ${CL_API_HEADERS}
  ${CMAKE_SOURCE_DIR}/backend/dynamic_lib
)


add_executable(
  ${TARGET_NAME}
  OclBuiltinEmitter.cpp
  OclBuiltinsHeaderGen.cpp
  VectorizerTableGen.cpp
  TableGen.cpp
  CodeFormatter.cpp
  ConversionParser.cpp
  ClangUtils.cpp
)

target_link_libraries(${TARGET_NAME}
  name_mangle
  ${LLVM_MODULE_LIBS}
)

install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION tests/LITTests)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME} )
