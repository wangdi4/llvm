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
add_llvm_component_library(LLVMCore
  AbstractCallSite.cpp
  AsmWriter.cpp
  Assumptions.cpp
  Attributes.cpp
  AutoUpgrade.cpp
  BasicBlock.cpp
  BuiltinGCs.cpp
  Comdat.cpp
  ConstantFold.cpp
  ConstantRange.cpp
  Constants.cpp
  Core.cpp
  DIBuilder.cpp
  DataLayout.cpp
  DebugInfo.cpp
  DebugInfoMetadata.cpp
  DebugLoc.cpp
  DiagnosticHandler.cpp
  DiagnosticInfo.cpp
  DiagnosticPrinter.cpp
  Dominators.cpp
  FPEnv.cpp
  Function.cpp
  GCStrategy.cpp
  GVMaterializer.cpp
  Globals.cpp
  IRBuilder.cpp
  IRPrintingPasses.cpp
  SSAContext.cpp
  InlineAsm.cpp
  Instruction.cpp
  Instructions.cpp
# INTEL_CUSTOMIZATION
  Intel_VectorVariant.cpp
# end INTEL_CUSTOMIZATION
  IntrinsicInst.cpp
  LLVMContext.cpp
  LLVMContextImpl.cpp
  LLVMRemarkStreamer.cpp
  LegacyPassManager.cpp
  MDBuilder.cpp
  Mangler.cpp
  Metadata.cpp
  Module.cpp
  ModuleSummaryIndex.cpp
  Operator.cpp
  OptBisect.cpp
  Pass.cpp
  PassInstrumentation.cpp
  PassManager.cpp
  PassRegistry.cpp
  PassTimingInfo.cpp
  PrintPasses.cpp
  ProfDataUtils.cpp
  SafepointIRVerifier.cpp
  ProfileSummary.cpp
  PseudoProbe.cpp
  ReplaceConstant.cpp
  Statepoint.cpp
  StructuralHash.cpp
  Type.cpp
  TypedPointerType.cpp
  TypeFinder.cpp
  Use.cpp
  User.cpp
  Value.cpp
  ValueSymbolTable.cpp
  VectorBuilder.cpp
  Verifier.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/IR

  LINK_LIBS
  ${LLVM_PTHREAD_LIB}

  DEPENDS
  intrinsics_gen

  LINK_COMPONENTS
  BinaryFormat
  Remarks
  Support
  )
