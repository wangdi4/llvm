//===-- LPUMCAsmInfo.cpp - LPU asm properties -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of the LPUMCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "LPUMCAsmInfo.h"
#include "llvm/ADT/StringRef.h"
#include "../InstPrinter/LPUInstPrinter.h"
using namespace llvm;

void LPUMCAsmInfo::anchor() { }

LPUMCAsmInfo::LPUMCAsmInfo(const Triple &T) {
  PointerSize = CalleeSaveStackSlotSize = 8;
  // For now
  HasDotTypeDotSizeDirective = false;
  HasSingleParameterDotFile = false;
  MaxInstLength = 8;
  MinInstAlignment = 8;
  DollarIsPC = true;

  UsesELFSectionDirectiveForBSS = true;

  // For this to work, LPURegisterInfo.td needs dwarf register numbers for
  // registers.  This enables .loc, but it also enables a lot of other things
  // that we have no plans to deal with...
  // SupportsDebugInformation = true;

  // Maybe someday
  // UseIntegratedAssembler = true;

  // Override the global directive when we're wrapping LPU assembly. Since
  // we cannot wrap symbols written by MCAsmStreamer::EmitSymbolAttribute,
  //we'll write our own global symbols
  if (LPUInstPrinter::WrapLpuAsm()) {
    GlobalDirective = "#\t.globl\t";
  }
}
