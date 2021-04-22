//===- IntelVPlanTTIWrapper.cpp ---------------------------------*- C++ -*-===//
//
//   Copyright (C) 2021 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanTTIWrapper.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "vplan-tti-wrapper"

using namespace llvm;
using namespace llvm::vpo;

static const constexpr unsigned DefaultCacheLineSize = 64;

// TODO: TTI should return proper data.
static cl::opt<unsigned>
    CMCacheLineSize("vplan-cm-cache-line-size", cl::init(DefaultCacheLineSize),
                    cl::Hidden,
                    cl::desc("Defines size of a cache line (in bytes)"));

// Cost of the store should be 1.5x greater than the cost of a load.
// Store has two stages: allocate/read cache line(s) and place data to it. The
// load reads data from cache line(s). Moving the data to the cache buffer is
// more expensive than moving it from the buffer.
static cl::opt<unsigned> CMStoreCostAdjustment(
    "vplan-cm-store-cost-adjustment", cl::init(1000), cl::Hidden,
    cl::desc("Store cost adjustment on top of TTI value"));

static cl::opt<unsigned> CMLoadCostAdjustment(
    "vplan-cm-load-cost-adjustment", cl::init(500), cl::Hidden,
    cl::desc("Load cost adjustment on top of TTI value"));

// Estimate probability ([0.0f; 1.0f] range) of a memref to cross a cache line
// boundary.
// Alignment    - alignment of memory reference;
// RefBytes     - how many bytes is references, a power of 2 value;
// BytesCross   - how many bytes crosses cache line (number M of bytes).
//               (chunk of memory that crosses the the first cache lane);
// \/-------cache-line---------\/-------cache-line---------\/
// ||------|-----||------|-----||------|-----||------|-----||
//                       |---memref----|
//                       |--N--||--M---|
static float cacheLineCrossingProbability(Align Alignment, uint64_t RefBytes,
                                          unsigned &BytesCross) {
  unsigned CacheLineSize;
  // Enforce CacheLineSize to be a power of 2 value.
  switch (CMCacheLineSize) {
  case 16:
  case 32:
  case 64:
  case 128:
  case 256:
  case 512:
    CacheLineSize = CMCacheLineSize;
    break;
  default:
    CacheLineSize = DefaultCacheLineSize;
  }

  // This method returns only powers of 2 values.
  uint64_t Base = Alignment.value();

  if (RefBytes <= CacheLineSize) {
    if (Base >= CacheLineSize) {
      // \/-cache-line-\/-cache-line-\/-cache-line-\/-cache-line-\/
      // ||------|-----||------|-----||------|-----||------|-----||
      // /\-------alignment----------/\-------alignment----------/\
      // ||-ref1-|                   ||-ref2-|
      BytesCross = 0;
      return 0.0;
    } else { // Base < CacheLineSize
      if (RefBytes <= Base) {
        // In this case we have natural alignment.
        // \/-------cache-line---------\/-------cache-line---------\/
        // ||------|-----||------|-----||------|-----||------|-----||
        // /\--alignment-/\--alignment-/\--alignment-/\--alignment-/\
        // ||----ref1----||----ref2----||----ref3----||----ref4----||
        BytesCross = 0;
        return 0.0;
      } else { // Base < CacheLineSize && RefBytes > Base
        // It is impossible to estimate the number of bytes crossing with
        // certainty:
        // \/-----------------------cache-line---------------------\/
        // ||------|-----||------|-----||------|-----||------|-----||
        // /\--alignment-/\--alignment-/\--alignment-/\--alignment-/\
        // ||------------ref-----------|
        //               ||------------ref-----------|
        //                             ||------------ref-----------|
        //                                           ||------------ref----...|

        // We have N possible placements of RefBytes with specified alignment
        // within cache line. Both are powers of 2.
        unsigned N = CacheLineSize / Base;

        // Out of those N placementes K do not result in cache line crossing.
        // [0, 1, ..., K] first placements do not cross;
        // [K + 1, ..., N] do result in crossing.
        // 0 < K < N.
        // The greatest natural K to satisfy
        //   Base * K + RefBytes <= CacheLineSize
        // is:
        unsigned K = (CacheLineSize - RefBytes) / Base;

        // Number of possibilities to read RefBytes within cache line,
        // since K is an index that starts from 0.
        unsigned AlignPossibilites = K + 1;

        // Min/Max number of bytes crossing the cache line.
        unsigned MinCrossBytes =
          Base * AlignPossibilites + RefBytes - CacheLineSize;
        unsigned MaxCrossBytes =
          Base * (N - 1) + RefBytes - CacheLineSize;
        // Return an average, as the best guess is uniform distribution of
        // memory accesses.
        BytesCross = (MinCrossBytes + MaxCrossBytes) / 2;

        // Probability of an unaligned access would be
        // 1 - AlignPossibilites / N.
        return 1 - static_cast<float>(AlignPossibilites) /
                   static_cast<float>(N);
      }
    }
  } else { // RefBytes > CacheLineSize
    // Base >= CacheLineSize.
    // \/-cache-line-\/-cache-line-\/-cache-line-\/-cache-line-\/
    // ||------|-----||------|-----||------|-----||------|-----||
    // /\-------alignment----------/\-------alignment----------/\
    // ||-------------------------ref--------------------------|

    // RefBytes > CacheLineSize && Base < CacheLineSize.
    // \/-----------------------cache-line---------------------\/
    // ||------|-----||------|-----||------|-----||------|-----||
    // /\--alignment-/\--alignment-/\--alignment-/\--alignment-/\
    // ...----------------ref--------------------|
    // OR:
    //               ||-----------------ref-------------------...
    BytesCross = RefBytes - CacheLineSize;
    return 1.0;
  }
}

// Calculate cost adjustment for memref with particular Type and Alignment.
int VPlanTTIWrapper::getNonMaskedMemOpCostAdj(unsigned Opcode, Type *SrcTy,
                                              Align Alignment) const {
  // Non-vector types are handled using default costs.
  VectorType *VecTy = cast<VectorType>(SrcTy);

  // Number of parts for this Type.
  unsigned NumReg = TTI.getNumberOfParts(VecTy);

  // TTI model doesn't support vector types/registers.  Don't bother evaluating
  // cache split cost for such targets.
  if (NumReg == 0)
    return 0;

  assert(VecTy->getScalarType()->isSized() && "Expect only sizable types");

  uint64_t TypeSizeInBits = 0;
  if (VecTy->getScalarType()->isPointerTy())
    TypeSizeInBits = DL.getPointerTypeSizeInBits(VecTy);
  else {
    TypeSizeInBits = DL.getTypeStoreSizeInBits(VecTy);
  }

  uint64_t SizeOfWholeVector = TypeSizeInBits / 8;
  uint64_t SizeOfMemRef = SizeOfWholeVector / NumReg;

  unsigned BytesCross = 0;
  bool IsStore = Opcode == Instruction::Store;

  // TODO: tune the cost model once peel/rem loops can be generated.
  // For now the change should be miniscule enough to not have
  // noticeable regressions. Consider a multiplier for Load/Store cost.
  unsigned Cost = IsStore ? CMStoreCostAdjustment : CMLoadCostAdjustment;

  float CrossProbability =
      cacheLineCrossingProbability(Alignment, SizeOfMemRef, BytesCross);
  // Add 0.5f to fight the truncation (probability is not negative).
  return static_cast<int>(Cost * CrossProbability + 0.5f) * NumReg;
}

// Public interface implementation.
int VPlanTTIWrapper::getMemoryOpCost(unsigned Opcode, Type *Src,
                                     Align Alignment, unsigned AddressSpace,
                                     TTI::TargetCostKind CostKind,
                                     const Instruction *I) const {
  int VPTTICost = Multiplier * TTI.getMemoryOpCost(Opcode, Src, Alignment,
                                                   AddressSpace, CostKind, I);

  // Return not adjusted scaled up cost for non-vector types.
  if (!isa<VectorType>(Src))
    return VPTTICost;

  int AdjustedCost =
      VPTTICost + getNonMaskedMemOpCostAdj(Opcode, Src, Alignment);

  return AdjustedCost;
}
