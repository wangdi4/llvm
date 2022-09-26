//===--- Intel_MCTrace.cpp - Machine Code Trace Support ---------*- C++ -*-===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the methods of MCTraceLine to support the optimal line
/// record in .trace section.
///
//===----------------------------------------------------------------------===//

#include "llvm/MC/Intel_MCTrace.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Support/EndianStream.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdint>

using namespace llvm;

void MCTraceLine::emitNonOptimalValue(MCStreamer &OS) const {
  // Emit delta line.
  OS.AddComment(traceback::getTagString(Tag));
  OS.emitIntValue(traceback::getTagEncoding(Tag), 1);
  auto Att = traceback::getAttributeForTag(Tag);
  OS.AddComment(traceback::getAttributeString(Att));
  OS.emitIntValue(DeltaLine, traceback::getAttributeSize(Att));

  // Emit delta PC (Use PC4 to cover the widest PC range).
  auto PCTag = traceback::TB_TAG_PC4;
  OS.AddComment(traceback::getTagString(PCTag));
  OS.emitIntValue(traceback::getTagEncoding(PCTag), 1);
  auto PCAtt = traceback::getAttributeForTag(PCTag);
  OS.AddComment(traceback::getAttributeString(PCAtt));
  OS.emitValue(DeltaPC, traceback::getAttributeSize(PCAtt));
}

void MCTraceLine::encode(raw_ostream &OS, int32_t DeltaLine, uint32_t DeltaPC) {
  // First, check if we can use the format CO1/CO2.
  auto CorrelationTag = traceback::getOptimalCorrelationTag(DeltaLine, DeltaPC);
  if (CorrelationTag) {
    // CO1 (1 byte short form correlation)
    //   - Tag (high 2 bits): always 10 (binary)
    //   - Delta PC (low 6 bits): unsigned delta PC value minus 1
    //   - Delta line is always 1 (not encoded)

    // CO2 (2 byte short form correlation)
    //   - Tag (high 2 bits): always 11 (binary)
    //   - PC delta (low 6 bits): unsigned PC delta value minus 1
    //   - Line delta (1 byte): signed value
    uint8_t FirstByte = traceback::getTagEncoding(CorrelationTag.value());
    FirstByte = FirstByte | DeltaPC;
    OS << FirstByte;
    switch (CorrelationTag.value()) {
    default:
      llvm_unreachable("Unexpected tag");
    case traceback::TB_TAG_CO1:
      return;
    case traceback::TB_TAG_CO2: {
      int8_t SecondByte = static_cast<int8_t>(DeltaLine);
      OS << SecondByte;
      return;
    }
    }
  }

  // Emit delta line.
  auto LineTag = traceback::getOptimalLineTag(DeltaLine);
  OS << traceback::getTagEncoding(LineTag);
  switch (LineTag) {
  default:
    llvm_unreachable("Unexpected tag");
  case traceback::TB_TAG_LN1:
    OS << static_cast<int8_t>(DeltaLine);
    break;
  case traceback::TB_TAG_LN2:
    support::endian::write<int16_t>(OS, DeltaLine, support::little);
    break;
  case traceback::TB_TAG_LN4:
    support::endian::write<int32_t>(OS, DeltaLine, support::little);
    break;
  }

  // Emit delta PC.
  auto PCTag = traceback::getOptimalPCTag(DeltaPC);
  OS << traceback::getTagEncoding(PCTag);
  switch (PCTag) {
  default:
    llvm_unreachable("Unexpected tag");
  case traceback::TB_TAG_PC1:
    OS << static_cast<uint8_t>(DeltaPC);
    break;
  case traceback::TB_TAG_PC2:
    support::endian::write<uint16_t>(OS, DeltaPC, support::little);
    break;
  case traceback::TB_TAG_PC4:
    support::endian::write<uint32_t>(OS, DeltaPC, support::little);
    break;
  }
}
