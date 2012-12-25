cmake_minimum_required(VERSION 2.8)
use_eh(TRUE)

set(TARGET_NAME ReflectionModuleTest)

project (${TARGET_NAME})

include_directories(
  ${BACKEND_ROOT_DIR}/../externals/gtest/include
  ${CMAKE_SOURCE_DIR}/backend/name_mangling
  ${CMAKE_SOURCE_DIR}/backend/vectorizer/Driver/OpenCL
  ${CMAKE_SOURCE_DIR}/backend/vectorizer/Utils
  ../
)

link_directories(
    ${LLVM_LIBRARY_DIR}
)

add_executable(
  ${TARGET_NAME}
  TestBuiltinKeeper.cpp
)

target_link_libraries(
  ${TARGET_NAME}
  reflection_module
  googletest
  LLVMSupport
  LLVMVectorizer
  LLVMCore
  LLVMBitReader
  LLVMAsmParser
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_TESTS_FOLDER_NAME} )

install(FILES mybi.ll DESTINATION bin)
install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION bin)
