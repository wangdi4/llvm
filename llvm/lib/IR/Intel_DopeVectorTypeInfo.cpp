//===------- Intel_DopeVectorTypeInfo.cpp ---------------------------------===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Intel_DopeVectorTypeInfo.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "dopevectortype"

void DopeVectorTypeInfo::loadDopeVectorTypeMap(Module &M) {

  // Return the element type the 'MDN' with the given 'Index'.
  // Return 'nullptr' if the metadata is malformed.
  auto GetType = [](MDNode *MDN, unsigned Index) -> const Type * {
    auto *CI = dyn_cast<ConstantAsMetadata>(MDN->getOperand(Index));
    if (!CI)
      return nullptr;
    auto *CV = dyn_cast<Constant>(CI->getValue());
    if (!CV)
      return nullptr;
    return CV->getType();
  };

  NamedMDNode *NMDN = M.getNamedMetadata("ifx.types.dv");
  if (!NMDN) {
    LLVM_DEBUG(dbgs() << "DVTYPE: No metadata found.\n");
    return;
  }
  for (auto *MDN : NMDN->operands()) {
    const Type *DVType = GetType(MDN, 0);
    if (!DVType) {
      DopeVectorTypeMap.clear();
      MapCorrectlyInitialized = false;
      return;
    }
    const Type *DVElementType = GetType(MDN, 1);
    if (!DVElementType) {
      DopeVectorTypeMap.clear();
      MapCorrectlyInitialized = false;
      return;
    }
    if (DopeVectorTypeMap.count(DVType)) {
      if (DopeVectorTypeMap[DVType] != DVElementType) {
        LLVM_DEBUG(dbgs() << "DTYPE: Mismatched element types ");
        LLVM_DEBUG({
          dbgs() << "for ";
          DVType->print(dbgs(), /*IsForDebug*/ false,
                        /*NoDetails=*/true);
        });
        LLVM_DEBUG(dbgs() << "\n");
        DopeVectorTypeMap.clear();
        MapCorrectlyInitialized = false;
        return;
      }
    } else {
      DopeVectorTypeMap.insert({DVType, DVElementType});
    }
  }
  if (DopeVectorTypeMap.empty()) {
    LLVM_DEBUG(dbgs() << "DVTYPE: Empty metadata list.\n");
    return;
  }
  LLVM_DEBUG(print());
}

bool DopeVectorTypeInfo::isDopeVectorType(const Type *Ty) {
  if (!MapCorrectlyInitialized)
    return false;
  return DopeVectorTypeMap.count(Ty);
}

unsigned DopeVectorTypeInfo::getDopeVectorArrayRank(const Type *Ty) {
  const unsigned int DVFieldCount = 7;
  if (!isDopeVectorType(Ty))
    return 0;
  auto STy = dyn_cast<StructType>(Ty);
  if (!STy)
    return 0;
  unsigned ContainedCount = STy->getNumContainedTypes();
  if (ContainedCount != DVFieldCount)
    return 0;
  llvm::Type *LastType = STy->getContainedType(ContainedCount - 1);
  auto *ArType = dyn_cast<ArrayType>(LastType);
  if (!ArType)
    return 0;
  return ArType->getArrayNumElements();
}

const Type *DopeVectorTypeInfo::getDopeVectorElementType(const Type *Ty) {
  if (!isDopeVectorType(Ty))
    return nullptr;
  return DopeVectorTypeMap[Ty];
}

void DopeVectorTypeInfo::print(void) {
  if (!MapCorrectlyInitialized) {
    dbgs() << "DVTYPE: Map not correctly initialized.\n";
    return;
  }
  for (auto &Pair : DopeVectorTypeMap) {
    dbgs() << "DVTYPE: ";
    Pair.first->print(dbgs(), /*IsForDebug=*/false,
                      /*NoDetails=*/true);
    dbgs() << " -> ";
    Pair.second->print(dbgs(), /*IsForDebug=*/false,
                       /*NoDetails=*/true);
    dbgs() << "\n";
  }
}
