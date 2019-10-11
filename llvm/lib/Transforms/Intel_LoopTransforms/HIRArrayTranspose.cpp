//===--------- HIRArrayTranspose.cpp - Implements array transpose ---------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file performs array transpose on arrays allocated using mlloc and
// accessed in a strided manner. This happens when the memory region returned by
// malloc is treated like a (flattened) two dimensional array and accessed in
// column-major order instead of the usual row-major order.
//
// Here are the generic concepts of array transpose-
// 1) Array with M rows and N columns is converted to array with N rows and M
// columns. A[M][N] -> A[N][M]
//
// 2) References to elements of the arrays are similarly transposed.
// A[i][j] -> A[j][i]
//
// 3) References to flattened 2-dim array references can be similary transposed.
// A[N*i + j] -> A[j*M + i]
//
// Given flattened subscript 'CE' in form (N*i + j), we obtain row component and
// column component as follows-
//
// i = CE / N
// j = CE % N
//
// After transpose, new CE' (j*M + i) is given by the following formula-
// CE' = (CE % N) * M + (CE / N)
//
//
// 1) IR example
//
// %t1 = malloc(msize)
// %t2 = bitcast.i8*.[arrsize x elemTy]*(&%t1[0])
//
// t2 is accessed like the following, indicating 2-dim array with 10 columns and
// (msize / (sizeof(elemTy) * 10)) rows-
//
// (%t2)[0][10 * i1] =
// (%t2)[0][100 * i1 + 10 * i2 + C]
//
// Note that we ignore arrsize as it may be less than (malloc size /
// sizeof(elemTy)).
//
// We can handle references containing multiple IVs as long as their
// coefficients are evenly divisible by stride (number of columns which is 10 in
// the example). This is because they all belong to row component as explained
// above. This leaves (C / stride) as the only column component.
//
//
// 2) IR example when array base is located at an offset
//
// %t1 = malloc(msize)
// %t2 = ptrtoint.i8*.i64(&((%t1)[offset]));
// %t3 = inttoptr.i64.[arrsize x elemTy]*(%t2);
//
// (%t3)[0][10 * i1]
//
// In this case we need to transpose the offset (in number of elements) as well-
//
// offset' = (offset % N) * M +  (offset / N)
//
// To transpose the memref derived using a base with an offset-
//
// a) Add the offset to bring the memref back in terms of original base so
// (%t3)[0][10 * i1] becomes (%t1)[0][10 * i1 + offset].
//
// b) Transpose the index using row/column component as described above.
//
// c) Subtract transposed base offset (offset' above) from the resulting index.
//
//
// Currently, we only handle the offset scenario as that is the benchmark case.
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/HIRArrayTranspose.h"

#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#define DEBUG_TYPE "hir-array-transpose"

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisableTranspose("disable-hir-array-transpose",
                                      cl::init(false), cl::Hidden,
                                      cl::desc("Disable HIR Array Transpose"));

namespace {

class HIRArrayTranspose {
  HIRFramework &HIRF;

public:
  HIRArrayTranspose(HIRFramework &HIRF)
      : HIRF(HIRF), MallocArrayBaseTy(nullptr), MallocSizeInBytes(0),
        ArrayOffsetInBytes(0), ArrayElementSizeInBytes(0),
        NumRowsInNumElements(0), NumColumnsInNumElements(0) {}

  bool run();

  bool hasValidMallocs(HLRegion *Reg);

  void performTranspose();
  void propagateConstantBlobsByCloning();
  int64_t getTransposedConstant(int64_t Const) const;
  int64_t transposeArrayOffset();
  void transposeStridedRefs(int64_t TransposedArrayOffsetInElements);

private:
  class MallocAnalyzer;

  // Used to handle stores containing a blob which is a constant in the
  // previous if-else case-
  //
  // if () {
  //  %t = const1;
  // } e;se {
  //  %t = const2;
  // }
  // A[20*i1 + %t] = %t1
  struct StoreWithConstantTempBlob {
    HLDDNode *Node;
    unsigned BlobIndex;
    int64_t ThenCaseBlobValue;
    int64_t ElseCaseBlobValue;

    StoreWithConstantTempBlob(HLDDNode *Node, unsigned BlobIndex,
                              int64_t ThenValue, int64_t ElseValue)
        : Node(Node), BlobIndex(BlobIndex), ThenCaseBlobValue(ThenValue),
          ElseCaseBlobValue(ElseValue) {}
  };

  // Used for escape analysis.
  DenseSet<unsigned> AllLvalTempSymbases;

  // Currently we only handle cases where all mallocs in the region are the same
  // size and are accessed with the same stride. We can extend this logic by
  // grouping mallocs according to size and stride and constructing following
  // structures for each group.

  // Set of malloc pointers with same size allocation.
  SmallSet<unsigned, 4> MallocPtrSymbases;

  // Set of temps which access mallocs using inttoptr instruction.
  SmallSet<unsigned, 16> MallocPtrToIntSymbases;

  // Set of temps which use the malloc allocated memory by accessing it as a
  // flattened, 2-dim array.
  SmallSet<unsigned, 16> MallocArrayBaseSymbases;

  // Set of temps which are used to free mallocs.
  SmallSet<unsigned, 4> FreePtrSymbases;

  // Set of address of refs used in ptrtoint insts. These will be used to
  // transpose the base of strided refs by changing the offset.
  SmallVector<RegDDRef *, 4> MallocOffsetRefs;

  // GEP refs using malloc allocated memory in strided manner.
  SmallVector<RegDDRef *, 32> StridedGEPRefUses;

  // Memrefs using the malloc allocated memory and containing a blob.
  SmallVector<StoreWithConstantTempBlob, 32> StoreWithConstantTempBlobUses;

  // Type of temps stored in MallocArrayBaseSymbases.
  Type *MallocArrayBaseTy;

  int64_t MallocSizeInBytes;

  // Offset of array base temps from the initial malloc address.
  int64_t ArrayOffsetInBytes;

  int64_t ArrayElementSizeInBytes;

  // Number of rows in the 2-dim array.
  int64_t NumRowsInNumElements;

  // Number of columns in the 2-dim array.
  int64_t NumColumnsInNumElements;
};
} // namespace

bool HIRArrayTranspose::run() {
  if (DisableTranspose) {
    LLVM_DEBUG(dbgs() << "HIR array transpose disabled \n");
    return false;
  }

  // Expect a function level region.
  if (HIRF.hir_begin() == HIRF.hir_end()) {
    return false;
  }

  auto *Reg = cast<HLRegion>(HIRF.hir_begin());
  if (!Reg->isFunctionLevel()) {
    return false;
  }

  if (!hasValidMallocs(Reg)) {
    return false;
  }

  performTranspose();

  Reg->setGenCode();

  return true;
}

class HIRArrayTranspose::MallocAnalyzer : public HLNodeVisitorBase {
  HIRArrayTranspose &HAT;
  bool HasValidMallocs;

private:
  void processPtrToIntInst(HLInst *HInst);
  void processIntToPtrInst(HLInst *HInst, const IntToPtrInst *ITPInst);
  void processCopy(HLInst *HInst);
  void processMalloc(HLInst *HInst);
  void processFree(HLInst *HInst);

  bool isValidStridedUseRef(RegDDRef *Ref);
  bool isValidUseRef(RegDDRef *Ref);
  void performEscapeAnalysis(HLDDNode *Node);

  static bool isMalloc(HLInst *HInst);
  static bool isFree(HLInst *HInst);

  // Returns the constant value of \p TempSymbase in \p Value by iterating
  // backwards starting from Node and looking for the temp definition.
  static bool hasConstValue(HLNode *Node, unsigned TempSymbase, int64_t *Value);

  // Returns the constant then/else value of \p TempBlobIndex from the previous
  // if node.
  static bool hasConstThenElseValue(RegDDRef *Ref, unsigned TempBlobIndex,
                                    int64_t *ThenValue, int64_t *ElseValue);

  /// Returns stride of \p MemRef at \p NodeLevel as the first return value. If
  /// the CE contains a single blob, returns its index as the second return
  /// value. Returns 0 as the invalid stride.
  static std::pair<int64_t, unsigned> getArrayRefStride(RegDDRef *MemRef,
                                                        unsigned NodeLevel);

public:
  MallocAnalyzer(HIRArrayTranspose &HAT) : HAT(HAT), HasValidMallocs(true) {}

  void visit(HLInst *HInst);

  void visit(HLDDNode *Node) { performEscapeAnalysis(Node); }

  void visit(HLNode *) {}
  void postVisit(HLNode *) {}

  bool isDone() const { return !HasValidMallocs; }

  bool foundStridedMallocs() const {
    return (HasValidMallocs && (HAT.NumColumnsInNumElements != 0));
  }
};

void HIRArrayTranspose::MallocAnalyzer::processPtrToIntInst(HLInst *HInst) {
  // Looking for instruction of the form-
  // %t2 = ptrtoint.i8*.i64(&((%tMalloc)[offset]))

  // No mallocs have been encoutered yet so we can ignore such instructions.
  if (HAT.MallocPtrSymbases.empty()) {
    return;
  }

  auto RvalRef = HInst->getRvalDDRef();
  unsigned Symbase = RvalRef->getBasePtrSymbase();

  // Does not belong to malloc.
  if (!HAT.MallocPtrSymbases.count(Symbase)) {
    return;
  }

  auto CE = RvalRef->getDimensionIndex(1);
  int64_t Offset;

  if (!CE->isIntConstant(&Offset) || (Offset < 0)) {
    // Cannot analyze malloc offset.
    HasValidMallocs = false;
    return;
  }

  if (HAT.MallocPtrToIntSymbases.empty()) {
    HAT.ArrayOffsetInBytes = Offset;

  } else if (HAT.ArrayOffsetInBytes != Offset) {
    HasValidMallocs = false;
    return;
  }

  HAT.MallocPtrToIntSymbases.insert(HInst->getLvalDDRef()->getSymbase());

  HAT.MallocOffsetRefs.push_back(RvalRef);
}

void HIRArrayTranspose::MallocAnalyzer::processIntToPtrInst(
    HLInst *HInst, const IntToPtrInst *ITPInst) {
  // Looking for instruction of the form-
  // %t3 = inttoptr.i64.[arrsize x elemTy]*(%tMallocPtrToInt);

  // No malloc pointer to int temps have been encountered yet so we can ignore
  // such instructions.
  if (HAT.MallocPtrToIntSymbases.empty()) {
    return;
  }

  auto RvalRef = HInst->getRvalDDRef();

  if (!RvalRef->isSelfBlob()) {
    HasValidMallocs = false;
    return;
  }

  unsigned Symbase = RvalRef->getSymbase();

  // Rval is not related to malloc.
  if (!HAT.MallocPtrToIntSymbases.count(Symbase)) {
    return;
  }

  auto DestTy = ITPInst->getDestTy();
  auto PElemTy = DestTy->getPointerElementType();
  unsigned LvalSymbase = HInst->getLvalDDRef()->getSymbase();

  if (HAT.MallocArrayBaseTy) {
    if (HAT.MallocArrayBaseTy == DestTy) {
      HAT.MallocArrayBaseSymbases.insert(LvalSymbase);

    } else {
      // We do not cast the destination type to an array for freeing it.
      // %tFree = inttoptr.i64.elemTy*(%tMallocPtrToInt);
      // @free(&((i8*)(%tFree)[offset]));
      if (!PElemTy->isSized() || (PElemTy->getPrimitiveSizeInBits() !=
                                  uint64_t(HAT.ArrayElementSizeInBytes * 8))) {
        // Give up if destination type size does not match element size.
        HasValidMallocs = false;
        return;
      }
      HAT.FreePtrSymbases.insert(LvalSymbase);
    }

    return;
  } else {
    HAT.MallocArrayBaseSymbases.insert(LvalSymbase);
  }

  // Validate destination type.
  auto ArrTy = dyn_cast<ArrayType>(PElemTy);

  if (!ArrTy) {
    // Unexpected type conversion.
    HasValidMallocs = false;
    return;
  }

  auto ArrElemTy = ArrTy->getElementType();

  if (!ArrElemTy->isIntegerTy() && !ArrElemTy->isFloatingPointTy()) {
    // Only allow int/fp types to keep it simple.
    HasValidMallocs = false;
    return;
  }

  HAT.MallocArrayBaseTy = DestTy;
  HAT.ArrayElementSizeInBytes =
      HInst->getCanonExprUtils().getTypeSizeInBytes(ArrElemTy);

  if ((HAT.MallocSizeInBytes % HAT.ArrayElementSizeInBytes) != 0) {
    // Cannot transpose if malloc size is not evenly divisible by array element
    // size.
    HasValidMallocs = false;
    return;
  }

  if ((HAT.ArrayOffsetInBytes % HAT.ArrayElementSizeInBytes) != 0) {
    // Cannot transpose the array base if array offset is not evenly divisible
    // by element size.
    HasValidMallocs = false;
    return;
  }
}

bool HIRArrayTranspose::MallocAnalyzer::isMalloc(HLInst *HInst) {
  auto Call = HInst->getCallInst();

  if (!Call) {
    return false;
  }

  Function *Func = Call->getCalledFunction();
  return (Func && (Func->getName() == "malloc"));
}

bool HIRArrayTranspose::MallocAnalyzer::isFree(HLInst *HInst) {
  auto Call = HInst->getCallInst();

  if (!Call) {
    return false;
  }

  Function *Func = Call->getCalledFunction();
  return (Func && (Func->getName() == "free"));
}

void HIRArrayTranspose::MallocAnalyzer::processCopy(HLInst *HInst) {
  auto LvalRef = HInst->getLvalDDRef();
  unsigned LvalSymbase = LvalRef->getSymbase();

  if (!HAT.MallocPtrSymbases.empty()) {
    auto RvalRef = HInst->getRvalDDRef();
    unsigned RvalSymbase = RvalRef->isTerminalRef()
                               ? RvalRef->getSymbase()
                               : RvalRef->getBasePtrSymbase();

    if (HAT.MallocPtrToIntSymbases.count(RvalSymbase)) {

      // We found a copy %t1 = %tmalloc where %tmalloc was obtained by
      // converting the malloc into an integer. We need to make sure that either
      // %t1 also belongs to the same set of temps or it has never been visited
      // before. This is to avoid handling cases like this- if() {
      //   %t1 = %t2;
      // } else {
      //   %t1 = %tmalloc;
      // }
      //
      // In the above case, we cannot incorporate %t1 in the malloc converted
      // set so we have to give up.
      if (!HAT.MallocPtrToIntSymbases.count(LvalSymbase) &&
          HAT.AllLvalTempSymbases.count(LvalSymbase)) {
        HasValidMallocs = false;
      }

      HAT.MallocPtrToIntSymbases.insert(LvalSymbase);

    } else if (HAT.MallocArrayBaseSymbases.count(RvalSymbase)) {
      // Only allow copy of the form %t1 = &(%t2)[0]
      if (!RvalRef->getDimensionIndex(1)->isZero()) {
        HasValidMallocs = false;
      } else if (!HAT.MallocArrayBaseSymbases.count(LvalSymbase) &&
                 HAT.AllLvalTempSymbases.count(LvalSymbase)) {
        // Same logic as for MallocPtrToIntSymbases above.
        HasValidMallocs = false;
      }

      HAT.MallocArrayBaseSymbases.insert(LvalSymbase);

    } else if (HAT.MallocPtrSymbases.count(RvalSymbase) ||
               HAT.FreePtrSymbases.count(RvalSymbase)) {
      // Do not allow any other kind of malloc temp to be copied.
      HasValidMallocs = false;
    } else if (HAT.MallocPtrSymbases.count(LvalSymbase) ||
               HAT.MallocPtrToIntSymbases.count(LvalSymbase) ||
               HAT.MallocArrayBaseSymbases.count(LvalSymbase) ||
               HAT.FreePtrSymbases.count(LvalSymbase)) {
      // Do not allow malloc temps to be overwritten.
      HasValidMallocs = false;
    }
  }

  HAT.AllLvalTempSymbases.insert(LvalSymbase);
}

void HIRArrayTranspose::MallocAnalyzer::processMalloc(HLInst *HInst) {
  auto ArgOpRef = HInst->getOperandDDRef(1);

  int64_t IntVal;

  if (!ArgOpRef->isIntConstant(&IntVal)) {
    HasValidMallocs = false;
    return;
  }

  if (HAT.MallocPtrSymbases.empty()) {
    HAT.MallocSizeInBytes = IntVal;

  } else if (HAT.MallocSizeInBytes != IntVal) {
    HasValidMallocs = false;
  }

  HAT.MallocPtrSymbases.insert(HInst->getLvalDDRef()->getSymbase());
}

void HIRArrayTranspose::MallocAnalyzer::processFree(HLInst *HInst) {
  auto ArgOpRef = HInst->getOperandDDRef(0);
  unsigned Symbase = ArgOpRef->getBasePtrSymbase();

  // Verify that free call looks like this-
  // @free(&((i8*)(%FreePtrSymbase)[-offset]));
  if (!HAT.FreePtrSymbases.count(Symbase)) {
    HasValidMallocs = false;
    return;
  }

  if (ArgOpRef->getNumDimensions() != 1) {
    HasValidMallocs = false;
    return;
  }

  auto CE = ArgOpRef->getDimensionIndex(1);

  if (!CE->isIntConstant()) {
    HasValidMallocs = false;
    return;
  }

  // This is treated just like other uses which need to be transposed.
  HAT.StridedGEPRefUses.push_back(ArgOpRef);
}

std::pair<int64_t, unsigned>
HIRArrayTranspose::MallocAnalyzer::getArrayRefStride(RegDDRef *MemRef,
                                                     unsigned NodeLevel) {
  // Looking for ref of type A[0][C1*i1 + C2*i2 + ... + Cn*in + C] where Cn is
  // greater than 1 and evenly divides (C1...Cn-1).

  if (NodeLevel == 0) {
    return std::make_pair(0, 0);
  }

  if ((MemRef->getNumDimensions() != 2) ||
      !MemRef->getDimensionIndex(2)->isZero()) {
    return std::make_pair(0, 0);
  }

  auto CE = MemRef->getDimensionIndex(1);

  if (CE->getDenominator() != 1) {
    return std::make_pair(0, 0);
  }

  unsigned NumBlobs = CE->numBlobs();
  unsigned SingleBlobIndex = InvalidBlobIndex;

  if (NumBlobs > 1) {
    return std::make_pair(0, 0);
  } else if (NumBlobs == 1) {
    // We need to handle single blob case-
    //   if ()
    //   {
    //      %1852 = -20000;
    //   }
    //   else
    //   {
    //      %1852 = 200035;
    //   }
    //   (%1359)[0][20 * i2 + %1852] = %1853;
    SingleBlobIndex = CE->getSingleBlobIndex();
  }

  unsigned Index;
  int64_t InnermostCoeff;

  CE->getIVCoeff(NodeLevel, &Index, &InnermostCoeff);

  if ((InnermostCoeff <= 1) || (Index != InvalidBlobIndex)) {
    return std::make_pair(0, 0);
  }

  int64_t Coeff;

  for (unsigned I = 1; I < NodeLevel; ++I) {
    CE->getIVCoeff(I, &Index, &Coeff);

    if ((Coeff < 0) || (Coeff % InnermostCoeff != 0) ||
        (Index != InvalidBlobIndex)) {
      return std::make_pair(0, 0);
    }
  }

  return std::make_pair(InnermostCoeff, SingleBlobIndex);
}

bool HIRArrayTranspose::MallocAnalyzer::hasConstValue(HLNode *Node,
                                                      unsigned TempSymbase,
                                                      int64_t *Value) {

  // Iterate backwards to find temp definition.
  for (; Node; Node = Node->getPrevNode()) {
    auto Inst = dyn_cast<HLInst>(Node);

    if (!Inst) {
      break;
    }

    auto LvalRef = Inst->getLvalDDRef();

    if (!LvalRef || (LvalRef->getSymbase() != TempSymbase)) {
      continue;
    }

    // Obtain constant value from the CanonExpr.
    auto CE = LvalRef->getSingleCanonExpr();

    return CE->isIntConstant(Value);
  }

  return false;
}

bool HIRArrayTranspose::MallocAnalyzer::hasConstThenElseValue(
    RegDDRef *Ref, unsigned TempBlobIndex, int64_t *ThenValue,
    int64_t *ElseValue) {
  auto PrevIfNode = dyn_cast_or_null<HLIf>(Ref->getHLDDNode()->getPrevNode());

  if (!PrevIfNode) {
    return false;
  }

  auto &BU = Ref->getBlobUtils();

  if (!BU.isTempBlob(TempBlobIndex)) {
    return false;
  }

  auto TempSymbase = BU.getTempBlobSymbase(TempBlobIndex);

  if (!hasConstValue(PrevIfNode->getLastThenChild(), TempSymbase, ThenValue)) {
    return false;
  }

  if (!hasConstValue(PrevIfNode->getLastElseChild(), TempSymbase, ElseValue)) {
    return false;
  }

  return true;
}

bool HIRArrayTranspose::MallocAnalyzer::isValidStridedUseRef(RegDDRef *Ref) {
  int64_t Stride;
  unsigned SingleBlobIndex;

  auto DestTy = Ref->getBitCastDestType();

  if (DestTy) {
    auto DestElemTy = DestTy->getPointerElementType();
    // We have to give up if we are accessing number of bytes greater than
    // element size because consecutive elements before transpose are not
    // consecutive after transpose.
    if (!DestElemTy->isSized() || (DestElemTy->getPrimitiveSizeInBits() >
                                   uint64_t(HAT.ArrayElementSizeInBytes * 8))) {
      return false;
    }
  }

  std::tie(Stride, SingleBlobIndex) =
      getArrayRefStride(Ref, Ref->getNodeLevel());

  if (!Stride) {
    return false;
  } else if (!HAT.NumColumnsInNumElements) {

    HAT.NumColumnsInNumElements = Stride;
    HAT.NumRowsInNumElements =
        HAT.MallocSizeInBytes / (Stride * HAT.ArrayElementSizeInBytes);

  } else if (HAT.NumColumnsInNumElements != Stride) {
    return false;
  }

  if (SingleBlobIndex != InvalidBlobIndex) {
    // We need to handle single blob case-
    //   if ()
    //   {
    //      %1852 = -20000;
    //   }
    //   else
    //   {
    //      %1852 = 200035;
    //   }
    //   (%1359)[0][20 * i2 + %1852] = %1853;
    //
    // Only handle stores to make transformation easier.
    if (!Ref->isLval()) {
      return false;
    }

    int64_t ThenValue, ElseValue;
    if (!hasConstThenElseValue(Ref, SingleBlobIndex, &ThenValue, &ElseValue)) {
      return false;
    }

    HAT.StoreWithConstantTempBlobUses.emplace_back(
        Ref->getHLDDNode(), SingleBlobIndex, ThenValue, ElseValue);
  }

  HAT.StridedGEPRefUses.push_back(Ref);

  return true;
}

bool HIRArrayTranspose::MallocAnalyzer::isValidUseRef(RegDDRef *Ref) {

  if (Ref->isLval() && Ref->isTerminalRef()) {

    unsigned LvalSymbase = Ref->getSymbase();

    // Do not allow malloc temps to be overwritten.
    if (HAT.MallocPtrSymbases.count(LvalSymbase) ||
        HAT.MallocPtrToIntSymbases.count(LvalSymbase) ||
        HAT.MallocArrayBaseSymbases.count(LvalSymbase) ||
        HAT.FreePtrSymbases.count(LvalSymbase)) {
      return false;
    }

    HAT.AllLvalTempSymbases.insert(Ref->getSymbase());

    return true;
  }

  // No mallocs found yet.
  if (HAT.MallocPtrSymbases.empty()) {
    return true;
  }

  if (Ref->isAddressOf()) {
    // Only allow &MallocPtr[0] usage in non-insts.
    unsigned Symbase = Ref->getBasePtrSymbase();

    if (HAT.MallocPtrSymbases.count(Symbase)) {
      if (!Ref->getDimensionIndex(1)->isZero() ||
          isa<HLInst>(Ref->getHLDDNode())) {
        return false;
      }
    }

    if (HAT.MallocArrayBaseSymbases.count(Symbase) ||
        HAT.FreePtrSymbases.count(Symbase)) {
      return false;
    }

  } else if (Ref->isMemRef()) {
    // Only allow ArrayBase[0][stride * IV] usage.
    unsigned Symbase = Ref->getBasePtrSymbase();

    if (HAT.MallocPtrSymbases.count(Symbase) ||
        HAT.FreePtrSymbases.count(Symbase)) {
      return false;
    } else if (HAT.MallocArrayBaseSymbases.count(Symbase) &&
               !isValidStridedUseRef(Ref)) {
      return false;
    }
  }

  SmallVector<unsigned, 6> Symbases;
  Ref->populateTempBlobSymbases(Symbases);

  for (auto SB : Symbases) {
    if (HAT.MallocPtrToIntSymbases.count(SB)) {
      return false;
    }
  }

  return true;
}

void HIRArrayTranspose::MallocAnalyzer::performEscapeAnalysis(HLDDNode *Node) {

  // Check usage of malloc related temps.
  for (auto RefIt = Node->op_ddref_begin(), E = Node->op_ddref_end();
       RefIt != E; ++RefIt) {
    auto Ref = *RefIt;

    if (!isValidUseRef(Ref)) {
      HasValidMallocs = false;
      break;
    }
  }
}

void HIRArrayTranspose::MallocAnalyzer::visit(HLInst *HInst) {
  auto Inst = HInst->getLLVMInstruction();

  if (isa<PtrToIntInst>(Inst)) {
    processPtrToIntInst(HInst);
    return;

  } else if (auto ITPInst = dyn_cast<IntToPtrInst>(Inst)) {
    processIntToPtrInst(HInst, ITPInst);
    return;

  } else if (HInst->isCopyInst()) {
    processCopy(HInst);
    return;

  } else if (isMalloc(HInst)) {
    processMalloc(HInst);
    return;

  } else if (isFree(HInst)) {
    processFree(HInst);
    return;
  }

  performEscapeAnalysis(HInst);
}

bool HIRArrayTranspose::hasValidMallocs(HLRegion *Reg) {
  MallocAnalyzer MA(*this);

  HLNodeUtils::visitRange(MA, Reg->child_begin(), Reg->child_end());

  return MA.foundStridedMallocs();
}

void HIRArrayTranspose::propagateConstantBlobsByCloning() {

  // Transform this-
  //
  //   if ()
  //   {
  //      %1852 = -20000;
  //   }
  //   else
  //   {
  //      %1852 = 200035;
  //   }
  //   (%1359)[0][20 * i2 + %1852] = %1853;
  //
  // Into this-
  //
  //   if ()
  //   {
  //      %1852 = -20000;
  //      (%1359)[0][20 * i2 + -20000] = %1853;
  //   }
  //   else
  //   {
  //      %1852 = 200035;
  //      (%1359)[0][20 * i2 + 200035] = %1853;
  //   }
  for (auto &Store : StoreWithConstantTempBlobUses) {
    auto ElseClone = Store.Node->clone();
    auto PrevIfNode = cast<HLIf>(Store.Node->getPrevNode());

    HLNodeUtils::moveAsLastThenChild(PrevIfNode, Store.Node);
    HLNodeUtils::insertAsLastElseChild(PrevIfNode, ElseClone);

    // Replace blob with stored constant.
    auto ThenLval = Store.Node->getLvalDDRef();
    auto CE = ThenLval->getDimensionIndex(1);
    CE->replaceTempBlobByConstant(Store.BlobIndex, Store.ThenCaseBlobValue);
    ThenLval->makeConsistent();

    auto ElseLval = cast<HLDDNode>(ElseClone)->getLvalDDRef();
    CE = ElseLval->getDimensionIndex(1);
    CE->replaceTempBlobByConstant(Store.BlobIndex, Store.ElseCaseBlobValue);
    ElseLval->makeConsistent();

    // Push newly created extra use ref.
    StridedGEPRefUses.push_back(ElseLval);
  }
}

int64_t HIRArrayTranspose::getTransposedConstant(int64_t Const) const {
  // RowComp = C / NumColumns
  // ColComp = C % NumColumns
  // Transposed constant = RowComp + ColComp * NumRows

  int64_t RowComponent = Const / NumColumnsInNumElements;
  int64_t ColComponent = Const % NumColumnsInNumElements;

  return RowComponent + (ColComponent * NumRowsInNumElements);
}

int64_t HIRArrayTranspose::transposeArrayOffset() {
  int64_t NewOffsetInElements =
      getTransposedConstant(ArrayOffsetInBytes / ArrayElementSizeInBytes);

  for (auto Ref : MallocOffsetRefs) {
    Ref->getDimensionIndex(1)->setConstant(NewOffsetInElements *
                                           ArrayElementSizeInBytes);
  }

  return NewOffsetInElements;
}

void HIRArrayTranspose::transposeStridedRefs(
    int64_t TransposedArrayOffsetInElements) {

  int64_t ArrayOffsetInElements = ArrayOffsetInBytes / ArrayElementSizeInBytes;

  for (auto Ref : StridedGEPRefUses) {
    auto CE = Ref->getDimensionIndex(1);

    assert(!CE->hasBlob() && "blobs in CE not expected!");

    // Add mallloc offset to bring the index in terms of original malloc base
    // pointer.
    CE->addConstant(ArrayOffsetInElements, false);

    // First handle IV coefficients. These are all part of row component as they
    // are evenly divisible by number of columns.
    for (auto IV = CE->iv_begin(), E = CE->iv_end(); IV != E; ++IV) {
      int64_t Coeff = CE->getIVConstCoeff(IV);

      if (Coeff == 0) {
        continue;
      }

      CE->setIVConstCoeff(IV,
                          CE->getIVConstCoeff(IV) / NumColumnsInNumElements);
    }

    // Now handle constant which can have both row and column component.
    CE->setConstant(getTransposedConstant(CE->getConstant()));

    // Now subtract transposed array offset from the index so we can continue
    // using the offsetted base.
    CE->addConstant(-TransposedArrayOffsetInElements, false);
  }
}

void HIRArrayTranspose::performTranspose() {

  // First handle refs with blobs as these lead to additional strided ref uses
  // due to cloning.
  propagateConstantBlobsByCloning();

  int64_t TransposedOffset = transposeArrayOffset();

  transposeStridedRefs(TransposedOffset);
}

PreservedAnalyses
HIRArrayTransposePass::run(llvm::Function &F,
                           llvm::FunctionAnalysisManager &AM) {
  HIRArrayTranspose(AM.getResult<HIRFrameworkAnalysis>(F)).run();
  return PreservedAnalyses::all();
}

class HIRArrayTransposeLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRArrayTransposeLegacyPass() : HIRTransformPass(ID) {
    initializeHIRArrayTransposeLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.setPreservesAll();
  }

  bool runOnFunction(Function &F) {
    if (skipFunction(F)) {
      LLVM_DEBUG(dbgs() << "HIR array transpose skipped\n");
      return false;
    }

    return HIRArrayTranspose(getAnalysis<HIRFrameworkWrapperPass>().getHIR())
        .run();
  }
};

char HIRArrayTransposeLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRArrayTransposeLegacyPass, "hir-array-transpose",
                      "HIR Array Transpose", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_END(HIRArrayTransposeLegacyPass, "hir-array-transpose",
                    "HIR Array Transpose", false, false)

FunctionPass *llvm::createHIRArrayTransposePass() {
  return new HIRArrayTransposeLegacyPass();
}
