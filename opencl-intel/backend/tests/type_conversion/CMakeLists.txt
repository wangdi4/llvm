set(TARGET_NAME TestTypeConversion)

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/externals/gtest/include
  ${CMAKE_SOURCE_DIR}/backend/utils
  ${CMAKE_SOURCE_DIR}/backend/name_mangling
)

add_executable(${TARGET_NAME}
  TestTypeConversion.cpp
)

target_link_libraries(
  ${TARGET_NAME}
  name_mangle
  OclBackendUtils
  googletest
  ${LLVM_MODULE_LIBS}
)

if( ANDROID ) 
  target_link_libraries( ${TARGET_NAME} gnustl_shared )
  set_property(TARGET ${TARGET_NAME} APPEND_STRING PROPERTY LINK_FLAGS " -pie")
endif( ANDROID )

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_TESTS_FOLDER_NAME} )

install(FILES TestTypeConversion.pm DESTINATION tests/type_conversion)
install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION tests/type_conversion)
