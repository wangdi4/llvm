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
  BinaryFormat
  Core
  FrontendOpenMP
  Support
  )

# INTEL_CUSTOMIZATION
if (MSVC)
  set_source_files_properties(ASTContext.cpp PROPERTIES COMPILE_FLAGS /bigobj)
endif()
# end INTEL_CUSTOMIZATION

# FIXME: the entry points to the interpreter should be moved out of clangAST
# into the parser or Sema in order to allow the interpreter to be moved to
# another library which depends on clangAST.
clang_tablegen(Opcodes.inc
  -gen-clang-opcodes
  SOURCE Interp/Opcodes.td
  TARGET Opcodes)

clang_tablegen(AttrDocTable.inc -gen-clang-attr-doc-table
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../include/
  SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/../../include/clang/Basic/Attr.td
  TARGET ClangAttrDocTable)

add_clang_library(clangAST
  APValue.cpp
  ASTConcept.cpp
  ASTConsumer.cpp
  ASTContext.cpp
  ASTDiagnostic.cpp
  ASTDumper.cpp
  ASTImporter.cpp
  ASTImporterLookupTable.cpp
  ASTStructuralEquivalence.cpp
  ASTTypeTraits.cpp
  AttrDocTable.cpp
  AttrImpl.cpp
  Comment.cpp
  CommentBriefParser.cpp
  CommentCommandTraits.cpp
  CommentLexer.cpp
  CommentParser.cpp
  CommentSema.cpp
  ComparisonCategories.cpp
  ComputeDependence.cpp
  CXXInheritance.cpp
  DataCollection.cpp
  Decl.cpp
  DeclarationName.cpp
  DeclBase.cpp
  DeclCXX.cpp
  DeclFriend.cpp
  DeclGroup.cpp
  DeclObjC.cpp
  DeclOpenMP.cpp
  DeclPrinter.cpp
  DeclTemplate.cpp
  ParentMapContext.cpp
  Expr.cpp
  ExprClassification.cpp
  ExprConcepts.cpp
  ExprConstant.cpp
  ExprCXX.cpp
  ExprObjC.cpp
  ExternalASTMerger.cpp
  ExternalASTSource.cpp
  FormatString.cpp
  InheritViz.cpp
# INTEL_CUSTOMIZATION
  intel/IntelAST.cpp
# end INTEL_CUSTOMIZATION
  Interp/ByteCodeEmitter.cpp
  Interp/ByteCodeExprGen.cpp
  Interp/ByteCodeGenError.cpp
  Interp/ByteCodeStmtGen.cpp
  Interp/Context.cpp
  Interp/Descriptor.cpp
  Interp/Disasm.cpp
  Interp/EvalEmitter.cpp
  Interp/Frame.cpp
  Interp/Function.cpp
  Interp/Interp.cpp
  Interp/InterpBlock.cpp
  Interp/InterpFrame.cpp
  Interp/InterpStack.cpp
  Interp/InterpState.cpp
  Interp/Pointer.cpp
  Interp/PrimType.cpp
  Interp/Program.cpp
  Interp/Record.cpp
  Interp/Source.cpp
  Interp/State.cpp
  ItaniumCXXABI.cpp
  ItaniumMangle.cpp
  JSONNodeDumper.cpp
  Mangle.cpp
  MicrosoftCXXABI.cpp
  MicrosoftMangle.cpp
  NestedNameSpecifier.cpp
  NSAPI.cpp
  ODRDiagsEmitter.cpp
  ODRHash.cpp
  OpenMPClause.cpp
  OSLog.cpp
  ParentMap.cpp
  PrintfFormatString.cpp
  QualTypeNames.cpp
  Randstruct.cpp
  RawCommentList.cpp
  RecordLayout.cpp
  RecordLayoutBuilder.cpp
  ScanfFormatString.cpp
  SelectorLocationsKind.cpp
  Stmt.cpp
  StmtCXX.cpp
  StmtIterator.cpp
  StmtObjC.cpp
  StmtOpenMP.cpp
  StmtPrinter.cpp
  StmtProfile.cpp
  StmtViz.cpp
  TemplateBase.cpp
  TemplateName.cpp
  TextNodeDumper.cpp
  Type.cpp
  TypeLoc.cpp
  TypePrinter.cpp
  VTableBuilder.cpp
  VTTBuilder.cpp

  LINK_LIBS
  clangBasic
  clangLex

  DEPENDS
  ClangAttrDocTable
  Opcodes
  omp_gen
  )
