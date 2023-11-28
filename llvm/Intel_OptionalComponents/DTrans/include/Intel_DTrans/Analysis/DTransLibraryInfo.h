//===---------DTransLibraryInfo.h - Library function information-----------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares a class that is used to provide DTrans type information
// about library and intrinsic functions to support using them without having
// DTrans metadata on them. This is used because these functions have
// standardized type interfaces, but may be inserted by arbitrary passes which
// won't need to know about DTrans metadata.

#if !INTEL_FEATURE_SW_DTRANS
#error DTransLibraryInfo.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_ANALYSIS_DTRANSLIBRARYINFO_H
#define INTEL_DTRANS_ANALYSIS_DTRANSLIBRARYINFO_H

#include "llvm/Analysis/TargetLibraryInfo.h"

#include <map>

namespace llvm {
class Function;
class Module;
class TargetLibraryInfo;

namespace dtransOP {

class DTransArrayType;
class DTransFunctionType;
class DTransStructType;
class DTransType;
class DTransTypeManager;
class DTransPointerType;

class DTransLibraryInfo {
public:
  DTransLibraryInfo(
      DTransTypeManager &TM,
      std::function<const TargetLibraryInfo &(const Function &)> GetTLI);

  void initialize(Module &M);

  DTransFunctionType *getDTransFunctionType(const Function *F) const;
  DTransType *getFunctionReturnType(const Function *F) const;
  DTransType *getFunctionArgumentType(const Function *F, unsigned Idx) const;

  DTransPointerType *getDTransIOPtrType() const { return DTransIOPtrType; }

private:
  DTransFunctionType *getDTransFunctionTypeImpl(const Function *F) const;
  DTransFunctionType *getDTransFunctionTypeImpl(LibFunc TheLibFunc) const;
  DTransFunctionType *getDTransFunctionTypeImpl(Intrinsic::ID Id) const;

  DTransStructType *findIdentTStructType(Module &M);
  DTransPointerType *findIOPtrType(Module &M);

  DTransTypeManager &TM;
  std::function<const TargetLibraryInfo &(const Function &)> GetTLI;

  // Representation of various integer sized types in DTransType system
  DTransType *DTransI1Type = nullptr;
  DTransType *DTransI8Type = nullptr;
  DTransType *DTransI32Type = nullptr;
  DTransType *DTransI64Type = nullptr;
  DTransType *DTransSizeType = nullptr;
  DTransType *DTransMDType = nullptr;

  DTransType *DTransVoidType = nullptr;

  // Representation of various pointer to integer types in DTransType system
  DTransPointerType *DTransI8PtrType = nullptr;
  DTransPointerType *DTransI32PtrType = nullptr;
  DTransPointerType *DTransI64PtrType = nullptr;

  // Pointer to structure used for file I/O, such as %struct._IO_FILE*
  DTransPointerType *DTransIOPtrType = nullptr;

  // The PAROPT %struct.ident_t* in use for the external KMPC function calls
  // inserted by PAROPT.
  DTransPointerType *DTransIdentTPtrType = nullptr;

  // Some KMPC functions take a pointer to an [8 x i32] array.
  const uint32_t KMPCritcalNameArrayLen = 8;
  DTransArrayType *DTransKMPCriticalNameType = nullptr;
  DTransPointerType *DTransKMPCriticalNamePtrType = nullptr;

  // Cache of computed Function -> DTransFunctionType entries.
  mutable std::map<const Function *, DTransFunctionType *> FunctionCache;
};

} // end namespace dtransOP
} // end namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANSLIBRARYINFO_H
