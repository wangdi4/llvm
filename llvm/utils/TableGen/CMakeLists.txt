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
# INTEL_CUSTOMIZATION
set(INTEL_SOURCE_FILES_TO_BUILD)
intel_add_file(INTEL_SOURCE_FILES_TO_BUILD
  COMPLEMENT LLVM_OPTIONAL_SOURCES
# Add custom Intel files below.  Files listed here will be included in
# INTEL_CUSTOMIZATION build.
  )

# INTEL_FEATURE_CSA
intel_add_file(INTEL_SOURCE_FILES_TO_BUILD
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_CSA
  ${LLVM_MAIN_SRC_DIR}/Intel_OptionalComponents/CSA/utils/TableGen/CSAOpSizes.cpp
  )
# end INTEL_FEATURE_CSA
# end INTEL_CUSTOMIZATION

add_subdirectory(GlobalISel)

set(LLVM_LINK_COMPONENTS Support)

add_tablegen(llvm-tblgen LLVM
  DESTINATION "${LLVM_TOOLS_INSTALL_DIR}"
  EXPORT LLVM
  AsmMatcherEmitter.cpp
  AsmWriterEmitter.cpp
  AsmWriterInst.cpp
  Attributes.cpp
  CallingConvEmitter.cpp
  CodeEmitterGen.cpp
  CodeGenDAGPatterns.cpp
  CodeGenHwModes.cpp
  CodeGenInstruction.cpp
  CodeGenMapTable.cpp
  CodeGenRegisters.cpp
  CodeGenSchedule.cpp
  CodeGenTarget.cpp
# INTEL_CUSTOMIZATION
  ${INTEL_SOURCE_FILES_TO_BUILD}
# end INTEL_CUSTOMIZATION
  DAGISelEmitter.cpp
  DAGISelMatcherEmitter.cpp
  DAGISelMatcherGen.cpp
  DAGISelMatcherOpt.cpp
  DAGISelMatcher.cpp
  DecoderEmitter.cpp
  DFAEmitter.cpp
  DFAPacketizerEmitter.cpp
  DirectiveEmitter.cpp
  DisassemblerEmitter.cpp
  DXILEmitter.cpp
  ExegesisEmitter.cpp
  FastISelEmitter.cpp
  GICombinerEmitter.cpp
  GlobalISelEmitter.cpp
  InfoByHwMode.cpp
  InstrInfoEmitter.cpp
  InstrDocsEmitter.cpp
  IntrinsicEmitter.cpp
# INTEL_COLLAB
  DirectivesEmitter.cpp
# end INTEL_COLLAB
  Intel_LibmvecEmitter.cpp    # INTEL_CUSTOMIZATION
  Intel_MAPatternsEmitter.cpp # INTEL_CUSTOMIZATION
  Intel_SVMLEmitter.cpp       # INTEL_CUSTOMIZATION -- TODO: VEC to COLLAB
  OptEmitter.cpp
  OptParserEmitter.cpp
  OptRSTEmitter.cpp
  PredicateExpander.cpp
  PseudoLoweringEmitter.cpp
  CompressInstEmitter.cpp
  RegisterBankEmitter.cpp
  RegisterInfoEmitter.cpp
  SDNodeProperties.cpp
  SearchableTableEmitter.cpp
  SubtargetEmitter.cpp
  SubtargetFeatureInfo.cpp
  TableGen.cpp
  Types.cpp
  VarLenCodeEmitterGen.cpp
  X86DisassemblerTables.cpp
  X86EVEX2VEXTablesEmitter.cpp
  X86FoldTablesEmitter.cpp
  X86MnemonicTables.cpp
  X86ModRMFilters.cpp
  X86RecognizableInstr.cpp
  WebAssemblyDisassemblerEmitter.cpp
  CTagsEmitter.cpp
  )
target_link_libraries(llvm-tblgen PRIVATE LLVMTableGenGlobalISel)
set_target_properties(llvm-tblgen PROPERTIES FOLDER "Tablegenning")
