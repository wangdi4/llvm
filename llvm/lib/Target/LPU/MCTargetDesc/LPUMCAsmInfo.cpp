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
using namespace llvm;

void LPUMCAsmInfo::anchor() { }

LPUMCAsmInfo::LPUMCAsmInfo(StringRef TT) {
  PointerSize = CalleeSaveStackSlotSize = 8;

  CommentString = "#";

  UsesELFSectionDirectiveForBSS = true;

  // For this to work, LPURegisterInfo.td needs dwarf register numbers for
  // registers.  This enables .loc, but it also enables a lot of other things
  // that we have no plans to deal with...
  // SupportsDebugInformation = true;
}
