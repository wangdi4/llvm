set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  Core
  Option
  Support
  TargetParser
  )

add_clang_library(clangDependencyScanning
  DependencyScanningFilesystem.cpp
  DependencyScanningService.cpp
  DependencyScanningWorker.cpp
  DependencyScanningTool.cpp
  ModuleDepCollector.cpp

  DEPENDS
  ClangDriverOptions

  LINK_LIBS
  clangAST
  clangBasic
  clangCodeGen
  clangDriver
  clangFrontend
  clangLex
<<<<<<< HEAD
  clangParse
  clangSema
=======
>>>>>>> a2d1611b7b78b62b573ab04b09c106933007a4d7
  clangSerialization
  clangTooling
  )
