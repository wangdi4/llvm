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
#include "llvm/Analysis/Intel_LoopAnalysis/IR/CanonExpr.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLInst.h"
#include "llvm/Support/Debug.h"

using namespace llvm;
using namespace llvm::loopopt;

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
      LoadCost = TTI.getMemoryOpCost(Instruction::Load, VecTy,
                                     MaybeAlign(Alignment), AS);
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
    // SK_Broadcast, SK_Reverse, SK_Select, SK_InsertSubvector, or
    // SK_ExtractSubvector
    // FIXME 1: Extend the OVLS cost interface to gather also one-time costs
    // (such as loading indices).
    // FIXME 2: Extend the LLVM TTI interface for shuffles.
    // FORNOW: temporary dummy implementation.
    Cost = TTI.getShuffleCost(TargetTransformInfo::SK_Select, VecTy);
    return Cost;
  }

  llvm_unreachable("unsupported OVLSInstruction");
  return 0;
}

DenseMap<uint64_t, Value *>
OVLSConverter::genLLVMIR(IRBuilder<> &Builder,
                         const OVLSInstructionVector &InstVec,
                         ShuffleVectorInst *InterleavingShuffleInst,
                         Value *Addr, Type *ElemTy, unsigned Alignment) {
  DenseMap<uint64_t, Value *> InstMap;
  // Only used when a shuffle instruction needs to be generated for an
  // OVLSMemref.
  DenseMap<const OVLSMemref *, Value *> MemrefShuffleMap;
  for (auto &OInst : InstVec) {
    if (const OVLSLoad *const OLI = dyn_cast<const OVLSLoad>(OInst)) {
      // Bitcast Addr to OLI's type
      OVLSType Ty = OInst->getType();
      unsigned TySize = Ty.getSize() / 8;
      VectorType *VecTy = VectorType::get(ElemTy, Ty.getNumElements());
      Type *BasePtrTy = VecTy->getPointerTo();
      Value *VecBasePtr = Builder.CreateBitCast(Addr, BasePtrTy);

      // Create GEP instruction
      int64_t Offset = OLI->getPointerOperand().getOffset();
      unsigned GEPIndex = Offset == 0 ? 0 : Offset / TySize;
      Value *NewBasePtr =
          Builder.CreateInBoundsGEP(VecBasePtr, Builder.getInt32(GEPIndex));

      // Generate the load
      Instruction *NewLoad = Builder.CreateAlignedLoad(NewBasePtr, Alignment);
      InstMap[OInst->getId()] = NewLoad;
    } else if (const OVLSShuffle *const OSI =
                   dyn_cast<const OVLSShuffle>(OInst)) {
      const OVLSOperand *OVLSOp[2];
      OVLSOp[0] = OSI->getOperand(0);
      OVLSOp[1] = OSI->getOperand(1);
      Value *Op[2];

      // In case of optimization of Interleaving Loads, the Shuffles will have
      // the operands corresponding to OVLSInstruction. In case of optimization
      // of Interleaving Stores, the Shuffles can have operands as undef,
      // results of other Load/Shuffle Instructions or OVLSAddress/OVLSMemrefs.
      // In case, it is OVLSAddress, the Memrefs need to be generated using
      // Shuffles with the same operands as InterleavingShuffleVector
      // Instruction.
      //
      bool InvalidOperands = false;
      for (int i = 0; i < 2; i++) {
        switch (OVLSOp[i]->getKind()) {
        case OVLSOperand::OK_Undef:
          if (OVLSOp[(i + 1) % 2]->IsKindUndefined())
            InvalidOperands = true;
          // else {the processing of an undef operand is taken care of during
          // the other operand's processing, since we need the other operand's
          // type.}
          break;
        case OVLSOperand::OK_Instruction:
          Op[i] = InstMap[OVLSOp[i]->getId()];
          if (OVLSOp[(i + 1) % 2]->IsKindUndefined())
            Op[(i + 1) % 2] = UndefValue::get(Op[i]->getType());
          break;
        case OVLSOperand::OK_Constant:
          // Not supported yet.
          InvalidOperands = true;
          break;
        case OVLSOperand::OK_Address: {
          const OVLSMemref *TempBase =
              cast<const OVLSAddress>(OVLSOp[i])->getBase();
          auto FindExistingSh = MemrefShuffleMap.find(TempBase);
          if (FindExistingSh != MemrefShuffleMap.end())
            Op[i] = FindExistingSh->getSecond();
          else {
            // Create and add the corresponding needed shuffle in the
            // MemrefShuffleMap. In case of InterleavedAccessPass, we only see
            // this case when we are optimizing Interleaving Stores.
            Value *Op1 = InterleavingShuffleInst->getOperand(0);
            Value *Op2 = InterleavingShuffleInst->getOperand(1);
            int64_t StartIndex =
                cast<const OVLSAddress>(OVLSOp[i])->getOffset();
            SmallVector<uint32_t, 4> Mask;
            uint32_t ElemSize = TempBase->getType().getElementSize();
            uint32_t NumElements = TempBase->getType().getNumElements();
            // StartIndex, represents the offset from the base,
            // when taking the data from the InterleavingShuffleInst, it
            // resolves to the following starting value for ShuffleMask
            // computation.
            StartIndex = ((StartIndex * 8) / ElemSize) * NumElements;
            // Sequential mask from StartIndex to StartIndex + NumElements.
            for (int i = StartIndex; i < (StartIndex + NumElements); i++)
              Mask.push_back(i);
            Op[i] = Builder.CreateShuffleVector(Op1, Op2, Mask);
            MemrefShuffleMap[TempBase] = Op[i];
          }
          if (OVLSOp[(i + 1) % 2]->IsKindUndefined())
            Op[(i + 1) % 2] = UndefValue::get(Op[i]->getType());
        } break;
        }
      }
      (void)InvalidOperands;
      assert((InvalidOperands == false) &&
             "Unexpected Operand for OVLSShuffle Instruction.");

      SmallVector<uint32_t, 4> Mask;
      OSI->getShuffleMask(Mask);
      Value *Shuffle = Builder.CreateShuffleVector(Op[0], Op[1], Mask);
      InstMap[OInst->getId()] = Shuffle;
    } else if (const OVLSStore *const OStI = dyn_cast<const OVLSStore>(OInst)) {
      // Bitcast Addr to OStI's type
      OVLSType Ty = OInst->getType();
      unsigned TySize = Ty.getSize() / 8;
      VectorType *VecTy = VectorType::get(ElemTy, Ty.getNumElements());
      Type *BasePtrTy = VecTy->getPointerTo();
      Value *VecBasePtr = Builder.CreateBitCast(Addr, BasePtrTy);

      // Create GEP instruction
      OVLSAddress OffsetAddr = OStI->getDst(); // Dst->destination operand.
      int64_t Offset = OffsetAddr.getOffset();
      assert((Offset % TySize == 0) && "Unexpected Offset for OVLSStore.");
      unsigned GEPIndex = Offset == 0 ? 0 : Offset / TySize;
      Value *NewBasePtr =
          Builder.CreateInBoundsGEP(VecBasePtr, Builder.getInt32(GEPIndex));

      // Generate the store
      Value *SrcReg = InstMap[OStI->getSrc()->getId()];
      Instruction *NewStore =
          Builder.CreateAlignedStore(SrcReg, NewBasePtr, Alignment);
      InstMap[OInst->getId()] = NewStore;
    } else
      assert(false && "Unexpected OVLSInstruction.");
  }
  return InstMap;
}
