set(TARGET_NAME TestTypeConversion)

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${GTEST_INCLUDE_DIR}
  ${OCL_SOURCE_DIR}/backend/utils
  ${OCL_SOURCE_DIR}/backend/name_mangling
)

add_opencl_executable(${TARGET_NAME}
  EXCLUDE_FROM_ALL_BUILD

  TestTypeConversion.cpp
)

target_link_libraries(
  ${TARGET_NAME}
  OclBackendUtils
  gtest
  LLVMCore
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_TESTS_FOLDER_NAME} )

install(FILES TestTypeConversion.pm DESTINATION tests/type_conversion)
install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION tests/type_conversion)
