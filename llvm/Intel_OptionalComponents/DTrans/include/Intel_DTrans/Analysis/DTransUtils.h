//===--------------------------DTransUtils.h--------------------------------===//
//
// Copyright (C) 2020-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
//
// General utilities for DTrans analysis and transforms that do not require the
// type/class definitions from DTrans.h.
//
// ===--------------------------------------------------------------------=== //

#if !INTEL_INCLUDE_DTRANS
#error DTransUtils.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#ifndef INTEL_DTRANS_ANALYSIS_DTRANSUTILS_H
#define INTEL_DTRANS_ANALYSIS_DTRANSUTILS_H

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm {
class raw_ostream;
class CallBase;
class Function;
class Type;
class Value;

namespace dtrans {

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
extern cl::opt<bool> DTransPrintAnalyzedTypes;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

extern cl::opt<bool> DTransOutOfBoundsOK;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// This template function is to support dumping a collection of items in
// lexically sorted order so that debug traces do not change due to pointer
// addresses changing. This is done by first printing each item within the
// iterator range to a string, sorting the strings, and then outputting the
// strings to the output stream.
//
// \p OS               - Output stream
// \p Begin and \p End - Range of elements to be output.
// \p ToString         - Function that converts an element of the collection to
//                       a string. Signature should be:
//                       std::string F(IterType::value_type V);
// \p Separator        - Delimiter to use between elements output.
template <typename IterType, class Fn>
static void printCollectionSorted(raw_ostream &OS, IterType Begin, IterType End,
                                  const char *Separator, Fn ToString) {
  SmallVector<std::string, 8> Outputs;
  for (auto I = Begin; I != End; ++I)
    Outputs.emplace_back(ToString(*I));

  std::sort(Outputs.begin(), Outputs.end());
  bool First = true;
  for (auto &Str : Outputs) {
    if (!First)
      OS << Separator;
    OS << Str;
    First = false;
  }
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

/// There is a possibility that a call to be analyzed is inside a BitCast, in
/// which case we need to strip the pointer casting from the \p Call operand to
/// identify the Function. The call may also be using an Alias to a Function,
/// in which case we need to get the aliasee. If a function is found, return it.
/// Otherwise, return nullptr.
Function *getCalledFunction(const CallBase &Call);

// Helper function to check whether \p Ty is a pointer type, or contains a
// reference to a pointer type.
bool hasPointerType(llvm::Type *Ty);

// Return 'true' if the value is only used as the destination pointer of memset
// calls.
bool valueOnlyUsedForMemset(llvm::Value *V);

} // namespace dtrans
} // namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANSUTILS_H
