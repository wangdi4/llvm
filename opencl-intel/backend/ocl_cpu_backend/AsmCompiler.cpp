// INTEL CONFIDENTIAL
//
// Copyright 2019 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "AsmCompiler.h"
#include "CompilationUtils.h"

#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCParser/AsmLexer.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Compression.h"
#include "llvm/Support/FileUtilities.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Mutex.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/WithColor.h"
#include <mutex>

using namespace llvm;

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

int AsmCompiler::assembleInput(const Target *TheTarget, SourceMgr &SrcMgr,
                               MCContext &Ctx, MCStreamer &Str, MCAsmInfo &MAI,
                               MCSubtargetInfo &STI, MCInstrInfo &MCII,
                               MCTargetOptions &MCOptions) {
  std::unique_ptr<MCAsmParser> Parser(createMCAsmParser(SrcMgr, Ctx, Str, MAI));
  std::unique_ptr<MCTargetAsmParser> TAP(
      TheTarget->createMCAsmParser(STI, *Parser, MCII, MCOptions));

  if (!TAP) {
    throw Exceptions::CompilerException(
        "This target does not support assembly parsing");
  }

  Parser->setShowParsedOperands(false);
  Parser->setTargetParser(*TAP);
  Parser->getLexer().setLexMasmIntegers(false);

  int Res = Parser->Run(false);

  return Res;
}

const Target *AsmCompiler::getTarget(const std::string &TripleName) {
  static sys::Mutex Lock;
  std::lock_guard<sys::Mutex> locked(Lock);
  std::string Err;
  const Target *TheTarget = TargetRegistry::lookupTarget(TripleName, Err);
  if (!TheTarget)
    throw Exceptions::CompilerException("Unable to get Target");

  if (!TheTarget->hasMCAsmParser()) {
    llvm::InitializeAllAsmParsers();
    TheTarget = TargetRegistry::lookupTarget(TripleName, Err);
    if (!TheTarget)
      throw Exceptions::CompilerException(
          "Unable to get Target with MCAsmParser");
  }
  return TheTarget;
}

// This function is based on llvm's llvm-mc.cpp
int AsmCompiler::compileAsmToObjectFile(std::unique_ptr<MemoryBuffer> BufferPtr,
                                        raw_fd_ostream *OS,
                                        const std::string &TripleName) {
  MCTargetOptions MCOptions;

  const Target *TheTarget = getTarget(TripleName);

  // Now that getTarget() has (potentially) replaced TripleName, it's safe to
  // construct the Triple object.
  Triple TheTriple(TripleName);

  SourceMgr SrcMgr;

  // Tell SrcMgr about this buffer, which is what the parser will pick up.
  SrcMgr.AddNewSourceBuffer(std::move(BufferPtr), SMLoc());

  std::unique_ptr<MCRegisterInfo> MRI(TheTarget->createMCRegInfo(TripleName));
  assert(MRI && "Unable to create target register info!");

  std::unique_ptr<MCAsmInfo> MAI(TheTarget->createMCAsmInfo(*MRI, TripleName,
                                                            MCOptions));
  assert(MAI && "Unable to create target asm info!");

  MAI->setRelaxELFRelocations(true);

  MAI->setPreserveAsmComments(false);

  // FIXME: This is not pretty. MCContext has a ptr to MCObjectFileInfo and
  // MCObjectFileInfo needs a MCContext reference in order to initialize itself.
  MCObjectFileInfo MOFI;
  MCContext Ctx(MAI.get(), MRI.get(), &MOFI, &SrcMgr);
  bool LargeCodeModel = false;
  bool PIC = true;
  MOFI.InitMCObjectFileInfo(TheTriple, PIC, Ctx, LargeCodeModel);

  Ctx.setGenDwarfForAssembly(false);

  SmallString<128> CWD;
  if (!sys::fs::current_path(CWD))
    Ctx.setCompilationDir(CWD);

  std::unique_ptr<buffer_ostream> BOS;
  std::unique_ptr<MCStreamer> Str;

  std::unique_ptr<MCInstrInfo> MCII(TheTarget->createMCInstrInfo());
  std::string MCPU = "";
  std::string FeaturesStr = "";
  std::unique_ptr<MCSubtargetInfo> STI(
      TheTarget->createMCSubtargetInfo(TripleName, MCPU, FeaturesStr));

  // Don't waste memory on names of temp labels.
  Ctx.setUseNamesOnTempLabels(false);

  assert(OS->supportsSeeking());

  MCCodeEmitter *CE = TheTarget->createMCCodeEmitter(*MCII, *MRI, Ctx);
  MCAsmBackend *MAB = TheTarget->createMCAsmBackend(*STI, *MRI, MCOptions);
  Str.reset(TheTarget->createMCObjectStreamer(
      TheTriple, Ctx, std::unique_ptr<MCAsmBackend>(MAB),
      MAB->createObjectWriter(*OS), std::unique_ptr<MCCodeEmitter>(CE), *STI,
      MCOptions.MCRelaxAll, MCOptions.MCIncrementalLinkerCompatible,
      /*DWARFMustBeAtTheEnd*/ false));

  // Use Assembler information for parsing.
  Str->setUseAssemblerInfoForParsing(true);

  int Res =
      assembleInput(TheTarget, SrcMgr, Ctx, *Str, *MAI, *STI, *MCII, MCOptions);

  return Res;
}

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
