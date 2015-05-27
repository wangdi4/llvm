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

  CommentString = "//";

  AlignmentIsInBytes = false;
  UsesELFSectionDirectiveForBSS = true;
}
