//===- ArgumentPromotion.cpp - Promote by-reference arguments -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass promotes "by reference" arguments to be "by value" arguments.  In
// practice, this means looking for internal functions that have pointer
// arguments.  If it can prove, through the use of alias analysis, that an
// argument is *only* loaded, then it can pass the value into the function
// instead of the address of the value.  This can cause recursive simplification
// of code and lead to the elimination of allocas (especially in C++ template
// code like the STL).
//
// This pass also handles aggregate arguments that are passed into a function,
// scalarizing them if the elements of the aggregate are only loaded.  Note that
// by default it refuses to scalarize aggregates which would require passing in
// more than three operands to the function, because passing thousands of
// operands for a large array or structure is unprofitable! This limit can be
// configured or disabled, however.
//
// Note that this transformation could also be done for arguments that are only
// stored to (returning the value instead), but does not currently.  This case
// would be best handled when and if LLVM begins supporting multiple return
// values from functions.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/ArgumentPromotion.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/None.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/ScopeExit.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/CallGraphSCCPass.h"
#include "llvm/Analysis/Intel_Andersens.h"      // INTEL
#include "llvm/Analysis/Intel_WP.h"             // INTEL
#include "llvm/Analysis/LazyCallGraph.h"
#include "llvm/Analysis/Loads.h"
#include "llvm/Analysis/MemoryLocation.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/AbstractCallSite.h" // INTEL
#include "llvm/IR/Argument.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/NoFolder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Use.h"
#include "llvm/IR/User.h"
#include "llvm/IR/Value.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/Intel_InlineReport.h"      // INTEL
#include "llvm/Transforms/IPO/Intel_MDInlineReport.h"    // INTEL
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <functional>
#include <iterator>
#include <map>
#include <set>
#include <utility>
#include <vector>

using namespace llvm;

#define DEBUG_TYPE "argpromotion"

STATISTIC(NumArgumentsPromoted, "Number of pointer arguments promoted");
STATISTIC(NumAggregatesPromoted, "Number of aggregate arguments promoted");
STATISTIC(NumByValArgsPromoted, "Number of byval arguments promoted");
STATISTIC(NumArgumentsDead, "Number of dead pointer args eliminated");
#if INTEL_CUSTOMIZATION
// Force removal of homed arguments. Primarily intended for LIT tests.
// Would not normally be enabled.
static cl::opt<bool>
  ForceRemoveHomedArguments("argpro-force-remove-homed-arguments",
    cl::init(false), cl::ReallyHidden);
#endif // INTEL_CUSTOMIZATION
/// A vector used to hold the indices of a single GEP instruction
using IndicesVector = std::vector<uint64_t>;

/// DoPromotion - This method actually performs the promotion of the specified
/// arguments, and returns the new function.  At this point, we know that it's
/// safe to do so.
static Function *
doPromotion(Function *F, SmallPtrSetImpl<Argument *> &ArgsToPromote,
            SmallPtrSetImpl<Argument *> &ByValArgsToTransform,
            bool isCallback, // INTEL
            Optional<function_ref<void(CallBase &OldCS, CallBase &NewCS)>>
                ReplaceCallSite) {
  getInlineReport()->initFunctionClosure(F); // INTEL
  // Start by computing a new prototype for the function, which is the same as
  // the old function, but has modified arguments.
  FunctionType *FTy = F->getFunctionType();
  std::vector<Type *> Params;

  using ScalarizeTable = std::set<std::pair<Type *, IndicesVector>>;

  // ScalarizedElements - If we are promoting a pointer that has elements
  // accessed out of it, keep track of which elements are accessed so that we
  // can add one argument for each.
  //
  // Arguments that are directly loaded will have a zero element value here, to
  // handle cases where there are both a direct load and GEP accesses.
  std::map<Argument *, ScalarizeTable> ScalarizedElements;

  // OriginalLoads - Keep track of a representative load instruction from the
  // original function so that we can tell the alias analysis implementation
  // what the new GEP/Load instructions we are inserting look like.
  // We need to keep the original loads for each argument and the elements
  // of the argument that are accessed.
  std::map<std::pair<Argument *, IndicesVector>, LoadInst *> OriginalLoads;

  // Attribute - Keep track of the parameter attributes for the arguments
  // that we are *not* promoting. For the ones that we do promote, the parameter
  // attributes are lost
  SmallVector<AttributeSet, 8> ArgAttrVec;
  AttributeList PAL = F->getAttributes();
  const DataLayout &DL = F->getParent()->getDataLayout(); // INTEL

  // First, determine the new argument list
  unsigned ArgNo = 0;
  for (Function::arg_iterator I = F->arg_begin(), E = F->arg_end(); I != E;
       ++I, ++ArgNo) {
    if (ByValArgsToTransform.count(&*I)) {
      // Simple byval argument? Just add all the struct element types.
      Type *AgTy = I->getParamByValType();
      StructType *STy = cast<StructType>(AgTy);
      llvm::append_range(Params, STy->elements());
      ArgAttrVec.insert(ArgAttrVec.end(), STy->getNumElements(),
                        AttributeSet());
      ++NumByValArgsPromoted;
    } else if (!ArgsToPromote.count(&*I)) {
      // Unchanged argument
      Params.push_back(I->getType());
      ArgAttrVec.push_back(PAL.getParamAttrs(ArgNo));
    } else if (I->use_empty()) {
      // Dead argument (which are always marked as promotable)
      ++NumArgumentsDead;
    } else {
      // Okay, this is being promoted. This means that the only uses are loads
      // or GEPs which are only used by loads

      // In this table, we will track which indices are loaded from the argument
      // (where direct loads are tracked as no indices).
      ScalarizeTable &ArgIndices = ScalarizedElements[&*I];
      for (User *U : make_early_inc_range(I->users())) {
        Instruction *UI = cast<Instruction>(U);
        Type *SrcTy;
        if (LoadInst *L = dyn_cast<LoadInst>(UI))
          SrcTy = L->getType();
        else
          SrcTy = cast<GetElementPtrInst>(UI)->getSourceElementType();
        // Skip dead GEPs and remove them.
        if (isa<GetElementPtrInst>(UI) && UI->use_empty()) {
          UI->eraseFromParent();
          continue;
        }

        IndicesVector Indices;
        Indices.reserve(UI->getNumOperands() - 1);
        // Since loads will only have a single operand, and GEPs only a single
        // non-index operand, this will record direct loads without any indices,
        // and gep+loads with the GEP indices.
        for (const Use &I : llvm::drop_begin(UI->operands()))
          Indices.push_back(cast<ConstantInt>(I)->getSExtValue());
        // GEPs with a single 0 index can be merged with direct loads
        if (Indices.size() == 1 && Indices.front() == 0)
          Indices.clear();
        ArgIndices.insert(std::make_pair(SrcTy, Indices));
        LoadInst *OrigLoad;
        if (LoadInst *L = dyn_cast<LoadInst>(UI))
          OrigLoad = L;
        else
          // Take any load, we will use it only to update Alias Analysis
          OrigLoad = cast<LoadInst>(UI->user_back());
        OriginalLoads[std::make_pair(&*I, Indices)] = OrigLoad;
      }

      // Add a parameter to the function for each element passed in.
      for (const auto &ArgIndex : ArgIndices) {
        // not allowed to dereference ->begin() if size() is 0
#if INTEL_CUSTOMIZATION
        Type *ParamTy = GetElementPtrInst::getIndexedType(
            cast<PointerType>(I->getType())->getElementType(),
            ArgIndex.second);
        if (isCallback && !isa<PointerType>(ParamTy))
          ParamTy = DL.getIntPtrType(I->getType());
        Params.push_back(ParamTy);
#endif // INTEL_CUSTOMIZATION
        ArgAttrVec.push_back(AttributeSet());
        assert(Params.back());
      }

      if (ArgIndices.size() == 1 && ArgIndices.begin()->second.empty())
        ++NumArgumentsPromoted;
      else
        ++NumAggregatesPromoted;
    }
  }

  Type *RetTy = FTy->getReturnType();

  // Construct the new function type using the new arguments.
  FunctionType *NFTy = FunctionType::get(RetTy, Params, FTy->isVarArg());

  // Create the new function body and insert it into the module.
  Function *NF = Function::Create(NFTy, F->getLinkage(), F->getAddressSpace(),
                                  F->getName());
  NF->copyAttributesFrom(F);
  NF->copyMetadata(F, 0);

  // The new function will have the !dbg metadata copied from the original
  // function. The original function may not be deleted, and dbg metadata need
  // to be unique so we need to drop it.
  F->setSubprogram(nullptr);

  LLVM_DEBUG(dbgs() << "ARG PROMOTION:  Promoting to:" << *NF << "\n"
                    << "From: " << *F);

  // Recompute the parameter attributes list based on the new arguments for
  // the function.
  NF->setAttributes(AttributeList::get(F->getContext(), PAL.getFnAttrs(),
                                       PAL.getRetAttrs(), ArgAttrVec));
  ArgAttrVec.clear();

  F->getParent()->getFunctionList().insert(F->getIterator(), NF);
  NF->takeName(F);

#if INTEL_CUSTOMIZATION
  if (F->hasComdat())
    NF->setComdat(F->getComdat());
#endif // INTEL_CUSTOMIZATION

  // Loop over all of the callers of the function, transforming the call sites
  // to pass in the loaded pointers.
  //
  SmallVector<Value *, 16> Args;
  while (!F->use_empty()) {
#if INTEL_CUSTOMIZATION
    AbstractCallSite ACS(&*F->use_begin());
    assert(ACS.getCalledFunction() == F);
    CallBase &CB = *(cast<CallBase>(ACS.getInstruction()));
#endif // INTEL_CUSTOMIZATION
    const AttributeList &CallPAL = CB.getAttributes();
    IRBuilder<NoFolder> IRB(&CB);

#if INTEL_CUSTOMIZATION
    // Build mapping between actual and formal arguments for the call site.
    DenseMap<int, int> Actual2Formal;
    for (unsigned ArgNo = 0; ArgNo < F->arg_size(); ++ArgNo) {
      int OpndNo = ACS.getCallArgOperandNo(ArgNo);
      if (OpndNo >= 0)
        Actual2Formal[OpndNo] = ArgNo;
    }

    // Cast given value to a pointer-sized integer type for callback functions.
    auto MaybeCastTo = [&](Value *Val, Argument &Arg) {
      if (isCallback && !isa<PointerType>(Val->getType())) {
        // Bitcast to integer.
        Value *IntVal = IRB.CreateBitOrPointerCast(
            Val, IRB.getIntNTy(DL.getTypeStoreSizeInBits(Val->getType())));

        // Zero-extend to pointer-sized integer.
        Val = IRB.CreateZExt(IntVal, DL.getIntPtrType(Arg.getType()));
      }
      return Val;
    };
#endif // INTEL_CUSTOMIZATION

    // Loop over the operands, inserting GEP and loads in the caller as
    // appropriate.
    ArgNo = 0;
#if INTEL_CUSTOMIZATION
    for (auto AI = CB.arg_begin(), E = CB.arg_end(); AI != E; ++AI, ++ArgNo) {
      if (ACS.isCallbackCall() &&
          static_cast<unsigned>(ACS.getCallArgOperandNoForCallee()) == ArgNo) {
        // Use new function for the the callback call's callee operand.
        const Use &CBA = ACS.getCalleeUseForCallback();
        Args.push_back(NF->getType() != CBA->getType()
                           ? ConstantExpr::getPointerCast(NF, CBA->getType())
                           : NF);
        ArgAttrVec.push_back(CallPAL.getParamAttrs(ArgNo));
        continue;
      }

      if (!Actual2Formal.count(ArgNo)) {
        Args.push_back(*AI);
        ArgAttrVec.push_back(CallPAL.getParamAttrs(ArgNo));
        continue;
      }

      Function::arg_iterator I =
          std::next(F->arg_begin(), Actual2Formal[ArgNo]);
#endif // INTEL_CUSTOMIZATION
      if (!ArgsToPromote.count(&*I) && !ByValArgsToTransform.count(&*I)) {
#if INTEL_CUSTOMIZATION
        if (isCallback && I->use_empty()) {
          // Argument with no uses. Pass undef value for callbacks.
          Args.push_back(UndefValue::get(AI->get()->getType()));
          ArgAttrVec.push_back(AttributeSet());
          continue;
        }
#endif // INTEL_CUSTOMIZATION
        Args.push_back(*AI); // Unmodified argument
        ArgAttrVec.push_back(CallPAL.getParamAttrs(ArgNo));
      } else if (ByValArgsToTransform.count(&*I)) {
        // Emit a GEP and load for each element of the struct.
        Type *AgTy = I->getParamByValType();
        StructType *STy = cast<StructType>(AgTy);
        Value *Idxs[2] = {
            ConstantInt::get(Type::getInt32Ty(F->getContext()), 0), nullptr};
        const StructLayout *SL = DL.getStructLayout(STy);
        Align StructAlign = *I->getParamAlign();
        for (unsigned i = 0, e = STy->getNumElements(); i != e; ++i) {
          Idxs[1] = ConstantInt::get(Type::getInt32Ty(F->getContext()), i);
          auto *Idx =
              IRB.CreateGEP(STy, *AI, Idxs, (*AI)->getName() + "." + Twine(i));
          // TODO: Tell AA about the new values?
          Align Alignment =
              commonAlignment(StructAlign, SL->getElementOffset(i));
          Args.push_back(IRB.CreateAlignedLoad(
              STy->getElementType(i), Idx, Alignment, Idx->getName() + ".val"));
          ArgAttrVec.push_back(AttributeSet());
        }
      } else if (!I->use_empty()) {
        // Non-dead argument: insert GEPs and loads as appropriate.
        ScalarizeTable &ArgIndices = ScalarizedElements[&*I];
        // Store the Value* version of the indices in here, but declare it now
        // for reuse.
        std::vector<Value *> Ops;
        for (const auto &ArgIndex : ArgIndices) {
          Value *V = *AI;
          LoadInst *OrigLoad =
              OriginalLoads[std::make_pair(&*I, ArgIndex.second)];
          if (!ArgIndex.second.empty()) {
            Ops.reserve(ArgIndex.second.size());
            Type *ElTy = V->getType();
            for (auto II : ArgIndex.second) {
              // Use i32 to index structs, and i64 for others (pointers/arrays).
              // This satisfies GEP constraints.
              Type *IdxTy =
                  (ElTy->isStructTy() ? Type::getInt32Ty(F->getContext())
                                      : Type::getInt64Ty(F->getContext()));
              Ops.push_back(ConstantInt::get(IdxTy, II));
              // Keep track of the type we're currently indexing.
              if (auto *ElPTy = dyn_cast<PointerType>(ElTy))
                ElTy = ElPTy->getElementType();
              else
                ElTy = GetElementPtrInst::getTypeAtIndex(ElTy, II);
            }
            // And create a GEP to extract those indices.
            V = IRB.CreateGEP(ArgIndex.first, V, Ops, V->getName() + ".idx");
            Ops.clear();
          }
#if INTEL_CUSTOMIZATION
          // For callback call sites type of the actual argument may differ
          // from the formal, so add a type cast if necessary.
          if (ACS.isCallbackCall() &&
              OrigLoad->getPointerOperandType() != V->getType())
            V = IRB.CreatePointerCast(V, OrigLoad->getPointerOperandType(),
                                      V->getName() + ".cst");
#endif // INTEL_CUSTOMIZATION
          // Since we're replacing a load make sure we take the alignment
          // of the previous load.
          LoadInst *newLoad =
              IRB.CreateLoad(OrigLoad->getType(), V, V->getName() + ".val");
          newLoad->setAlignment(OrigLoad->getAlign());
          // Transfer the AA info too.
          newLoad->setAAMetadata(OrigLoad->getAAMetadata());

          Args.push_back(MaybeCastTo(newLoad, *I)); // INTEL
          ArgAttrVec.push_back(AttributeSet());
        }
      }
    }

    SmallVector<OperandBundleDef, 1> OpBundles;
    CB.getOperandBundlesAsDefs(OpBundles);

    CallBase *NewCS = nullptr;
#if INTEL_CUSTOMIZATION
    Function *NewF = ACS.isCallbackCall() ? CB.getCalledFunction() : NF;
    FunctionType *FType = NewF->getFunctionType();
#endif // INTEL_CUSTOMIZATION
    if (InvokeInst *II = dyn_cast<InvokeInst>(&CB)) {
#if INTEL_CUSTOMIZATION
      NewCS = InvokeInst::Create(FType, NewF, II->getNormalDest(),
                                 II->getUnwindDest(), Args, OpBundles, "", &CB);
#endif // INTEL_CUSTOMIZATION
    } else {
      auto *NewCall =
          CallInst::Create(FType, NewF, Args, OpBundles, "", &CB); // INTEL
      NewCall->setTailCallKind(cast<CallInst>(&CB)->getTailCallKind());
      NewCS = NewCall;
    }
    NewCS->setCallingConv(CB.getCallingConv());
    NewCS->setAttributes(AttributeList::get(F->getContext(),
                                            CallPAL.getFnAttrs(),
                                            CallPAL.getRetAttrs(), ArgAttrVec));
    NewCS->copyMetadata(CB, {LLVMContext::MD_prof, LLVMContext::MD_dbg});
#if INTEL_CUSTOMIZATION
    MDNode *MD = CB.getMetadata(LLVMContext::MD_intel_profx);
    if (MD)
      NewCS->setMetadata(LLVMContext::MD_intel_profx, MD);

    getInlineReport()->replaceCallBaseWithCallBase(&CB, NewCS);
    getMDInlineReport()->replaceCallBaseWithCallBase(&CB, NewCS);
#endif // INTEL_CUSTOMIZATION
    Args.clear();
    ArgAttrVec.clear();

    // Update the callgraph to know that the callsite has been transformed.
    if (ReplaceCallSite)
      (*ReplaceCallSite)(CB, *NewCS);

    if (!CB.use_empty()) {
      CB.replaceAllUsesWith(NewCS);
      NewCS->takeName(&CB);
    }

    // Finally, remove the old call from the program, reducing the use-count of
    // F.
    CB.eraseFromParent();

#if INTEL_CUSTOMIZATION
    // Remove dead constants referencing this function that may remain after
    // removing the call.
    F->removeDeadConstantUsers();
#endif // INTEL_CUSTOMIZATION
  }

#if INTEL_CUSTOMIZATION
  // All uses of the old function have been replaced with the new function,
  // transfer the function entry count to the replacement function to maintain
  // the ability to place the functions into hot/cold sections.
  Optional<Function::ProfileCount> OldCount = F->getEntryCount();
  if (OldCount.hasValue())
    NF->setEntryCount(OldCount->getCount());
#endif // INTEL_CUSTOMIZATION

  // Since we have now created the new function, splice the body of the old
  // function right into the new function, leaving the old rotting hulk of the
  // function empty.
  NF->getBasicBlockList().splice(NF->begin(), F->getBasicBlockList());

#if INTEL_CUSTOMIZATION
  auto MaybeCastFrom = [&](Value *Val, Instruction *Inst) {
    if (isCallback && !isa<PointerType>(Val->getType())) {
      IRBuilder<> IRB(Inst);
      Type *Ty = Inst->getType();

      // Truncate to type-sized integer.
      Value *IntVal =
          IRB.CreateTrunc(Val, IRB.getIntNTy(DL.getTypeStoreSizeInBits(Ty)));

      // Bitcast to destination type.
      Val = IRB.CreateBitOrPointerCast(IntVal, Ty);
    }
    return Val;
  };
#endif // INTEL_CUSTOMIZATION

  // Loop over the argument list, transferring uses of the old arguments over to
  // the new arguments, also transferring over the names as well.
  for (Function::arg_iterator I = F->arg_begin(), E = F->arg_end(),
                              I2 = NF->arg_begin();
       I != E; ++I) {
    if (!ArgsToPromote.count(&*I) && !ByValArgsToTransform.count(&*I)) {
      // If this is an unmodified argument, move the name and users over to the
      // new version.
      I->replaceAllUsesWith(&*I2);
      I2->takeName(&*I);
      ++I2;
      continue;
    }

    if (ByValArgsToTransform.count(&*I)) {
      // In the callee, we create an alloca, and store each of the new incoming
      // arguments into the alloca.
      Instruction *InsertPt = &NF->begin()->front();

      // Just add all the struct element types.
      Type *AgTy = I->getParamByValType();
      Align StructAlign = *I->getParamAlign();
      Value *TheAlloca = new AllocaInst(AgTy, DL.getAllocaAddrSpace(), nullptr,
                                        StructAlign, "", InsertPt);
      StructType *STy = cast<StructType>(AgTy);
      Value *Idxs[2] = {ConstantInt::get(Type::getInt32Ty(F->getContext()), 0),
                        nullptr};
      const StructLayout *SL = DL.getStructLayout(STy);

      for (unsigned i = 0, e = STy->getNumElements(); i != e; ++i) {
        Idxs[1] = ConstantInt::get(Type::getInt32Ty(F->getContext()), i);
        Value *Idx = GetElementPtrInst::Create(
            AgTy, TheAlloca, Idxs, TheAlloca->getName() + "." + Twine(i),
            InsertPt);
        I2->setName(I->getName() + "." + Twine(i));
        Align Alignment = commonAlignment(StructAlign, SL->getElementOffset(i));
        new StoreInst(&*I2++, Idx, false, Alignment, InsertPt);
      }

      // Anything that used the arg should now use the alloca.
      I->replaceAllUsesWith(TheAlloca);
      TheAlloca->takeName(&*I);
      continue;
    }

    // There potentially are metadata uses for things like llvm.dbg.value.
    // Replace them with undef, after handling the other regular uses.
    auto RauwUndefMetadata = make_scope_exit(
        [&]() { I->replaceAllUsesWith(UndefValue::get(I->getType())); });

    if (I->use_empty())
      continue;

    // Otherwise, if we promoted this argument, then all users are load
    // instructions (or GEPs with only load users), and all loads should be
    // using the new argument that we added.
    ScalarizeTable &ArgIndices = ScalarizedElements[&*I];

    while (!I->use_empty()) {
      if (LoadInst *LI = dyn_cast<LoadInst>(I->user_back())) {
        assert(ArgIndices.begin()->second.empty() &&
               "Load element should sort to front!");
        I2->setName(I->getName() + ".val");
        LI->replaceAllUsesWith(MaybeCastFrom(&*I2, LI)); // INTEL
        LI->eraseFromParent();
        LLVM_DEBUG(dbgs() << "*** Promoted load of argument '" << I->getName()
                          << "' in function '" << F->getName() << "'\n");
      } else {
        GetElementPtrInst *GEP = cast<GetElementPtrInst>(I->user_back());
        assert(!GEP->use_empty() &&
               "GEPs without uses should be cleaned up already");
        IndicesVector Operands;
        Operands.reserve(GEP->getNumIndices());
        for (const Use &Idx : GEP->indices())
          Operands.push_back(cast<ConstantInt>(Idx)->getSExtValue());

        // GEPs with a single 0 index can be merged with direct loads
        if (Operands.size() == 1 && Operands.front() == 0)
          Operands.clear();

        Function::arg_iterator TheArg = I2;
        for (ScalarizeTable::iterator It = ArgIndices.begin();
             It->second != Operands; ++It, ++TheArg) {
          assert(It != ArgIndices.end() && "GEP not handled??");
        }

        TheArg->setName(formatv("{0}.{1:$[.]}.val", I->getName(),
                                make_range(Operands.begin(), Operands.end())));

        LLVM_DEBUG(dbgs() << "*** Promoted agg argument '" << TheArg->getName()
                          << "' of function '" << NF->getName() << "'\n");

        // All of the uses must be load instructions.  Replace them all with
        // the argument specified by ArgNo.
        while (!GEP->use_empty()) {
          LoadInst *L = cast<LoadInst>(GEP->user_back());
          L->replaceAllUsesWith(MaybeCastFrom(&*TheArg, L)); // INTEL
          L->eraseFromParent();
        }
        GEP->eraseFromParent();
      }
    }

    // Increment I2 past all of the arguments added for this promoted pointer.
    std::advance(I2, ArgIndices.size());
  }

  return NF;
}

/// Return true if we can prove that all callees pass in a valid pointer for the
/// specified function argument.
static bool allCallersPassValidPointerForArgument(Argument *Arg, Type *Ty) {
  Function *Callee = Arg->getParent();
  const DataLayout &DL = Callee->getParent()->getDataLayout();

  unsigned ArgNo = Arg->getArgNo();

#if INTEL_CUSTOMIZATION
  // Check if Arg is marked with dereferenceable attribute.
  if (isDereferenceablePointer(Arg, Ty, DL))
    return true;
  // Look at all call sites of the function.  At this point we know we only have
  // direct or callback callees.
  for (const Use &U : Callee->uses()) {
    AbstractCallSite CS(&U);
    assert((CS.isDirectCall() || CS.isCallbackCall()) &&
           "Should only have direct or callback calls!");

    if (!isDereferenceablePointer(CS.getCallArgOperand(ArgNo), Ty, DL))
      return false;
  }
#endif // INTEL_CUSTOMIZATION
  return true;
}

/// Returns true if Prefix is a prefix of longer. That means, Longer has a size
/// that is greater than or equal to the size of prefix, and each of the
/// elements in Prefix is the same as the corresponding elements in Longer.
///
/// This means it also returns true when Prefix and Longer are equal!
static bool isPrefix(const IndicesVector &Prefix, const IndicesVector &Longer) {
  if (Prefix.size() > Longer.size())
    return false;
  return std::equal(Prefix.begin(), Prefix.end(), Longer.begin());
}

/// Checks if Indices, or a prefix of Indices, is in Set.
static bool prefixIn(const IndicesVector &Indices,
                     std::set<IndicesVector> &Set) {
  std::set<IndicesVector>::iterator Low;
  Low = Set.upper_bound(Indices);
  if (Low != Set.begin())
    Low--;
  // Low is now the last element smaller than or equal to Indices. This means
  // it points to a prefix of Indices (possibly Indices itself), if such
  // prefix exists.
  //
  // This load is safe if any prefix of its operands is safe to load.
  return Low != Set.end() && isPrefix(*Low, Indices);
}

/// Mark the given indices (ToMark) as safe in the given set of indices
/// (Safe). Marking safe usually means adding ToMark to Safe. However, if there
/// is already a prefix of Indices in Safe, Indices are implicitely marked safe
/// already. Furthermore, any indices that Indices is itself a prefix of, are
/// removed from Safe (since they are implicitely safe because of Indices now).
static void markIndicesSafe(const IndicesVector &ToMark,
                            std::set<IndicesVector> &Safe) {
  std::set<IndicesVector>::iterator Low;
  Low = Safe.upper_bound(ToMark);
  // Guard against the case where Safe is empty
  if (Low != Safe.begin())
    Low--;
  // Low is now the last element smaller than or equal to Indices. This
  // means it points to a prefix of Indices (possibly Indices itself), if
  // such prefix exists.
  if (Low != Safe.end()) {
    if (isPrefix(*Low, ToMark))
      // If there is already a prefix of these indices (or exactly these
      // indices) marked a safe, don't bother adding these indices
      return;

    // Increment Low, so we can use it as a "insert before" hint
    ++Low;
  }
  // Insert
  Low = Safe.insert(Low, ToMark);
  ++Low;
  // If there we're a prefix of longer index list(s), remove those
  std::set<IndicesVector>::iterator End = Safe.end();
  while (Low != End && isPrefix(ToMark, *Low)) {
    std::set<IndicesVector>::iterator Remove = Low;
    ++Low;
    Safe.erase(Remove);
  }
}

/// isSafeToPromoteArgument - As you might guess from the name of this method,
/// it checks to see if it is both safe and useful to promote the argument.
/// This method limits promotion of aggregates to only promote up to three
/// elements of the aggregate in order to avoid exploding the number of
/// arguments passed in.
static bool isSafeToPromoteArgument(Argument *Arg, Type *ByValTy, AAResults &AAR,
                                    bool isCallback,             // INTEL
                                    bool RemoveHomedArguments,   // INTEL
                                    unsigned MaxElements) {
  using GEPIndicesSet = std::set<IndicesVector>;
#if INTEL_CUSTOMIZATION
  //
  // Return the unique LoadInst, if it exists, which retrieves the
  // Argument 'A' that is homed by the StoreInst 'U'.
  //
  auto UniqueLoadInst = [](Argument *A, User *U) -> LoadInst * {
    StoreInst *SI = dyn_cast<StoreInst>(U);
    if (!SI || SI->getValueOperand() != A)
      return nullptr;
    auto AI = dyn_cast<AllocaInst>(SI->getPointerOperand());
    if (!AI)
      return nullptr;
    LoadInst *ULI = nullptr;
    for (User *UU : AI->users()) {
      if (UU == SI)
        continue;
      auto LI = dyn_cast<LoadInst>(UU);
      if (!LI || ULI)
        return nullptr;
      ULI = LI;
    }
    return ULI;
  };

  //
  // Remove the homed store 'SI' and its unique load 'LI' and corresponding
  // AllocaInst to enable argument promotion and update 'TestUsers' with new
  // Users that now have 'Arg' as their pointer operand.
  //
  auto RemoveHomedStore = [](SmallVectorImpl<User *> &TestUsers,
                             LoadInst *LI, StoreInst *SI) {
      auto Arg = cast<Argument>(SI->getValueOperand());
      auto AI = cast<AllocaInst>(SI->getPointerOperand());
      for (User *U : LI->users())
        TestUsers.push_back(U);
      LI->replaceAllUsesWith(Arg);
      LI->eraseFromParent();
      SI->eraseFromParent();
      AI->eraseFromParent();
  };
#endif // INTEL_CUSTOMIZATION

  // Quick exit for unused arguments
  if (Arg->use_empty())
    return !isCallback; // INTEL

  // We can only promote this argument if all of the uses are loads, or are GEP
  // instructions (with constant indices) that are subsequently loaded.
  //
  // Promoting the argument causes it to be loaded in the caller
  // unconditionally. This is only safe if we can prove that either the load
  // would have happened in the callee anyway (ie, there is a load in the entry
  // block) or the pointer passed in at every call site is guaranteed to be
  // valid.
  // In the former case, invalid loads can happen, but would have happened
  // anyway, in the latter case, invalid loads won't happen. This prevents us
  // from introducing an invalid load that wouldn't have happened in the
  // original code.
  //
  // This set will contain all sets of indices that are loaded in the entry
  // block, and thus are safe to unconditionally load in the caller.
  GEPIndicesSet SafeToUnconditionallyLoad;

  // This set contains all the sets of indices that we are planning to promote.
  // This makes it possible to limit the number of arguments added.
  GEPIndicesSet ToPromote;

  // If the pointer is always valid, any load with first index 0 is valid.

  if (ByValTy)
    SafeToUnconditionallyLoad.insert(IndicesVector(1, 0));

  // Whenever a new underlying type for the operand is found, make sure it's
  // consistent with the GEPs and loads we've already seen and, if necessary,
  // use it to see if all incoming pointers are valid (which implies the 0-index
  // is safe).
  Type *BaseTy = ByValTy;
  auto UpdateBaseTy = [&](Type *NewBaseTy) {
    if (BaseTy)
      return BaseTy == NewBaseTy;

    BaseTy = NewBaseTy;
    if (allCallersPassValidPointerForArgument(Arg, BaseTy)) {
      assert(SafeToUnconditionallyLoad.empty());
      SafeToUnconditionallyLoad.insert(IndicesVector(1, 0));
    }

    return true;
  };

  // First, iterate the entry block and mark loads of (geps of) arguments as
  // safe.
  BasicBlock &EntryBlock = Arg->getParent()->front();
  // Declare this here so we can reuse it
  IndicesVector Indices;
  for (Instruction &I : EntryBlock)
    if (LoadInst *LI = dyn_cast<LoadInst>(&I)) {
      Value *V = LI->getPointerOperand();
      if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(V)) {
        V = GEP->getPointerOperand();
        if (V == Arg) {
          // This load actually loads (part of) Arg? Check the indices then.
          Indices.reserve(GEP->getNumIndices());
          for (Use &Idx : GEP->indices())
            if (ConstantInt *CI = dyn_cast<ConstantInt>(Idx))
              Indices.push_back(CI->getSExtValue());
            else
              // We found a non-constant GEP index for this argument? Bail out
              // right away, can't promote this argument at all.
              return false;

          if (!UpdateBaseTy(GEP->getSourceElementType()))
            return false;

          // Indices checked out, mark them as safe
          markIndicesSafe(Indices, SafeToUnconditionallyLoad);
          Indices.clear();
        }
      } else if (V == Arg) {
        // Direct loads are equivalent to a GEP with a single 0 index.
        markIndicesSafe(IndicesVector(1, 0), SafeToUnconditionallyLoad);

        if (BaseTy && LI->getType() != BaseTy)
          return false;

        BaseTy = LI->getType();
      }
    }

  // Now, iterate all uses of the argument to see if there are any uses that are
  // not (GEP+)loads, or any (GEP+)loads that are not safe to promote.
  SmallVector<LoadInst *, 16> Loads;
  IndicesVector Operands;
#if INTEL_CUSTOMIZATION
  SmallVector<User *, 16> TestUsers;
  TestUsers.append(Arg->users().begin(), Arg->users().end());
  while (!TestUsers.empty()) {
    User *UR = TestUsers.back();
    TestUsers.pop_back();
#endif // INTEL_CUSTOMIZATION
    Operands.clear();
    if (LoadInst *LI = dyn_cast<LoadInst>(UR)) {
      // Don't hack volatile/atomic loads
      if (!LI->isSimple())
        return false;
      Loads.push_back(LI);
      // Direct loads are equivalent to a GEP with a zero index and then a load.
      Operands.push_back(0);

      if (!UpdateBaseTy(LI->getType()))
        return false;
    } else if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(UR)) {
      if (GEP->use_empty()) {
        // Dead GEP's cause trouble later.  Just remove them if we run into
        // them.
        continue;
      }

      if (!UpdateBaseTy(GEP->getSourceElementType()))
        return false;

      // Ensure that all of the indices are constants.
      for (Use &Idx : GEP->indices())
        if (ConstantInt *C = dyn_cast<ConstantInt>(Idx))
          Operands.push_back(C->getSExtValue());
        else
          return false; // Not a constant operand GEP!

      // Ensure that the only users of the GEP are load instructions.
      for (User *GEPU : GEP->users())
        if (LoadInst *LI = dyn_cast<LoadInst>(GEPU)) {
          // Don't hack volatile/atomic loads
          if (!LI->isSimple())
            return false;
          Loads.push_back(LI);
        } else {
          // Other uses than load?
          return false;
        }
#if INTEL_CUSTOMIZATION
    } else if (RemoveHomedArguments) {
      if (auto LI = UniqueLoadInst(Arg, UR)) {
        RemoveHomedStore(TestUsers, LI, cast<StoreInst>(UR));
        continue;
      }
      return false;
#endif // INTEL_CUSTOMIZATION
    } else {
      return false; // Not a load or a GEP.
    }

    // Now, see if it is safe to promote this load / loads of this GEP. Loading
    // is safe if Operands, or a prefix of Operands, is marked as safe.
    if (!prefixIn(Operands, SafeToUnconditionallyLoad))
      return false;

    // See if we are already promoting a load with these indices. If not, check
    // to make sure that we aren't promoting too many elements.  If so, nothing
    // to do.
    if (ToPromote.find(Operands) == ToPromote.end()) {
      if (MaxElements > 0 && ToPromote.size() == MaxElements) {
        LLVM_DEBUG(dbgs() << "argpromotion not promoting argument '"
                          << Arg->getName()
                          << "' because it would require adding more "
                          << "than " << MaxElements
                          << " arguments to the function.\n");
        // We limit aggregate promotion to only promoting up to a fixed number
        // of elements of the aggregate.
        return false;
      }
      ToPromote.insert(std::move(Operands));
    }
  }

  if (Loads.empty())
    return true; // No users, this is a dead argument.

#if INTEL_CUSTOMIZATION
  if (isCallback) {
    // We cannot change the number of arguments for callbacks.
    if (ToPromote.size() > 1)
      return false;

    // Promoted argument should fit into pointer size.
    const DataLayout &DL = Arg->getParent()->getParent()->getDataLayout();
    if (DL.getTypeStoreSize(BaseTy) > DL.getTypeStoreSize(Arg->getType()))
      return false;
  }

  // Since the argument is only used by load instructions (i.e not escaped)
  // and the argument is marked with NoAlias, we don't need to prove that
  // the argument pointer is not modified before its uses. It is safe to
  // assume that the argument pointer is not modified in the current routine.
  if (isNoAliasOrByValArgument(Arg))
    return true;
#endif // INTEL_CUSTOMIZATION

  // Okay, now we know that the argument is only used by load instructions and
  // it is safe to unconditionally perform all of them. Use alias analysis to
  // check to see if the pointer is guaranteed to not be modified from entry of
  // the function to each of the load instructions.

  // Because there could be several/many load instructions, remember which
  // blocks we know to be transparent to the load.
  df_iterator_default_set<BasicBlock *, 16> TranspBlocks;

  for (LoadInst *Load : Loads) {
    // Check to see if the load is invalidated from the start of the block to
    // the load itself.
    BasicBlock *BB = Load->getParent();

    MemoryLocation Loc = MemoryLocation::get(Load);
    if (AAR.canInstructionRangeModRef(BB->front(), *Load, Loc, ModRefInfo::Mod))
      return false; // Pointer is invalidated!

    // Now check every path from the entry block to the load for transparency.
    // To do this, we perform a depth first search on the inverse CFG from the
    // loading block.
    for (BasicBlock *P : predecessors(BB)) {
      for (BasicBlock *TranspBB : inverse_depth_first_ext(P, TranspBlocks))
        if (AAR.canBasicBlockModify(*TranspBB, Loc))
          return false;
    }
  }

  // If the path from the entry of the function to each load is free of
  // instructions that potentially invalidate the load, we can make the
  // transformation!
  return true;
}

bool ArgumentPromotionPass::isDenselyPacked(Type *type, const DataLayout &DL) {
  // There is no size information, so be conservative.
  if (!type->isSized())
    return false;

  // If the alloc size is not equal to the storage size, then there are padding
  // bytes. For x86_fp80 on x86-64, size: 80 alloc size: 128.
  if (DL.getTypeSizeInBits(type) != DL.getTypeAllocSizeInBits(type))
    return false;

  // FIXME: This isn't the right way to check for padding in vectors with
  // non-byte-size elements.
  if (VectorType *seqTy = dyn_cast<VectorType>(type))
    return isDenselyPacked(seqTy->getElementType(), DL);

  // For array types, check for padding within members.
  if (ArrayType *seqTy = dyn_cast<ArrayType>(type))
    return isDenselyPacked(seqTy->getElementType(), DL);

  if (!isa<StructType>(type))
    return true;

  // Check for padding within and between elements of a struct.
  StructType *StructTy = cast<StructType>(type);
  const StructLayout *Layout = DL.getStructLayout(StructTy);
  uint64_t StartPos = 0;
  for (unsigned i = 0, E = StructTy->getNumElements(); i < E; ++i) {
    Type *ElTy = StructTy->getElementType(i);
    if (!isDenselyPacked(ElTy, DL))
      return false;
    if (StartPos != Layout->getElementOffsetInBits(i))
      return false;
    StartPos += DL.getTypeAllocSizeInBits(ElTy);
  }

  return true;
}

/// Checks if the padding bytes of an argument could be accessed.
static bool canPaddingBeAccessed(Argument *arg) {
  assert(arg->hasByValAttr());

  // Track all the pointers to the argument to make sure they are not captured.
  SmallPtrSet<Value *, 16> PtrValues;
  PtrValues.insert(arg);

  // Track all of the stores.
  SmallVector<StoreInst *, 16> Stores;

  // Scan through the uses recursively to make sure the pointer is always used
  // sanely.
  SmallVector<Value *, 16> WorkList(arg->users());
  while (!WorkList.empty()) {
    Value *V = WorkList.pop_back_val();
    if (isa<GetElementPtrInst>(V) || isa<PHINode>(V)) {
      if (PtrValues.insert(V).second)
        llvm::append_range(WorkList, V->users());
    } else if (StoreInst *Store = dyn_cast<StoreInst>(V)) {
      Stores.push_back(Store);
    } else if (!isa<LoadInst>(V)) {
      return true;
    }
  }

  // Check to make sure the pointers aren't captured
  for (StoreInst *Store : Stores)
    if (PtrValues.count(Store->getValueOperand()))
      return true;

  return false;
}

bool ArgumentPromotionPass::areFunctionArgsABICompatible(
    const Function &F, const TargetTransformInfo &TTI,
    SmallPtrSetImpl<Argument *> &ArgsToPromote,
    SmallPtrSetImpl<Argument *> &ByValArgsToTransform) {
  // TODO: Check individual arguments so we can promote a subset?
  SmallVector<Type *, 32> Types;
  for (Argument *Arg : ArgsToPromote)
    Types.push_back(Arg->getType()->getPointerElementType());
  for (Argument *Arg : ByValArgsToTransform)
    Types.push_back(Arg->getParamByValType());

  for (const Use &U : F.uses()) {
#ifdef INTEL_CUSTOMIZATION
    AbstractCallSite CS(&U);
    if (!CS)
      return false;
    const Function *Caller = CS.getInstruction()->getCaller();
    const Function *Callee = CS.getCalledFunction();
#endif // INTEL_CUSTOMIZATION
    if (!TTI.areTypesABICompatible(Caller, Callee, Types))
      return false;
  }
  return true;
}

/// PromoteArguments - This method checks the specified function to see if there
/// are any promotable arguments and if it is safe to promote the function (for
/// example, all callers are direct).  If safe to promote some arguments, it
/// calls the DoPromotion method.
static Function *
promoteArguments(Function *F, function_ref<AAResults &(Function &F)> AARGetter,
                 bool RemoveHomedArguments, unsigned MaxElements, // INTEL
                 Optional<function_ref<void(CallBase &OldCS, CallBase &NewCS)>>
                     ReplaceCallSite,
                 const TargetTransformInfo &TTI) {
  RemoveHomedArguments |= ForceRemoveHomedArguments; // INTEL
  // Don't perform argument promotion for naked functions; otherwise we can end
  // up removing parameters that are seemingly 'not used' as they are referred
  // to in the assembly.
  if(F->hasFnAttribute(Attribute::Naked))
    return nullptr;

  // Make sure that it is local to this module.
  if (!F->hasLocalLinkage())
    return nullptr;

  // Don't promote arguments for variadic functions. Adding, removing, or
  // changing non-pack parameters can change the classification of pack
  // parameters. Frontends encode that classification at the call site in the
  // IR, while in the callee the classification is determined dynamically based
  // on the number of registers consumed so far.
  if (F->isVarArg())
    return nullptr;

  // Don't transform functions that receive inallocas, as the transformation may
  // not be safe depending on calling convention.
  if (F->getAttributes().hasAttrSomewhere(Attribute::InAlloca))
    return nullptr;

  // First check: see if there are any pointer arguments!  If not, quick exit.
  SmallVector<Argument *, 16> PointerArgs;
  for (Argument &I : F->args())
    if (I.getType()->isPointerTy())
      PointerArgs.push_back(&I);
  if (PointerArgs.empty())
    return nullptr;

  // Second check: make sure that all callers are direct callers.  We can't
  // transform functions that have indirect callers.  Also see if the function
  // is self-recursive and check that target features are compatible.
  bool isSelfRecursive = false;
  bool isCallback = false; // INTEL
  for (Use &U : F->uses()) {
#if INTEL_CUSTOMIZATION
    AbstractCallSite CS(&U);
    // Must be a direct or a callback call.
    if (!CS || !(CS.isDirectCall() || CS.isCallbackCall()) || !CS.isCallee(&U))
      return nullptr;

    if (CS.isDirectCall()) {
      // Can't change signature of musttail callee
      if (CS.getInstruction()->isMustTailCall())
        return nullptr;

      if (CS.getInstruction()->getParent()->getParent() == F)
        isSelfRecursive = true;
    }

    if (CS.isCallbackCall())
      isCallback = true;
#endif // INTEL_CUSTOMIZATION
  }

  // Can't change signature of musttail caller
  // FIXME: Support promoting whole chain of musttail functions
  for (BasicBlock &BB : *F)
    if (BB.getTerminatingMustTailCall())
      return nullptr;

  const DataLayout &DL = F->getParent()->getDataLayout();

  AAResults &AAR = AARGetter(*F);

  // Check to see which arguments are promotable.  If an argument is promotable,
  // add it to ArgsToPromote.
  SmallPtrSet<Argument *, 8> ArgsToPromote;
  SmallPtrSet<Argument *, 8> ByValArgsToTransform;
  for (Argument *PtrArg : PointerArgs) {
    Type *AgTy = cast<PointerType>(PtrArg->getType())->getElementType();

    // Replace sret attribute with noalias. This reduces register pressure by
    // avoiding a register copy.
    if (PtrArg->hasStructRetAttr()) {
      unsigned ArgNo = PtrArg->getArgNo();
      F->removeParamAttr(ArgNo, Attribute::StructRet);
      F->addParamAttr(ArgNo, Attribute::NoAlias);
      for (Use &U : F->uses()) {
        CallBase &CB = cast<CallBase>(*U.getUser());
        CB.removeParamAttr(ArgNo, Attribute::StructRet);
        CB.addParamAttr(ArgNo, Attribute::NoAlias);
      }
    }

#if INTEL_CUSTOMIZATION
    // For callbacks need to check that broker function propagates argument to
    // the callee at all callback call sites.
    if (isCallback && !all_of(F->uses(), [&](const Use &U) {
          AbstractCallSite ACS(&U);
          if (ACS.isCallbackCall()) {
            int ArgNo = ACS.getCallArgOperandNo(*PtrArg);

            // Broker function must propagate argument to the callback.
            if (ArgNo < 0)
              return false;

            // And it should be passed to the broker as a vararg argument.
            // Otherwise we would need to change broker function signature.
            Function *Broker = ACS.getInstruction()->getCalledFunction();
            assert(Broker && "Expecting broker function");
            if (!Broker->isVarArg() ||
                static_cast<unsigned>(ArgNo) < Broker->arg_size())
              return false;
          }
          return true;
        }))
      continue;
#endif // INTEL_CUSTOMIZATION

    // If this is a byval argument, and if the aggregate type is small, just
    // pass the elements, which is always safe, if the passed value is densely
    // packed or if we can prove the padding bytes are never accessed.
    //
    // Only handle arguments with specified alignment; if it's unspecified, the
    // actual alignment of the argument is target-specific.
    bool isSafeToPromote = PtrArg->hasByValAttr() && PtrArg->getParamAlign() &&
                           !isCallback && // INTEL
                           (ArgumentPromotionPass::isDenselyPacked(AgTy, DL) ||
                            !canPaddingBeAccessed(PtrArg));
#if INTEL_COLLAB
    if (cast<PointerType>(PtrArg->getType())->getAddressSpace() !=
        DL.getAllocaAddrSpace()) {
      // Replacing arguments with non-default address space is incorrect:
      //   define void @foo(
      //       %struct.ty addrspace(4)* byval(%struct.ty) %x' argument) {
      //   entry:
      //     <use of %struct.ty addrspace(4)* %x>
      //   }
      //
      // The following alloca will be created to replace %x:
      //   %new.x = alloca %struct.ty
      //
      // Since the new value's type is '%struct.ty*' the RAUW will fail.
      isSafeToPromote = false;
    }
#endif // INTEL_COLLAB
    if (isSafeToPromote) {
      if (StructType *STy = dyn_cast<StructType>(AgTy)) {
        if (MaxElements > 0 && STy->getNumElements() > MaxElements) {
          LLVM_DEBUG(dbgs() << "argpromotion disable promoting argument '"
                            << PtrArg->getName()
                            << "' because it would require adding more"
                            << " than " << MaxElements
                            << " arguments to the function.\n");
          continue;
        }

        // If all the elements are single-value types, we can promote it.
        bool AllSimple = true;
        for (const auto *EltTy : STy->elements()) {
          if (!EltTy->isSingleValueType()) {
            AllSimple = false;
            break;
          }
        }

        // Safe to transform, don't even bother trying to "promote" it.
        // Passing the elements as a scalar will allow sroa to hack on
        // the new alloca we introduce.
        if (AllSimple) {
          ByValArgsToTransform.insert(PtrArg);
          continue;
        }
      }
    }

    // If the argument is a recursive type and we're in a recursive
    // function, we could end up infinitely peeling the function argument.
    if (isSelfRecursive) {
      if (StructType *STy = dyn_cast<StructType>(AgTy)) {
        bool RecursiveType =
            llvm::is_contained(STy->elements(), PtrArg->getType());
        if (RecursiveType)
          continue;
      }
    }

    // Otherwise, see if we can promote the pointer to its value.
    Type *ByValTy =
        PtrArg->hasByValAttr() ? PtrArg->getParamByValType() : nullptr;
#if INTEL_CUSTOMIZATION
    if (isSafeToPromoteArgument(PtrArg, ByValTy, AAR, isCallback,
                                RemoveHomedArguments, MaxElements))
#endif // INTEL_CUSTOMIZATION
      ArgsToPromote.insert(PtrArg);
  }

  // No promotable pointer arguments.
  if (ArgsToPromote.empty() && ByValArgsToTransform.empty())
    return nullptr;

  if (!ArgumentPromotionPass::areFunctionArgsABICompatible(
          *F, TTI, ArgsToPromote, ByValArgsToTransform))
    return nullptr;

  return doPromotion(F, ArgsToPromote, ByValArgsToTransform, // INTEL
                     isCallback, ReplaceCallSite);           // INTEL
}

PreservedAnalyses ArgumentPromotionPass::run(LazyCallGraph::SCC &C,
                                             CGSCCAnalysisManager &AM,
                                             LazyCallGraph &CG,
                                             CGSCCUpdateResult &UR) {
  bool Changed = false, LocalChange;

  // Iterate until we stop promoting from this SCC.
  do {
    LocalChange = false;

    FunctionAnalysisManager &FAM =
        AM.getResult<FunctionAnalysisManagerCGSCCProxy>(C, CG).getManager();

    for (LazyCallGraph::Node &N : C) {
      Function &OldF = N.getFunction();

      // FIXME: This lambda must only be used with this function. We should
      // skip the lambda and just get the AA results directly.
      auto AARGetter = [&](Function &F) -> AAResults & {
        assert(&F == &OldF && "Called with an unexpected function!");
        return FAM.getResult<AAManager>(F);
      };

      const TargetTransformInfo &TTI = FAM.getResult<TargetIRAnalysis>(OldF);
#if INTEL_CUSTOMIZATION
      Function *NewF =
          promoteArguments(&OldF, AARGetter, RemoveHomedArguments, MaxElements,
                           None, TTI);
#endif // INTEL_CUSTOMIZATION
      if (!NewF)
        continue;
      LocalChange = true;

#if INTEL_CUSTOMIZATION
      getInlineReport()->replaceFunctionWithFunction(&OldF, NewF);
      getMDInlineReport()->replaceFunctionWithFunction(&OldF, NewF);
#endif // INTEL_CUSTOMIZATION

      // Directly substitute the functions in the call graph. Note that this
      // requires the old function to be completely dead and completely
      // replaced by the new function. It does no call graph updates, it merely
      // swaps out the particular function mapped to a particular node in the
      // graph.
      C.getOuterRefSCC().replaceNodeFunction(N, *NewF);
      FAM.clear(OldF, OldF.getName());
      OldF.eraseFromParent();

      PreservedAnalyses FuncPA;
      FuncPA.preserveSet<CFGAnalyses>();
      for (auto *U : NewF->users()) {
#if INTEL_CUSTOMIZATION
        // CMPLRLLVM-32836: Accommodate bitcasts within the arguments of
        // broker functions, which is an Intel extension.
        auto CB = dyn_cast<CallBase>(U);
        if (CB) {
          auto *UserF = CB->getFunction();
          FAM.invalidate(*UserF, FuncPA);
        } else {
          for (auto *V : U->users()) {
            auto *UserF = cast<CallBase>(V)->getFunction();
            FAM.invalidate(*UserF, FuncPA);
          }
        }
#endif // INTEL_CUSTOMIZATION
      }
    }

    Changed |= LocalChange;
  } while (LocalChange);

  if (!Changed)
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  // We've cleared out analyses for deleted functions.
  PA.preserve<FunctionAnalysisManagerCGSCCProxy>();
#if INTEL_CUSTOMIZATION
  PA.preserve<AndersensAA>();
  PA.preserve<WholeProgramAnalysis>();
#endif // INTEL_CUSTOMIZATION
  // We've manually invalidated analyses for functions we've modified.
  PA.preserveSet<AllAnalysesOn<Function>>();
  return PA;
}

namespace {

/// ArgPromotion - The 'by reference' to 'by value' argument promotion pass.
struct ArgPromotion : public CallGraphSCCPass {
  // Pass identification, replacement for typeid
  static char ID;

#if INTEL_CUSTOMIZATION
  explicit ArgPromotion(bool RemoveHomedArguments = false,
                        unsigned MaxElements = 3)
      : CallGraphSCCPass(ID), RemoveHomedArguments(RemoveHomedArguments),
        MaxElements(MaxElements) {
#endif // INTEL_CUSTOMIZATION
    initializeArgPromotionPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<AssumptionCacheTracker>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addPreserved<AndersensAAWrapperPass>();      // INTEL
    AU.addPreserved<WholeProgramWrapperPass>();     // INTEL
    AU.addRequired<TargetTransformInfoWrapperPass>();
    getAAResultsAnalysisUsage(AU);
    CallGraphSCCPass::getAnalysisUsage(AU);
  }

  bool runOnSCC(CallGraphSCC &SCC) override;

private:
  using llvm::Pass::doInitialization;

  bool doInitialization(CallGraph &CG) override;

#if INTEL_CUSTOMIZATION
  /// True if we should remove homed parameters before attempting
  /// argument promotion.
  bool RemoveHomedArguments;
#endif // INTEL_CUSTOMIZATION
  /// The maximum number of elements to expand, or 0 for unlimited.
  unsigned MaxElements;
};

} // end anonymous namespace

char ArgPromotion::ID = 0;

INITIALIZE_PASS_BEGIN(ArgPromotion, "argpromotion",
                      "Promote 'by reference' arguments to scalars", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(AssumptionCacheTracker)
INITIALIZE_PASS_DEPENDENCY(CallGraphWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_END(ArgPromotion, "argpromotion",
                    "Promote 'by reference' arguments to scalars", false, false)

#if INTEL_CUSTOMIZATION
Pass *llvm::createArgumentPromotionPass(bool RemoveHomedArguments,
                                        unsigned MaxElements) {
  return new ArgPromotion(RemoveHomedArguments, MaxElements);
}
#endif // INTEL_CUSTOMIZATION

bool ArgPromotion::runOnSCC(CallGraphSCC &SCC) {
  if (skipSCC(SCC))
    return false;

  // Get the callgraph information that we need to update to reflect our
  // changes.
  CallGraph &CG = getAnalysis<CallGraphWrapperPass>().getCallGraph();

  LegacyAARGetter AARGetter(*this);

  bool Changed = false, LocalChange;

  // Iterate until we stop promoting from this SCC.
  do {
    LocalChange = false;
    // Attempt to promote arguments from all functions in this SCC.
    for (CallGraphNode *OldNode : SCC) {
      Function *OldF = OldNode->getFunction();
      if (!OldF)
        continue;

      auto ReplaceCallSite = [&](CallBase &OldCS, CallBase &NewCS) {
        Function *Caller = OldCS.getParent()->getParent();
        CallGraphNode *NewCalleeNode =
            CG.getOrInsertFunction(NewCS.getCalledFunction());
        CallGraphNode *CallerNode = CG[Caller];
        CallerNode->replaceCallEdge(cast<CallBase>(OldCS),
                                    cast<CallBase>(NewCS), NewCalleeNode);
      };

      const TargetTransformInfo &TTI =
          getAnalysis<TargetTransformInfoWrapperPass>().getTTI(*OldF);
#if INTEL_CUSTOMIZATION
      if (Function *NewF = promoteArguments(OldF, AARGetter,
                                            RemoveHomedArguments,
                                            MaxElements,
                                           {ReplaceCallSite}, TTI)) {
#endif // INTEL_CUSTOMIZATION
        LocalChange = true;

        // INTEL Argument promotion is replacing F with NF.
        // INTEL We need to update all of the call graph reports to reflect
        // INTEL this.
#if INTEL_CUSTOMIZATION
        getInlineReport()->replaceFunctionWithFunction(OldF, NewF);
        getMDInlineReport()->replaceFunctionWithFunction(OldF, NewF);
#endif // INTEL_CUSTOMIZATION

        // Update the call graph for the newly promoted function.
        CallGraphNode *NewNode = CG.getOrInsertFunction(NewF);
        NewNode->stealCalledFunctionsFrom(OldNode);
        if (OldNode->getNumReferences() == 0)
          delete CG.removeFunctionFromModule(OldNode);
        else
          OldF->setLinkage(Function::ExternalLinkage);

        // And updat ethe SCC we're iterating as well.
        SCC.ReplaceNode(OldNode, NewNode);
      }
    }
    // Remember that we changed something.
    Changed |= LocalChange;
  } while (LocalChange);

  return Changed;
}

bool ArgPromotion::doInitialization(CallGraph &CG) {
  return CallGraphSCCPass::doInitialization(CG);
}
