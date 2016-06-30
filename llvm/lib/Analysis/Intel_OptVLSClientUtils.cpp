//===---------------- Intel_OptVLSClientUtils.cpp --------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===---------------------------------------------------------------------===//
///
/// \file
/// Utilities that multiple VLS clients can use.
///
//===---------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_OptVLSClientUtils.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

// --------- HIR Specific implementations

// Helper function: Return the data type of the pointer that accesses Mrf.
PointerType *getPtrType(const OVLSMemref &Mrf) {
  assert(isa<HIRVLSClientMemref>(Mrf) && "Expecting HIR Memref.\n");
  const RegDDRef *DDRef = (cast<HIRVLSClientMemref>(Mrf)).getRef();
  assert(DDRef->hasGEPInfo() && "Expecting a memref DDReft, not a terminal");
  const CanonExpr *CE = DDRef->getBaseCE();
  PointerType *BaseTy = cast<PointerType>(CE->getSrcType());
  return BaseTy;
}

// Helper function: Return the underlying LLVMIR Value of the pointer that 
// accesses Mrf.
Value *getPtrVal(const OVLSMemref &Mrf) {
  assert(isa<HIRVLSClientMemref>(Mrf) && "Expecting HIR Memref.\n");
  const RegDDRef *DDRef = (cast<HIRVLSClientMemref>(Mrf)).getRef();
  assert(DDRef->hasGEPInfo() && "Expecting a memref DDReft, not a terminal");
  const HLNode *Node = DDRef->getHLDDNode();
  const HLInst *INode = dyn_cast<HLInst>(Node);
  assert(INode && "not an HLIInst Node");
  const Instruction *ConstInst = INode->getLLVMInstruction();
  Instruction *I = const_cast<Instruction *>(ConstInst);
  StoreInst *SI = dyn_cast<StoreInst>(I);
  LoadInst *LI = dyn_cast<LoadInst>(I);
  Value *PtrVal = SI ? SI->getPointerOperand() : LI->getPointerOperand();
  return PtrVal;
}

unsigned OVLSTTICostModelHIR::getMrfAddressSpace(const OVLSMemref &Mrf) const {
  PointerType *BaseTy = getPtrType(Mrf);
  return BaseTy->getPointerAddressSpace();
}

uint64_t
OVLSTTICostModelHIR::getGatherScatterOpCost(const OVLSMemref &Mrf) const {
  bool isLoad = Mrf.getAccessType().isStridedLoad();
  uint64_t GatherScatterCost;
  PointerType *BaseTy = getPtrType(Mrf);
  Type *DataTy = BaseTy->getElementType();
  bool isGatherOrScatterLegal = (isLoad && TTI.isLegalMaskedGather(DataTy)) ||
                                (!isLoad && TTI.isLegalMaskedScatter(DataTy));
  if (!isGatherOrScatterLegal)
    return 0;
  unsigned Opcode = isLoad ? Instruction::Load : Instruction::Store;
  uint32_t NumElements = Mrf.getType().getNumElements();
  assert(NumElements > 1 && "Unexpected NumElements");
  Type *VectorTy = VectorType::get(DataTy, NumElements);
  Value *PtrVal = getPtrVal(Mrf);
  bool isMaskRequired = false; // TODO
  unsigned Alignment = 0;      // TODO
  GatherScatterCost = TTI.getGatherScatterOpCost(Opcode, VectorTy, PtrVal,
                                                 isMaskRequired, Alignment);
  GatherScatterCost += TTI.getAddressComputationCost(VectorTy);
  return GatherScatterCost;
}

// ----------- General implementations (common to all clients)

Type *OVLSTTICostModel::getVectorDataType(Type *ElemType,
                                          OVLSType &VLSType) const {
  // Obtain the NumElements from the VLSType
  uint32_t NumElements = VLSType.getNumElements();

  // Obtain the element type from ElemType, if available.
  // Otherwise, "make up" a vector type according to the element-size.
  if (ElemType && (ElemType->isVectorTy() || ElemType->isVoidTy()))
    return ElemType;
  if (!ElemType) {
    uint32_t ElementSize = VLSType.getElementSize();
    ElemType = Type::getIntNTy(C, ElementSize * 8);
  }
  if (NumElements == 1)
    return ElemType;
  return VectorType::get(ElemType, NumElements);
}

uint64_t OVLSTTICostModel::getInstructionCost(const OVLSInstruction *I) const {

  uint64_t Cost;

  // Consecutive load
  // (Extend similarly to stores once OVLSStore is defined).
  if (isa<OVLSLoad>(I)) {
    // Get the Address Space
    //
    const OVLSMemref *Mrf = (cast<OVLSLoad>(I))->getSrc().getBase();
    assert(Mrf && "Expecting a valid Mrf");
    unsigned AS = getMrfAddressSpace(*Mrf);

    // Get the Alignment
    //
    unsigned Alignment = 0; // TODO

    // Get the VectorType and the Mask
    //
    Type *ElementTy = getMrfDataType(Mrf);
    OVLSType VLSType = I->getType();
    // Taking the number of elements from the VLSType of the Instruction.
    // CHECKME: When NumElements is 3 and mask is 111, we end up with a vector
    // type of 3 elements (e.g. 3 x 32i) and query the cost of an unmasked 
    // load of 3 elements. We may want to also query the cost of a masked load
    // of 4 elements and take the minimum of the two.
    Type *VecTy = getVectorDataType(ElementTy, VLSType);
    // VecTy->dump();
    // If mask is all ones, use unmasked load.
    uint64_t ElementMask = (cast<OVLSLoad>(I))->getMask();
    bool NeedMask = false;
    uint32_t NumElemsInALoad = 0;
    uint64_t EMask = ElementMask;
    while (EMask != 0) {
      NeedMask |= !(EMask & 1); // 0 in mask
      EMask = EMask >> 1;
      NumElemsInALoad++;
    }
    assert(NumElemsInALoad == VLSType.getNumElements() &&
           "unexpected OVLS type/mask");

    uint64_t AddrCost = TTI.getAddressComputationCost(VecTy);
    uint64_t LoadCost;
    if (NeedMask)
      LoadCost =
          TTI.getMaskedMemoryOpCost(Instruction::Load, VecTy, Alignment, AS);
    else
      LoadCost = TTI.getMemoryOpCost(Instruction::Load, VecTy, Alignment, AS);
    return AddrCost + LoadCost;
  }

  if (isa<OVLSShuffle>(I)) {
    // The actual vector type shouldn't matter, just the width, but the
    // TTI interface requires an actual type. Calling getVectorDataType
    // with nullptr as the first argument means we will make up a vector
    // type according to the element size of VLSType.
    OVLSType VLSType = I->getType();
    Type *VecTy = getVectorDataType(nullptr, VLSType);

    // TODO:
    // In AVX512, this would probably map to a single shuffle, which involves
    // the cost of loading indices (paid once per loop); saving the source
    // vector (move), and doing the shuffle.
    // In AVX2 this may map to several extracts and inserts.
    // Currently there is no TTI interface for a generic shuffle; the current
    // interface (TTI.getShuffleCost) supports only the following cases:
    // SK_Broadcast, SK_Reverse, SK_Alternate, SK_InsertSubvector, or
    // SK_ExtractSubvector
    // FIXME 1: Extend the OVLS cost interface to gather also one-time costs
    // (such as loading indices).
    // FIXME 2: Extend the LLVM TTI interface for shuffles.
    // FORNOW: temporary dummy implementation.
    Cost = TTI.getShuffleCost(TargetTransformInfo::SK_Alternate, VecTy);
    return Cost;
  }

  llvm_unreachable("unsupported OVLSInstruction");
  return 0;
}
