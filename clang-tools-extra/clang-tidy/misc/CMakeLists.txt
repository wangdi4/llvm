# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Modifications, Copyright (C) 2022 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may not
# use, modify, copy, publish, distribute, disclose or transmit this software or
# the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the
# License.
#
# end INTEL_CUSTOMIZATION
set(LLVM_LINK_COMPONENTS
  FrontendOpenMP
  Support
  )

setup_host_tool(clang-tidy-confusable-chars-gen CLANG_TIDY_CONFUSABLE_CHARS_GEN clang_tidy_confusable_chars_gen clang_tidy_confusable_chars_gen_target)

add_subdirectory(ConfusableTable)

include_directories(BEFORE "${CMAKE_CURRENT_SOURCE_DIR}/../../include-cleaner/include")

add_custom_command(
    OUTPUT Confusables.inc
    COMMAND ${clang_tidy_confusable_chars_gen} ${CMAKE_CURRENT_SOURCE_DIR}/ConfusableTable/confusables.txt ${CMAKE_CURRENT_BINARY_DIR}/Confusables.inc
    DEPENDS ${clang_tidy_confusable_chars_gen_target} ConfusableTable/confusables.txt)

add_custom_target(genconfusable DEPENDS Confusables.inc)

add_clang_library(clangTidyMiscModule
  ConstCorrectnessCheck.cpp
  CoroutineHostileRAIICheck.cpp
  DefinitionsInHeadersCheck.cpp
  ConfusableIdentifierCheck.cpp
  HeaderIncludeCycleCheck.cpp
  IncludeCleanerCheck.cpp
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
  UseAnonymousNamespaceCheck.cpp

  LINK_LIBS
  clangAnalysis
  clangTidy
  clangTidyUtils

  DEPENDS
  omp_gen
  genconfusable
  ClangDriverOptions
  )

clang_target_link_libraries(clangTidyMiscModule
  PRIVATE
  clangAnalysis
  clangAST
  clangASTMatchers
  clangBasic
  clangFormat
  clangLex
  clangSerialization
  clangTooling
  clangToolingInclusions
  clangToolingInclusionsStdlib
  )
target_link_libraries(clangTidyMiscModule
  PRIVATE
  clangIncludeCleaner
  )
