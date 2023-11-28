set(TARGET_NAME TestTypeConversion)

include_directories(
  ${LLVM_INCLUDE_DIRS} ${OCL_SOURCE_DIR}/backend/utils
  ${OCL_SOURCE_DIR}/backend/name_mangling
  ${OCL_SOURCE_DIR}/tests/test_utils/export)

add_opencl_executable(${TARGET_NAME}
  EXCLUDE_FROM_ALL_BUILD
  TestTypeConversion.cpp
  LINK_LIBS
  OclBackendUtils
  llvm_gtest
  LLVMCore)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER
                                                ${BACKEND_TESTS_FOLDER_NAME})

install(FILES TestTypeConversion.pm DESTINATION tests/type_conversion)
install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION tests/type_conversion)
