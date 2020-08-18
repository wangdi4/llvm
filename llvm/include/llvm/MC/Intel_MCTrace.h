//===--- Intel_MCTrace.h - Machine Code Trace Support -----------*- C++ -*-===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the MCTraceLine to support the optimal
// line record in .trace section.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_MC_MCTRACE_H
#define LLVM_MC_MCTRACE_H

#include "llvm/BinaryFormat/Intel_Trace.h"

namespace llvm {
class MCExpr;
class MCStreamer;
class raw_ostream;

class MCTraceLine {
private:
  traceback::Tag Tag;
  int DeltaLine;
  const MCExpr *DeltaPC;

public:
  MCTraceLine(traceback::Tag Tag, int DeltaLine, const MCExpr *DeltaPC)
      : Tag(Tag), DeltaLine(DeltaLine), DeltaPC(DeltaPC) {}

  MCTraceLine() = delete;
  MCTraceLine(const MCTraceLine &) = delete;
  MCTraceLine &operator=(const MCTraceLine &) = delete;

  int getDeltaLine() const { return DeltaLine; }
  const MCExpr *getDeltaPC() const { return DeltaPC; }

  /// Emit the pair of DeltaLine and DeltaPC with non-optimal value(we can not
  /// know the accurate DeltaPC until finishing layout).
  void emitNonOptimalValue(MCStreamer &OS) const;

  /// Utility function to encode a TraceBack pair of DeltaLine and DeltaPC.
  static void encode(raw_ostream &OS, int DeltaLine, unsigned DeltaPC);
};
} // namespace llvm

#endif // LLVM_MC_MCTRACE_H
