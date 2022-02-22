//===--------------------------DTransUtils.h--------------------------------===//
//
// Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
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

#if !INTEL_FEATURE_SW_DTRANS
#error DTransUtils.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_ANALYSIS_DTRANSUTILS_H
#define INTEL_DTRANS_ANALYSIS_DTRANSUTILS_H

#include "llvm/ADT/SetVector.h"
#include "llvm/IR/GlobalAlias.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm {
class raw_ostream;
class CallBase;
class DataLayout;
class Function;
class Type;
class Value;
class Instruction;
class TargetLibraryInfo;
class StructType;

namespace dtrans {

struct MemfuncRegion;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
extern cl::opt<bool> DTransPrintAnalyzedTypes;
extern cl::opt<bool> DTransPrintAnalyzedCalls;
extern cl::opt<bool> DTransPrintImmutableAnalyzedTypes;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

extern cl::opt<bool> DTransOutOfBoundsOK;

extern cl::opt<bool> DTransUseCRuleCompat;

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

// Helper function that checks if at least one field in the structure \p StTy
// is an opaque pointer type, or if it contains a reference to an opaque
// pointer.
bool hasOpaquePointerFields(llvm::StructType *StTy);

// Return 'true' if the value is only used as the destination pointer of memset
// calls.
bool valueOnlyUsedForMemset(llvm::Value *V);

// Return 'true' if the value loaded, 'V',  is only stored to the same address
// from which it was loaded, 'LoadAddr' and does not escape through a function
// call. For example:
//   struct.x = struct.x + 1;
// In this case, the load of 'struct.x' is only used to store a new value into
// the memory location it was loaded from.
bool isLoadedValueUnused(Value *V, Value *LoadAddr);

// Return 'true' if "I" is either llvm.type_test or llvm.assume intrinsic.
bool isTypeTestRelatedIntrinsic(const Instruction *I);

// Trace back instruction sequence corresponding to the following code:
//     foo (..., int n, ...) {
//         struct s *s_ptr = malloc(c1 + c2 * n);
//     }
// Returns false if it cannot trace \p InVal back to constants and calculate
// the size.
bool traceNonConstantValue(Value *InVal, uint64_t ElementSize,
                           bool EndsInZeroSizedArray);

// Analysis to determine the set of fields of a structure that will be used by
// a MemIntrinsic call to determine whether the call is supported by DTrans.
//
// For supported cases, return 'true' and populate the \p RegionDesc structure
// with a description of the fields affected.
//
// Supported cases are cases where the MemIntrinsic will operate on a whole
// number of fields, with possible pre/post field padding bytes due to field
// alignment, of the input structure.
//
// \p DataLayout - The data layout.
// \p StructTy - The structure type to analyze.
// \p FieldNum - The number for the first field affected by the
//               MemIntrinsic call.
// \p PrePadBytes - If the actual pointer to the MemIntrinsic does not
//                  correspond to the address the field starts at, number of
//                  padding bytes prior to the field that is affected.
// \p AccessSizeVal - Parameter for the size operand of the MemIntrinsic call.
// \p [out] RegionDesc - Gets filled with description of the fields affected.
bool analyzePartialStructUse(const DataLayout &DL, StructType *StructTy,
                             size_t FieldNum, uint64_t PrePadBytes,
                             const Value *AccessSizeVal,
                             MemfuncRegion *RegionDesc);

// This is a specialized form of the analyzePartialStructUse function that is
// used to trigger the MemFuncNestedStructsPartialWrite safety bit when the
// fields affected by the MemIntrinsic call end on a field boundary within an
// inner structure, rather than on a boundary within the input structure type
// itself.
//
// \p DataLayout - The data layout.
// \p StructTy - The structure type to analyze.
// \p SetSize - Parameter for the size operand of the MemIntrinsic call.
bool analyzePartialAccessNestedStructures(const DataLayout &DL,
                                          StructType *StructTy, Value *SetSize);

/// Check if the called function has only one basic block that ends with
/// 'unreachable' instruction.
bool isDummyFuncWithUnreachable(const CallBase *Call,
                                const TargetLibraryInfo &TLI);

/// Check if the called function has two arguments ('this' pointer and an
/// integer size) and is dummy.
bool isDummyFuncWithThisAndIntArgs(const CallBase *Call,
                                   const TargetLibraryInfo &TLI);

/// Check if the called function has two arguments ('this' pointer and a
/// pointer) and is dummy.
bool isDummyFuncWithThisAndPtrArgs(const CallBase *Call,
                                   const TargetLibraryInfo &TLI);

/// There is a possibility that a call to be analyzed is inside a BitCast, in
/// which case we need to strip the pointer casting from the \p Call operand to
/// identify the Function. The call may also be using an Alias to a Function,
/// in which case we need to get the aliasee. If a function is found, return it.
/// Otherwise, return nullptr.
inline Function *getCalledFunction(const CallBase &Call) {
  Value *CalledValue = Call.getCalledOperand()->stripPointerCasts();
  if (auto *CalledF = dyn_cast<Function>(CalledValue))
    return CalledF;

  if (auto *GA = dyn_cast<GlobalAlias>(CalledValue))
    if (!GA->isInterposable())
      if (auto *AliasF =
              dyn_cast<Function>(GA->getAliasee()->stripPointerCasts()))
        return AliasF;

  return nullptr;
}

/// Compare two structure names for sorted output. If the structure does
/// not have a name, compare it by the printed representation. Treat named
/// structures as less than literal structures.
/// Returns 'true' if 'Ty1' < 'Ty2'
bool compareStructName(const llvm::StructType *Ty1,
                       const llvm::StructType *Ty2);

} // namespace dtrans
} // namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANSUTILS_H
