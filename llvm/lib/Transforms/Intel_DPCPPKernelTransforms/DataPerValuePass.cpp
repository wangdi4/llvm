//==--- DataPerBarrierValue.cpp - Collect Data per value - C++ -*-----------==//
//
// Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
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
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"

#define DEBUG_TYPE "dpcpp-kernel-data-per-value-analysis"

using namespace llvm;

INITIALIZE_PASS_BEGIN(DataPerValueWrapper, DEBUG_TYPE,
                      "Barrier Pass - Collect Data per Value", false, true)
INITIALIZE_PASS_DEPENDENCY(DataPerBarrierWrapper)
INITIALIZE_PASS_DEPENDENCY(WIRelatedValueWrapper)
INITIALIZE_PASS_END(DataPerValueWrapper, DEBUG_TYPE,
                    "Barrier Pass - Collect Data per Value", false, true)

char DataPerValueWrapper::ID = 0;

DataPerValueWrapper::DataPerValueWrapper() : ModulePass(ID) {
  initializeDataPerValueWrapperPass(*::PassRegistry::getPassRegistry());
}

bool DataPerValueWrapper::runOnModule(Module &M) {
  auto *DPB = &getAnalysis<DataPerBarrierWrapper>().getDPB();
  auto *WRV = &getAnalysis<WIRelatedValueWrapper>().getWRV();
  DPV.reset(new DataPerValue{M, DPB, WRV});
  return false;
}

AnalysisKey DataPerValueAnalysis::Key;
DataPerValue DataPerValueAnalysis::run(Module &M, ModuleAnalysisManager &AM) {
  auto *DPB = &AM.getResult<DataPerBarrierAnalysis>(M);
  auto *WRV = &AM.getResult<WIRelatedValueAnalysis>(M);
  return DataPerValue{M, DPB, WRV};
}

PreservedAnalyses DataPerValuePrinter::run(Module &M,
                                           ModuleAnalysisManager &MAM) {
  MAM.getResult<DataPerValueAnalysis>(M).print(OS, &M);
  return PreservedAnalyses::all();
}

DataPerValue::DataPerValue(Module &M, DataPerBarrier *DPB, WIRelatedValue *WRV)
    : SyncInstructions(nullptr), DL(nullptr), DPB(DPB), WRV(WRV) {
  analyze(M);
}

void DataPerValue::analyze(Module &M) {
  // Initialize barrier utils class with current module.
  Utils.init(&M);

  // Obtain DataLayout of the module.
  DL = &M.getDataLayout();
  assert(DL && "Failed to obtain instance of DataLayout!");

  // Find and sort all connected function into disjointed groups.
  calculateConnectedGraph(M);

  for (Function &F : M)
    runOnFunction(F);

  // Find all functions that call synchronize instructions.
  CompilationUtils::FuncSet FunctionsWithSync =
      Utils.getAllFunctionsWithSynchronization();

  // Collect data for each function with synchronize instruction.
  for (Function *F : FunctionsWithSync)
    markSpecialArguments(*F);

  // Check that stide size is aligned with max alignment.
  for (auto &P : LeaderFuncToBufferDataMap) {
    auto &SpecialBufferData = P.second;
    unsigned int MaxAlignment = SpecialBufferData.MaxAlignment;
    unsigned int CurrentOffset = SpecialBufferData.CurrentOffset;
    if (MaxAlignment != 0 && (CurrentOffset % MaxAlignment) != 0)
      CurrentOffset = (CurrentOffset + MaxAlignment) & (~(MaxAlignment - 1));
    SpecialBufferData.BufferTotalSize = CurrentOffset;
  }
}

void DataPerValue::runOnFunction(Function &F) {
  if (F.isDeclaration())
    return;

  SyncInstructions =
      DPB->hasSyncInstruction(&F) ? &DPB->getSyncInstructions(&F) : nullptr;

  // Run over all the values of the function and Cluster into 3 groups
  // Group-A   : Alloca instructions
  //   Important: we make exclusion for Alloca instructions which
  //              reside between 2 dummyBarrier calls:
  //              a) one     - the 1st instruction which inserted by
  //              BarrierInFunctionPass b) another - the instruction which marks
  //              the bottom of Allocas
  //                           of WG function return value accumulators
  // Group-B.1 : Values crossed barriers and the value is
  //             related to WI-Id or initialized inside a loop
  // Group-B.2 : Value crossed barrier but does not suit Group-B.2

  // At first - collect exclusions from Group-A (allocas for WG function
  // results)
  std::set<Instruction *> AllocaExclusions;
  inst_iterator FirstInstr = inst_begin(F);
  if (FirstInstr != inst_end(F)) {
    if (CallInst *FirstCallInst = dyn_cast<CallInst>(&*FirstInstr)) {
      if (Utils.isDummyBarrierCall(FirstCallInst)) {
        // If 1st instruction is a dummy barrier call.
        for (inst_iterator II = ++FirstInstr, IE = inst_end(F); II != IE;
             ++II) {
          // Collect allocas until next dummy-barrier-call boundary,
          // or drop the collection altogether if barrier call is encountered.
          Instruction *Inst = &*II;
          if (isa<AllocaInst>(Inst)) {
            // This alloca is a candidate for exclusion.
            AllocaExclusions.insert(Inst);
          } else if (CallInst *CI = dyn_cast<CallInst>(Inst)) {
            // Locate boundary of code extent where exclusions are possible:
            // next dummy barrier, w/o a barrier call in the way.
            if (Utils.isDummyBarrierCall(CI)) {
              break;
            } else if (Utils.isBarrierCall(CI)) {
              // If there is a barrier call - discard all exclusions.
              AllocaExclusions.clear();
              break;
            }
          }
        } // end of collect-allocas loop
      }   // end of 1st-instruction-is-a-dummy-barrier-call case
    }
  }

  // Then - sort-out instructions among Group-A, Group-B.1 and Group-B.2.
  for (Instruction &I : instructions(F)) {
    Instruction *Inst = &I;
    if (isa<AllocaInst>(Inst)) {
      // It is an alloca value, add it to Group_A container.
      if (AllocaExclusions.find(Inst) == AllocaExclusions.end()) {
        // Filter-out exclusions.
        AllocaValuesPerFuncMap[&F].push_back(Inst);
      }
      continue;
    }
    if (CallInst *CI = dyn_cast<CallInst>(Inst)) {
      Function *CalledFunc = CI->getCalledFunction();
      if (CalledFunc && !CalledFunc->getReturnType()->isVoidTy()) {
        std::string FuncName = CalledFunc->getName().str();
        // The __finalize_ WG function is also uniform if the original WG
        // function is uniform. So remove the '__finalize_' prefix if any.
        if (CompilationUtils::hasWorkGroupFinalizePrefix(FuncName))
          FuncName = CompilationUtils::removeWorkGroupFinalizePrefix(FuncName);
        if (CompilationUtils::isWorkGroupUniform(FuncName)) {
          // Uniform WG functions always produce cross-barrier value.
          CrossBarrierValuesPerFuncMap[&F].push_back(Inst);
          continue;
        }
        if (CompilationUtils::isWorkGroupScan(FuncName)) {
          // Call instructions to WG functions which produce WI-specific
          // result, need to be stored in the special buffer.
          assert(WRV->isWIRelated(Inst) && "Must be work-item realted value!");
          SpecialValuesPerFuncMap[&F].push_back(Inst);
          continue;
        }
      }
    }
    collectCrossBarrierUses(Inst);
    switch (isSpecialValue(Inst, WRV->isWIRelated(Inst))) {
    case SpecialValueTypeB1:
      // It is a special value, and add it to special value container.
      SpecialValuesPerFuncMap[&F].push_back(Inst);
      break;
    case SpecialValueTypeB2:
      // It is an uniform value whose usage crosses a barrier,
      // and add it to cross-barrier value container.
      CrossBarrierValuesPerFuncMap[&F].push_back(Inst);
      break;
    case SpecialValueTypeNone:
      // No need to handle this value.
      break;
    default:
      llvm_unreachable("Unknown special value type!");
    }
  }

  calculateOffsets(F);
}

bool DataPerValue::crossesBarrier(Use &U) {
  Instruction *Inst = cast<Instruction>(U.get());
  Instruction *User = cast<Instruction>(U.getUser());
  BasicBlock *ValBB = Inst->getParent();
  BasicBlock *UserBB = User->getParent();

  // If Inst and its user reside in the same basic block, and the user isn't
  // a PHI node, then Inst must dominate its user. And as sync instructions
  // only exist at the beginning of basic blocks, the def-use of the Inst
  // doesn't cross barrier.
  if (UserBB == ValBB && !isa<PHINode>(User))
    return false;

  // Check if the def-use crosses a barrier.
  if (none_of(*SyncInstructions, [=](Instruction *SyncInst) {
        BasicBlock *SyncBB = SyncInst->getParent();
        return DPB->getPredecessors(SyncBB).count(ValBB) ||
               DPB->getSuccessors(SyncBB).count(UserBB);
      }))
    return false;

  BasicBlock *UseBB;
  if (PHINode *PHI = dyn_cast<PHINode>(User))
    UseBB = PHI->getIncomingBlock(U);
  else
    UseBB = User->getParent();

  return BarrierUtils::isCrossedByBarrier(*SyncInstructions, UseBB, ValBB);
}

void DataPerValue::collectCrossBarrierUses(Instruction *Inst) {
  if (!SyncInstructions)
    return;

  UseSet US;
  for (Use &U : Inst->uses()) {
    Instruction *User = cast<Instruction>(U.getUser());

    if (!crossesBarrier(U))
      continue;

    // Uses in 'ret' instructions don't belong to either Group-B.1 or
    // Group-B.2. We don't consider them as special values here, and they
    // are handled by BarrierPass::fixNonInlineFunction.
    if (isa<ReturnInst>(User)) {
      CrossBarrierReturnedValues.insert(Inst);
      continue;
    }

    // The def-use of the instruction crosses barrier, so insert the
    // users into the set.
    US.insert(&U);
  }
  if (!US.empty()) {
    Function *F = Inst->getFunction();
    CrossBarrierUses[F][Inst] = std::move(US);
  }
}

DataPerValue::SpecialValueType DataPerValue::isSpecialValue(Instruction *Inst,
                                                            bool IsWIRelated) {
  // SpecialValueTypeNone if there are no synchronize instructions.
  auto *UserMap = getCrossBarrierUses(Inst->getFunction());
  if (!UserMap || UserMap->empty())
    return SpecialValueTypeNone;

  auto InstIt = UserMap->find(Inst);
  if (InstIt == UserMap->end() || InstIt->second.empty())
    return SpecialValueTypeNone;

  BasicBlock *ValBB = Inst->getParent();

  // Value that is not dependent on WI-Id and initialized outside a loop
  // can not be in Group-B.1. If it cross a barrier it will be in Group-B.2.
  bool IsIntializedInLoop = DPB->getPredecessors(ValBB).count(ValBB);
  bool IsGroupB1Type = IsWIRelated || IsIntializedInLoop;

  if (IsWIRelated)
    LLVM_DEBUG(dbgs() << "[Group-B.1 : WI-related]" << *Inst << '\n');
  if (IsIntializedInLoop)
    LLVM_DEBUG(dbgs() << "[Group-B.1 : Initialized-in-loop]" << *Inst << '\n');

  return IsGroupB1Type ? SpecialValueTypeB1 : SpecialValueTypeB2;
}

void DataPerValue::calculateOffsets(Function &F) {

  CompilationUtils::ValueVec &SpecialValues = SpecialValuesPerFuncMap[&F];
  SpecialBufferData &BufferData = getSpecialBufferData(&F);

  // Run over all special values in function.
  for (Value *Val : SpecialValues) {
    // Get Offset of special value type.
    ValueOffsetMap[Val] = getValueOffset(Val, Val->getType(), 0, BufferData);
  }

  CompilationUtils::ValueVec &AllocaValues = AllocaValuesPerFuncMap[&F];

  // Run over all alloca values in function.
  for (Value *Val : AllocaValues) {
    AllocaInst *AI = cast<AllocaInst>(Val);
    // Get Offset of alloca instruction contained type.
    ValueOffsetMap[Val] =
        getValueOffset(Val, Val->getType()->getContainedType(0),
                       AI->getAlign().value(), BufferData);
  }
}

unsigned int DataPerValue::getValueOffset(Value *Val, Type *Ty,
                                          unsigned int AllocaAlignment,
                                          SpecialBufferData &BufferData) {

  // TODO: check what is better to use for Alignment?
  // unsigned int Alignment = DL->getABITypeAlignment(Ty);
  unsigned int Alignment =
      (AllocaAlignment) ? AllocaAlignment : DL->getPrefTypeAlignment(Ty);
  unsigned int SizeInBits = DL->getTypeAllocSizeInBits(Ty);

  Type *EleType = Ty;
  FixedVectorType *VecType = dyn_cast<FixedVectorType>(Ty);
  if (VecType)
    EleType = VecType->getElementType();
  assert(!isa<VectorType>(EleType) &&
         "element type of a vector is another vector!");

  if (DL->getTypeSizeInBits(EleType) == 1) {
    // We have a Value with base type i1.
    OneBitElementValues.insert(Val);
    // We will extend i1 to i32 before storing to special buffer.
    Alignment = PowerOf2Ceil((VecType ? VecType->getNumElements() : 1) * 4);
    SizeInBits = (VecType ? VecType->getNumElements() : 1) * 32;

    // This assertion seems to not hold for all Data Layouts
    // assert(DL.getPrefTypeAlignment(Ty) ==
    //   (VecType ? VecType->getNumElements() : 1) &&
    //   "assumes alignment of vector of i1 type equals to vector length");
  }

  if (AllocaInst *AI = dyn_cast_or_null<AllocaInst>(Val)) {
    if (AI->isArrayAllocation()) {
      const ConstantInt *C = dyn_cast<ConstantInt>(AI->getArraySize());
      assert(C && "We cannot handle non static allocas in barrier!");
      assert((C->getValue().getActiveBits() <= 64) &&
             "The array size cannot be bigger than a uint64_t!");
      SizeInBits = SizeInBits * (C->getZExtValue());
    }
  }

  assert(Alignment && "Alignment is 0");

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
    //       test_basic.exe kernel_memory_alignment_private
    // assert( (Alignment <= 32) && "Alignment is bigger than 32 bytes (should
    // we align to more than 32 bytes?)" );
    BufferData.CurrentOffset =
        (BufferData.CurrentOffset + Alignment) & (~(Alignment - 1));
  }
  assert((BufferData.CurrentOffset % Alignment) == 0 &&
         "Offset is not aligned on value size!");
  // Found offset of given type.
  unsigned int Offset = BufferData.CurrentOffset;
  // Increment current available offset with Val size.
  BufferData.CurrentOffset += SizeInBytes;

  return Offset;
}

void DataPerValue::calculateConnectedGraph(Module &M) {
  for (Function &F : M) {
    Function *Func = &F;
    if (Func->isDeclaration()) {
      // Skip non defined functions.
      continue;
    }
    FuncEquivalenceClasses.insert(Func);
    // Function with no barrier is on its own equivalence class.
    if (!DPB->hasSyncInstruction(Func))
      continue;
    for (auto *U : Func->users()) {
      CallInst *CI = dyn_cast<CallInst>(U);
      // Usage of Func can be a global variable!
      if (!CI)
        continue;
      // Caller and callee are in the same class.
      Function *CallerFunc = CI->getCaller();
      FuncEquivalenceClasses.unionSets(CallerFunc, Func);
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

  SpecialBufferData &BufferData = getSpecialBufferData(&F);

  SmallVector<bool, 16> ArgsFunction;
  ArgsFunction.assign(NumOfArgsWithReturnValue, false);
  // Check each call to F function searching parameters stored in special buffer.
  for (User *U : F.users()) {
    CallInst *CI = dyn_cast<CallInst>(U);
    // Usage of Func can be a global variable!
    if (!CI)
      continue;

    for (unsigned int I = 0; I < NumOfArgsWithReturnValue; ++I) {
      Value *Val = (I == NumOfArgs) ? CI : CI->getArgOperand(I);
      // Cross-barrier returned value don't have offset yet and it'll have
      // offset in Barrier::fixReturnValue, so we need to set marker.
      if (hasOffset(Val) ||
          (I == NumOfArgs && CrossBarrierReturnedValues.count(Val))) {
        // If reach here, means that this function has at least one caller with
        // argument value in special buffer. Set this argument marker for
        // handling
        ArgsFunction[I] = true;
      }
    }
  }
  for (auto &ArgIdxPair : enumerate(F.args())) {
    if (!ArgsFunction[ArgIdxPair.index()])
      continue;
    Value *Val = &ArgIdxPair.value();
    // Argument is marked for handling, get a new offset for this argument.
    ValueOffsetMap[Val] = getValueOffset(Val, Val->getType(), 0, BufferData);
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
    const ValueVec &VV = KV.second;
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
    const ValueVec &VV = KV.second;
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
    const ValueVec &VV = KV.second;
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

  // Print equivalence classes.
  OS << "Function Equivalence Classes:\n";
  for (auto I = FuncEquivalenceClasses.begin(),
            E = FuncEquivalenceClasses.end();
       I != E; ++I) {
    if (!I->isLeader())
      continue;
    // Print leader func.
    OS << '[' << I->getData()->getName() << "]: ";
    // Print members in the same class.
    for (auto MI = FuncEquivalenceClasses.member_begin(I);
         MI != FuncEquivalenceClasses.member_end(); ++MI)
      OS << (*MI)->getName() << ' ';
    OS << '\n';
  }

  OS << "Buffer Total Size:\n";
  for (const auto &KV : LeaderFuncToBufferDataMap) {
    // Print leader func & its structure stride.
    OS << "leader(" << KV.first->getName() << ") : ("
       << KV.second.BufferTotalSize << ")\n";
  }

  OS << "DONE\n";
}

ModulePass *llvm::createDataPerValueWrapperPass() {
  return new DataPerValueWrapper();
}
