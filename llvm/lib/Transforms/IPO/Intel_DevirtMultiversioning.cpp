#if INTEL_FEATURE_SW_DTRANS
//=- Intel_DevirtMultiversioning.cpp - Intel Devirtualization Multiversion --=//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// This file contains a helper class used by the Whole Program Devirtualization
// pass to apply multiversioning rather than generate a branch funnel intrinsic.
// For example, consider that there is a base class called Base with a pure
// virtual function foo. Also, consider that there are 2 derived classes:
// Derived and Derived2, and both classes have a definition for foo. The IR
// first looks as follows:
//
//  %. = select i1 %cmp,
//       i1 (%class.Base*, i32)*** bitcast (%class.Derived* @d
//          to i1 (%class.Base*, i32)***),
//       i1 (%class.Base*, i32)*** bitcast (%class.Derived2* @d2
//          to i1 (%class.Base*, i32)***)
//  %.4 = select i1 %cmp,
//        %class.Base* getelementptr inbounds (%class.Derived,
//          %class.Derived* @d, i64 0, i32 0),
//        %class.Base* getelementptr inbounds (%class.Derived2,
//          %class.Derived2* @d2, i64 0, i32 0)
//  store i1 (%class.Base*, i32)*** %., i1 (%class.Base*, i32)****
//     bitcast (%class.Base** @b to i1 (%class.Base*, i32)****),
//     align 8, !tbaa !8
//  %vtable = load i1 (%class.Base*, i32)**, i1 (%class.Base*, i32)*** %.,
//            align 8, !tbaa !12
//  %0 = bitcast i1 (%class.Base*, i32)** %vtable to i8*
//  %1 = tail call i1 @llvm.type.test(i8* %0, metadata !"_ZTS4Base")
//  tail call void @llvm.assume(i1 %1)
//  %2 = load i1 (%class.Base*, i32)*, i1 (%class.Base*, i32)** %vtable,
//       align 8
//  %call = tail call zeroext i1 %2(%class.Base* %.4, i32 %argc)
//  %call.i = tail call dereferenceable(272)
//              %"class.std::basic_ostream"* @_ZNSo9_M_insertIbEERSoT_(
//                %"class.std::basic_ostream"* nonnull @_ZSt4cout,
//                i1 zeroext %call)
//
// The %vtable is collected from %. and the actual function is in %2. The
// values %0 and %1 are related to the metadata added by the CFE. The call
// to the virtual function happens in %call. The multiversioning will transform
// the IR as follows:
//
//  %2 = bitcast i1 (%class.Base*, i32)* %1 to i8*
//  %3 = icmp eq i8* %2, bitcast (i1 (%class.Derived*, i32)*
//       @_ZN7Derived3fooEi to i8*)
//  br i1 %3, label %BBDevirt__ZN7Derived3fooEi_0_0,
//            label %ElseDevirt__ZN7Derived3fooEi_0_0
//
// BBDevirt__ZN7Derived3fooEi_0_0:
//  %4 = tail call zeroext i1 bitcast (i1 (%class.Derived*, i32)*
//       @_ZN7Derived3fooEi to i1 (%class.Base*, i32)*)
//       (%class.Base* %.4, i32 %argc)
//  br label %MergeBB_0_0
//
// ElseDevirt__ZN7Derived3fooEi_0_0:
//  %5 = icmp eq i8* %2, bitcast (i1 (%class.Derived2*, i32)*
//       @_ZN8Derived23fooEi to i8*)
//  br i1 %5, label %BBDevirt__ZN8Derived23fooEi_0_0, label %DefaultBB_0_0
//
// BBDevirt__ZN8Derived23fooEi_0_0:
//  %6 = tail call zeroext i1 bitcast (i1 (%class.Derived2*, i32)*
//       @_ZN8Derived23fooEi to i1 (%class.Base*, i32)*)
//       (%class.Base* %.4, i32 %argc)
//  br label %MergeBB_0_0
//
// DefaultBB_0_0:
//  %7 = tail call zeroext i1 %1(%class.Base* %.4, i32 %argc)
//  br label %MergeBB_0_0
//
// MergeBB_0_0:
//  %8 = phi i1 [ %4, %BBDevirt__ZN7Derived3fooEi_0_0 ],
//              [ %6, %BBDevirt__ZN8Derived23fooEi_0_0 ],
//              [ %7, %DefaultBB_0_0]
//  br label %9
//
// <label>:9:
//  %call.i = tail call dereferenceable(272)
//            %"class.std::basic_ostream"* @_ZNSo9_M_insertIbEERSoT_(
//              %"class.std::basic_ostream"* nonnull @_ZSt4cout,
//              i1 zeroext %8)
//
// The address of the virtual function will be compared with the address
// of each of the targets. If an address matches, then call the direct
// target, else call the virtual function.
//
// In case whole program safe is achieved then the virtual call won't be added.
// The result will be the following:
//
//  %2 = bitcast i1 (%class.Base*, i32)* %1 to i8*
//  %3 = icmp eq i8* %2, bitcast (i1 (%class.Derived*, i32)*
//       @_ZN7Derived3fooEi to i8*)
//  br i1 %3, label %BBDevirt__ZN7Derived3fooEi_0_0,
//            label %BBDevirt__ZN8Derived23fooEi_0_0:
//
// BBDevirt__ZN7Derived3fooEi_0_0:
//  %4 = tail call zeroext i1 bitcast (i1 (%class.Derived*, i32)*
//       @_ZN7Derived3fooEi to i1 (%class.Base*, i32)*)
//       (%class.Base* %.4, i32 %argc)
//  br label %MergeBB_0_0
//
// BBDevirt__ZN8Derived23fooEi_0_0:
//  %5 = tail call zeroext i1 bitcast (i1 (%class.Derived2*, i32)*
//       @_ZN8Derived23fooEi to i1 (%class.Base*, i32)*)
//       (%class.Base* %.4, i32 %argc)
//  br label %MergeBB_0_0
//
// MergeBB_0_0:
//  %6 = phi i1 [ %4, %BBDevirt__ZN7Derived3fooEi_0_0 ],
//              [ %5, %BBDevirt__ZN8Derived23fooEi_0_0 ]
//  br label %7
//
// <label>:7:
//  %call.i = tail call dereferenceable(272)
//            %"class.std::basic_ostream"* @_ZNSo9_M_insertIbEERSoT_(
//              %"class.std::basic_ostream"* nonnull @_ZSt4cout,
//              i1 zeroext %6)
//
// In the previous example the virtual call is removed because we know that
// everything is inside the LTO unit, therefore all targets were found.
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/Intel_DevirtMultiversioning.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;
using namespace dtransOP;

// Allows us to perform the multiversioning for the devirtualization
// as opposed to generating the branch funnels
static cl::opt<bool> WPDevirtMultiversion("wholeprogramdevirt-multiversion",
                                          cl::init(true), cl::ReallyHidden);

// Set the threshold for the maximum targets we can devirtualize using
// the multiversioning
static cl::opt<unsigned>
    WPDevirtMaxBranchTargets("wholeprogramdevirt-max-branch-targets",
                             cl::init(4), cl::ReallyHidden);

// Run the verifier to check if the multiversion generated something wrong
static cl::opt<bool>
    WPDevirtMultiversionVerify("wholeprogramdevirt-multiversion-verify",
                               cl::init(false), cl::ReallyHidden);

// Enable downcasting filtering for opaque pointers and DTrans metadata
static cl::opt<bool>
    WPDevirtDownCastingFilteringForOP("wholeprogramdevirt-downcasting-filter",
                                      cl::init(true), cl::ReallyHidden);

IntelDevirtMultiversion::IntelDevirtMultiversion(
    Module &M, WholeProgramInfo &WPInfo,
    std::function<const TargetLibraryInfo &(const Function &F)> GetTLI)
    : M(M), WPInfo(WPInfo), GetTLI(GetTLI),
      EnableDevirtMultiversion(WPDevirtMultiversion) {

  DevirtCallMDNode = MDNode::get(
      M.getContext(), MDString::get(M.getContext(), "_Intel.Devirt.Call"));
}

// Add a new target function into the list of targets. If the target is a
// libfunc or an external function then set VCallsData.HasLibFuncAsTarget
// as true.
void IntelDevirtMultiversion::addTarget(Function *Fn) {
  if (!Fn)
    return;

  if (functionIsLibFuncOrExternal(Fn))
    VCallsData.HasLibFuncAsTarget = true;

  VCallsData.TargetFunctions.insert(Fn);
}

// Insert a new virtual call site that needs to be multiversioned. If
// the casting in CallBase refers to an actual Function, then don't
// insert it. This means that the indirect call was converted already
// by another pass.
void IntelDevirtMultiversion::addVirtualCallSite(CallBase *VCallSite) {
  if (!VCallSite)
    return;

  // Skip those calls that have been devirtualized already
  const Value *CalledOperand =
      VCallSite->getCalledOperand()->stripPointerCasts();
  if (isa<Function>(CalledOperand))
    return;

  VCallsData.VirtualCallSites.push_back(VCallSite);
}

// Clear all the data in VCallsData
void IntelDevirtMultiversion::resetData() {
  VCallsData.TargetFunctions.clear();
  VCallsData.VirtualCallSites.clear();
  VCallsData.HasLibFuncAsTarget = false;
}

// Return true if the type of the target function is the same as the virtual
// call function's type. If they aren't the same, then check if the return
// type, number of arguments, and all the arguments except arg 0 are the same.
// The reason we can skip arg 0 is because it represents the '*this' pointer,
// and we know that the target function is in the derived class, and the
// virtual call is using a base class.
//
// NOTE: This function assumes that the target belongs to a derived structure
// and the virtual call uses its base structure. If this function returns true
// for a particular case where it shouldn't, the it means that the target
// function was collected incorrectly.
bool IntelDevirtMultiversion::basedDerivedFunctionTypeMatches(
    FunctionType *VCallType, FunctionType *TargetFuncType) {

  if (!VCallType || !TargetFuncType)
    return false;

  if (VCallType == TargetFuncType)
    return true;

  if (VCallType->isVarArg() != TargetFuncType->isVarArg())
    return false;

  unsigned VCallNumParams = VCallType->getNumParams();
  unsigned TargetFuncNumParams = TargetFuncType->getNumParams();
  if (VCallNumParams != TargetFuncNumParams)
    return false;

  // Return type must be the same
  auto *VCallRetType = VCallType->getReturnType();
  auto *TargetFuncRetType = TargetFuncType->getReturnType();
  if (VCallRetType != TargetFuncRetType)
    return false;

  // If there is no parameter to check then return.
  if (VCallNumParams == 0)
    return true;

  // We can skip parameter 0, which represents the '*this' pointer, since we
  // know at this point that the target function belongs to a derived
  // structure, and the virtual call to the base structure.

  // The rest of the parameters should be the same
  for (unsigned I = 1, E = TargetFuncNumParams; I < E; I++) {
    if (VCallType->getParamType(I) != TargetFuncType->getParamType(I))
      return false;
  }

  return true;
}

// Create the BasicBlocks with the direct call sites.
//   TargetVector:    a TargetData vector that will be used to store the
//                      information related to the targets
//   VCallSite:       callsite of the virtual call
//   TargetFunctions: possible targets to the current virtual callsite
//   MDNode:          node containing the metadata information for the
//                      target functions
bool IntelDevirtMultiversion::createCallSiteBasicBlocks(
    Module &M, std::vector<TargetData *> &TargetVector, CallBase *VCallSite,
    const SetVector<Function *> &TargetFunctions, MDNode *Node) {

  IRBuilder<> Builder(M.getContext());
  StringRef BaseName = StringRef("BBDevirt_");
  Instruction *CSInst = VCallSite;
  Function *Func = CSInst->getFunction();
  SmallPtrSet<Function *, 10> FuncsProcessed;
  bool AllTargetsAreNotTheSame = false;

  // Add all the function addresses and create the BasicBlocks
  // with the direct calls
  for (auto TargetFunc : TargetFunctions) {

    // CMPLRLLVM-22269: The TargetsForSlot array can have repeated
    // entries. This is caused by the process in tryFindVirtualCallTargets,
    // which collects the target functions from multiple vtables, and
    // one target function can be in two or more vtables. If we processed a
    // function already, then prevent generating another branch for the same
    // function.
    if (!FuncsProcessed.insert(TargetFunc).second)
      continue;

    // If the function type doesn't match then don't generate a multiversion
    // target for it, but we are going to include the default case.
    //
    // NOTE: We use basedDerivedFunctionTypeMatches to identify if the *this
    // pointers for the virtual call and the target function are the same or
    // derived. Once we fully move to opaque pointers then this function is not
    // needed and we can have direct comparison
    // (VCallSite->getFunctionType() == TargetFunc->getFunctionType()).
    if (!basedDerivedFunctionTypeMatches(VCallSite->getFunctionType(),
                                         TargetFunc->getFunctionType())) {
      AllTargetsAreNotTheSame = true;
      continue;
    }

    Builder.SetInsertPoint(CSInst);

    TargetData *NewTarget = new TargetData();
    std::string FuncName = TargetFunc->getName().str();

    NewTarget->TargetFunc = TargetFunc;

    // Create a new BasicBlock with the name
    // BBDevirt_TARGETNAME
    std::string BBName = Twine(BaseName, FuncName.c_str()).str();
    NewTarget->TargetName = FuncName;

    NewTarget->TargetBasicBlock =
        BasicBlock::Create(M.getContext(), BBName.c_str(), Func);

    // Clone the call instruction
    Instruction *CloneCS = CSInst->clone();

    // Change the builder inside the basic block created
    // and insert the cloned function
    Builder.SetInsertPoint(NewTarget->TargetBasicBlock);
    Builder.Insert(CloneCS);

    // Replace the called function with the direct call
    CallBase *NewCB = cast<CallBase>(CloneCS);
    if (TargetFunc->getFunctionType() != VCallSite->getFunctionType()) {
      NewCB->setCalledOperand(ConstantExpr::getBitCast(
          TargetFunc, VCallSite->getCalledOperand()->getType()));

      // Because a bitcast operation has been performed to match the callsite to
      // the call target for the object type, mark the call to allow DTrans
      // analysis to treat the 'this' pointer argument as being the expected
      // type for the call, rather than a mismatched argument type. The
      // devirtualizer has proven the types to match, so this marking avoids
      // needing to try to prove the types match again during DTrans analysis.
      NewCB->setMetadata("_Intel.Devirt.Call", DevirtCallMDNode);
    } else {
      NewCB->setCalledFunction(TargetFunc);
    }

    // Save the new instruction for PHINode
    NewTarget->CallInstruction = NewCB;

    // Add the metadata "_Intel.Devirt.Target" in the target function
    if (!TargetFunc->hasMetadata() ||
        TargetFunc->getMetadata("_Intel.Devirt.Target") == nullptr)
      TargetFunc->setMetadata("_Intel.Devirt.Target", Node);

    TargetVector.push_back(NewTarget);
  }

  return AllTargetsAreNotTheSame;
}

// Create a BasicBlock that will be the merge point for all targets.
// Also, collect the BasicBlock that will continue the function
//   M:          current module
//   VCallSite:  CallBase pointing to the virtual call site
// This function returns the BasicBlock that all call sites will jump to.
BasicBlock *IntelDevirtMultiversion::getMergePoint(Module &M,
                                                   CallBase *VCallSite) {

  BasicBlock *EndPointBB = nullptr;
  BasicBlock *MergePointBB = nullptr;
  IRBuilder<> Builder(M.getContext());
  std::string MergePointName = "MergeBB";
  Instruction *CSInst = VCallSite;
  Function *Func = CSInst->getFunction();
  BasicBlock *BB = CSInst->getParent();

  // Build the merge point with the following name
  // MergeBB
  MergePointBB =
      BasicBlock::Create(M.getContext(), MergePointName.c_str(), Func);

  // Split the main BasicBlock in case the call instruction
  // is a CallInst
  //   BB: everything before the virtual function call
  //   BBEndPoint: Everything from the virtual function call until the end
  if (isa<CallInst>(VCallSite)) {
    EndPointBB = BB->splitBasicBlock(CSInst->getNextNode());
    // The current terminator branch is not needed since is going to be
    // replaced with an if/else
    BB->getTerminator()->eraseFromParent();
    // NOTE: The PHINodes that are pointing to the main BasicBlock
    // aren't replaced in this path since the splitting operation
    // will take care of fixing them.
  }
  // Else, InvokeInst, collect the destination
  else if (isa<InvokeInst>(VCallSite)) {
    // Replace the PHINodes that are pointing to the main BasicBlock
    // with the merge point. The PHINodes that are in the unwind
    // destinations will be fixed later.
    BB->replaceSuccessorsPhiUsesWith(MergePointBB);
    EndPointBB = cast<InvokeInst>(CSInst)->getNormalDest();
  } else {
    llvm_unreachable("wholeprogramdevirt-multiversion:"
                     " Branch end point not found");
  }

  // Create a branch in the merge point that will jump into the end point
  Builder.SetInsertPoint(MergePointBB);
  Builder.CreateBr(EndPointBB);

  return MergePointBB;
}

// Build the default case that will call the virtual call in case
// the address doesn't match.
//   M:          current module
//   VCallSite:  CallBase pointing to the virtual call site
// This function returns a new TargetData object with the information
// needed to call the virtual instruction.
IntelDevirtMultiversion::TargetData *
IntelDevirtMultiversion::buildDefaultCase(Module &M, CallBase *VCallSite) {

  Instruction *CSInst = VCallSite;
  Value *CalledVal = VCallSite->getCalledOperand();
  Function *Func = CSInst->getFunction();
  IRBuilder<> Builder(M.getContext());
  std::string DefaultBBName = "DefaultBB";

  // Build the Basic Block for the default case with the name
  // DefaultBB
  BasicBlock *DefaultBB =
      BasicBlock::Create(M.getContext(), DefaultBBName.c_str(), Func);
  Builder.SetInsertPoint(DefaultBB);
  CSInst->removeFromParent();
  Builder.Insert(CSInst);

  IntelDevirtMultiversion::TargetData *DefaultTarget =
      new IntelDevirtMultiversion::TargetData();

  DefaultTarget->TargetFunc = CalledVal;
  DefaultTarget->TargetBasicBlock = DefaultBB;
  DefaultTarget->CallInstruction = CSInst;
  DefaultTarget->TargetName = DefaultBBName;

  return DefaultTarget;
}

// If we are multiversioning an Invoke instruction, there is a chance
// that replacing the PHINodes with the merge point can damage the
// the unwind destinations. This function will remove the merge
// point from those PHINodes in the unwind destinations and replace them
// with the BasicBlocks where the targets are called from.
//   VCallSite:     Information related to the actual virtual call
//   MergePointBB:  BasicBlock that where all targets will branch into
//   TargetsVector: Vector containing all the targets for the virtual call
//   DefaultTarget: TargetData generated that contains the call to
//                    the virtual function
void IntelDevirtMultiversion::fixUnwindPhiNodes(
    CallBase *VCallSite, BasicBlock *MergePointBB,
    std::vector<TargetData *> &TargetsVector, TargetData *DefaultTarget,
    bool DefaultTargetNeeded) {

  if (!isa<InvokeInst>(VCallSite))
    return;

  InvokeInst *InvokeI = cast<InvokeInst>(VCallSite);
  BasicBlock *UnwindBB = InvokeI->getUnwindDest();

  // Traverse through each PHINode in the unwind destination
  for (PHINode &PhiNode : UnwindBB->phis()) {

    // If the MergePoint is not found, then the result will be -1
    int BBIdx = PhiNode.getBasicBlockIndex(MergePointBB);
    if (BBIdx < 0)
      continue;

    // Collect the value that is used by the merge point
    Value *Val = PhiNode.getIncomingValue(BBIdx);

    // Check if the Value is produced by the return of the InvokeInst.
    // If so, then it means that we need to replace the Value and the
    // BasicBlock.
    Value *DefaultCall = cast<Instruction>(DefaultTarget->CallInstruction);
    bool SameCall = Val == DefaultCall;

    // Remove the merge point from the current PHINode
    PhiNode.removeIncomingValue(BBIdx);

    // Add the targets into the PHINode
    for (TargetData *Target : TargetsVector) {
      if (SameCall) {
        Value *TargetCall = cast<Value>(Target->CallInstruction);
        PhiNode.addIncoming(TargetCall, Target->TargetBasicBlock);
      } else
        PhiNode.addIncoming(Val, Target->TargetBasicBlock);
    }

    // Add the default case into the PHINode if it is needed
    if (DefaultTargetNeeded)
      PhiNode.addIncoming(Val, DefaultTarget->TargetBasicBlock);
  }
}

// Generate the branching that will do the multiversioning. Each compare
// instruction will check if the address of the virtual call matches with
// the address of a target function.
//   M:             current module
//   MainBB:        BasicBlock where the virtual call instruction
//                    originally came from
//   IsCallInst:    true if the virtual call site is a CallInst,
//                    false if it is an InvokeInst
//   TargetsVector: vector with all target functions and the information
//                    needed to generate the branching
//   DefaultTarget: TargetData with the information related to the virtual
//                    call site, this is used to generate the default case
// If whole program safe is achieved, then the virtual call won't be added
// since all possible targets were found and they are stored in the input
// TargetsVector.
void IntelDevirtMultiversion::generateBranching(
    Module &M, BasicBlock *MainBB, BasicBlock *MergePointBB, bool IsCallInst,
    std::vector<TargetData *> &TargetsVector, TargetData *DefaultTarget,
    bool DefaultTargetNeeded) {

  unsigned int TargetI = 0;
  StringRef ElseBaseName = StringRef("ElseDevirt_");
  Function *Func = DefaultTarget->CallInstruction->getFunction();
  IRBuilder<> Builder(M.getContext());

  Builder.SetInsertPoint(MainBB);

  // NOTE: For now we are going to use getInt8PtrTy to create an i8* pointer.
  // It also supports opaque pointers. If the community decides to remove its
  // support then we need to update it.
  PointerType *Int8PtrTy = Type::getInt8PtrTy(M.getContext());

  BitCastInst *DefaultAddress =
      new BitCastInst(DefaultTarget->TargetFunc, Int8PtrTy);
  Builder.Insert(DefaultAddress);

  BasicBlock *InsertPointBB = MainBB;

  TargetData *LastTarget = nullptr;

  unsigned int EndTarget = TargetsVector.size();

  // If whole program safe is achieved, then we are going to traverse until
  // the second to last
  if (WPInfo.isWholeProgramSafe() && !DefaultTargetNeeded)
    EndTarget--;

  // Create the branching and connect the whole structure
  for (TargetI = 0; TargetI < EndTarget; TargetI++) {

    TargetData *Target = TargetsVector[TargetI];

    // Build the else BasicBlock
    BasicBlock *ElseBB;
    if (TargetI == EndTarget - 1) {

      // If whole program safe is achieved then the last target will
      // be the else case
      if (WPInfo.isWholeProgramSafe() && !DefaultTargetNeeded)
        LastTarget = TargetsVector[TargetI + 1];
      // Else, the virtual call will be added as default case
      else
        LastTarget = DefaultTarget;
      ElseBB = LastTarget->TargetBasicBlock;
    } else {
      // Create the name ElseDevirt_TARGETNAME_CallSlotI_VCallI
      std::string TargetName = Target->TargetName;
      std::string ElseBBName = Twine(ElseBaseName, TargetName.c_str()).str();
      ElseBB = BasicBlock::Create(M.getContext(), ElseBBName.c_str(), Func);
    }

    BasicBlock *IfBB = Target->TargetBasicBlock;

    // Create the condition and branching
    Builder.SetInsertPoint(InsertPointBB);

    BitCastInst *TargetAddress = new BitCastInst(Target->TargetFunc, Int8PtrTy);
    Builder.Insert(TargetAddress);

    Value *Cond = Builder.CreateICmpEQ(DefaultAddress, TargetAddress);
    Builder.CreateCondBr(Cond, IfBB, ElseBB);

    // Insert the branch to merge point in case of CallInst
    if (IsCallInst) {
      Builder.SetInsertPoint(IfBB);
      Builder.CreateBr(MergePointBB);
    } else {
      InvokeInst *InvInst = cast<InvokeInst>(&(IfBB->front()));
      InvInst->setNormalDest(MergePointBB);
    }

    IfBB->moveAfter(InsertPointBB);
    ElseBB->moveAfter(IfBB);
    // The next insert point will be the else branch
    InsertPointBB = ElseBB;
  }

  // Fix the last branch added
  if (IsCallInst) {
    Builder.SetInsertPoint(LastTarget->TargetBasicBlock);
    Builder.CreateBr(MergePointBB);
  } else {
    InvokeInst *InvInst = cast<InvokeInst>(LastTarget->CallInstruction);
    InvInst->setNormalDest(MergePointBB);
  }

  // Rearrange the basic blocks
  if (DefaultTargetNeeded)
    DefaultTarget->TargetBasicBlock->moveAfter(InsertPointBB);

  MergePointBB->moveAfter(LastTarget->TargetBasicBlock);
}

// Generate a PHINode that will replace all the Users of the virtual
// call site. This PHINode will be added to the merge BasicBlock.
//   M:             current module
//   MergePointBB:  BasicBlock that where all targets will branch into
//   TargetsVector: vector with all target functions and the information
//                    needed to generate the branching
//   DefaultTarget: TargetData with the information related to the virtual
//                    call site, this is used to generate the default case
void IntelDevirtMultiversion::generatePhiNodes(
    Module &M, BasicBlock *MergePointBB,
    std::vector<TargetData *> TargetsVector, TargetData *DefaultTarget,
    bool DefaultTargetNeeded) {

  Instruction *CSInst = DefaultTarget->CallInstruction;

  if (CSInst->user_empty())
    return;

  IRBuilder<> Builder(M.getContext());

  // Compute the number of incoming values
  // (number of targets plus default case)
  unsigned int NumTargets = TargetsVector.size() + 1;

  // Insert in the merge point
  Builder.SetInsertPoint(&(MergePointBB->front()));

  // Create the new PHINode
  PHINode *Phi = Builder.CreatePHI(CSInst->getType(), NumTargets);

  // Replace all users of the virtual call instruction with the new PHINode
  CSInst->replaceAllUsesWith(Phi);

  // Insert the incoming Values from each target into the new PHINode
  for (TargetData *Target : TargetsVector) {
    Phi->addIncoming(Target->CallInstruction, Target->TargetBasicBlock);
  }

  // If the default target is needed then insert it.
  if (DefaultTargetNeeded)
    Phi->addIncoming(CSInst, DefaultTarget->TargetBasicBlock);
}

// Helper function that will generate the multiversioning. This function
// will replace the virtual callsite 'VCallSite' with the multiple targets
// in 'TargetFunctions'. If 'LibFuncFound' is true then the default case
// will be included in the multiversioning regardless of whether we achieved
// whole program safe or not.
void IntelDevirtMultiversion::multiversionVCallSite(
    Module &M, CallBase *VCallSite, bool LibFuncFound,
    const SetVector<Function *> &TargetFunctions) {

  if (TargetFunctions.empty() || !EnableDevirtMultiversion)
    return;

  MDNode *Node = MDNode::get(
      M.getContext(), MDString::get(M.getContext(), "_Intel.Devirt.Target"));

  BasicBlock *MainBB = VCallSite->getParent();

  std::vector<TargetData *> TargetsVector;

  // Generate the BasicBlocks that contain the direct call sites
  bool DefaultTargetNeeded = createCallSiteBasicBlocks(
      M, TargetsVector, VCallSite, TargetFunctions, Node);

  // If TargetsVector is empty then it means that the virtual call collected
  // doesn't match the target functions. In this case return because there
  // is nothing to do.
  if (TargetsVector.empty())
    return;

  // Insert the incoming Value from the default case if whole program is not
  // safe, at least one target is a LibFunc, or at least the type of one
  // function in the targets list doesn't match the virtual call.
  if (!DefaultTargetNeeded)
    DefaultTargetNeeded = !WPInfo.isWholeProgramSafe() || LibFuncFound;

  // Compute the merge point
  BasicBlock *MergePoint = getMergePoint(M, VCallSite);

  // Create the default target
  TargetData *DefaultTarget = buildDefaultCase(M, VCallSite);

  // Fix the PHINodes in the unwind destinations
  fixUnwindPhiNodes(VCallSite, MergePoint, TargetsVector, DefaultTarget,
                    DefaultTargetNeeded);

  // Build the branches that will do the multiversioning
  generateBranching(M, MainBB, MergePoint, isa<CallInst>(VCallSite),
                    TargetsVector, DefaultTarget, DefaultTargetNeeded);

  // Build the PHINode and the replacement in case there is any use
  generatePhiNodes(M, MergePoint, TargetsVector, DefaultTarget,
                   DefaultTargetNeeded);

  // Destroy the default case since we have whole program and there is no
  // LibFunc in the list of targets, or the virtual call is not inside a
  // LibFunc with IR.
  if (WPInfo.isWholeProgramSafe() && !DefaultTargetNeeded) {
    DefaultTarget->CallInstruction->eraseFromParent();
    DefaultTarget->TargetBasicBlock->eraseFromParent();
  }

  // Cleanup the data since it isn't needed anymore
  for (auto *TargetInfo : TargetsVector)
    delete TargetInfo;

  delete DefaultTarget;

  TargetsVector.clear();
}

// Return true if the caller function or the target function for the input
// virtual call site are libfuncs, or we couldn't prove the legality of the
// downcasting. In this case, we are going to multiversion the call with the
// default target included. Else, return false.
bool IntelDevirtMultiversion::tryAddingDefaultTargetIntoVCallSite(
    CallBase *VCallSite, Function *TargetFunc, Function *CallerFunc) {

  if (!EnableDevirtMultiversion)
    return false;

  if (!VCallSite || !TargetFunc || !CallerFunc)
    return false;

  bool LibFuncFound = functionIsLibFuncOrExternal(TargetFunc) ||
                      functionIsLibFuncOrExternal(CallerFunc);
  bool TypeNotAvailable = VCallsWithDefaultCase.contains(VCallSite);

  // If the caller is a libfunc, the call is a libfunc or a virtual call site
  // that requires the default case, then we need to multiversion.
  if (!LibFuncFound && !TypeNotAvailable)
    return false;

  SetVector<Function *> TargetFunction;
  TargetFunction.insert(TargetFunc);
  multiversionVCallSite(M, VCallSite, true /* LibFuncFound */, TargetFunction);

  return true;
}

// Main function that generates the multiversioning when a virtual
// call has multiple targets. Return true if multiversioning was
// succefully done, else return false.
bool IntelDevirtMultiversion::tryMultiVersionDevirt() {

  // Check if multi-version flag is available
  if (!EnableDevirtMultiversion)
    return false;

  if (VCallsData.TargetFunctions.empty() || VCallsData.VirtualCallSites.empty())
    return false;

  // If the number of targets is larger than the threshold then don't do
  // the multiversioning.
  unsigned NumBranches = VCallsData.HasLibFuncAsTarget
                             ? VCallsData.TargetFunctions.size() + 1
                             : VCallsData.TargetFunctions.size();

  if (NumBranches > WPDevirtMaxBranchTargets)
    return false;

  IRBuilder<> Builder(M.getContext());

  for (auto *VCallSite : VCallsData.VirtualCallSites) {
    // CMPLRLLVM-23243: If there isn't a libfunc as target function, but
    // the caller function is a libfunc with IR then we will include the
    // default case in the multiversioning.
    auto CallerFunc = VCallSite->getCaller();
    bool DefaultTargetNeeded = VCallsData.HasLibFuncAsTarget ||
                               functionIsLibFuncOrExternal(CallerFunc) ||
                               VCallsWithDefaultCase.contains(VCallSite);

    multiversionVCallSite(M, VCallSite, DefaultTargetNeeded,
                          VCallsData.TargetFunctions);
  }

  return true;
}

// Return true if the input function is a LibFunc or a function
// that wasn't internalized, except main.
bool IntelDevirtMultiversion::functionIsLibFuncOrExternal(Function *F) {
  if (!F)
    return false;

  if (!EnableDevirtMultiversion)
    return false;

  if (WPInfo.getWholeProgramLinkerUtils()->isMainEntryPoint(F->getName()))
    return false;

  LibFunc TheLibFunc;
  const TargetLibraryInfo &TLI = GetTLI(*const_cast<Function *>(F));
  if ((TLI.getLibFunc(F->getName(), TheLibFunc) && TLI.has(TheLibFunc)) ||
      F->isIntrinsic() || !F->hasLocalLinkage())
    return true;

  return false;
}

// Return true if SrcType is one of the elements of DestType, else
// return false.
static bool DowncastingFound(StructType *SrcType, StructType *DestType) {
  if (!SrcType || !DestType)
    return false;

  unsigned NumElem = DestType->getNumElements();
  if (NumElem == 0)
    return false;

  for (unsigned CurrElem = 0; CurrElem < NumElem; CurrElem++)
    if (DestType->getElementType(CurrElem) == SrcType)
      return true;

  return false;
}

// Collect the calls to the assume function that may produce a downcasting.
// This is used in the case of non-opaque pointers where we can identify
// the pointer types casting.
void IntelDevirtMultiversion::collectAssumeCallSitesNonOpaque(
    Function *AssumeFunc, std::vector<CallBase *> &AssumesVector) {

  // Given a Type, find the first non-pointer type that it points-to.
  auto GetElemType = [](llvm::Type *InputType) {
    Type *RootType = nullptr;

    if (!InputType)
      return RootType;

    RootType = InputType;

    // We can use getElementType here since we have typed pointers.
    while (RootType && RootType->isPointerTy()) {
      PointerType *PtrTy = cast<PointerType>(RootType);
      RootType = PtrTy->getElementType();
    }

    return RootType;
  };

  if (!AssumeFunc || !AssumeFunc->isIntrinsic() ||
      AssumeFunc->getIntrinsicID() != Intrinsic::assume)
    return;

  // This process is for typed pointers only
  if (!AssumeFunc->getParent()->getContext().supportsTypedPointers())
    return;

  // Go through each of the users for the intrinsic assume
  for (User *User : AssumeFunc->users()) {

    CallBase *AssumeCall = dyn_cast<CallBase>(User);
    if (!AssumeCall)
      continue;

    // Collect type.test intrinsic
    CallBase *TestCall = dyn_cast<CallBase>(AssumeCall->getArgOperand(0));
    if (!TestCall)
      continue;

    // Collect the VTable that the metadata is being assigned to
    BitCastInst *VTableBCInst =
        dyn_cast<BitCastInst>(TestCall->getArgOperand(0));
    if (!VTableBCInst)
      continue;

    // Get the pointer loaded
    LoadInst *LoadPtr = dyn_cast<LoadInst>(VTableBCInst->getOperand(0));
    if (!LoadPtr)
      continue;

    // Collect the bitcasting from base to derived
    BitCastInst *TypeCasting = dyn_cast<BitCastInst>(LoadPtr->getOperand(0));
    if (!TypeCasting)
      continue;

    // Get the source
    StructType *SrcType =
        dyn_cast<StructType>(GetElemType(TypeCasting->getSrcTy()));
    if (!SrcType)
      continue;

    // Get the destination
    FunctionType *DestType =
        dyn_cast<FunctionType>(GetElemType(TypeCasting->getDestTy()));
    if (!DestType)
      continue;
    // Go through each parameter of the virtual function and check
    // if there is a possible downcasting
    unsigned NumParams = DestType->getNumParams();
    for (unsigned CurrParam = 0; CurrParam < NumParams; CurrParam++) {
      StructType *ParamStruct =
          dyn_cast<StructType>(GetElemType(DestType->getParamType(CurrParam)));

      if (!ParamStruct)
        continue;

      // If SrcType is in ParamStruct's element types list then
      // we found a possible downcasting (Base -> Derived).
      if (DowncastingFound(SrcType, ParamStruct)) {
        AssumesVector.push_back(AssumeCall);
        break;
      }
    }
  }
}

// Collect the calls to the assume function that may produce a downcasting.
// This is used in the case of opaque pointers where we need to do a pointer
// analysis in order to collect the information.
void IntelDevirtMultiversion::collectAssumeCallSitesOpaque(
    Function *AssumeFunc, std::vector<CallBase *> &AssumesVector,
    dtransOP::PtrTypeAnalyzer &Analyzer,
    dtransOP::TypeMetadataReader &MDReader) {

  auto FindVirtualCall = [](LoadInst *FieldLoaded, Value *Ptr) -> CallBase * {
    for (auto *LU : FieldLoaded->users()) {
      auto *Call = dyn_cast<CallBase>(LU);
      if (!Call)
        continue;

      if (Call->arg_size() == 0)
        continue;

      // The loaded pointer is the called function
      if (Call->getCalledOperand() != FieldLoaded)
        continue;

      Value *ThisPtr = Call->getArgOperand(0);
      // Make sure that the pointer is "this"
      if (isa<BitCastInst>(Ptr) && isa<BitCastInst>(ThisPtr)) {
        auto *BcPtr = cast<BitCastInst>(Ptr);
        auto *BcThisPtr = cast<BitCastInst>(ThisPtr);
        if (BcPtr->getOperand(0) == BcThisPtr->getOperand(0))
          return Call;
      } else if (Ptr == ThisPtr) {
        return Call;
      }
    }
    return nullptr;
  };

  if (!AssumeFunc || !AssumeFunc->isIntrinsic() ||
      AssumeFunc->getIntrinsicID() != Intrinsic::assume)
    return;

  // Go through each of the users for the intrinsic assume
  for (User *User : AssumeFunc->users()) {

    CallBase *AssumeCall = dyn_cast<CallBase>(User);
    if (!AssumeCall)
      continue;

    // Collect type.test intrinsic
    CallBase *TestCall = dyn_cast<CallBase>(AssumeCall->getArgOperand(0));
    if (!TestCall)
      continue;

    Function *TestFunc = TestCall->getCalledFunction();
    if (!TestFunc)
      continue;

    if (!TestFunc->isIntrinsic() ||
        TestFunc->getIntrinsicID() != Intrinsic::type_test)
      continue;

    Value *VTablePtr = TestCall->getArgOperand(0);

    // Bitcast instructions won't be available in the case of opaque pointers.
    // This check was added if we have typed pointers and DTrans metadata.
    if (auto *BC = dyn_cast<BitCastInst>(VTablePtr))
      VTablePtr = BC->getOperand(0);

    // Collect the VTable that the metadata is being assigned to
    LoadInst *VTableLoadInst = dyn_cast<LoadInst>(VTablePtr);
    if (!VTableLoadInst)
      continue;

    // Collect the pointer used in the VTable
    Value *Ptr = VTableLoadInst->getOperand(0);

    CallBase *VCall = nullptr;
    // Prove that the pointer and the vtable is used in an indirect call
    for (auto *U : VTableLoadInst->users()) {
      // Virtual functions are part of a class, they are going to be accessed
      // like a field of a class. This means that there could be an GEP for
      // non-zero element access, or a direct load for the zero element access.
      auto *CurrUser = U;
      if (isa<GetElementPtrInst>(CurrUser)) {
        if (!CurrUser->hasOneUser())
          continue;
        CurrUser = CurrUser->user_back();
      }

      auto *Load = dyn_cast<LoadInst>(CurrUser);
      if (!Load)
        continue;

      VCall = FindVirtualCall(Load, Ptr);
      if (VCall)
        break;
    }

    if (!VCall)
      continue;

    Value *ThisPtr = VCall->getArgOperand(0);
    ValueTypeInfo *Info = Analyzer.getValueTypeInfo(ThisPtr);
    if (!Info)
      return;

    // Collect the dominant type, which should be a base structure
    auto *DominantTy = dyn_cast_or_null<DTransPointerType>(
        Analyzer.getDominantType(*Info, ValueTypeInfo::VAT_Decl));
    if (!DominantTy) {

      // PtrTypeAnalyzer doesn't help finding DominantTy, try to get
      // type of ThisPtr (i.e first argument) from intel_dtrans_type
      // metadata that is attached to the call.
      DTransType *ThisArgType = nullptr;
      DTransType *CallMDType = MDReader.getDTransTypeFromMD(VCall);
      auto CallFType = dyn_cast_or_null<DTransFunctionType>(CallMDType);
      if (CallFType && VCall->getFunctionType()->getNumParams() > 0)
        ThisArgType = CallFType->getArgType(0);
      DominantTy = dyn_cast_or_null<DTransPointerType>(ThisArgType);
    }

    if (!DominantTy) {
      VCallsWithDefaultCase.insert(VCall);
      continue;
    }

    StructType *SourceTy = dyn_cast<StructType>(
        DominantTy->getPointerElementType()->getLLVMType());
    if (!SourceTy)
      continue;

    // Now check if a derived type is part of the pointer aliases.
    for (auto *Ty : Info->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use)) {
      auto *PtrTy = dyn_cast<DTransPointerType>(Ty);
      if (!PtrTy)
        continue;

      StructType *DstUseType =
          dyn_cast<StructType>(PtrTy->getPointerElementType()->getLLVMType());
      if (!DstUseType)
        continue;

      if (DstUseType == SourceTy)
        continue;

      if (DowncastingFound(SourceTy, DstUseType)) {
        AssumesVector.push_back(AssumeCall);
        break;
      }
    }
  }
}

// Prevent devirtualization on those virtual call sites that are related to
// downcasting. A possible downcasting occurs when a pointer from a base class
// is cast to a derived class. For example:
//
//   %"class.Base" = type { i32 (...)** }
//   %"class.DerivedA" = type { %"class.Base" }
//   %"class.DerivedB" = type { %"class.Base" }
//
//   %2 = bitcast %"class.Base"* %1 to i32 (%"class.DerivedA"*)***
//   %3 = load i32 (%"class.Base"*)**, i32 (%"class.DerivedA"*)*** %2
//   %4 = bitcast i32 (%"class.DerivedA"*)** %3 to i8*
//   %5 = call i1 @llvm.type.test(i8* %4, metadata !"_ZTS8DerivedA")
//   call void @llvm.assume(i1 %5)
//
// The types %"class.DerivedA" and %"class.DerivedB" have Base as element.
// This means that the classes DerivedA and DerivedB could be derived from
// Base. Consider that %1 is a Value and its type is %"class.Base". The
// bitcasting in %2 converts the %1 into a function pointer. This casting goes
// from %"class.Base" to %"class.DerivedA". This could cause that the
// devirtualization process to convert the virtual call into an incorrect
// direct call.

// The following function will identify the bitcasting mentioned above by
// tracing from the assume call site up to the bitcasting (%2). Then it will
// check if the source type of the casting (%"class.Base") is an element of
// the destination type (%"class.DerivedA"). If so, then remove the assume
// intrinsic, which will prevent the devirtualization.
void IntelDevirtMultiversion::filterDowncasting(Function *AssumeFunc) {
  if (!WPInfo.isWholeProgramSafe() || !AssumeFunc || AssumeFunc->use_empty() ||
      !AssumeFunc->isIntrinsic() ||
      AssumeFunc->getIntrinsicID() != Intrinsic::assume)
    return;

  // Vector that holds the assume callsites that will be removed
  std::vector<CallBase *> AssumesVector;

  if (WPDevirtDownCastingFilteringForOP) {
    // If we have DTrans metadata then we are going to collect the information
    // from the DTrans pointer analyzer. In the case of opaque pointers the
    // pattern will look as follows:
    //
    //   %tmp = load ptr, ptr %p, align 8, !tbaa !75
    //   %tmp2 = tail call i1 @llvm.type.test(ptr %tmp,
    //                                        metadata !"_ZTS8DerivedB")
    //   tail call void @llvm.assume(i1 %tmp2)
    //   %tmp3 = load ptr, ptr %tmp, align 8
    //   tail call void %tmp3(ptr nonnull align 8 dereferenceable(8) %p)
    //
    // Basically, there is going to be a loaded pointer (vtable %tmp) and will
    // be used to check the type.test metadata. Then the virtual function is
    // loaded (%tmp3). What we are looking for is if we have a dominant type
    // for the vtable, then the derived type should not be in the list of
    // dominant types. If we can't prove that there is a dominant type, then
    // the virtual call will be multiversioned with the default case included.
    NamedMDNode *DTransMDTypes = M.getNamedMetadata("intel.dtrans.types");
    if (DTransMDTypes) {
      LLVMContext &Ctx = M.getContext();
      DTransTypeManager TM(Ctx);
      TypeMetadataReader Reader(TM);
      if (Reader.initialize(M)) {
        const DataLayout &DL = M.getDataLayout();
        dtransOP::PtrTypeAnalyzer Analyzer(Ctx, TM, Reader, DL, GetTLI);
        Analyzer.run(M);
        collectAssumeCallSitesOpaque(AssumeFunc, AssumesVector, Analyzer,
                                     Reader);
      }
    }
  }

  // Collect the calls to the assume function in the case of typed pointers.
  // This function will look for the pattern mentioned before that produces
  // a downcasting.
  if (AssumesVector.empty() && VCallsWithDefaultCase.empty())
    collectAssumeCallSitesNonOpaque(AssumeFunc, AssumesVector);

  // Remove the assumes related to downcasting
  for (CallBase *AssumeCall : AssumesVector) {
    CallBase *TestCall = cast<CallBase>(AssumeCall->getArgOperand(0));
    Instruction *TestCallArg =
        dyn_cast<Instruction>(TestCall->getArgOperand(0));

    // Delete the call to assume
    AssumeCall->eraseFromParent();

    // Delete the call to type.test
    if (TestCall->use_empty())
      TestCall->eraseFromParent();

    // Delete the bitcast for the vtable
    if (TestCallArg && TestCallArg->use_empty())
      TestCallArg->eraseFromParent();
  }
}

// Delete the bitcast for the vtable if there is no use of it.
void IntelDevirtMultiversion::deleteVTableCast(Value *VTablePtr) {
  if (!VTablePtr)
    return;

  BitCastInst *PtrInst = dyn_cast<BitCastInst>(VTablePtr);
  if (!PtrInst)
    return;

  if (WPInfo.isWholeProgramSafe() && PtrInst && PtrInst->use_empty())
    PtrInst->eraseFromParent();
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Given a VTableSlotInfo and an array of Targets, print the indirect
// call and the possible targets. This function is used for debugging.
void IntelDevirtMultiversion::PrintVTableInfoAndTargets() {

  for (auto *VCallSite : VCallsData.VirtualCallSites) {
    Function *Caller = VCallSite->getCaller();
    dbgs() << "Function: " << Caller->getName() << "\n";
    dbgs() << "  Indirect Call: " << *VCallSite << "\n";
    dbgs() << "  Targets:";

    if (VCallsData.TargetFunctions.empty()) {
      dbgs() << " No targets\n";
    } else {
      dbgs() << "\n";
      for (auto *TargetFn : VCallsData.TargetFunctions)
        dbgs() << "    " << TargetFn->getName() << "\n";
    }
    dbgs() << "\n";
  }
}
#endif // NDEBUG || LLVM_ENABLE_DUMP

// Run the verifier if it is requested.
void IntelDevirtMultiversion::runDevirtVerifier(Module &M) {
  if (EnableDevirtMultiversion && WPDevirtMultiversionVerify) {
    for (Function &F : M) {
      if (verifyFunction(F)) {
        report_fatal_error("Whole Program Devirtualization: Fails in"
                           " function: " +
                           F.getName() + "()\n");
      }
    }
  }
}

// End functions related to IntelDevirtMultiversion
#endif // INTEL_FEATURE_SW_DTRANS
