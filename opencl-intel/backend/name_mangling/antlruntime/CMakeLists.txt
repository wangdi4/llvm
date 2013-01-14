cmake_minimum_required(VERSION 2.8.4)

set (TARGET_NAME antlr)

set (ANTLR_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(src)

if(APPLE)
  file(GLOB ANTLR_INCLUDE_FILES ${ANTLR_INCLUDE_DIR}/antlr/*.hpp)
  install( FILES ${ANTLR_INCLUDE_FILES}
           DESTINATION src/${SOURCE_RELATIVE_DIR}/include/antlr
         )
endif(APPLE)