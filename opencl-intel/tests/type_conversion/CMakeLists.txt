cmake_minimum_required(VERSION 2.8.7)

set(TARGET_NAME TestTypeConversion)

include_directories(
  ${CMAKE_SOURCE_DIR}/externals/gtest/include
  ${CMAKE_SOURCE_DIR}/backend/utils
  ${CMAKE_SOURCE_DIR}/backend/name_mangling
)

link_directories( 
    ${LLVM_LIBRARY_DIR}
)


add_definitions(-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS)

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

if (NOT WIN32)
  if(NOT ANDROID)
    target_link_libraries(${TARGET_NAME}
        pthread
        dl
    )
  endif()
endif(NOT WIN32)

set( BACKEND_FOLDER_NAME       Backend )
set( BACKEND_TESTS_FOLDER_NAME  ${BACKEND_FOLDER_NAME}/Tests )
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_TESTS_FOLDER_NAME} )

install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION bin)
