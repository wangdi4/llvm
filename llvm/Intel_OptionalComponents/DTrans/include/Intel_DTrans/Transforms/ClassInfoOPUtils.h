//==ClassInfoOPUtils.h - common for SOAToAOSOP, MemManageOP and CodeAlignOP ==//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file has utility routines related to classes.
//
//===----------------------------------------------------------------------===//

#if !INTEL_FEATURE_SW_DTRANS
#error ClassInfoOPUtils.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_TRANSFORMS_CLASSINFOOPUTILS_H
#define INTEL_DTRANS_TRANSFORMS_CLASSINFOOPUTILS_H

namespace llvm {

class Function;

namespace dtransOP {

class TypeMetadataReader;
class DTransType;
class DTransStructType;

// Get class type of the given function if there is one.
DTransStructType *getClassType(const Function *F, TypeMetadataReader &MDReader);

// Returns true if Ty is pointer to pointer to a function.
bool isPtrToVFTable(DTransType *Ty);

// Returns field type of DTy struct if it has only one field.
DTransType *getSOASimpleBaseType(DTransType *DTy);

DTransStructType *getValidStructTy(DTransType *Ty);

// Returns type of pointee if 'Ty' is pointer.
DTransType *getPointeeType(DTransType *Ty);

// Returns true if 'Ty' is potential padding field that
// is created to fill gaps in structs.
bool isPotentialPaddingField(DTransType *Ty);

} // namespace dtransOP

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_CLASSINFOOPUTILS_H
