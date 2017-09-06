//===-- CSAMCAsmInfo.cpp - CSA asm properties -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of the CSAMCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "CSAMCAsmInfo.h"
#include "llvm/ADT/StringRef.h"
#include "../InstPrinter/CSAInstPrinter.h"
using namespace llvm;

void CSAMCAsmInfo::anchor() { }

CSAMCAsmInfo::CSAMCAsmInfo(const Triple &T) {
  CodePointerSize = CalleeSaveStackSlotSize = 8;
  // For now
  HasDotTypeDotSizeDirective = false;
  HasSingleParameterDotFile = false;
  MaxInstLength = 8;
  MinInstAlignment = 8;
  DollarIsPC = true;

  UsesELFSectionDirectiveForBSS = true;

  // For this to work, CSARegisterInfo.td needs dwarf register numbers for
  // registers.  This enables .loc, but it also enables a lot of other things
  // that we have no plans to deal with...
  // SupportsDebugInformation = true;

  UseIntegratedAssembler = true;
}
