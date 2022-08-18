# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Modifications, Copyright (C) 2021 Intel Corporation
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
  Core
  FrontendOpenMP
  MC
  Support
  )

if (MSVC)
# INTEL_CUSTOMIZATION
  set_source_files_properties(SemaSYCL.cpp PROPERTIES COMPILE_FLAGS /bigobj)
  set_source_files_properties(SemaAvailability.cpp PROPERTIES COMPILE_FLAGS /bigobj)
  set_source_files_properties(SemaDeclCXX.cpp PROPERTIES COMPILE_FLAGS /bigobj)
# end INTEL_CUSTOMIZATION
endif()

clang_tablegen(OpenCLBuiltins.inc -gen-clang-opencl-builtins
  SOURCE OpenCLBuiltins.td
  TARGET ClangOpenCLBuiltinsImpl
  )

clang_tablegen(SPIRVBuiltins.inc -gen-clang-spirv-builtins
 SOURCE SPIRVBuiltins.td
 TARGET ClangSPIRVBuiltinsImpl
 )

add_clang_library(clangSema
  AnalysisBasedWarnings.cpp
  CodeCompleteConsumer.cpp
  DeclSpec.cpp
  DelayedDiagnostic.cpp
  HLSLExternalSemaSource.cpp
  IdentifierResolver.cpp
  JumpDiagnostics.cpp
  MultiplexExternalSemaSource.cpp
  ParsedAttr.cpp
  Scope.cpp
  ScopeInfo.cpp
  Sema.cpp
  SemaAccess.cpp
  SemaAttr.cpp
  SemaAvailability.cpp
  SemaCXXScopeSpec.cpp
  SemaCast.cpp
  SemaChecking.cpp
  SemaCodeComplete.cpp
  SemaConcept.cpp
  SemaConsumer.cpp
  SemaCoroutine.cpp
  SemaCUDA.cpp
  SemaDecl.cpp
  SemaDeclAttr.cpp
  SemaDeclCXX.cpp
  SemaDeclObjC.cpp
  SemaExceptionSpec.cpp
  SemaExpr.cpp
  SemaExprCXX.cpp
  SemaExprMember.cpp
  SemaExprObjC.cpp
  SemaFixItUtils.cpp
  SemaInit.cpp
  SemaLambda.cpp
  SemaLookup.cpp
  SemaModule.cpp
  SemaObjCProperty.cpp
  SemaOpenMP.cpp
  SemaOverload.cpp
  SemaPseudoObject.cpp
  SemaRISCVVectorLookup.cpp
  SemaStmt.cpp
  SemaStmtAsm.cpp
  SemaStmtAttr.cpp
  SemaSYCL.cpp
  SemaTemplate.cpp
  SemaTemplateDeduction.cpp
  SemaTemplateInstantiate.cpp
  SemaTemplateInstantiateDecl.cpp
  SemaTemplateVariadic.cpp
  SemaType.cpp
  TypeLocBuilder.cpp

  # INTEL_CUSTOMIZATION
  intel/FPGAAnalyzeChannelsUsage.cpp
  # end INTEL_CUSTOMIZATION
  DEPENDS
  ClangOpenCLBuiltinsImpl
  ClangSPIRVBuiltinsImpl
  omp_gen

  LINK_LIBS
  clangAST
  clangAnalysis
  clangBasic
  clangEdit
  clangLex
  clangSupport
  )
