cmake_minimum_required(VERSION 2.8.4)

include_directories(${ANTLR_INCLUDE_DIR})

set (ANTLR_SRC
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
  set( ANTLR_SRC ${ANTLR_SRC} dll.cpp)
endif(WIN32)

add_library(${TARGET_NAME} STATIC
  ${ANTLR_SRC}
)
