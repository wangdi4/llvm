set(TARGET_NAME test_metadataapi)

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${BACKEND_ROOT_DIR}/../externals/gtest/include
  ${BACKEND_ROOT_DIR}/metadata_api
)

set(TARGET_INCLUDE_FILES
    MetadataAPITestFixture.h
)

set(TARGET_SOURCE_FILES
    MetadataAPITest.cpp
)

add_executable(
  ${TARGET_NAME}
  ${TARGET_INCLUDE_FILES}
  ${TARGET_SOURCE_FILES}
)

target_link_libraries(
  ${TARGET_NAME}
  googletest
  metadataapi
  ${LLVM_MODULE_LIBS}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_TESTS_FOLDER_NAME} )

install(FILES
  metadatatest.ll
  test_metadataapi.pm
  DESTINATION tests/metadataapi)
install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION tests/metadataapi)
