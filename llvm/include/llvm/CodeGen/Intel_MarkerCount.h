#if INTEL_FEATURE_MARKERCOUNT
//===- Intel_MarkerCount.h - Marker Count Information -----------*- C++ -*-===//
//
// Copyright (C) 2016-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines a number of light weight data structures used to describe
// and track marker count information.
//
// Marker count is a form of forward-progress tracking that is equatable
// (portable and comparable) across binaries, so that forward progress can be
// compared even in the face of doing comparisions across binaries that have
// been compiled with different ISAs.
//
// It can be performance-friendly and location-accurate at the same time and
// currently it inserted at
//    1. Prolog/Epilog of the function
//    2. Loop header
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CODEGEN_MARKERCOUNT_H
#define LLVM_CODEGEN_MARKERCOUNT_H

#include <string>
#include <vector>

namespace llvm {

struct MCK {
  enum SimpleValueType : uint8_t {
    None = 0,
    Prolog = 1U,
    Epilog = 1U << 1,
    LoopHeader = 1U << 2,
    Mask = (1U << 3) - 1U
  };
};

class MarkerCount {
private:
  unsigned Kinds;
  static std::string getKindString(unsigned K) {
    switch (K) {
    case MCK::Prolog:
      return "markercount.prolog";
    case MCK::Epilog:
      return "markercount.epilog";
    case MCK::LoopHeader:
      return "markercount.loop_header";
    default:
      llvm_unreachable("unknown markercount kind!");
    }
  }
public:
    MarkerCount(unsigned K = 0) : Kinds(K) {}
    MarkerCount(const MarkerCount &) = default;
    operator unsigned () const { return Kinds; }
    bool operator == (const MarkerCount That) const { return Kinds == That.Kinds; }
    bool operator != (const MarkerCount That) const { return Kinds != That.Kinds; }
    bool hasKind(unsigned K) const { return (!K) || Kinds & K; }
    bool hasKinds(unsigned K) const { return (!K) || (Kinds & K) == K; }
    void setKinds(unsigned K) { Kinds = K & MCK::Mask; }
    void addKinds(unsigned K) { Kinds |= K & MCK::Mask; }
    void removeKinds(unsigned K) { Kinds &= ~(K & MCK::Mask); }
    void clearKinds() { Kinds = MCK::None; }
    explicit operator std::string() const {
      std::string S;
      for (unsigned K : {MCK::Prolog, MCK::Epilog, MCK::LoopHeader})
        if (hasKind(K))
          S += " " + getKindString(K);
      return S;
    }
};

} // end namespace llvm

#endif // LLVM_CODEGEN_MARKERCOUNT_H
#endif // INTEL_FEATURE_MARKERCOUNT
