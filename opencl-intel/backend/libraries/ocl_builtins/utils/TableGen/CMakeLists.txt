use_eh(TRUE)
use_rtti(TRUE)

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
  ocl-tblgen
  OclBuiltinEmitter.cpp
  OclBuiltinsHeaderGen.cpp
  VectorizerTableGen.cpp
  TableGen.cpp
  CodeFormatter.cpp
  ConversionParser.cpp
  ClangUtils.cpp
)


target_link_libraries(ocl-tblgen
  name_mangle
  ${LLVM_MODULE_LIBS}
)

if (NOT WIN32)
  if( NOT ANDROID)
  target_link_libraries(ocl-tblgen pthread)
  endif()
endif (NOT WIN32)

if( CMAKE_CROSSCOMPILING )
  add_custom_target( ocl-tblgen-native ALL
    COMMAND make ocl-tblgen
    DEPENDS ConfigureNativeTargets
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/native
    COMMENT "LLVM Cross-Compile: Building Native ocl-tblgen tool"
  )
endif( CMAKE_CROSSCOMPILING )

