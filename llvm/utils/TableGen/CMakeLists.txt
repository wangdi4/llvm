# INTEL_CUSTOMIZATION
set(INTEL_SOURCE_FILES
  )
set(INTEL_CSA_SOURCE_FILES
  CSAOpSizes.cpp
  )

set(INTEL_SOURCE_FILES_TO_BUILD)
if (INTEL_CUSTOMIZATION)
#  if(";${LLVM_TARGETS_TO_BUILD};" MATCHES ";CSA;")
    list(APPEND INTEL_SOURCE_FILES ${INTEL_CSA_SOURCE_FILES})
#  else()
#    list(APPEND LLVM_OPTIONAL_SOURCES ${INTEL_CSA_SOURCE_FILES})
#  endif()

  set(INTEL_SOURCE_FILES_TO_BUILD ${INTEL_SOURCE_FILES})
else()
  list(APPEND LLVM_OPTIONAL_SOURCES
    ${INTEL_SOURCE_FILES}
    ${INTEL_CSA_SOURCE_FILES}
    )
endif()
# end INTEL_CUSTOMIZATION

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
  DFAPacketizerEmitter.cpp
  DisassemblerEmitter.cpp
  FastISelEmitter.cpp
  FixedLenDecoderEmitter.cpp
  GlobalISelEmitter.cpp
  InfoByHwMode.cpp
  InstrInfoEmitter.cpp
  InstrDocsEmitter.cpp
  IntrinsicEmitter.cpp
  Intel_DirectivesEmitter.cpp # INTEL_COLLAB
  Intel_LibmvecEmitter.cpp    # INTEL_CUSTOMIZATION
  Intel_MAPatternsEmitter.cpp # INTEL_CUSTOMIZATION
  Intel_SVMLEmitter.cpp       # INTEL_CUSTOMIZATION -- TODO: VEC to COLLAB
  OptParserEmitter.cpp
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
set_target_properties(llvm-tblgen PROPERTIES FOLDER "Tablegenning")
