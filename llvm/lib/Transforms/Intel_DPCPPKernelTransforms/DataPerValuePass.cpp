//==--- DataPerBarrierValue.cpp - Collect Data per value - C++ -*-----------==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DataPerValuePass.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelBarrierUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"

using namespace llvm;

INITIALIZE_PASS_BEGIN(DataPerValue, "dpcpp-kernel-data-per-value-analysis",
                      "Barrier Pass - Collect Data per Value", false, true)
INITIALIZE_PASS_DEPENDENCY(DataPerBarrier)
INITIALIZE_PASS_DEPENDENCY(WIRelatedValue)
INITIALIZE_PASS_END(DataPerValue, "dpcpp-kernel-data-per-value-analysis",
                    "Barrier Pass - Collect Data per Value", false, true)

namespace llvm {
char DataPerValue::ID = 0;

DataPerValue::DataPerValue()
    : ModulePass(ID), SyncInstructions(nullptr), DL(nullptr) {
  initializeDataPerValuePass(*llvm::PassRegistry::getPassRegistry());
}

bool DataPerValue::runOnModule(Module &M) {
  // Get Analysis data.
  DataPerBarrierAnalysis = &getAnalysis<DataPerBarrier>();
  WIRelatedValueAnalysis = &getAnalysis<WIRelatedValue>();

  // Initialize barrier utils class with current module.
  BarrierUtils.init(&M);

  // obtain DataLayout of the module.
  DL = &M.getDataLayout();
  assert(DL && "Failed to obtain instance of DataLayout!");

  // Find and sort all connected function into disjointed groups.
  calculateConnectedGraph(M);

  for (auto &F : M) {
    runOnFunction(F);
  }

  // Find all functions that call synchronize instructions.
  FuncSet &FunctionsWithSync =
      BarrierUtils.getAllFunctionsWithSynchronization();
  // Collect data for each function with synchronize instruction.
  for (Function *F : FunctionsWithSync) {
    markSpecialArguments(*F);
  }

  // Check that stride size is aligned with max alignment.
  for (auto &EntryBufferPair : EntryBufferDataMap) {
    unsigned int MaxAlignment = EntryBufferPair.second.MaxAlignment;
    unsigned int CurrentOffset = EntryBufferPair.second.CurrentOffset;
    // Check if CurrentOffset needs to be changed to be divisible
    // by MaxAlignment.
    if (MaxAlignment != 0 && (CurrentOffset % MaxAlignment) != 0) {
      // If yes, round up to the next divisible.
      CurrentOffset = (CurrentOffset + MaxAlignment) & (~(MaxAlignment - 1));
    }
    EntryBufferPair.second.BufferTotalSize = CurrentOffset;
  }
  return false;
}

bool DataPerValue::runOnFunction(Function &F) {
  if (F.isDeclaration())
    return false;

  if (DataPerBarrierAnalysis->hasSyncInstruction(&F)) {
    SyncInstructions = &DataPerBarrierAnalysis->getSyncInstructions(&F);
  } else {
    // CSSD100016517: workaround
    // a function has no synchronize instruction but it still needs to calculate
    // its private memory size.
    SyncInstructions = nullptr;
  }

  // Run over all the values of the function and Cluster into 3 groups.
  // Group-A   : Alloca instructions
  //  Important: we make exclusion for Alloca instructions which
  //             reside between 2 dummyBarrier calls:
  //             a) one     - the 1st instruction which inserted by
  //               BarrierInFunctionPass;
  //             b) another - the instruction which marks
  //               the bottom of Allocas
  //               of WG function return value accumulators.
  // Group-B.1 : Values crossed barriers and the value is
  //            related to WI-Id or initialized inside a loop
  // Group-B.2 : Value crossed barrier but does not suit Group-B.2

  // At first - collect exclusions from Group-A (allocas for WG function
  // results).
  SetVector<Instruction *> AllocaExclusions;
  inst_iterator FirstInstr = inst_begin(F);
  if (FirstInstr != inst_end(F)) {
    if (CallInst *FirstCallInst = dyn_cast<CallInst>(&*FirstInstr)) {
      if (BarrierUtils.isDummyBarrierCall(FirstCallInst)) {
        // if 1st instruction is a dummy barrier call.
        for (inst_iterator IIter = ++FirstInstr, IIterEnd = inst_end(F); IIter != IIterEnd;
             ++IIter) {
          // Collect allocas until next dummy-barrier-call boundary,
          // or drop the collection altogether if barrier call is encountered.
          Instruction *I = &*IIter;
          if (isa<AllocaInst>(I)) {
            // This alloca is a candidate for exclusion.
            AllocaExclusions.insert(I);
          } else if (CallInst *CI = dyn_cast<CallInst>(I)) {
            // Locate boundary of code extent where exclusions are possible:
            // next dummy barrier, w/o a barrier call in the way.
            if (BarrierUtils.isDummyBarrierCall(CI)) {
              break;
            } else if (BarrierUtils.isBarrierCall(CI)) {
              // If there is a barrier call - discard ALL exclusions.
              AllocaExclusions.clear();
              break;
            }
          }
        } // end of collect-allocas loop.
      }   // end of 1st-instruction-is-a-dummy-barrier-call case.
    }
  }

  // Then - sort-out instructions among Group-A, Group-B.1 and Group-B.2.
  for (auto &IRef : instructions(F)) {
    Instruction *I = &IRef;
    if (isa<AllocaInst>(I)) {
      // It is an alloca value, add it to Group_A container.
      if (AllocaExclusions.count(I) == 0) {
        // Filter-out exclusions.
        AllocaValuesPerFuncMap[&F].push_back(I);
      }
      continue;
    }
    // TODO: port work group level functions.
    switch (isSpecialValue(I, WIRelatedValueAnalysis->isWIRelated(I))) {
    case SpecialValueTypeB1:
      // It is an special value, add it to special value container.
      SpecialValuesPerFuncMap[&F].push_back(I);
      break;
    case SpecialValueTypeB2:
      // It is an uniform value that usage cross a barrier
      // add it to cross barrier value container.
      CrossBarrierValuesPerFuncMap[&F].push_back(I);
      break;
    case SpecialValueTypeNone:
      // No need to handle this value.
      break;
    default:
      llvm_unreachable("Unknown special value type!");
    }
  }

  calculateOffsets(F);

  return false;
}

DataPerValue::SpecialValueType DataPerValue::isSpecialValue(Value *V,
                                                            bool IsWIRelated) {
  // CSSD100016517: workaround
  // SpecialValueTypeNone if there are no synchronize instructions.
  if (!SyncInstructions)
    return SpecialValueTypeNone;

  // Value "v" is special (cross barrier) if there is
  // one barrier instruction "i" and one value usage "u" such that:
  // BB(v) in BB(i)->predecessors and BB(u) in B(i)->successors.
  Instruction *I = dyn_cast<Instruction>(V);
  assert(I && "trying check if non-instruction is a special value!");
  BasicBlock *BB = I->getParent();

  // Value that is not dependent on WI-Id and initialized outside a loop
  // can not be in Group-B.1. If it cross a barrier it will be in Group-B.2.
  bool IsNotGroupB1Type =
      !IsWIRelated &&
      !DataPerBarrierAnalysis->getPredecessors(BB).count(BB);

  // By default we assume value is not special till prove otherwise.
  SpecialValueType RetType = SpecialValueTypeNone;

  // Run over all usages of V and check if one crosses a barrier.
  for (auto *U : V->users()) {
    Instruction *InstUsage = cast<Instruction>(U);
    BasicBlock *UsageBB = InstUsage->getParent();

    if (UsageBB == BB && !isa<PHINode>(InstUsage)) {
      // V and InstUsage has same basic block and V apears before
      // InstUsage Sync instruction exists only at begin of basic block, thus
      // these values are not crossed by sync instruction, check next usage of V.
      continue;
    }

    if (isa<ReturnInst>(InstUsage)) {
      // Return value is saved on special buffer by
      // BarrierPass::fixNonInlinedInternalFunction
      // should not consider it special value at this point!
      continue;
    }

    // Run over all sync instructions.
    for (Instruction *SyncInst : *SyncInstructions) {
      BasicBlock *SyncBB = SyncInst->getParent();
      if (SyncBB->getParent() != BB->getParent()) {
        assert(false &&
               "can we reach sync instructions from other functions?!");
        // This sync instructions is from another function.
        continue;
      }
      if (DataPerBarrierAnalysis->getPredecessors(SyncBB).count(BB) &&
          DataPerBarrierAnalysis->getSuccessors(SyncBB).count(UsageBB)) {
        // Found value usage "u" and barrier "i" such that
        // BB(v) in BB(i)->predecessors and BB(u) in B(i)->successors.

        if (IsWIRelated &&
            !DataPerBarrierAnalysis->getPredecessors(SyncBB).count(SyncBB)) {
          // V depends on work item id and crosses a barrier that is not in a
          // loop.
          return SpecialValueTypeB1;
        }

        if (DataPerBarrierAnalysis->getPredecessors(SyncBB).count(SyncBB)) {
          // SyncBB is a predecessor of itself
          // means synchronize instruction is inside a loop.

          BasicBlock *PrevBB =
              DPCPPKernelBarrierUtils::findBasicBlockOfUsageInst(I,
                                                                 InstUsage);
          if (BarrierUtils.isCrossedByBarrier(*SyncInstructions,
                                              PrevBB, BB)) {
            // V does not depend on work item id but it is crossed by loop
            // barrier.
            return (IsNotGroupB1Type) ? SpecialValueTypeB2 : SpecialValueTypeB1;
          }
          // Current usage of V are not crossed by barrier,
          // skip other barriers and check the next usage of V.
          break;
        }
        // V does not depend on work item id and it is crossed by a non loop
        // barrier Upgrade RetType to be special value of group-B.2.
        if (IsNotGroupB1Type) {
          // We can return at this point as we know that IsNotGroupB1Type ==
          // true!
          return SpecialValueTypeB2;
        }
        // But still need to check if it cross other loop barriers.
        RetType = SpecialValueTypeB2;
      }
      assert(!(DataPerBarrierAnalysis->getPredecessors(SyncBB).count(
                   UsageBB) &&
               DataPerBarrierAnalysis->getSuccessors(SyncBB).count(BB)) &&
             "can usage come before value?! (Handle such case)");
    }
  }
  return RetType;
}

void DataPerValue::calculateOffsets(Function &F) {

  ValueVector &SpecialValues = SpecialValuesPerFuncMap[&F];
  const unsigned int Entry = FunctionEntryMap[&F];
  SpecialBufferData &BufferData = EntryBufferDataMap[Entry];

  // Run over all special values in function.
  for (Value *V : SpecialValues) {
    // Get Offset of special value type.
    ValueOffsetMap[V] = getValueOffset(V, V->getType(), 0, BufferData);
  }

  ValueVector &AllocaValues = AllocaValuesPerFuncMap[&F];

  // Run over all special values in function.
  for (Value *V : AllocaValues) {
    AllocaInst *AI = cast<AllocaInst>(V);
    // Get Offset of alloca instruction contained type.
    ValueOffsetMap[V] =
        getValueOffset(V, V->getType()->getContainedType(0),
                       AI->getAlignment(), BufferData);
  }
}

unsigned int DataPerValue::getValueOffset(Value *V, Type *Ty,
                                          unsigned int AllocaAlignment,
                                          SpecialBufferData &BufferData) {

  // TODO: check what is better to use for alignment?
  // unsigned int alignment = DL->getABITypeAlignment(Ty);
  unsigned int Alignment =
      (AllocaAlignment) ? AllocaAlignment : DL->getPrefTypeAlignment(Ty);
  unsigned int SizeInBits = DL->getTypeSizeInBits(Ty);

  Type *ElementType = Ty;
  VectorType *VecType = dyn_cast<VectorType>(Ty);
  if (VecType) {
    ElementType = VecType->getElementType();
  }
  if (isa<VectorType>(ElementType))
    llvm_unreachable("Element type of a vector is another vector!");
  if (DL->getTypeSizeInBits(ElementType) == 1) {
    // We have a Value with base type i1.
    OneBitElementValues.insert(V);
    // We will extend i1 to i32 before storing to special buffer.
    // In case of vector type, scale alignment and size.
    Alignment = (VecType ? VecType->getNumElements() : 1) * 4;
    SizeInBits = (VecType ? VecType->getNumElements() : 1) * 32;

    // This assertion seems to not hold for all Data Layouts
    // assert(DL.getPrefTypeAlignment(Ty) ==
    //  (VecType ? VecType->getNumElements() : 1) &&
    //  "assumes alignment of vector of i1 type equals to vector length");
  }

  unsigned int SizeInBytes = SizeInBits / 8;
  assert(SizeInBytes && "SizeInBytes is 0");

  // Update max alignment.
  if (Alignment > BufferData.MaxAlignment) {
    BufferData.MaxAlignment = Alignment;
  }

  if ((BufferData.CurrentOffset % Alignment) != 0) {
    // Offset is not aligned on value size.
    assert(((Alignment & (Alignment - 1)) == 0) &&
           "Alignment is not power of 2!");
    // TODO: check what to do with the following assert - it fails on
    //      test_basic.exe kernel_memory_alignment_private
    // assert( (alignment <= 32) && "alignment is bigger than 32 bytes (should
    // we align to more than 32 bytes?)" );
    BufferData.CurrentOffset =
        (BufferData.CurrentOffset + Alignment) & (~(Alignment - 1));
  }
  assert((BufferData.CurrentOffset % Alignment) == 0 &&
         "Offset is not aligned on value size!");
  // Found offset of given type.
  unsigned int Offset = BufferData.CurrentOffset;
  // Increment current available offset with V size.
  BufferData.CurrentOffset += SizeInBytes;

  return Offset;
}

void DataPerValue::calculateConnectedGraph(Module &M) {
  unsigned int CurrEntry = 0;

  // Run on all functions in module
  for (Function &FRef : M) {
    Function *F = &FRef;
    if (F->isDeclaration()) {
      // Skip non defined functions
      continue;
    }
    // Check if function has no synchronize instruction!
    if (!DataPerBarrierAnalysis->hasSyncInstruction(F)) {
      // Function has no synchronize instruction: skip it!
      // CSSD100016517: workaround
      // Functions with no barrier still need to have an entry.
      if (FunctionEntryMap.count(F))
        fixEntryMap(FunctionEntryMap[F], CurrEntry++);
      else
        FunctionEntryMap[F] = CurrEntry++;
      continue;
    }
    if (FunctionEntryMap.count(F)) {
      // F already has an entry number,
      // replace all its instances with the current entry number.
      fixEntryMap(FunctionEntryMap[F], CurrEntry);
    } else {
      // F has no entry number yet, give it the current entry number
      FunctionEntryMap[F] = CurrEntry;
    }
    for (auto *U : F->users()) {
      CallInst *CI = dyn_cast<CallInst>(U);
      // Usage of F can be a global variable!
      if (!CI)
        continue;

      Function *CallerFunc = CI->getFunction();
      if (FunctionEntryMap.count(CallerFunc)) {
        // pCallerFunc already has an entry number,
        // replace all appears of it with the current entry number.
        fixEntryMap(FunctionEntryMap[CallerFunc], CurrEntry);
      } else {
        // pCallerFunc has no entry number yet, give it the current entry
        // number
        FunctionEntryMap[CallerFunc] = CurrEntry;
      }
    }
    CurrEntry++;
  }
}

void DataPerValue::fixEntryMap(unsigned int From, unsigned int To) {
  if (From == To) {
    // No need to fix anything.
    return;
  }
  // Replace all occurences of value "From" with value "To".
  for (auto &KV : FunctionEntryMap) {
    if (KV.second == From) {
      KV.second = To;
    }
  }
}

void DataPerValue::markSpecialArguments(Function &F) {
  unsigned int NumOfArgs = F.getFunctionType()->getNumParams();
  bool HasReturnValue = !(F.getFunctionType()->getReturnType()->isVoidTy());
  // Keep one last argument for return value.
  unsigned int NumOfArgsWithReturnValue =
      HasReturnValue ? NumOfArgs + 1 : NumOfArgs;

  if (0 == NumOfArgsWithReturnValue) {
    // Function takes no arguments, nothing to check.
    return;
  }

  const unsigned int Entry = FunctionEntryMap[&F];
  SpecialBufferData &BufferData = EntryBufferDataMap[Entry];

  SmallVector<bool, 16> ArgsFunction;
  ArgsFunction.assign(NumOfArgsWithReturnValue, false);
  // Check each call to F function searching parameters stored in special buffer.
  for (auto *U : F.users()) {
    CallInst *CI = dyn_cast<CallInst>(U);
    // Usage of F can be a global variable!
    if (!CI)
      continue;

    for (unsigned int i = 0; i < NumOfArgsWithReturnValue; ++i) {
      Value *V = (i == NumOfArgs) ? CI : CI->getArgOperand(i);
      if (hasOffset(V)) {
        // If reach here, means that this function has at least
        // one caller with argument value in special buffer
        // Set this argument marker for handling
        ArgsFunction[i] = true;
      }
    }
  }
  for (auto &KV : enumerate(F.args())) {
    if (!ArgsFunction[KV.index()])
      continue;
    Value *V = &KV.value();
    // Argument is marked for handling, get a new offset for this argument.
    ValueOffsetMap[V] = getValueOffset(V, V->getType(), 0, BufferData);
  }
  if (HasReturnValue && ArgsFunction[NumOfArgs]) {
    // Return value is marked for handling, get new offset for this function.
    ValueOffsetMap[&F] = getValueOffset(
        nullptr, F.getFunctionType()->getReturnType(), 0, BufferData);
  }
}

void DataPerValue::print(raw_ostream &OS, const Module *M) const {
  if (!M) {
    OS << "No Module!\n";
    return;
  }
  // Print Module.
  OS << *M;

  // Run on all alloca values.
  OS << "\nGroup-A Values\n";
  for (const auto &KV : AllocaValuesPerFuncMap) {
    const Function *F = KV.first;
    const ValueVector &VV = KV.second;
    if (VV.empty()) {
      // Function has no values of Group-A.
      continue;
    }
    // Print function name.
    OS << "+" << F->getName() << "\n";
    for (const Value *V : VV) {
      // Print alloca value name.
      assert(ValueOffsetMap.count(V) && "V has no offset!");
      OS << "\t-" << V->getName() << "\t("
         << ValueOffsetMap.find(V)->second << ")\n";
    }
    OS << "*"
       << "\n";
  }

  // Run on all special values
  OS << "\nGroup-B.1 Values\n";
  for (const auto &KV : SpecialValuesPerFuncMap) {
    const Function *F = KV.first;
    const ValueVector &VV = KV.second;
    if (VV.empty()) {
      // Function has no values of Group-B.1.
      continue;
    }
    // Print function name.
    OS << "+" << F->getName() << "\n";
    for (const Value *V : VV) {
      // Print special value name.
      assert(ValueOffsetMap.count(V) && "V has no offset!");
      OS << "\t-" << V->getName() << "\t("
         << ValueOffsetMap.find(V)->second << ")\n";
    }
    OS << "*"
       << "\n";
  }

  // Run on all cross barrier unifrom values.
  OS << "\nGroup-B.2 Values\n";
  for (const auto &KV : CrossBarrierValuesPerFuncMap) {
    Function *F = KV.first;
    const ValueVector &VV = KV.second;
    if (VV.empty()) {
      // Function has no values of Group-B.2.
      continue;
    }
    // Print function name.
    OS << "+" << F->getName() << "\n";
    for (const Value *V : VV) {
      // Print cross barrier uniform value name.
      OS << "\t-" << V->getName() << "\n";
    }
    OS << "*"
       << "\n";
  }

  OS << "Buffer Total Size:\n";
  for (const auto &KV : FunctionEntryMap) {
    // Print function name & its entry.
    OS << "+" << KV.first->getName() << " : [" << KV.second << "]\n";
  }
  for (const auto &KV : EntryBufferDataMap) {
    // Print entry & its structure stride.
    OS << "entry(" << KV.first << ") : (" << KV.second.BufferTotalSize
       << ")\n";
  }

  OS << "DONE\n";
}

ModulePass *createDataPerValuePass() { return new llvm::DataPerValue(); }

} // namespace llvm
