//===----------- VectorUtils.cpp - Vectorizer utility functions -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines vectorizer utilities.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/EquivalenceClasses.h"
#include "llvm/Analysis/DemandedBits.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/GetElementPtrTypeIterator.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Constants.h"
#include "llvm/Analysis/Intel_VectorVariant.h" // INTEL

using namespace llvm;
using namespace llvm::PatternMatch;

/// \brief Identify if the intrinsic is trivially vectorizable.
/// This method returns true if the intrinsic's argument types are all
/// scalars for the scalar form of the intrinsic and all vectors for
/// the vector form of the intrinsic.
bool llvm::isTriviallyVectorizable(Intrinsic::ID ID) {
  switch (ID) {
  case Intrinsic::sqrt:
  case Intrinsic::sin:
  case Intrinsic::cos:
  case Intrinsic::exp:
  case Intrinsic::exp2:
  case Intrinsic::log:
  case Intrinsic::log10:
  case Intrinsic::log2:
  case Intrinsic::fabs:
  case Intrinsic::minnum:
  case Intrinsic::maxnum:
  case Intrinsic::copysign:
  case Intrinsic::floor:
  case Intrinsic::ceil:
  case Intrinsic::trunc:
  case Intrinsic::rint:
  case Intrinsic::nearbyint:
  case Intrinsic::round:
  case Intrinsic::bswap:
  case Intrinsic::bitreverse:
  case Intrinsic::ctpop:
  case Intrinsic::pow:
  case Intrinsic::fma:
  case Intrinsic::fmuladd:
  case Intrinsic::ctlz:
  case Intrinsic::cttz:
  case Intrinsic::powi:
    return true;
  default:
    return false;
  }
}

/// \brief Identifies if the intrinsic has a scalar operand. It check for
/// ctlz,cttz and powi special intrinsics whose argument is scalar.
bool llvm::hasVectorInstrinsicScalarOpd(Intrinsic::ID ID,
                                        unsigned ScalarOpdIdx) {
  switch (ID) {
  case Intrinsic::ctlz:
  case Intrinsic::cttz:
  case Intrinsic::powi:
    return (ScalarOpdIdx == 1);
  default:
    return false;
  }
}

/// \brief Returns intrinsic ID for call.
/// For the input call instruction it finds mapping intrinsic and returns
/// its ID, in case it does not found it return not_intrinsic.
Intrinsic::ID llvm::getVectorIntrinsicIDForCall(const CallInst *CI,
                                                const TargetLibraryInfo *TLI) {
  Intrinsic::ID ID = getIntrinsicForCallSite(CI, TLI);
  if (ID == Intrinsic::not_intrinsic)
    return Intrinsic::not_intrinsic;

  if (isTriviallyVectorizable(ID) || ID == Intrinsic::lifetime_start ||
      ID == Intrinsic::lifetime_end || ID == Intrinsic::assume)
    return ID;
  return Intrinsic::not_intrinsic;
}

/// \brief Find the operand of the GEP that should be checked for consecutive
/// stores. This ignores trailing indices that have no effect on the final
/// pointer.
unsigned llvm::getGEPInductionOperand(const GetElementPtrInst *Gep) {
  const DataLayout &DL = Gep->getModule()->getDataLayout();
  unsigned LastOperand = Gep->getNumOperands() - 1;
  unsigned GEPAllocSize = DL.getTypeAllocSize(Gep->getResultElementType());

  // Walk backwards and try to peel off zeros.
  while (LastOperand > 1 && match(Gep->getOperand(LastOperand), m_Zero())) {
    // Find the type we're currently indexing into.
    gep_type_iterator GEPTI = gep_type_begin(Gep);
    std::advance(GEPTI, LastOperand - 1);

    // If it's a type with the same allocation size as the result of the GEP we
    // can peel off the zero index.
    if (DL.getTypeAllocSize(*GEPTI) != GEPAllocSize)
      break;
    --LastOperand;
  }

  return LastOperand;
}

/// \brief If the argument is a GEP, then returns the operand identified by
/// getGEPInductionOperand. However, if there is some other non-loop-invariant
/// operand, it returns that instead.
Value *llvm::stripGetElementPtr(Value *Ptr, ScalarEvolution *SE, Loop *Lp) {
  GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(Ptr);
  if (!GEP)
    return Ptr;

  unsigned InductionOperand = getGEPInductionOperand(GEP);

  // Check that all of the gep indices are uniform except for our induction
  // operand.
  for (unsigned i = 0, e = GEP->getNumOperands(); i != e; ++i)
    if (i != InductionOperand &&
        !SE->isLoopInvariant(SE->getSCEV(GEP->getOperand(i)), Lp))
      return Ptr;
  return GEP->getOperand(InductionOperand);
}

/// \brief If a value has only one user that is a CastInst, return it.
Value *llvm::getUniqueCastUse(Value *Ptr, Loop *Lp, Type *Ty) {
  Value *UniqueCast = nullptr;
  for (User *U : Ptr->users()) {
    CastInst *CI = dyn_cast<CastInst>(U);
    if (CI && CI->getType() == Ty) {
      if (!UniqueCast)
        UniqueCast = CI;
      else
        return nullptr;
    }
  }
  return UniqueCast;
}

/// \brief Get the stride of a pointer access in a loop. Looks for symbolic
/// strides "a[i*stride]". Returns the symbolic stride, or null otherwise.
#if INTEL_CUSTOMIZATION
/// This function was modified to also return constant strides for the purpose
/// of analyzing call arguments (specifically, sincos calls) in order to
/// generate more efficient stores to memory. Previously, this function only
/// returned loop invariant symbolic strides for loop versioning. This expands
/// the functionality of this function to a broader set of applications.
#endif // INTEL_CUSTOMIZATION
Value *llvm::getStrideFromPointer(Value *Ptr, ScalarEvolution *SE, Loop *Lp) {
  auto *PtrTy = dyn_cast<PointerType>(Ptr->getType());
  if (!PtrTy || PtrTy->isAggregateType())
    return nullptr;

  // Try to remove a gep instruction to make the pointer (actually index at this
  // point) easier analyzable. If OrigPtr is equal to Ptr we are analzying the
  // pointer, otherwise, we are analyzing the index.
  Value *OrigPtr = Ptr;

  // The size of the pointer access.
  int64_t PtrAccessSize = 1;

  Ptr = stripGetElementPtr(Ptr, SE, Lp);
  const SCEV *V = SE->getSCEV(Ptr);

  if (Ptr != OrigPtr)
    // Strip off casts.
    while (const SCEVCastExpr *C = dyn_cast<SCEVCastExpr>(V))
      V = C->getOperand();

  const SCEVAddRecExpr *S = dyn_cast<SCEVAddRecExpr>(V);
  if (!S)
    return nullptr;

  V = S->getStepRecurrence(*SE);
  if (!V)
    return nullptr;

  // Strip off the size of access multiplication if we are still analyzing the
  // pointer.
  if (OrigPtr == Ptr) {
    if (const SCEVMulExpr *M = dyn_cast<SCEVMulExpr>(V)) {
      if (M->getOperand(0)->getSCEVType() != scConstant)
        return nullptr;

      const APInt &APStepVal = cast<SCEVConstant>(M->getOperand(0))->getAPInt();

      // Huge step value - give up.
      if (APStepVal.getBitWidth() > 64)
        return nullptr;

      int64_t StepVal = APStepVal.getSExtValue();
      if (PtrAccessSize != StepVal)
        return nullptr;
      V = M->getOperand(1);
    }
  }

  // Strip off casts.
  Type *StripedOffRecurrenceCast = nullptr;
  if (const SCEVCastExpr *C = dyn_cast<SCEVCastExpr>(V)) {
    StripedOffRecurrenceCast = C->getType();
    V = C->getOperand();
  }

#if INTEL_CUSTOMIZATION
  // Look for constant stride.
  const SCEVConstant *C = dyn_cast<SCEVConstant>(V);
  if (C) {
    return C->getValue();
  }
#endif // INTEL_CUSTOMIZATION

  // Look for the loop invariant symbolic value.
  const SCEVUnknown *U = dyn_cast<SCEVUnknown>(V);
  if (!U)
    return nullptr;

  Value *Stride = U->getValue();
  if (!Lp->isLoopInvariant(Stride))
    return nullptr;

  // If we have stripped off the recurrence cast we have to make sure that we
  // return the value that is used in this loop so that we can replace it later.
  if (StripedOffRecurrenceCast)
    Stride = getUniqueCastUse(Stride, Lp, StripedOffRecurrenceCast);

  return Stride;
}

/// \brief Given a vector and an element number, see if the scalar value is
/// already around as a register, for example if it were inserted then extracted
/// from the vector.
Value *llvm::findScalarElement(Value *V, unsigned EltNo) {
  assert(V->getType()->isVectorTy() && "Not looking at a vector?");
  VectorType *VTy = cast<VectorType>(V->getType());
  unsigned Width = VTy->getNumElements();
  if (EltNo >= Width)  // Out of range access.
    return UndefValue::get(VTy->getElementType());

  if (Constant *C = dyn_cast<Constant>(V))
    return C->getAggregateElement(EltNo);

  if (InsertElementInst *III = dyn_cast<InsertElementInst>(V)) {
    // If this is an insert to a variable element, we don't know what it is.
    if (!isa<ConstantInt>(III->getOperand(2)))
      return nullptr;
    unsigned IIElt = cast<ConstantInt>(III->getOperand(2))->getZExtValue();

    // If this is an insert to the element we are looking for, return the
    // inserted value.
    if (EltNo == IIElt)
      return III->getOperand(1);

    // Otherwise, the insertelement doesn't modify the value, recurse on its
    // vector input.
    return findScalarElement(III->getOperand(0), EltNo);
  }

  if (ShuffleVectorInst *SVI = dyn_cast<ShuffleVectorInst>(V)) {
    unsigned LHSWidth = SVI->getOperand(0)->getType()->getVectorNumElements();
    int InEl = SVI->getMaskValue(EltNo);
    if (InEl < 0)
      return UndefValue::get(VTy->getElementType());
    if (InEl < (int)LHSWidth)
      return findScalarElement(SVI->getOperand(0), InEl);
    return findScalarElement(SVI->getOperand(1), InEl - LHSWidth);
  }

  // Extract a value from a vector add operation with a constant zero.
  Value *Val = nullptr; Constant *Con = nullptr;
  if (match(V, m_Add(m_Value(Val), m_Constant(Con))))
    if (Constant *Elt = Con->getAggregateElement(EltNo))
      if (Elt->isNullValue())
        return findScalarElement(Val, EltNo);

  // Otherwise, we don't know.
  return nullptr;
}

/// \brief Get splat value if the input is a splat vector or return nullptr.
/// This function is not fully general. It checks only 2 cases:
/// the input value is (1) a splat constants vector or (2) a sequence
/// of instructions that broadcast a single value into a vector.
///
const llvm::Value *llvm::getSplatValue(const Value *V) {

  if (auto *C = dyn_cast<Constant>(V))
    if (isa<VectorType>(V->getType()))
      return C->getSplatValue();

  auto *ShuffleInst = dyn_cast<ShuffleVectorInst>(V);
  if (!ShuffleInst)
    return nullptr;
  // All-zero (or undef) shuffle mask elements.
  for (int MaskElt : ShuffleInst->getShuffleMask())
    if (MaskElt != 0 && MaskElt != -1)
      return nullptr;
  // The first shuffle source is 'insertelement' with index 0.
  auto *InsertEltInst =
    dyn_cast<InsertElementInst>(ShuffleInst->getOperand(0));
  if (!InsertEltInst || !isa<ConstantInt>(InsertEltInst->getOperand(2)) ||
      !cast<ConstantInt>(InsertEltInst->getOperand(2))->isNullValue())
    return nullptr;

  return InsertEltInst->getOperand(1);
}

MapVector<Instruction *, uint64_t>
llvm::computeMinimumValueSizes(ArrayRef<BasicBlock *> Blocks, DemandedBits &DB,
                               const TargetTransformInfo *TTI) {

  // DemandedBits will give us every value's live-out bits. But we want
  // to ensure no extra casts would need to be inserted, so every DAG
  // of connected values must have the same minimum bitwidth.
  EquivalenceClasses<Value *> ECs;
  SmallVector<Value *, 16> Worklist;
  SmallPtrSet<Value *, 4> Roots;
  SmallPtrSet<Value *, 16> Visited;
  DenseMap<Value *, uint64_t> DBits;
  SmallPtrSet<Instruction *, 4> InstructionSet;
  MapVector<Instruction *, uint64_t> MinBWs;

  // Determine the roots. We work bottom-up, from truncs or icmps.
  bool SeenExtFromIllegalType = false;
  for (auto *BB : Blocks)
    for (auto &I : *BB) {
      InstructionSet.insert(&I);

      if (TTI && (isa<ZExtInst>(&I) || isa<SExtInst>(&I)) &&
          !TTI->isTypeLegal(I.getOperand(0)->getType()))
        SeenExtFromIllegalType = true;

      // Only deal with non-vector integers up to 64-bits wide.
      if ((isa<TruncInst>(&I) || isa<ICmpInst>(&I)) &&
          !I.getType()->isVectorTy() &&
          I.getOperand(0)->getType()->getScalarSizeInBits() <= 64) {
        // Don't make work for ourselves. If we know the loaded type is legal,
        // don't add it to the worklist.
        if (TTI && isa<TruncInst>(&I) && TTI->isTypeLegal(I.getType()))
          continue;

        Worklist.push_back(&I);
        Roots.insert(&I);
      }
    }
  // Early exit.
  if (Worklist.empty() || (TTI && !SeenExtFromIllegalType))
    return MinBWs;

  // Now proceed breadth-first, unioning values together.
  while (!Worklist.empty()) {
    Value *Val = Worklist.pop_back_val();
    Value *Leader = ECs.getOrInsertLeaderValue(Val);

    if (Visited.count(Val))
      continue;
    Visited.insert(Val);

    // Non-instructions terminate a chain successfully.
    if (!isa<Instruction>(Val))
      continue;
    Instruction *I = cast<Instruction>(Val);

    // If we encounter a type that is larger than 64 bits, we can't represent
    // it so bail out.
    if (DB.getDemandedBits(I).getBitWidth() > 64)
      return MapVector<Instruction *, uint64_t>();

    uint64_t V = DB.getDemandedBits(I).getZExtValue();
    DBits[Leader] |= V;
    DBits[I] = V;

    // Casts, loads and instructions outside of our range terminate a chain
    // successfully.
    if (isa<SExtInst>(I) || isa<ZExtInst>(I) || isa<LoadInst>(I) ||
        !InstructionSet.count(I))
      continue;

    // Unsafe casts terminate a chain unsuccessfully. We can't do anything
    // useful with bitcasts, ptrtoints or inttoptrs and it'd be unsafe to
    // transform anything that relies on them.
    if (isa<BitCastInst>(I) || isa<PtrToIntInst>(I) || isa<IntToPtrInst>(I) ||
        !I->getType()->isIntegerTy()) {
      DBits[Leader] |= ~0ULL;
      continue;
    }

    // We don't modify the types of PHIs. Reductions will already have been
    // truncated if possible, and inductions' sizes will have been chosen by
    // indvars.
    if (isa<PHINode>(I))
      continue;

    if (DBits[Leader] == ~0ULL)
      // All bits demanded, no point continuing.
      continue;

    for (Value *O : cast<User>(I)->operands()) {
      ECs.unionSets(Leader, O);
      Worklist.push_back(O);
    }
  }

  // Now we've discovered all values, walk them to see if there are
  // any users we didn't see. If there are, we can't optimize that
  // chain.
  for (auto &I : DBits)
    for (auto *U : I.first->users())
      if (U->getType()->isIntegerTy() && DBits.count(U) == 0)
        DBits[ECs.getOrInsertLeaderValue(I.first)] |= ~0ULL;

  for (auto I = ECs.begin(), E = ECs.end(); I != E; ++I) {
    uint64_t LeaderDemandedBits = 0;
    for (auto MI = ECs.member_begin(I), ME = ECs.member_end(); MI != ME; ++MI)
      LeaderDemandedBits |= DBits[*MI];

    uint64_t MinBW = (sizeof(LeaderDemandedBits) * 8) -
                     llvm::countLeadingZeros(LeaderDemandedBits);
    // Round up to a power of 2
    if (!isPowerOf2_64((uint64_t)MinBW))
      MinBW = NextPowerOf2(MinBW);

    // We don't modify the types of PHIs. Reductions will already have been
    // truncated if possible, and inductions' sizes will have been chosen by
    // indvars.
    // If we are required to shrink a PHI, abandon this entire equivalence class.
    bool Abort = false;
    for (auto MI = ECs.member_begin(I), ME = ECs.member_end(); MI != ME; ++MI)
      if (isa<PHINode>(*MI) && MinBW < (*MI)->getType()->getScalarSizeInBits()) {
        Abort = true;
        break;
      }
    if (Abort)
      continue;

    for (auto MI = ECs.member_begin(I), ME = ECs.member_end(); MI != ME; ++MI) {
      if (!isa<Instruction>(*MI))
        continue;
      Type *Ty = (*MI)->getType();
      if (Roots.count(*MI))
        Ty = cast<Instruction>(*MI)->getOperand(0)->getType();
      if (MinBW < Ty->getScalarSizeInBits())
        MinBWs[cast<Instruction>(*MI)] = MinBW;
    }
  }

  return MinBWs;
}

#if INTEL_CUSTOMIZATION
// This function marks the CallInst VecCall with the appropriate stride
// information determined by getStrideFromPointer(), which is used later in
// LLVM IR generation for loads/stores. Initial use of this information is
// used during SVML translation for sincos vectorization, but could be
// applicable to any situation where we need to analyze memory references.
void llvm::analyzeCallArgMemoryReferences(CallInst *CI, CallInst *VecCall,
                                          const TargetLibraryInfo *TLI,
                                          ScalarEvolution *SE, Loop *OrigLoop)
{
  for (unsigned I = 0; I < CI->getNumArgOperands(); ++I) {

    Value *CallArg = CI->getArgOperand(I);
    GetElementPtrInst *ArgGep = dyn_cast<GetElementPtrInst>(CallArg);

    if (ArgGep) {

      Value *Stride = getStrideFromPointer(CallArg, SE, OrigLoop);
      AttrBuilder AttrList;

      if (Stride) {
        // 2nd and 3rd args to sincos should always be pointers, but assert just
        // in case.
        PointerType *PtrArgType = dyn_cast<PointerType>(CallArg->getType());

        if (PtrArgType) {

          ConstantInt *StrideConst = dyn_cast<ConstantInt>(Stride);
          if (StrideConst) {

            int64_t StrideVal = StrideConst->getSExtValue();

            // Mark the call argument with the stride value in number of
            // elements.
            AttrList.addAttribute("stride",
                                  APInt(32, StrideVal).toString(10, false));
          }
        }
      } else {
        // Undef stride means that we must treat the memory reference as
        // gather/scatter or resort to store scalarization.
        AttrList.addAttribute("stride", "indirect");
      }

      if (AttrList.hasAttributes()) {
        VecCall->setAttributes(
            VecCall->getAttributes().addAttributes(
                VecCall->getContext(), I + 1,
                AttributeSet::get(VecCall->getContext(), I + 1, AttrList)));
      }
    }
  }
}

std::vector<Attribute> llvm::getVectorVariantAttributes(Function& F) {
  std::vector<Attribute> RetVal;
  AttributeSet Attributes = F.getAttributes().getFnAttributes();
  AttributeSet::iterator ItA = Attributes.begin(0);
  AttributeSet::iterator EndA = Attributes.end(0);
  for (; ItA != EndA; ++ItA) {
    if (!ItA->isStringAttribute())
      continue;
    StringRef AttributeKind = ItA->getKindAsString();
    if (VectorVariant::isVectorVariant(AttributeKind))
      RetVal.push_back(*ItA);
  }
  return RetVal;
}

Type* llvm::calcCharacteristicType(Function& F, VectorVariant& Variant)
{
  Type* ReturnType = F.getReturnType();
  Type* CharacteristicDataType = NULL;

  if (!ReturnType->isVoidTy())
    CharacteristicDataType = ReturnType;

  if (!CharacteristicDataType) {

    std::vector<VectorKind>& ParmKinds = Variant.getParameters();
    const Function::ArgumentListType& Args = F.getArgumentList();
    Function::ArgumentListType::const_iterator ArgIt = Args.begin();
    Function::ArgumentListType::const_iterator ArgEnd = Args.end();
    std::vector<VectorKind>::iterator VKIt = ParmKinds.begin();

    for (; ArgIt != ArgEnd; ++ArgIt, ++VKIt) {
      if (VKIt->isVector()) {
        CharacteristicDataType = (*ArgIt).getType();
        break;
      }
    }
  }

  // TODO except Clang's ComplexType
  if (!CharacteristicDataType || CharacteristicDataType->isStructTy()) {
    CharacteristicDataType = Type::getInt32Ty(F.getContext());
  }

  // Promote char/short types to int for Xeon Phi.
  CharacteristicDataType =
    VectorVariant::promoteToSupportedType(CharacteristicDataType, Variant);

  if (CharacteristicDataType->isPointerTy()) {
    // For such cases as 'int* foo(int x)', where x is a non-vector type, the
    // characteristic type at this point will be i32*. If we use the DataLayout
    // to query the supported pointer size, then a promotion to i64* is
    // incorrect because the mask element type will mismatch the element type
    // of the characteristic type.
    PointerType *PointerTy = cast<PointerType>(CharacteristicDataType);
    CharacteristicDataType = PointerTy->getElementType();
  }

  return CharacteristicDataType;
}

void llvm::getFunctionsToVectorize(
  llvm::Module &M, std::map<Function*, std::vector<StringRef> > &FuncVars) {

  // FuncVars will contain a 1-many mapping between the original scalar
  // function and the vector variant encoding strings (represented as
  // attributes). The encodings correspond to functions that will be created by
  // the caller of this function as vector versions of the original function.
  // For example, if foo() is a function marked as a simd function, it will have
  // several vector variant encodings like: "_ZGVbM4_foo", "_ZGVbN4_foo",
  // "_ZGVcM8_foo", "_ZGVcN8_foo", "_ZGVdM8_foo", "_ZGVdN8_foo", "_ZGVeM16_foo",
  // "_ZGVeN16_foo". The caller of this function will then clone foo() and name
  // the clones using the above name manglings. The variant encodings correspond
  // to differences in masked/non-masked execution, vector length, and target
  // vector register size, etc. For more details, please refer to the following
  // reference for details on the vector function encodings.
  // https://www.cilkplus.org/sites/default/files/open_specifications/
  // Intel-ABI-Vector-Function-2012-v0.9.5.pdf

  for (auto It = M.begin(), End = M.end(); It != End; ++It) {
    Function &F = *It;
    if (F.hasFnAttribute("vector-variants")) {
      Attribute Attr = F.getFnAttribute("vector-variants");
      StringRef VariantsStr = Attr.getValueAsString();
      SmallVector<StringRef, 8> Variants;
      VariantsStr.split(Variants, ',');
      for (unsigned i = 0; i < Variants.size(); i++) {
        FuncVars[&F].push_back(Variants[i]);
      }
    }
  }
}

#if INTEL_OPENCL
bool llvm::isOpenCLReadChannel(StringRef FnName) {
  return (FnName == "__read_pipe_2_bl_intel");
}

bool llvm::isOpenCLWriteChannel(StringRef FnName) {
  return (FnName == "__write_pipe_2_bl_intel");
}

bool llvm::isOpenCLReadChannelDest(StringRef FnName, unsigned i) {
  return (isOpenCLReadChannel(FnName) && i == 1);
}

bool llvm::isOpenCLWriteChannelSrc(StringRef FnName, unsigned i) {
  return (isOpenCLWriteChannel(FnName) && i == 1);
}

Value* llvm::getOpenCLReadChannelDestAlloc(const CallInst *Call) {

  AddrSpaceCastInst *Arg = dyn_cast<AddrSpaceCastInst>(Call->getArgOperand(1));

  assert(Arg && "Expected addrspacecast in traceback of __read_pipe argument");

  BitCastInst *ArgCast = dyn_cast<BitCastInst>(Arg->getOperand(0));
  assert(ArgCast && "Expected bitcast in traceback of __read_pipe argument");

  AllocaInst *ReadDst = dyn_cast<AllocaInst>(ArgCast->getOperand(0));
  assert(ReadDst && "Expected alloca in traceback of __read_pipe argument");

  return ReadDst;
}
#endif // INTEL_OPENCL

Function* llvm::getOrInsertVectorFunction(const CallInst *Call, unsigned VL,
                                          SmallVectorImpl<Type*> &ArgTys,
                                          TargetLibraryInfo *TLI,
                                          Intrinsic::ID ID,
                                          VectorVariant *VecVariant,
                                          bool Masked) {

  // OrigF is the original scalar function being called. Widen the scalar
  // call to a vector call if it is known to be vectorizable as SVML or
  // an intrinsic.
  Function *OrigF = Call->getCalledFunction();
  StringRef FnName = OrigF->getName();
  if (!TLI->isFunctionVectorizable(FnName, VL) && !ID && !VecVariant) {
    return nullptr;
  }

  Module *M = OrigF->getParent();
  Function *VectorF = nullptr;
  Type *RetTy = OrigF->getReturnType();
  Type *VecRetTy = RetTy;
  if (!RetTy->isVoidTy()) {
    VecRetTy = VectorType::get(RetTy, VL);
  }

  if (VecVariant) {
    std::string VFnName = VecVariant->encode() + FnName.str();
    VectorF = M->getFunction(VFnName);
    if (!VectorF) {
      FunctionType *FTy = FunctionType::get(VecRetTy, ArgTys, false);
      VectorF =
        Function::Create(FTy, Function::ExternalLinkage, VFnName, M);
      VectorF->copyAttributesFrom(OrigF);
    }
  } else if (ID) {
    // Generate a vector intrinsic. Remember, all intrinsics defined in
    // Intrinsics.td that can be vectorized are those for which the return
    // type matches the call arguments. Thus, TysForDecl should only contain
    // 1 type in order to be able to generate the right declaration. Inserting
    // multiple instances of this type will cause assertions when attempting
    // to generate the declaration. This code will need to be changed to
    // support different types of function signatures.
    assert(!RetTy->isVoidTy() && "Expected non-void function");
    for (unsigned i = 0; i < ArgTys.size(); i++) {
      assert(VecRetTy == ArgTys[i] && "Expected return type to match arg type");
    }
    SmallVector<Type*, 1> TysForDecl;
    TysForDecl.push_back(VecRetTy);
    VectorF = Intrinsic::getDeclaration(M, ID, TysForDecl);
  } else {
    // Generate a vector library call.
    StringRef VFnName = TLI->getVectorizedFunction(FnName, VL, Masked);
    VectorF = M->getFunction(VFnName);
    if (!VectorF) {
      // isFunctionVectorizable() returned true, so it is guaranteed that
      // the svml function exists and the call is legal. Generate a declaration
      // for it if one does not already exist.
#if INTEL_OPENCL
      if (isOpenCLReadChannel(FnName)) {
        // The return type of the vector read channel call is a vector of the
        // pointer element type of the read destination pointer alloca. The
        // function call below traces back through bitcast instructions to
        // find the alloca.
        Value *ReadDst = getOpenCLReadChannelDestAlloc(Call);
        VecRetTy =
          VectorType::get(ReadDst->getType()->getPointerElementType(), VL);
      }
      if (isOpenCLWriteChannel(FnName)) {
        VecRetTy = RetTy;
      }
#endif // INTEL_OPENCL
      FunctionType *FTy = FunctionType::get(VecRetTy, ArgTys, false);
      VectorF = Function::Create(FTy, Function::ExternalLinkage, VFnName, M);
      // Note: The function signature is different for the vector version of
      // these functions. E.g., in the case of __read_pipe the 2nd parameter
      // is dropped, and for __write_pipe the 2nd parameter becomes vector of
      // float instead of a pointer. Thus, the attributes cannot blindly be
      // copied because some attributes for the parameters on the original
      // scalar call will be incompatible with the vector parameter types.
      // Or, in the case of __read_pipe, the attribute for the 2nd parameter
      // will still be copied to the vector call site and will result in an
      // assert in the verifier because there is no longer a 2nd parameter.
#if INTEL_OPENCL
      if (!isOpenCLReadChannel(FnName) && !isOpenCLWriteChannel(FnName))
#endif
        VectorF->copyAttributesFrom(OrigF);
    }
  }

  assert(VectorF && "Can't create vector function.");
  return VectorF;
}
#endif // INTEL_CUSTOMIZATION

/// \returns \p I after propagating metadata from \p VL.
Instruction *llvm::propagateMetadata(Instruction *Inst, ArrayRef<Value *> VL) {
  Instruction *I0 = cast<Instruction>(VL[0]);
  SmallVector<std::pair<unsigned, MDNode *>, 4> Metadata;
  I0->getAllMetadataOtherThanDebugLoc(Metadata);

  for (auto Kind :
       {LLVMContext::MD_tbaa, LLVMContext::MD_alias_scope,
        LLVMContext::MD_noalias, LLVMContext::MD_fpmath,
        LLVMContext::MD_nontemporal, LLVMContext::MD_invariant_load}) {
    MDNode *MD = I0->getMetadata(Kind);

    for (int J = 1, E = VL.size(); MD && J != E; ++J) {
      const Instruction *IJ = cast<Instruction>(VL[J]);
      MDNode *IMD = IJ->getMetadata(Kind);
      switch (Kind) {
      case LLVMContext::MD_tbaa:
        MD = MDNode::getMostGenericTBAA(MD, IMD);
        break;
      case LLVMContext::MD_alias_scope:
        MD = MDNode::getMostGenericAliasScope(MD, IMD);
        break;
      case LLVMContext::MD_fpmath:
        MD = MDNode::getMostGenericFPMath(MD, IMD);
        break;
      case LLVMContext::MD_noalias:
      case LLVMContext::MD_nontemporal:
      case LLVMContext::MD_invariant_load:
        MD = MDNode::intersect(MD, IMD);
        break;
      default:
        llvm_unreachable("unhandled metadata");
      }
    }

    Inst->setMetadata(Kind, MD);
  }

  return Inst;
}
