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
  DFAEmitter.cpp
  DFAPacketizerEmitter.cpp
  DisassemblerEmitter.cpp
  ExegesisEmitter.cpp
  FastISelEmitter.cpp
  FixedLenDecoderEmitter.cpp
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
  OptParserEmitter.cpp
  PredicateExpander.cpp
  PseudoLoweringEmitter.cpp
  RISCVCompressInstEmitter.cpp
  RegisterBankEmitter.cpp
  RegisterInfoEmitter.cpp
  SDNodeProperties.cpp
  SearchableTableEmitter.cpp
  SubtargetEmitter.cpp
  SubtargetFeatureInfo.cpp
  TableGen.cpp
  Types.cpp
  X86DisassemblerTables.cpp
  X86EVEX2VEXTablesEmitter.cpp
  X86FoldTablesEmitter.cpp
  X86ModRMFilters.cpp
  X86RecognizableInstr.cpp
  WebAssemblyDisassemblerEmitter.cpp
  CTagsEmitter.cpp
  )
target_link_libraries(llvm-tblgen PRIVATE LLVMTableGenGlobalISel)
set_target_properties(llvm-tblgen PROPERTIES FOLDER "Tablegenning")
