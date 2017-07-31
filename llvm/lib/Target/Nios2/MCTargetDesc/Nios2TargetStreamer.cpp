//===-- Nios2TargetStreamer.cpp - Nios2 Target Streamer Methods -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides Nios2 specific target streamer methods.
//
//===----------------------------------------------------------------------===//

#include "InstPrinter/Nios2InstPrinter.h"
#include "Nios2MCTargetDesc.h"
#include "Nios2TargetObjectFile.h"
#include "Nios2TargetStreamer.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbolELF.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ELF.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"

using namespace llvm;

Nios2TargetStreamer::Nios2TargetStreamer(MCStreamer &S)
    : MCTargetStreamer(S) {
}

Nios2TargetAsmStreamer::Nios2TargetAsmStreamer(MCStreamer &S,
                                             formatted_raw_ostream &OS)
    : Nios2TargetStreamer(S), OS(OS) {}
