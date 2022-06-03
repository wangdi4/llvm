set(LLVM_LINK_COMPONENTS
  FrontendOpenMP
  Support
  )

<<<<<<< HEAD
# INTEL_CUSTOMIZATION
# The clang-tidy additions for misc-homoglyph are marked INTEL_CUSTOMIZATION
# but there is an expected conflict when these changes are pulled down from
# community.
=======
>>>>>>> b94db7ed7eaf4a3b21f600653a09c55cab77b79f
add_subdirectory(ConfusableTable)

add_custom_command(
    OUTPUT Confusables.inc
    COMMAND make_confusable_table ${CMAKE_CURRENT_SOURCE_DIR}/ConfusableTable/confusables.txt ${CMAKE_CURRENT_BINARY_DIR}/Confusables.inc
    DEPENDS make_confusable_table ConfusableTable/confusables.txt)

add_custom_target(genconfusable DEPENDS Confusables.inc)
<<<<<<< HEAD
# end INTEL_CUSTOMIZATION

add_clang_library(clangTidyMiscModule
  DefinitionsInHeadersCheck.cpp
  Homoglyph.cpp # INTEL
=======

add_clang_library(clangTidyMiscModule
  DefinitionsInHeadersCheck.cpp
  Homoglyph.cpp
>>>>>>> b94db7ed7eaf4a3b21f600653a09c55cab77b79f
  MiscTidyModule.cpp
  MisleadingBidirectional.cpp
  MisleadingIdentifier.cpp
  MisplacedConstCheck.cpp
  NewDeleteOverloadsCheck.cpp
  NoRecursionCheck.cpp
  NonCopyableObjects.cpp
  NonPrivateMemberVariablesInClassesCheck.cpp
  RedundantExpressionCheck.cpp
  StaticAssertCheck.cpp
  ThrowByValueCatchByReferenceCheck.cpp
  UnconventionalAssignOperatorCheck.cpp
  UniqueptrResetReleaseCheck.cpp
  UnusedAliasDeclsCheck.cpp
  UnusedParametersCheck.cpp
  UnusedUsingDeclsCheck.cpp

  LINK_LIBS
  clangTidy
  clangTidyUtils

  DEPENDS
  omp_gen
<<<<<<< HEAD
  genconfusable # INTEL
=======
  genconfusable
>>>>>>> b94db7ed7eaf4a3b21f600653a09c55cab77b79f
  )

clang_target_link_libraries(clangTidyMiscModule
  PRIVATE
  clangAnalysis
  clangAST
  clangASTMatchers
  clangBasic
  clangLex
  clangSerialization
  clangTooling
  )
