cmake_minimum_required(VERSION 2.8.4)


include_directories(${ANTLR_INCLUDE_DIR})

set (TARGET_SOURCE_FILES
  ANTLRUtil.cpp
  BaseAST.cpp
  CommonToken.cpp
  NoViableAltForCharException.cpp
  TokenBuffer.cpp
  TokenStreamHiddenTokenFilter.cpp
  ASTFactory.cpp
  BitSet.cpp
  CommonAST.cpp
  MismatchedCharException.cpp
  Parser.cpp
  Token.cpp
  TokenStreamRewriteEngine.cpp
  ASTNULLType.cpp
  CharBuffer.cpp
  CommonASTWithHiddenTokens.cpp
  InputBuffer.cpp
  MismatchedTokenException.cpp
  RecognitionException.cpp
  TokenRefCount.cpp
  TokenStreamSelector.cpp
  ASTRefCount.cpp
  CharScanner.cpp
  CommonHiddenStreamToken.cpp
  LLkParser.cpp
  NoViableAltException.cpp
  String.cpp
  TokenStreamBasicFilter.cpp
  TreeParser.cpp
)

if (WIN32)
  set( TARGET_SOURCE_FILES ${TARGET_SOURCE_FILES} dll.cpp)
endif(WIN32)

add_library(${TARGET_NAME} STATIC
  ${TARGET_SOURCE_FILES}
)

if(APPLE)
  file(RELATIVE_PATH SOURCE_RELATIVE_DIR ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
  install( FILES ${TARGET_INCLUDE_FILES} ${TARGET_SOURCE_FILES}
           DESTINATION src/${SOURCE_RELATIVE_DIR}
         )
endif(APPLE)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/reflection )
