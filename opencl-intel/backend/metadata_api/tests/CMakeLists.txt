cmake_minimum_required(VERSION 2.8.4)

set(TARGET_NAME test_metadataapi)

project (${TARGET_NAME})

add_executable(
  ${TARGET_NAME}
  MetaDataApiTest.cpp
)

include_directories(
  ${LLVM_MAIN_SRC_DIR}/utils/unittest/googletest/include
  ${BACKEND_ROOT_DIR}/metadata_api
)

target_link_libraries(
  ${TARGET_NAME}
  gtest
  gtest_main
  metadataapi
  LLVMSupport
  LLVMCore
  LLVMBitReader
  LLVMAsmParser
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_TESTS_FOLDER_NAME} )

install(FILES metadatatest.ll DESTINATION bin)
install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION bin)
