set(TARGET_NAME test_metadataapi)

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${BACKEND_ROOT_DIR}/../externals/gtest/include
  ${BACKEND_ROOT_DIR}/metadata_api
)

add_executable(
  ${TARGET_NAME}
  MetaDataApiTest.cpp
)

target_link_libraries(
  ${TARGET_NAME}
  googletest
  metadataapi
  ${LLVM_MODULE_LIBS}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_TESTS_FOLDER_NAME} )

install(FILES metadatatest.ll DESTINATION tests/metadataapi)
install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION tests/metadataapi)
