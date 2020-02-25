//===- LoopOptReportSupport.cpp - Utils to support emitters -*- C++ -*------==//
//
// Copyright (C) 2019-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements a set of routines to support various emitters
// of loopopt and vectorizer related Loop Optimization Reports.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/BitVector.h"
#include "llvm/Analysis/Intel_LoopAnalysis/OptReport/LoopOptReportSupport.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "opt-report-support-utils"

namespace llvm {
namespace LoopOptReportSupport {
static int getMDNodeAsInt(const ConstantAsMetadata *CM) {
  return cast<ConstantInt>(CM->getValue())->getValue().getSExtValue();
}

// TODO (vzakhari 02/11/2019): this is an experimental implementation
//       of binary opt-report representation for tech.preview release.
//       I guess this implementation should actually belong to LoopOpt
//       and Vectorizer opt-report support libraries, so that here we just
//       query this libraries for the releavant parts of the stream.
std::string formatBinaryStream(LoopOptReport OptReport) {
  BitVector LoopBits(64);
  BitVector VecBits(64);

  uint32_t MvVersion = 0;
  uint32_t UnrollFactor = 1;

  // Return 0 stream, if there is no opt-report attached.
  if (!OptReport)
    return "";

  for (const LoopOptRemark Remark : OptReport.origin()) {
    const MDString *R = cast<MDString>(Remark.getOperand(0));
    std::string OriginString = std::string(R->getString());

    if (OriginString == "Remainder loop for vectorization") {
      VecBits.set(2);
      LLVM_DEBUG(dbgs() << "VecBits: bits 2-2 set to 1\n");
    } else if (OriginString == "Multiversioned loop") {
      // We only support DD multiversioning right now.
      LoopBits.set(17);
      LLVM_DEBUG(dbgs() << "LoopBits: bits 17-20 set to 1\n");
      // Loop with "Multiversioned loop" origin is the "optimized"
      // version #1, the default version is #0.
      // If we later meet "The loop has been multiversioned" for this
      // loop, this would mean version #1 was multiversioned again,
      // thus, it would be version #2.  There is currently no way
      // to recognize version #3.
      MvVersion++;
    }
  }

  for (const LoopOptRemark Remark : OptReport.remarks()) {
    const auto *R = cast<MDString>(Remark.getOperand(0));
    std::string FormatString = std::string(R->getString());

    if (FormatString == "LOOP WAS VECTORIZED") {
      // TODO (vzakhari 02/10/2019): bits 47-49 must specify main vectorization
      //       type, which does not seem to exist in the opt-report now.
      //       Default is 8-bit integer type.
      VecBits.set(0);
      LLVM_DEBUG(dbgs() << "VecBits: bits 0-0 set to 1\n");
    } else if (FormatString == "vectorization support: vector length %s") {
      const MDString *SM = nullptr;
      if (Remark.getNumOperands() >= 2)
        SM = dyn_cast<MDString>(Remark.getOperand(1));
      else
        llvm_unreachable("Missing 'vector length' operand.");

      assert(SM && "Expected string argument");
      // Use 1 vectorlength for release builds in case of incorrect
      // argument specification.
      uint32_t VecLen = SM ? std::stoi(std::string(SM->getString())) : 1u;
      uint32_t Log2VecLen = std::min(Log2_32(VecLen), 15u);
      uint32_t Mask[] { 0, Log2VecLen << 3 };
      VecBits.setBitsInMask(Mask);
      LLVM_DEBUG(dbgs() << "VecBits: bits 35-38 set to " << Log2VecLen << "\n");
    } else if (FormatString == "The loop has been multiversioned") {
      // We only support DD multiversioning right now.
      LoopBits.set(17);
      LLVM_DEBUG(dbgs() << "LoopBits: bits 17-20 set to 1\n");
      // This loop has "Multiversioned loop" origin, and it was
      // multiversioned again - version #2.
      if (MvVersion != 0)
        MvVersion++;
    } else if (FormatString == "LLorg: Loop has been completely unrolled" ||
               FormatString == "Loop completely unrolled") {
      // Set UnrollFactor to zero, so that all following "unroll by factor"
      // opt-reports are ignored.  UnrollFactor equal to zero means complete
      // unroll.
      UnrollFactor = 0;
    } else if (FormatString == "Loop has been unrolled by %d factor" ||
               FormatString == "LLorg: Loop has been unrolled by %d factor") {
      const ConstantAsMetadata *CM = nullptr;
      if (Remark.getNumOperands() >= 2)
        CM = dyn_cast<ConstantAsMetadata>(Remark.getOperand(1));
      else
        llvm_unreachable("Missing 'unroll factor' operand.");

      assert(CM && "Expected constant in argument");
      // In case of invalid argument use 1 for release build.
      int UF = CM ? getMDNodeAsInt(CM) : 1;
      // Treat invalid 0 unroll factor as 1.
      UF = std::max(UF, 1);
      UnrollFactor *= UF;
    }
  }

  if (MvVersion != 0) {
    MvVersion = std::min(MvVersion, 3u);
    uint32_t Mask[] { MvVersion << 21, 0 };
    LoopBits.setBitsInMask(Mask);
    LLVM_DEBUG(dbgs() << "LoopBits: bits 21-22 set to " << MvVersion << "\n");
  }

  if (UnrollFactor > 1) {
    // Set unroll factor.  Maximum value is 31.
    UnrollFactor = (UnrollFactor > 31) ? 0 : UnrollFactor;
    uint32_t Mask[] { 0 , UnrollFactor << 4 };
    LoopBits.setBitsInMask(Mask);
    LLVM_DEBUG(dbgs() <<
               "LoopBits: bits 36-40 set to " << UnrollFactor << "\n");
    // Set unroll type to UNROLLED (2).
    LoopBits.set(35);
    LLVM_DEBUG(dbgs() << "LoopBits: bits 34-35 set to 2\n");
  } else if (UnrollFactor == 0) {
    // Set unroll type to COMPLETELY_UNROLLED (1).
    LoopBits.set(34);
    LLVM_DEBUG(dbgs() << "LoopBits: bits 34-35 set to 1\n");
  }

  // Form a byte stream, where 64 loop bits are followed by 64 vector bits.
  std::string Stream;
  for (int Pos = 0; Pos < 64; Pos += 8) {
    uint8_t Byte = 0;
    for (int SubPos = 0; SubPos < 8; ++SubPos) {
      Byte |= (LoopBits.test(Pos + SubPos) << SubPos);
    }
    Stream.push_back(Byte);
  }
  for (int Pos = 0; Pos < 64; Pos += 8) {
    uint8_t Byte = 0;
    for (int SubPos = 0; SubPos < 8; ++SubPos) {
      Byte |= (VecBits.test(Pos + SubPos) << SubPos);
    }
    Stream.push_back(Byte);
  }

  assert(Stream.length() == 16 &&
         "Opt-report binary stream must be 16 bytes long.");

  LLVM_DEBUG(
      dbgs() << "Opt-report binary stream:\n";
      for (int Pos = 0; Pos < 16; ++Pos) {
        (dbgs() << "0x").write_hex(Stream[Pos]) << " ";
        if (((Pos + 1) % 4) == 0 && Pos != 15)
          dbgs() << "| ";
      }
      dbgs() << "\n";);

  return Stream;
}
} // namespace LoopOptReportSupport
} // namespace llvm
