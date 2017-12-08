set(LLVM_LINK_COMPONENTS
  Support
  )

add_clang_unittest(FrontendTests
  ASTUnitTest.cpp
  CompilerInstanceTest.cpp
  FrontendActionTest.cpp
  CodeGenActionTest.cpp
  IntelOutputStreamTest.cpp #INTEL
  ParsedSourceLocationTest.cpp
  PCHPreambleTest.cpp
  )
target_link_libraries(FrontendTests
  clangAST
  clangBasic
  clangFrontend
  clangLex
  clangSema
  clangCodeGen
  clangFrontendTool
  )
