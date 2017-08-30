set(LLVM_LINK_COMPONENTS
  MC
  MCParser
  Support
  )

add_clang_library(clangParse
  ParseAST.cpp
  ParseCXXInlineMethods.cpp
  ParseDecl.cpp
  ParseDeclCXX.cpp
  ParseExpr.cpp
  ParseExprCXX.cpp
  ParseInit.cpp
  ParseObjc.cpp
  ParseOpenMP.cpp
  ParsePragma.cpp
  ParseStmt.cpp
  ParseStmtAsm.cpp
  ParseTemplate.cpp
  ParseTentative.cpp
  Parser.cpp
# INTEL_CUSTOMIZATION
  intel/ParseSIMD.cpp
  intel/ParseStmt.cpp
  intel/ParseIntelExpr.cpp
  intel/ParseIntelPragma.cpp
  intel/PPIntelDirectives.cpp
  intel/CheckStmt.cpp
# end INTEL_CUSTOMIZATION

  LINK_LIBS
  clangAST
  clangBasic
  clangLex
  clangSema
  )
