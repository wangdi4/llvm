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

set(CLANG_TIDY_CONFUSABLE_CHARS_GEN "clang-tidy-confusable-chars-gen" CACHE
  STRING "Host clang-tidy-confusable-chars-gen executable. Saves building if cross-compiling.")

if(NOT CLANG_TIDY_CONFUSABLE_CHARS_GEN STREQUAL "clang-tidy-confusable-chars-gen")
  set(make_confusable_table ${CLANG_TIDY_CONFUSABLE_CHARS_GEN})
  set(make_confusable_table_target ${CLANG_TIDY_CONFUSABLE_CHARS_GEN})
elseif(LLVM_USE_HOST_TOOLS)
  build_native_tool(clang-tidy-confusable-chars-gen make_confusable_table)
  set(make_confusable_table_target "${make_confusable_table}")
else()
  set(make_confusable_table $<TARGET_FILE:clang-tidy-confusable-chars-gen>)
  set(make_confusable_table_target clang-tidy-confusable-chars-gen)
endif()

add_subdirectory(ConfusableTable)


add_custom_command(
    OUTPUT Confusables.inc
    COMMAND ${make_confusable_table} ${CMAKE_CURRENT_SOURCE_DIR}/ConfusableTable/confusables.txt ${CMAKE_CURRENT_BINARY_DIR}/Confusables.inc
    DEPENDS ${make_confusable_table_target} ConfusableTable/confusables.txt)

add_custom_target(genconfusable DEPENDS Confusables.inc)

add_clang_library(clangTidyMiscModule
  ConstCorrectnessCheck.cpp
  DefinitionsInHeadersCheck.cpp
  ConfusableIdentifierCheck.cpp
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
  clangAnalysis
  clangTidy
  clangTidyUtils

  DEPENDS
  omp_gen
  genconfusable
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
