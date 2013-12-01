cmake_minimum_required(VERSION 2.8.7)

set(TARGET_NAME test_metadataapi)

project (${TARGET_NAME})

include_directories(
  ${BACKEND_ROOT_DIR}/../externals/gtest/include
  ${BACKEND_ROOT_DIR}/metadata_api
)

link_directories(
    ${LLVM_LIBRARY_DIR}
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

if (NOT WIN32)
  if (NOT ANDROID)
    target_link_libraries(${TARGET_NAME} pthread dl)
  endif()
endif (NOT WIN32)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_TESTS_FOLDER_NAME} )

install(FILES metadatatest.ll DESTINATION bin)
install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION bin)
