//===--------- X86InterleavedAccess.cpp ----------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===--------------------------------------------------------------------===//
///
/// \file
/// This file contains the X86 implementation of the interleaved accesses
/// optimization generating X86-specific instructions/intrinsics for
/// interleaved access groups.
///
//===--------------------------------------------------------------------===//

#include "X86ISelLowering.h"
#include "X86TargetMachine.h"
#include "X86TargetTransformInfo.h"                // INTEL
#include "llvm/Analysis/Intel_OptVLSClientUtils.h" // INTEL

using namespace llvm;

namespace {
#ifdef INTEL_CUSTOMIZATION

/// Allows creating OVLS abstract memory-references for LLVM-IR based
/// interleaved accesses. E.g. %wide.vec = load <4 x double>, <4 x double>*
/// %ptr, align 16
///  %strided.v0 = shufflevector <8 x double> %wide.vec, <8 x double> undef,
///                              <2 x i32> <i32 0, i32 2>
///  %strided.v1 = shufflevector <4 x double> %wide.vec, <4 x double> undef,
///                              <2 x i32> <i32 1, i32 3>
/// In this example, %strided.v0 and  %strided.v1 reflect two access patterns
/// of this abstract memory reference.
///
/// Some of the callback functions(such as canMoveto(), haveSameNumElements())
/// are implemented based on the properties of an interleaved access group. An
/// interleaved access group(wide-load+shuffles or shuffles+wide-store) gets
/// formed if its shuffles have the same number of elements and the shuffles
/// come in a sequence which means there are no other instructions in between
/// the shuffles.
/// Therefore, this should not be used by any other clients other than the
/// X86InterleavedAccess group.
class X86InterleavedClientMemref : public OVLSMemref {
public:
  X86InterleavedClientMemref(char MemrefId, int Distance, Type *ElemType,
                             unsigned NumElements, OVLSAccessType AType,
                             bool CVStride, int VStride)
      : OVLSMemref(VLSK_X86InterleavedClientMemref,
                   OVLSType(ElemType->getPrimitiveSizeInBits(), NumElements),
                   AType) {
    MId = MemrefId;
    Dist = Distance;
    ConstVStride = CVStride;
    DataType = VectorType::get(ElemType, NumElements);
    VecStride = VStride;
  }

  static bool classof(const OVLSMemref *Memref) {
    return Memref->getKind() == VLSK_X86InterleavedClientMemref;
  }

  bool isAConstDistanceFrom(const OVLSMemref &Memref, int64_t *Distance) {
    assert(isa<X86InterleavedClientMemref>(&Memref) &&
           "Expected X86InterleavedClientMemref!!!");
    const X86InterleavedClientMemref *CLMemref =
        cast<const X86InterleavedClientMemref>(&Memref);

    *Distance = CLMemref->getDistance() - Dist;
    return true;
  }
  bool haveSameNumElements(const OVLSMemref &Memref) { return true; }

  bool canMoveTo(const OVLSMemref &MemRef) { return true; }

  bool hasAConstStride(int64_t *Stride) const {
    if (ConstVStride) {
      *Stride = VecStride;
      return true;
    }
    return false;
  }
  unsigned getLocation() const {
    return MId; // FIXME
  }
  int getDistance() const { return Dist; }

private:
  char MId;
  VectorType *DataType; // Data type for the memref.
  int64_t Dist;         // Distance between two memrefs in bytes.
  bool ConstVStride;
  int VecStride;
};
#endif // INTEL_CUSTOMIZATION

/// \brief This class holds necessary information to represent an interleaved
/// access group and supports utilities to lower the group into
/// X86-specific instructions/intrinsics.
///  E.g. A group of interleaving access loads (Factor = 2; accessing every
///       other element)
///        %wide.vec = load <8 x i32>, <8 x i32>* %ptr
///        %v0 = shuffle <8 x i32> %wide.vec, <8 x i32> undef, <0, 2, 4, 6>
///        %v1 = shuffle <8 x i32> %wide.vec, <8 x i32> undef, <1, 3, 5, 7>
class X86InterleavedAccessGroup {
  /// \brief Reference to the wide-load instruction of an interleaved access
  /// group.
  Instruction *const Inst;

  /// \brief Reference to the shuffle(s), consumer(s) of the (load) 'Inst'.
  ArrayRef<ShuffleVectorInst *> Shuffles;

  /// \brief Reference to the starting index of each user-shuffle.
  ArrayRef<unsigned> Indices;

  /// \brief Reference to the interleaving stride in terms of elements.
  const unsigned Factor;

  /// \brief Reference to the underlying target.
  const X86Subtarget &Subtarget;

  const DataLayout &DL;

  IRBuilder<> &Builder;

  /// \brief Breaks down a vector \p 'Inst' of N elements into \p NumSubVectors
  /// sub vectors of type \p T. Returns true and the sub-vectors in
  /// \p DecomposedVectors if it decomposes the Inst, returns false otherwise.
  bool decompose(Instruction *Inst, unsigned NumSubVectors, VectorType *T,
                 SmallVectorImpl<Instruction *> &DecomposedVectors);

  /// \brief Performs matrix transposition on a 4x4 matrix \p InputVectors and
  /// returns the transposed-vectors in \p TransposedVectors.
  /// E.g.
  /// InputVectors:
  ///   In-V0 = p1, p2, p3, p4
  ///   In-V1 = q1, q2, q3, q4
  ///   In-V2 = r1, r2, r3, r4
  ///   In-V3 = s1, s2, s3, s4
  /// OutputVectors:
  ///   Out-V0 = p1, q1, r1, s1
  ///   Out-V1 = p2, q2, r2, s2
  ///   Out-V2 = p3, q3, r3, s3
  ///   Out-V3 = P4, q4, r4, s4
  void transpose_4x4(ArrayRef<Instruction *> InputVectors,
                     SmallVectorImpl<Value *> &TrasposedVectors);

#ifdef INTEL_CUSTOMIZATION
  const TargetTransformInfo &TTI;

  /// Keeps mapping of a shufflevector to its OVLSMemref which ultimately helps
  /// mapping a shufflevector to an optimized LLVM-IR instruction.
  /// The detailed chain: shufflevector is mapped with an OVLSMemref; an
  /// OVLSMemref is mapped to an OVLSInstruction; an OVLSInstruction is mapped
  /// to an LLVM-IR insturction.
  std::multimap<ShuffleVectorInst *, OVLSMemref *> ShuffleToMemrefMap;

  /// Creates an OVLSMemref for each shuffle in the Shuffles and returns
  /// the memrefs in \p Memrefs.
  void createOVLSMemrefs(OVLSMemrefVector &Memrefs) {
    // Create OVLSMemref for each shuffle.
    for (unsigned i = 0; i < Shuffles.size(); ++i) {
      VectorType *VecTy = Shuffles[i]->getType();
      Type *ShuffleEltTy = VecTy->getVectorElementType();
      unsigned EltSizeInByte = DL.getTypeSizeInBits(ShuffleEltTy) / 8;
      int Dist = Indices[i] * EltSizeInByte;
      OVLSMemref *Mrf = new X86InterleavedClientMemref(
          i + 1, Dist, ShuffleEltTy, VecTy->getVectorNumElements(),
          OVLSAccessType::getStridedLoadTy(), true, Factor * EltSizeInByte);
      Memrefs.push_back(Mrf);
      ShuffleToMemrefMap.insert(
          std::pair<ShuffleVectorInst *, OVLSMemref *>(Shuffles[i], Mrf));
    }
  }
#endif // INTEL_CUSTOMIZATION

public:
  /// In order to form an interleaved access group X86InterleavedAccessGroup
  /// requires a wide-load instruction \p 'I', a group of interleaved-vectors
  /// \p Shuffs, reference to the first indices of each interleaved-vector
  /// \p 'Ind' and the interleaving stride factor \p F. In order to generate
  /// X86-specific instructions/intrinsics it also requires the underlying
  /// target information \p STarget.
  explicit X86InterleavedAccessGroup(Instruction *I,
                                     ArrayRef<ShuffleVectorInst *> Shuffs,
                                     ArrayRef<unsigned> Ind, const unsigned F,
                                     const X86Subtarget &STarget,
                                     IRBuilder<> &B,
                                     const TargetTransformInfo &T /* INTEL */)
      : Inst(I), Shuffles(Shuffs), Indices(Ind), Factor(F), Subtarget(STarget),
        DL(Inst->getModule()->getDataLayout()), Builder(B), TTI(T) /*INTEL*/ {}

  /// \brief Returns true if this interleaved access group can be lowered into
  /// x86-specific instructions/intrinsics, false otherwise.
  bool isSupported() const;

  /// \brief Lowers this interleaved access group into X86-specific
  /// instructions/intrinsics.
  bool lowerIntoOptimizedSequence();

#ifdef INTEL_CUSTOMIZATION
  /// \brief Lowers this interleaved access group into X86-specific
  /// instructions/intrinsics by an Intel customized optimization OptVLS
  /// which uses dynamic algorithm to generate the optimized sequence
  /// as opposed to hard-coded transposed function provided by this pass.
  bool lowerIntoOptimizedSequenceByOptVLS() {
    VectorType *VecTy = Shuffles[0]->getType();

    // There is nothing to optimize further, this pattern is already optimized.
    if (VecTy->getVectorNumElements() <= 2)
      return false;

    // FIXME: Support all other types.
    if (DL.getTypeSizeInBits(VecTy->getVectorElementType()) != 64 ||
        Factor != 2)
      return false;

    // Create OVLSMemrefVector for the members(shuffles) of
    // X86InterleavedAccessGroup.
    OVLSMemrefVector Mrfs;
    createOVLSMemrefs(Mrfs);

    // Create OVLSGroups for the shuffles.
    OVLSGroupVector Grps;
    unsigned ShuffleVecSize = DL.getTypeSizeInBits(VecTy);
    // Let's send the vector length computed by the vectorizer. It
    // might change in the future based on some other situations.
    OptVLSInterface::getGroups(Mrfs, Grps, ShuffleVecSize / BYTE);

    // TODO: Support multiple groups formed out of the shuffles.
    if (Grps.size() == 0 || Grps.size() > 1)
      return false;

    OVLSCostModel CM(TTI, VecTy->getContext());

    // Maps each OVLSMemref to an OVLSInstruction.
    OVLSMemrefToInstMap MemrefToInstMap;
    OVLSMemrefToInstMap::iterator It1;
    std::multimap<ShuffleVectorInst *, OVLSMemref *>::iterator It;

    // Maps each LLVM-IR Instruction to an int.
    DenseMap<uint64_t, Value *> InstMap;
    // Generate optimized-sequence for each OVLSGroup.
    for (OVLSGroup *Grp : Grps) {
      OVLSInstructionVector InstVec;
      // Get the optimized-sequence computed by OptVLS.
      if (OptVLSInterface::getSequence(*Grp, CM, InstVec, &MemrefToInstMap)) {
        Value *Addr;
        if (auto *LI = dyn_cast<LoadInst>(Inst))
          Addr = LI->getPointerOperand();
        else
          Addr = (cast<StoreInst>(Inst))->getPointerOperand();

        // Translate the optimized-sequence(from OVLS-pseudo instruction type )
        // to LLVM-IR instruction type.
        InstMap = OVLSConverter::genLLVMIR(
            Builder, InstVec, Addr, Inst->getType()->getVectorElementType(),
            16);
      } else
        return false;
    }

    // Now replace the unoptimized-interleaved-vectors with the
    // transposed-interleaved vectors.
    for (unsigned i = 0; i < Shuffles.size(); ++i) {
      It = ShuffleToMemrefMap.find(Shuffles[i]);
      assert(It != ShuffleToMemrefMap.end() && "Memref not found!!!");
      It1 = MemrefToInstMap.find(It->second);
      assert(It1 != MemrefToInstMap.end() && "OVLSInstrcution not found!!!");
      OVLSInstruction *ORInst = It1->second;
      Value *RInst = InstMap[ORInst->getId()];
      Shuffles[i]->replaceAllUsesWith(RInst);
    }

    return true;
  }    // end of lowerIntoOptimizedSequenceByOptVLS.
#endif // INTEL_CUSTOMIZATION
};
} // end anonymous namespace

bool X86InterleavedAccessGroup::isSupported() const {
  VectorType *ShuffleVecTy = Shuffles[0]->getType();
  uint64_t ShuffleVecSize = DL.getTypeSizeInBits(ShuffleVecTy);
  Type *ShuffleEltTy = ShuffleVecTy->getVectorElementType();

  if (DL.getTypeSizeInBits(Inst->getType()) < Factor * ShuffleVecSize)
    return false;

  // Currently, lowering is supported for 64 bits on AVX.
  if (!Subtarget.hasAVX() || ShuffleVecSize != 256 ||
      DL.getTypeSizeInBits(ShuffleEltTy) != 64 || Factor != 4)
    return false;

  return true;
}

bool X86InterleavedAccessGroup::decompose(
    Instruction *VecInst, unsigned NumSubVectors, VectorType *SubVecTy,
    SmallVectorImpl<Instruction *> &DecomposedVectors) {
  Type *VecTy = VecInst->getType();
  (void)VecTy;
  assert(VecTy->isVectorTy() &&
         DL.getTypeSizeInBits(VecTy) >=
             DL.getTypeSizeInBits(SubVecTy) * NumSubVectors &&
         "Invalid Inst-size!!!");
  assert(VecTy->getVectorElementType() == SubVecTy->getVectorElementType() &&
         "Element type mismatched!!!");

  if (!isa<LoadInst>(VecInst))
    return false;

  LoadInst *LI = cast<LoadInst>(VecInst);
  Type *VecBasePtrTy = SubVecTy->getPointerTo(LI->getPointerAddressSpace());

  Value *VecBasePtr =
      Builder.CreateBitCast(LI->getPointerOperand(), VecBasePtrTy);

  // Generate N loads of T type
  for (unsigned i = 0; i < NumSubVectors; i++) {
    // TODO: Support inbounds GEP
    Value *NewBasePtr = Builder.CreateGEP(VecBasePtr, Builder.getInt32(i));
    Instruction *NewLoad =
        Builder.CreateAlignedLoad(NewBasePtr, LI->getAlignment());
    DecomposedVectors.push_back(NewLoad);
  }

  return true;
}

void X86InterleavedAccessGroup::transpose_4x4(
    ArrayRef<Instruction *> Matrix,
    SmallVectorImpl<Value *> &TransposedMatrix) {
  assert(Matrix.size() == 4 && "Invalid matrix size");
  TransposedMatrix.resize(4);

  // dst = src1[0,1],src2[0,1]
  uint32_t IntMask1[] = {0, 1, 4, 5};
  ArrayRef<uint32_t> Mask = makeArrayRef(IntMask1, 4);
  Value *IntrVec1 = Builder.CreateShuffleVector(Matrix[0], Matrix[2], Mask);
  Value *IntrVec2 = Builder.CreateShuffleVector(Matrix[1], Matrix[3], Mask);

  // dst = src1[2,3],src2[2,3]
  uint32_t IntMask2[] = {2, 3, 6, 7};
  Mask = makeArrayRef(IntMask2, 4);
  Value *IntrVec3 = Builder.CreateShuffleVector(Matrix[0], Matrix[2], Mask);
  Value *IntrVec4 = Builder.CreateShuffleVector(Matrix[1], Matrix[3], Mask);

  // dst = src1[0],src2[0],src1[2],src2[2]
  uint32_t IntMask3[] = {0, 4, 2, 6};
  Mask = makeArrayRef(IntMask3, 4);
  TransposedMatrix[0] = Builder.CreateShuffleVector(IntrVec1, IntrVec2, Mask);
  TransposedMatrix[2] = Builder.CreateShuffleVector(IntrVec3, IntrVec4, Mask);

  // dst = src1[1],src2[1],src1[3],src2[3]
  uint32_t IntMask4[] = {1, 5, 3, 7};
  Mask = makeArrayRef(IntMask4, 4);
  TransposedMatrix[1] = Builder.CreateShuffleVector(IntrVec1, IntrVec2, Mask);
  TransposedMatrix[3] = Builder.CreateShuffleVector(IntrVec3, IntrVec4, Mask);
}

// Lowers this interleaved access group into X86-specific
// instructions/intrinsics.
bool X86InterleavedAccessGroup::lowerIntoOptimizedSequence() {
  SmallVector<Instruction *, 4> DecomposedVectors;
  VectorType *VecTy = Shuffles[0]->getType();
  // Try to generate target-sized register(/instruction).
  if (!decompose(Inst, Factor, VecTy, DecomposedVectors))
    return false;

  SmallVector<Value *, 4> TransposedVectors;
  // Perform matrix-transposition in order to compute interleaved
  // results by generating some sort of (optimized) target-specific
  // instructions.
  transpose_4x4(DecomposedVectors, TransposedVectors);

  // Now replace the unoptimized-interleaved-vectors with the
  // transposed-interleaved vectors.
  for (unsigned i = 0; i < Shuffles.size(); i++)
    Shuffles[i]->replaceAllUsesWith(TransposedVectors[Indices[i]]);

  return true;
}

// Lower interleaved load(s) into target specific instructions/
// intrinsics. Lowering sequence varies depending on the vector-types, factor,
// number of shuffles and ISA.
// Currently, lowering is supported for 4x64 bits with Factor = 4 on AVX.
bool X86TargetLowering::lowerInterleavedLoad(
    LoadInst *LI, ArrayRef<ShuffleVectorInst *> Shuffles,
    ArrayRef<unsigned> Indices, unsigned Factor) const {
  assert(Factor >= 2 && Factor <= getMaxSupportedInterleaveFactor() &&
         "Invalid interleave factor");
  assert(!Shuffles.empty() && "Empty shufflevector input");
  assert(Shuffles.size() == Indices.size() &&
         "Unmatched number of shufflevectors and indices");

  // Create an interleaved access group.
  IRBuilder<> Builder(LI);

#ifdef INTEL_CUSTOMIZATION
  const TargetMachine &TM = getTargetMachine();
  const X86TargetMachine *X86TM = static_cast<const X86TargetMachine *>(&TM);
  const TargetTransformInfo &TTI =
      TargetTransformInfo(X86TTIImpl(X86TM, *(LI->getFunction())));

  X86InterleavedAccessGroup Grp(LI, Shuffles, Indices, Factor, Subtarget,
                                Builder, TTI);

  if (Grp.isSupported() && Grp.lowerIntoOptimizedSequence())
    return true;

  return Grp.lowerIntoOptimizedSequenceByOptVLS();

#endif
}
