include_directories(..)
set(LLVM_LINK_COMPONENTS
  Support
  )

add_clang_library(clangParseIntel
  ParseSIMD.cpp
  ParseStmt.cpp
  ParseIntelPragma.cpp
  PPIntelDirectives.cpp
  CheckStmt.cpp
  )
