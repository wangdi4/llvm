//===- IRMover.h ------------------------------------------------*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LINKER_IRMOVER_H
#define LLVM_LINKER_IRMOVER_H

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
#include "Intel_DTrans/Analysis/DTransTypes.h"
#endif // INTEL_FEATURE_SW_DTRANS
#endif //INTEL_CUSTOMIZATION
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/FunctionExtras.h"
#include "llvm/ADT/StringRef.h" // INTEL
#include "llvm/IR/Intel_DopeVectorTypeInfo.h" // INTEL
#include <functional>

namespace llvm {
class Error;
class GlobalValue;
class Metadata;
class Module;
class StructType;
class TrackingMDRef;
class Type;

typedef std::pair<StructType *, DopeVectorTypeInfo *> StructDVType; // INTEL

class IRMover {
  struct StructTypeKeyInfo {
    struct KeyTy {
      ArrayRef<Type *> ETypes;
      bool IsPacked;
#if INTEL_CUSTOMIZATION
      StringRef Name;
      const Type *DVET; // Dope vector element type
      unsigned DVR;     // Dope vector rank
      KeyTy(ArrayRef<Type *> E, bool P, StringRef N, const Type *DVET,
            unsigned DVR);
      KeyTy(const StructDVType SDVTy);
#endif // INTEL_CUSTOMIZATION
      bool operator==(const KeyTy &that) const;
      bool operator!=(const KeyTy &that) const;
    };
#if INTEL_CUSTOMIZATION
    static StructDVType getEmptyKey();
    static StructDVType getTombstoneKey();
#endif // INTEL_CUSTOMIZATION
    static unsigned getHashValue(const KeyTy &Key);
#if INTEL_CUSTOMIZATION
    static unsigned getHashValue(const StructDVType SDVTy);
    static bool isEqual(const KeyTy &LHS, const StructDVType RHS);
    static bool isEqual(const StructDVType LHS, const StructDVType RHS);
#endif // INTEL_CUSTOMIZATION
  };

  /// Type of the Metadata map in \a ValueToValueMapTy.
  typedef DenseMap<const Metadata *, TrackingMDRef> MDMapT;

public:
  class IdentifiedStructTypeSet {
    // The set of opaque types is the composite module.
    DenseSet<StructType *> OpaqueStructTypes;

    // The set of identified but non opaque structures in the composite module.
#if INTEL_CUSTOMIZATION
    DenseSet<StructDVType, StructTypeKeyInfo> NonOpaqueStructTypes;
    // DopeVectorTypeInfo for accessing dope vector element type
    DopeVectorTypeInfo *DVTI = nullptr;
#endif // INTEL_CUSTOMIZATION
  public:
    void addNonOpaque(StructType *Ty);
    void switchToNonOpaque(StructType *Ty);
    void addOpaque(StructType *Ty);
#if INTEL_CUSTOMIZATION
    StructDVType findNonOpaque(ArrayRef<Type *> ETypes, bool IsPacked,
                               StringRef Name, StructType *STy);
#endif // INTEL_CUSTOMIZATION
    bool hasType(StructType *Ty);
#if INTEL_CUSTOMIZATION
    DopeVectorTypeInfo *getDVTI() { return DVTI; }
    void resetDVTI(void) { DVTI = nullptr; };
    void appendToDVTI(Module &M) {
      if (!DVTI)
        DVTI = new DopeVectorTypeInfo(M);
      else
        DVTI->appendToDopeVectorTypeMap(M);
    };
#endif // INTEL_CUSTOMIZATION
  };
  IRMover(Module &M);
#if INTEL_CUSTOMIZATION
  ~IRMover() {
    if (getDVTI())
      delete getDVTI();
  }
  DopeVectorTypeInfo *getDVTI() { return IdentifiedStructTypes.getDVTI(); }
  void resetDVTI(void) { IdentifiedStructTypes.resetDVTI(); }
  void appendToDVTI(Module &M) { IdentifiedStructTypes.appendToDVTI(M); }
#endif // INTEL_CUSTOMIZATION
  typedef std::function<void(GlobalValue &)> ValueAdder;
  using LazyCallback =
      llvm::unique_function<void(GlobalValue &GV, ValueAdder Add)>;

  /// Move in the provide values in \p ValuesToLink from \p Src.
  ///
  /// - \p AddLazyFor is a call back that the IRMover will call when a global
  ///   value is referenced by one of the ValuesToLink (transitively) but was
  ///   not present in ValuesToLink. The GlobalValue and a ValueAdder callback
  ///   are passed as an argument, and the callback is expected to be called
  ///   if the GlobalValue needs to be added to the \p ValuesToLink and linked.
  ///   Pass nullptr if there's no work to be done in such cases.
  /// - \p IsPerformingImport is true when this IR link is to perform ThinLTO
  ///   function importing from Src.
  Error move(std::unique_ptr<Module> Src, ArrayRef<GlobalValue *> ValuesToLink,
             LazyCallback AddLazyFor, bool IsPerformingImport);
  Module &getModule() { return Composite; }

private:
  Module &Composite;
  IdentifiedStructTypeSet IdentifiedStructTypes;
  MDMapT SharedMDs; ///< A Metadata map to use for all calls to \a move().
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_SW_DTRANS
  dtransOP::DTransTypeManager TM;
#endif // INTEL_FEATURE_SW_DTRANS
#endif //INTEL_CUSTOMIZATION
};

} // End llvm namespace

#endif
