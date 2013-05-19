cmake_minimum_required(VERSION 2.8.7)

if( ANDROID )
    set( BACKEND_BUILD_TESTS            OFF  CACHE STRING "")
    set( BACKEND_BUILD_VERIFICATION_LIB OFF  CACHE STRING "")
else( ANDROID )
    set( BACKEND_BUILD_TESTS            ON  CACHE STRING "")
    set( BACKEND_BUILD_VERIFICATION_LIB ON  CACHE STRING "")
endif( ANDROID )

set(TARGET_NAME name_mangle)

set(ANTLR_COMMAND
  java -cp ${CMAKE_CURRENT_SOURCE_DIR}/antlruntime/lib/antlr.jar antlr.Tool
)

project (${TARGET_NAME})

set(NAME_MANGLE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if (NOT APPLE)
    add_subdirectory(unittests)
endif (NOT APPLE)

add_subdirectory(antlruntime)

add_custom_command(
  OUTPUT
  ${CMAKE_CURRENT_SOURCE_DIR}/gen/DemangleParser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/gen/DemangleParser.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/gen/DemangleLexer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/gen/DemangleLexer.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/gen/DemangleParserTokenTypes.hpp
  COMMAND ${ANTLR_COMMAND}
  -o ${CMAKE_CURRENT_SOURCE_DIR}/gen ${CMAKE_CURRENT_SOURCE_DIR}/demangler.g
  DEPENDS demangler.g
  COMMENT "running ${ANTLR_COMMAND} on ${CMAKE_CURRENT_SOURCE_DIR}/demangler.g"
)

set(TARGET_SOURCE_FILES
  Type.cpp
  Demangler.cpp
  Mangler.cpp
  FunctionDescriptor.cpp
  TypeCast.cpp
  Utils.cpp
  gen/DemangleLexer.cpp
  gen/DemangleParser.cpp
)

set(TARGET_INCLUDE_FILES
  Type.h
  NameMangleAPI.h
  FunctionDescriptor.h
  Refcount.h
  TypeCast.h
  Utils.h
  gen/DemangleLexer.hpp
  gen/DemangleParser.hpp
  gen/DemangleParserTokenTypes.hpp
)

add_library(
  ${TARGET_NAME}
  STATIC
  ${TARGET_SOURCE_FILES}
  ${TARGET_INCLUDE_FILES}
)

if (NOT APPLE)
  include_directories(
    ${LLVM_MAIN_SRC_DIR}/utils/unittest/googletest/include
  )
endif (NOT APPLE)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/antlruntime/include
  ${CMAKE_CURRENT_SOURCE_DIR}/gen
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
  ${TARGET_NAME}
  antlr
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/reflection )

if(APPLE)
  file(RELATIVE_PATH SOURCE_RELATIVE_DIR ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
  install( FILES ${TARGET_INCLUDE_FILES} ${TARGET_SOURCE_FILES}
           DESTINATION src/${SOURCE_RELATIVE_DIR}
         )
endif(APPLE)
