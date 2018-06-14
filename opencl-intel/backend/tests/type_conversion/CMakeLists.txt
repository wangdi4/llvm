set(TARGET_NAME TestTypeConversion)

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${OCL_SOURCE_DIR}/externals/gtest/googletest/include
  ${OCL_SOURCE_DIR}/backend/utils
  ${OCL_SOURCE_DIR}/backend/name_mangling
)

add_executable(${TARGET_NAME}
  TestTypeConversion.cpp
)

target_link_libraries(
  ${TARGET_NAME}
  name_mangle
  OclBackendUtils
  gtest
  ${LLVM_MODULE_LIBS}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_TESTS_FOLDER_NAME} )

install(FILES TestTypeConversion.pm DESTINATION tests/type_conversion)
install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION tests/type_conversion)
