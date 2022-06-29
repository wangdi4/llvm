set(LLVM_LINK_COMPONENTS
  FrontendOpenMP
  Support
  )

<<<<<<< HEAD
if(LLVM_USE_HOST_TOOLS)
  build_native_tool(make-confusable-table make_confusable_table)
  set(make_confusable_table_target "${make_confusable_table}")
else()
  set(make_confusable_table $<TARGET_FILE:make-confusable-table>)
  set(make_confusable_table_target make-confusable-table)
endif()

add_subdirectory(ConfusableTable)


add_custom_command(
    OUTPUT Confusables.inc
    COMMAND ${make_confusable_table} ${CMAKE_CURRENT_SOURCE_DIR}/ConfusableTable/confusables.txt ${CMAKE_CURRENT_BINARY_DIR}/Confusables.inc
    DEPENDS ${make_confusable_table_target} ConfusableTable/confusables.txt)

add_custom_target(genconfusable DEPENDS Confusables.inc)

add_clang_library(clangTidyMiscModule
  DefinitionsInHeadersCheck.cpp
  ConfusableIdentifierCheck.cpp
=======
add_clang_library(clangTidyMiscModule
  DefinitionsInHeadersCheck.cpp
>>>>>>> 5fea31aaed3157a9e8db7bf97d0902d74d3dfadf
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
  genconfusable
=======
>>>>>>> 5fea31aaed3157a9e8db7bf97d0902d74d3dfadf
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
