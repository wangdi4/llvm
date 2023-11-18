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

add_llvm_library(LLVMTableGenCommon STATIC OBJECT EXCLUDE_FROM_ALL
  Attributes.cpp
  CodeGenIntrinsics.cpp
  DirectiveEmitter.cpp
  IntrinsicEmitter.cpp
  RISCVTargetDefEmitter.cpp
  SDNodeProperties.cpp
  VTEmitter.cpp
# INTEL_COLLAB
  DirectivesEmitter.cpp
# end INTEL_COLLAB
  Intel_SVMLEmitter.cpp       # INTEL -- TODO: VEC to COLLAB
  Intel_LibmvecEmitter.cpp    # INTEL
  PARTIAL_SOURCES_INTENDED

  LINK_COMPONENTS
  Support
  TableGen
  )
set_target_properties(LLVMTableGenCommon PROPERTIES FOLDER "Tablegenning")

set(LLVM_LINK_COMPONENTS Support)

add_tablegen(llvm-min-tblgen LLVM_HEADERS
  TableGen.cpp
  $<TARGET_OBJECTS:obj.LLVMTableGenCommon>
  PARTIAL_SOURCES_INTENDED
  )
set_target_properties(llvm-min-tblgen PROPERTIES FOLDER "Tablegenning")

set(LLVM_LINK_COMPONENTS
  CodeGenTypes
  Support
  )

add_tablegen(llvm-tblgen LLVM
  DESTINATION "${LLVM_TOOLS_INSTALL_DIR}"
  EXPORT LLVM
  AsmMatcherEmitter.cpp
  AsmWriterEmitter.cpp
  AsmWriterInst.cpp
  CTagsEmitter.cpp
  CallingConvEmitter.cpp
  CodeEmitterGen.cpp
  CodeGenDAGPatterns.cpp
  CodeGenHwModes.cpp
  CodeGenInstAlias.cpp
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
  DisassemblerEmitter.cpp
  DXILEmitter.cpp
  DynamicTableEmitter.cpp
  ExegesisEmitter.cpp
  FastISelEmitter.cpp
  GlobalISelCombinerEmitter.cpp
  GlobalISelEmitter.cpp
  GlobalISelMatchTable.cpp
  GlobalISelMatchTableExecutorEmitter.cpp
  InfoByHwMode.cpp
  InstrInfoEmitter.cpp
  InstrDocsEmitter.cpp
  Intel_MAPatternsEmitter.cpp # INTEL
  OptEmitter.cpp
  OptParserEmitter.cpp
  OptRSTEmitter.cpp
  PredicateExpander.cpp
  PseudoLoweringEmitter.cpp
  CompressInstEmitter.cpp
  RegisterBankEmitter.cpp
  RegisterInfoEmitter.cpp
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
  $<TARGET_OBJECTS:obj.LLVMTableGenCommon>

  DEPENDS
  intrinsics_gen # via llvm-min-tablegen
  )
target_link_libraries(llvm-tblgen PRIVATE LLVMTableGenGlobalISel)
set_target_properties(llvm-tblgen PROPERTIES FOLDER "Tablegenning")
