set(LLVM_LINK_COMPONENTS
  Support
  )

add_clang_unittest(FrontendTests
  ASTUnitTest.cpp
  FrontendActionTest.cpp
  CodeGenActionTest.cpp
<<<<<<< HEAD
  IntelOutputStreamTest.cpp
=======
  PCHPreambleTest.cpp
>>>>>>> e218fafc155154e1946723461ea64f1002b6b586
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
