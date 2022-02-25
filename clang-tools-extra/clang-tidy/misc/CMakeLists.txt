set(LLVM_LINK_COMPONENTS
  FrontendOpenMP
  Support
  )

# INTEL_CUSTOMIZATION
# The clang-tidy additions for misc-homoglyph are marked INTEL_CUSTOMIZATION
# but there is an expected conflict when these changes are pulled down from
# community.
add_subdirectory(ConfusableTable)

add_custom_command(
    OUTPUT Confusables.inc
    COMMAND make_confusable_table ${CMAKE_CURRENT_SOURCE_DIR}/ConfusableTable/confusables.txt ${CMAKE_CURRENT_BINARY_DIR}/Confusables.inc
    DEPENDS make_confusable_table ConfusableTable/confusables.txt)

add_custom_target(genconfusable DEPENDS Confusables.inc)
# end INTEL_CUSTOMIZATION

add_clang_library(clangTidyMiscModule
  DefinitionsInHeadersCheck.cpp
  Homoglyph.cpp # INTEL
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
  genconfusable # INTEL
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
