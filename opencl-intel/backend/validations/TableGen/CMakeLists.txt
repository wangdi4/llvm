use_eh(TRUE)

set(TARGET_NAME ref-ocl-tblgen)
set(CLANG_BIN_PATH "${LLVM_BINARY_DIR}/clang")
set(CLANG_INCLUDE_PATH "${BACKEND_ROOT_DIR}/clang_headers/")

add_definitions(
  -D CLANG_INCLUDE_PATH=${CLANG_INCLUDE_PATH}
  -D CLANG_BIN_PATH=${CLANG_BIN_PATH}
)

include_directories(
  ${CMAKE_SOURCE_DIR}/backend/validations/OCLBuilder/
  ${CMAKE_SOURCE_DIR}/backend/validations/Common/
  ${CMAKE_SOURCE_DIR}/backend/name_mangling/
  ${CMAKE_SOURCE_DIR}/cl_api
  ${CMAKE_SOURCE_DIR}/backend/dynamic_lib
)

link_directories(
    ${LLVM_LIBRARY_DIR}
)

add_executable(
  ${TARGET_NAME}
  OclBuiltinEmitter.cpp
  OclBuiltinsHeaderGen.cpp
  TableGen.cpp
  CodeFormatter.cpp
  ConversionParser.cpp
  ClangUtils.cpp
)

add_dependencies(${TARGET_NAME} 
	clang
)

target_link_libraries(${TARGET_NAME}
  name_mangle
  ${LLVM_MODULE_LIBS}
)

if (NOT WIN32)
  if(NOT ANDROID)
    target_link_libraries(${TARGET_NAME} pthread dl)
  endif(NOT ANDROID)
endif (NOT WIN32)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER "Backend" )
