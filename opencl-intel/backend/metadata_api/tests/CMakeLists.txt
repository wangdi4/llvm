set(TARGET_NAME test_metadataapi)

# MetaDataApiTest has to use deprecated OpenCL API functions.
if (WIN32)
    add_definitions(
        -wd4996 # Suppress 'function': was declared deprecated'
    )
endif (WIN32)

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

install(FILES 
  metadatatest.ll 
  test_metadataapi.pm
  DESTINATION tests/metadataapi)
install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION tests/metadataapi)
