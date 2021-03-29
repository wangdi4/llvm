//===-------------- MemManageTrans.cpp - DTransMemManageTransPass ---------===//
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans Initial Memory Management Transformation.
//
// Detects memory pool allocator class by analyzing member functions of
// the class like allocateBlock, destroyObject, commitAllocationObject etc and
// increases size of block (i.e number of objects allocated each time) if
// there are no legality issues.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/MemManageTrans.h"
#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/DTransCommon.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

using namespace llvm;

#define DTRANS_MEMMANAGETRANS "dtrans-memmanagetrans"

namespace {

class DTransMemManageTransWrapper : public ModulePass {
private:
  dtrans::MemManageTransPass Impl;

public:
  static char ID;

  DTransMemManageTransWrapper() : ModulePass(ID) {
    initializeDTransMemManageTransWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;
    DTransAnalysisWrapper &DTAnalysisWrapper =
        getAnalysis<DTransAnalysisWrapper>();
    DTransAnalysisInfo &DTInfo = DTAnalysisWrapper.getDTransInfo(M);
    auto GetTLI = [this](const Function &F) -> const TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };

    bool Changed = Impl.runImpl(
        M, DTInfo, getAnalysis<WholeProgramWrapperPass>().getResult(), GetTLI);
    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DTransAnalysisWrapper>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addPreserved<DTransAnalysisWrapper>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};

} // end anonymous namespace

char DTransMemManageTransWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransMemManageTransWrapper, "dtrans-memmanagetrans",
                      "DTrans Memory Manage Trans", false, false)
INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(DTransMemManageTransWrapper, "dtrans-memmanagetrans",
                    "DTrans Memory Manage Trans", false, false)

ModulePass *llvm::createDTransMemManageTransWrapperPass() {
  return new DTransMemManageTransWrapper();
}

namespace llvm {

namespace dtrans {

// Member functions are classified based on functionality.
enum MemManageFKind : unsigned {
  Constructor = 0,  // Constructor
  Destructor,       // Destructor
  AllocateBlock,    // Allocate Block
  CommitAllocation, // Commit Allocation
  DestroyObject,    // Destroy Object
  Reset,            // Reset
  GetMemManager,    // Get Memory Manager
  UnKnown           // Unknown functionality.
};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Dumper for MemManageFKind.
inline raw_ostream &operator<<(raw_ostream &OS, MemManageFKind FKind) {
  switch (FKind) {
  case Constructor:
    OS << "Constructor";
    break;
  case Destructor:
    OS << "Destructor";
    break;
  case AllocateBlock:
    OS << "AllocateBlock";
    break;
  case CommitAllocation:
    OS << "CommitAllocation";
    break;
  case DestroyObject:
    OS << "DestroyObject";
    break;
  case Reset:
    OS << "Reset";
    break;
  case GetMemManager:
    OS << "GetMemManager";
    break;
  case UnKnown:
    OS << "Unknown";
    break;
  }
  return OS;
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

class MemManageTransImpl {

  constexpr static int MaxNumCandidates = 1;

public:
  MemManageTransImpl(Module &M, DTransAnalysisInfo &DTInfo, MemTLITy GetTLI,
                     const DataLayout &DL)
      : M(M), DTInfo(DTInfo), GetTLI(GetTLI), DL(DL){};

  ~MemManageTransImpl() {
    for (auto *CInfo : Candidates)
      delete CInfo;
  }

  // Returns current processing candidate.
  MemManageCandidateInfo *getCurrentCandidate() {
    // For now, only one candidate is allowed.
    assert(Candidates.size() == 1 && "Unexpected number of candidates");
    auto Cand = *Candidates.begin();
    return Cand;
  }

  bool run(void);

private:
  Module &M;
  DTransAnalysisInfo &DTInfo;
  MemTLITy GetTLI;
  const DataLayout &DL;

  // List of candidates. For now, only one candidate is allowed.
  SmallVector<MemManageCandidateInfo *, MaxNumCandidates> Candidates;

  // Map of MemManageFKind to Function. Only one function is allowed
  // for each MemManageFKind.
  DenseMap<unsigned, Function *> FunctionalityMap;

  // Instruction that stores blockSize in ArenaAllocator.
  // This instruction will be changed during transformation
  // to change value of blockSize.
  StoreInst *BlockSizeStoreInst = nullptr;

  // This set is used to track instructions that are processed.
  std::set<Instruction *> Visited;

  // This StructType is not really used in interface functions except for
  // type casting. NextBlockTy helps to make sure that the same StructType
  // is used in all interface functions.
  StructType *NextBlockTy = nullptr;

  bool gatherCandidates(void);
  bool analyzeCandidates(void);
  bool categorizeFunctions(void);
  bool recognizeFunctions(void);
  bool recognizeGetMemManager(Function *);
  bool recognizeConstructor(Function *);
  bool recognizeAllocateBlock(Function *);
  bool checkConstantSize(Value *, int64_t);
  bool checkSizeValue(Value *, int64_t, Value *);
  bool processBBTerminator(BasicBlock *, Value **, Value **, BasicBlock **,
                           BasicBlock **, ICmpInst::Predicate *);
  BasicBlock *getSingleSucc(BasicBlock *);
  void collectStoreInst(BasicBlock *, SmallVectorImpl<StoreInst *> &);
  Instruction *getFirstNonDbg(BasicBlock *);
  bool isUnreachableOK(BasicBlock *);
  bool isIncrementByOne(Value *, Value **);
  bool isNextBlockFieldAccess(Value *, Value **, Value **, int32_t *);
  bool isNextBlockFieldLoad(Value *, Value **, Value **, int32_t *);
  bool getAllocDeallocCommonSucc(Instruction *, Instruction *, BasicBlock **,
                                 BasicBlock **);
  bool identifyDevirtChecks(BasicBlock *, Value *, Function **, BasicBlock **,
                            BasicBlock **);
  bool identifyAllocCall(BasicBlock *, Value *, Value **, Value **,
                         BasicBlock **);
  bool identifyDeallocCall(BasicBlock *, Value *, Value *, BasicBlock **);
  bool identifyCheckAndAllocNode(BasicBlock *, Value *, BasicBlock **,
                                 BasicBlock **, Value **, bool);
  bool identifyListHead(BasicBlock *, Value *, BasicBlock **, BasicBlock **,
                        Value **);
  bool identifyGetListHead(BasicBlock *, Value *, BasicBlock **, Value **);
  bool identifyListEmpty(BasicBlock *, Value *, BasicBlock **, BasicBlock **);
  bool identifyBlockAvailable(BasicBlock *, Value *, BasicBlock **,
                              BasicBlock **);
  bool identifyCreate(BasicBlock *, Value *, BasicBlock **, Value **);
  bool identifyArenaBlockInit(BasicBlock *, Value *, Value *, Value *,
                              BasicBlock **);
  bool identifyRABAllocateBlock(BasicBlock *, Value *);
  bool identifyPushFront(BasicBlock *, Value *, Value *, BasicBlock *);
  bool identifyPushAtPos(BasicBlock *, Value *, Value *, Value *, Value *,
                         Value *);
  bool getGEPBaseAddrIndex(Value *V, Value **BaseOp, int32_t *Idx);
  bool isArenaAllocatorAddr(Value *V, Value *Obj);
  bool isAllocatorBlockSizeAddr(Value *V, Value *Obj);
  bool isAllocatorBlockSizeLoad(Value *V, Value *Obj);
  bool isDestroyBlockFlagAddr(Value *V, Value *Obj);
  bool isListAddr(Value *V, Value *Obj);
  bool isListMemManagerAddr(Value *V, Value *Obj);
  bool isListMemManagerLoad(Value *V, Value *Obj);
  bool isListFreeHeadAddr(Value *V, Value *Obj);
  bool isListHeadAddr(Value *V, Value *Obj);
  bool isListHeadLoad(Value *V, Value *Obj);
  bool isListFreeHeadLoad(Value *V, Value *Obj);
  bool isListFreeHeadNextLoad(Value *V, Value *Obj);
  bool isNodeNextAddr(Value *V, Value *Obj);
  bool isListBegin(Value *V, Value *Obj);
  bool isListFrontNodeArenaBlockAddr(Value *V, Value *Obj);
  bool isFrontNodeObjectCountAddr(Value *V, Value *Obj);
  bool isFrontNodeObjectCountLoad(Value *V, Value *Obj);
  bool isFrontNodeBlockSizeLoad(Value *V, Value *Obj);
  bool isNodePosNextPrev(Value *V, Value *NodePos);
  bool isNodePosPrevNext(Value *V, Value *NodePos);
  bool isNodePosPrevLoad(Value *V, Value *NodePos);
  bool isNodePosNextLoad(Value *V, Value *NodePos);
  bool isNodePosNext(Value *V, Value *NodePos);
  bool isNodePosPrev(Value *V, Value *NodePos);
  bool isNodePosReusableArenaBlock(Value *V, Value *NodePos);
  bool isArenaBlockAddrFromRAB(Value *V, Value *Obj);
  bool isFirstFreeBlockAddrFromRAB(Value *V, Value *Obj);
  bool isFirstFreeBlockLoadFromRAB(Value *V, Value *Obj);
  bool isNextFreeBlockAddrFromRAB(Value *V, Value *Obj);
  bool isNextFreeBlockLoadFromRAB(Value *V, Value *Obj);
  bool isAllocatorAddrFromRAB(Value *V, Value *Obj);
  bool isObjectCountAddrFromRAB(Value *V, Value *Obj);
  bool isObjectCountLoadFromRAB(Value *V, Value *Obj);
  bool isBlockSizeAddrFromRAB(Value *V, Value *Obj);
  bool isBlockSizeLoadFromRAB(Value *V, Value *Obj);
  bool isObjectBlockAddrFromRAB(Value *V, Value *Obj);
  bool isObjectBlockLoadFromRAB(Value *V, Value *Obj);
  bool isAllocatorMemAddrFromRAB(Value *V, Value *Obj);
  bool isVTableAddrInReusableArenaAllocator(Value *V, Value *ThisObj);
  bool verifyAllInstsProcessed(Function *);
};

// Collect candidates.
bool MemManageTransImpl::gatherCandidates(void) {

  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANS,
                  { dbgs() << "MemManageTrans transformation: \n"; });

  for (TypeInfo *TI : DTInfo.type_info_entries()) {
    auto *StInfo = dyn_cast<StructInfo>(TI);
    if (!StInfo)
      continue;

    std::unique_ptr<MemManageCandidateInfo> CInfo(
        new MemManageCandidateInfo(M));

    if (!CInfo->isCandidateType(TI->getLLVMType()))
      continue;

    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANS, {
      dbgs() << "  Considering candidate: ";
      TI->getLLVMType()->print(dbgs(), true, true);
      dbgs() << "\n";
    });

    if (!CInfo->collectMemberFunctions(true)) {
      DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANS,
                      { dbgs() << "  Failed: member function collection.\n"; });
      continue;
    }
    if (Candidates.size() >= MaxNumCandidates) {
      DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANS, {
        dbgs() << "  Failed: Exceeding maximum candidate limit.\n";
      });
      return false;
    }
    Candidates.push_back(CInfo.release());
  }

  if (Candidates.empty()) {
    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANS,
                    { dbgs() << "  Failed: No candidates found.\n"; });
    return false;
  }
  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANS, {
    dbgs() << "  Possible candidate structs: \n";
    for (auto CInfo : Candidates) {
      StructType *St = CInfo->getStringAllocatorType();
      dbgs() << "      " << St->getName() << "\n";
    }
  });
  return true;
}

// Check legality issues for candidates.
bool MemManageTransImpl::analyzeCandidates(void) {

  // Returns true if "Call" is library function call.
  auto IsLibFunction = [](const CallBase *Call, const TargetLibraryInfo &TLI) {
    LibFunc LibF;
    auto *F = dyn_cast_or_null<Function>(Call->getCalledFunction());
    if (!F || !TLI.getLibFunc(*F, LibF) || !TLI.has(LibF))
      return false;
    return true;
  };

  auto Cand = getCurrentCandidate();

  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANS,
                  { dbgs() << "  Analyzing Candidate ...\n"; });
  // Check all inner function calls, which are calls in InterfaceFunctions,
  // are only:
  // 1. Library function calls
  // 2. DTrans alloc/free calls
  // 3. Dummy alloc/free calls.
  for (auto CB : Cand->inner_function_calls()) {
    auto &TLI = GetTLI(*CB->getFunction());
    if (IsLibFunction(CB, TLI))
      continue;
    auto *CallInfo = DTInfo.getCallInfo(CB);
    if (CallInfo &&
        (CallInfo->getCallInfoKind() == dtrans::CallInfo::CIK_Alloc ||
         CallInfo->getCallInfoKind() == dtrans::CallInfo::CIK_Free))
      continue;
    if (isDummyFuncWithThisAndIntArgs(CB, TLI) ||
        isDummyFuncWithThisAndPtrArgs(CB, TLI))
      continue;
    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANS, {
      dbgs() << "    Failed: Unexpected Inner call " << *CB << "\n";
    });
    return false;
  }

  // TODO: Add more checks here.
  // 1. Makes sure interface functions are called only from
  //    AllocatorStringFunctions
  // 2. Makes sure ReusableArenaAllocatorType and all other related
  //    types are not escaped.

  return true;
}

// Categorize interface functions (AllocatorInterfaceFunctions) using
// return type and signature.
// ReusableArenaAllocatorType or ArenaAllocatorType is considered as
// allocatator class type.
// StringObjectType is considered as Object type that is created/destroyed
// by the allocator. MemInterfaceType is the one that is used for
// memory allocation/deallocation.
//
// Class Type:
//  Constructor:
//     Return type: None
//     Args (4): Class Type (1), MemInterfaceType (1), Integer types (2)
//
//  AllocateBlock:
//     Return type: StringObjectType
//     Args (1): Class Type (1)
//
//  CommitAllocation:
//     Return type: None
//     Args (2): Class Type (1), StringObjectType(1)
//
//  DestroyObject:
//     Return type: Integer
//     Args (2): Class Type (1), StringObjectType(1)
//
//  GetMemManager:
//     Return type: MemInterfaceType
//     Args (1): Class Type (1)
//
//  Destructor or Reset:
//     Return type: None
//     Args (1): Class Type (1)
//     Heuristic is used to distinguish between Destructor and Reset.
//
bool MemManageTransImpl::categorizeFunctions() {

  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANS,
                  { dbgs() << "   Categorize Interface Functions\n"; });
  auto Cand = getCurrentCandidate();
  StructType *StringObjectType = Cand->getStringObjectType();
  StructType *ReusableArenaAllocatorType =
      Cand->getReusableArenaAllocatorType();
  StructType *ArenaAllocatorType = Cand->getArenaAllocatorType();
  StructType *MemInterfaceType = Cand->getMemInterfaceType();

  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANS, {
    dbgs() << "     StringObjectTy: " << StringObjectType->getName() << "\n";
    dbgs() << "     ReusableArenaAllocatorTy: "
           << ReusableArenaAllocatorType->getName() << "\n";
    dbgs() << "     ArenaAllocatorTy: " << ArenaAllocatorType->getName()
           << "\n";
    dbgs() << "     MemInterfaceType: " << MemInterfaceType->getName() << "\n";
  });

  // Returns true if "F" is called from any interface functions.
  auto IsCalledFromInterfaceFunction = [&](Function *F) -> bool {
    for (auto *U : F->users()) {
      auto *CB = dyn_cast<CallBase>(U);
      if (!CB)
        continue;
      Function *CalledF = CB->getFunction();
      for (auto InterF : Cand->interface_functions())
        if (InterF == CalledF)
          return true;
    }
    return false;
  };

  // Categorize "F" using return type and signature.
  auto CategorizeFunctionUsingSignature =
      [&, IsCalledFromInterfaceFunction](Function *F) -> MemManageFKind {
    bool NoReturn = false;
    bool MemManagerReturn = false;
    bool StrObjReturn = false;
    bool IntReturn = false;
    int32_t MemInterfaceArgs = 0;
    int32_t ClassArgs = 0;
    int32_t IntArgs = 0;
    int32_t StrObjArgs = 0;

    // Analyze return type here.
    Type *RTy = F->getReturnType();
    switch (RTy->getTypeID()) {
    default:
      return UnKnown;

    case Type::VoidTyID:
      NoReturn = true;
      break;

    case Type::PointerTyID: {
      Type *PTy = cast<PointerType>(RTy)->getElementType();
      if (PTy == StringObjectType)
        StrObjReturn = true;
      else if (PTy == MemInterfaceType)
        MemManagerReturn = true;
      else
        return UnKnown;
    } break;

    case Type::IntegerTyID:
      IntReturn = true;
      break;
    }

    // Analyze type of each argument.
    for (auto &Arg : F->args()) {
      Type *ArgTy = Arg.getType();
      switch (ArgTy->getTypeID()) {
      default:
        return UnKnown;

      case Type::PointerTyID: {
        auto *PTy = cast<PointerType>(ArgTy)->getElementType();
        if (PTy == ReusableArenaAllocatorType || PTy == ArenaAllocatorType)
          ClassArgs++;
        else if (PTy == MemInterfaceType)
          MemInterfaceArgs++;
        else if (PTy == StringObjectType)
          StrObjArgs++;
        else
          return UnKnown;
        break;
      }

      case Type::IntegerTyID:
        IntArgs++;
        break;
      }
    }

    // Categorize function based on return and argument types.
    auto ArgsSize = F->arg_size();
    if (NoReturn && MemInterfaceArgs == 1 && ClassArgs == 1 && IntArgs == 2 &&
        ArgsSize == 4)
      return Constructor;
    else if (NoReturn && ClassArgs == 1 && StrObjArgs == 1 && ArgsSize == 2)
      return CommitAllocation;
    else if (IntReturn && ClassArgs == 1 && StrObjArgs == 1 && ArgsSize == 2)
      return DestroyObject;
    else if (StrObjReturn && ClassArgs == 1 && ArgsSize == 1)
      return AllocateBlock;
    else if (MemManagerReturn && ClassArgs == 1 && ArgsSize == 1)
      return GetMemManager;
    else if (NoReturn && ClassArgs == 1 && ArgsSize == 1) {
      // A heuristic is used to distinguish between Reset and Destructor.
      // Destructor is not expected to be called from interface functions.
      if (IsCalledFromInterfaceFunction(F))
        return Reset;
      else
        return Destructor;
    }
    return UnKnown;
  };

  // Detect MemManageFKind for each interface function.
  unsigned NumInterfacefunctions = 0;
  for (auto F : Cand->interface_functions()) {
    NumInterfacefunctions++;
    MemManageFKind FKind = CategorizeFunctionUsingSignature(F);
    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANS, {
      dbgs() << "   " << F->getName() << ": " << FKind << "\n";
    });
    if (FKind == UnKnown) {
      DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANS,
                      { dbgs() << "   Failed: Unknown functionality\n"; });
      return false;
    }
    if (FunctionalityMap[FKind]) {
      DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANS,
                      { dbgs() << "   Failed: Multiple functionality\n"; });
      return false;
    }
    FunctionalityMap[FKind] = F;
  }
  assert(FunctionalityMap.size() == NumInterfacefunctions &&
         "Unexpected functionality");
  return true;
}

// Returns true if "V" represents address of VTable field in ArenaAllocator.
// Ex:
// %i9 = getelementptr %ReusableArenaAllocator, %ReusableArenaAllocator*
// %ThisObj, i64 0, i32 0, i32 0
bool MemManageTransImpl::isVTableAddrInReusableArenaAllocator(Value *V,
                                                              Value *ThisObj) {
  auto *GEP = dyn_cast<GetElementPtrInst>(V);
  if (!GEP)
    return false;
  if (GEP->getNumIndices() != 3 || !GEP->hasAllZeroIndices())
    return false;
  if (!isa<StructType>(GEP->getSourceElementType()))
    return false;
  auto *GEPTy = GEP->getResultElementType();
  if (!isVFTablePointer(GEPTy))
    return false;
  if (ThisObj != GEP->getOperand(0))
    return false;
  Visited.insert(GEP);
  return true;
}

// Returns true if "V" is GetElementPtrInst that is in expected format.
// Updates BaseOp with pointer operand and Idx with last index when
// returns true.
// Ex:
//   %i8 = getelementptr %XalanList, %XalanList* %i5, i64 0, i32 2
bool MemManageTransImpl::getGEPBaseAddrIndex(Value *V, Value **BaseOp,
                                             int32_t *Idx) {
  auto *GEP = dyn_cast<GetElementPtrInst>(V);
  if (!GEP)
    return false;
  if (GEP->getNumIndices() != 2)
    return false;
  if (!isa<StructType>(GEP->getSourceElementType()))
    return false;
  auto GepOp1 = dyn_cast<ConstantInt>(GEP->getOperand(1));
  if (!GepOp1 || !GepOp1->isZeroValue())
    return false;
  auto *ConstVal = dyn_cast<ConstantInt>(GEP->getOperand(2));
  if (!ConstVal)
    return false;
  *Idx = ConstVal->getLimitedValue();
  *BaseOp = GEP->getOperand(0);
  Visited.insert(GEP);
  return true;
}

// Returns true if "V" represents address of ArenaAllocator.
// Type of "This Pointer" for interface functions can be either
// ReusableArenaAllocator or ArenaAllocator.
// This routine handles both cases.
// Ex:
//   foo(ArenaAllocator *Obj) {
//     Obj
//   }
//
//   or
//
//   foo(ReusableArenaAllocator *Obj) {
//     Obj->ArenaAllocator
//   }
//
bool MemManageTransImpl::isArenaAllocatorAddr(Value *V, Value *Obj) {
  auto Cand = getCurrentCandidate();
  // Check if type of "This pointer" is ArenaAllocator.
  // If it is ArenaAllocator, address of ArenaAllocator is "obj".
  Type *ObjTy = nullptr;
  if (auto *PTy = dyn_cast<PointerType>(Obj->getType()))
    ObjTy = PTy->getElementType();
  if (ObjTy == Cand->getArenaAllocatorType()) {
    if (V != Obj)
      return false;
    return true;
  }
  // If type of "This pointer" is ReusableArenaAllocator, address of
  // ArenaAllocator is "Obj->ArenaAllocator".
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getArenaAllocatorObjectIndex())
    return false;
  if (BasePtr != Obj)
    return false;
  return true;
}

// Returns true if "V" represents address of List in ArenaAllocator.
// Ex:
//  foo(ReusableArenaAllocator *Obj) {
//     Obj->ArenaAllocator.List
//  }
bool MemManageTransImpl::isListAddr(Value *V, Value *Obj) {
  auto Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getListObjectIndex())
    return false;
  if (!isArenaAllocatorAddr(BasePtr, Obj))
    return false;
  return true;
}

// Returns true if "V" represents address of blockSize in ArenaAllocator.
// Ex:
//  foo(ReusableArenaAllocator *Obj) {
//     Obj->ArenaAllocator.blockSize
//  }
bool MemManageTransImpl::isAllocatorBlockSizeAddr(Value *V, Value *Obj) {
  auto Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getAllocatorBlockSizeIndex())
    return false;
  if (!isArenaAllocatorAddr(BasePtr, Obj))
    return false;
  return true;
}

// Returns true if "V" represents Load of blockSize in ArenaAllocator.
// Ex:
//  foo(ReusableArenaAllocator *Obj) {
//     Obj->ArenaAllocator.blockSize
//  }
bool MemManageTransImpl::isAllocatorBlockSizeLoad(Value *V, Value *Obj) {
  auto *LI = dyn_cast<LoadInst>(V);
  if (!LI)
    return false;
  Value *LoadAddr = LI->getPointerOperand();
  if (!isAllocatorBlockSizeAddr(LoadAddr, Obj))
    return false;
  Visited.insert(LI);
  return true;
}

// Returns true if "V" represents address of MemManager in List.
// Ex:
//  foo(ReusableArenaAllocator *Obj) {
//     Obj->ArenaAllocator.List.MemManager
//  }
bool MemManageTransImpl::isListMemManagerAddr(Value *V, Value *Obj) {
  auto Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getListMemManagerIndex())
    return false;
  if (!isListAddr(BasePtr, Obj))
    return false;
  return true;
}

// Returns true if "V" represents address of Head in List.
// Ex:
//  foo(ReusableArenaAllocator *Obj) {
//     Obj->ArenaAllocator.List.Head
//  }
bool MemManageTransImpl::isListHeadAddr(Value *V, Value *Obj) {
  auto Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getListHeadIndex())
    return false;
  if (!isListAddr(BasePtr, Obj))
    return false;
  return true;
}

// Returns true if "V" represents Load of Head in List.
// Ex:
//  foo(ReusableArenaAllocator *Obj) {
//     Obj->ArenaAllocator.List.Head
//  }
bool MemManageTransImpl::isListHeadLoad(Value *V, Value *Obj) {
  auto *LI = dyn_cast<LoadInst>(V);
  if (!LI)
    return false;
  if (!isListHeadAddr(LI->getPointerOperand(), Obj))
    return false;
  Visited.insert(LI);
  return true;
}

// Returns true if "V" represents address of FreeHead in List.
// Ex:
//  foo(ReusableArenaAllocator *Obj) {
//     Obj->ArenaAllocator.List.FreeHead
//  }
bool MemManageTransImpl::isListFreeHeadAddr(Value *V, Value *Obj) {
  auto Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getListFreeHeadIndex())
    return false;
  if (!isListAddr(BasePtr, Obj))
    return false;
  return true;
}

// Returns true if "V" represents Load of FreeHead in List.
// Ex:
//  foo(ReusableArenaAllocator *Obj) {
//     Obj->ArenaAllocator.List.FreeHead
//  }
bool MemManageTransImpl::isListFreeHeadLoad(Value *V, Value *Obj) {
  auto *LI = dyn_cast<LoadInst>(V);
  if (!LI)
    return false;
  if (!isListFreeHeadAddr(LI->getPointerOperand(), Obj))
    return false;
  Visited.insert(LI);
  return true;
}

// Returns true if "V" represents Load of Next from FreeHead in List.
// Ex:
//  foo(ReusableArenaAllocator *Obj) {
//     Obj->ArenaAllocator.List.FreeHead->Next
//  }
bool MemManageTransImpl::isListFreeHeadNextLoad(Value *V, Value *Obj) {
  auto *LI = dyn_cast<LoadInst>(V);
  if (!LI)
    return false;
  auto Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(LI->getPointerOperand(), &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getNodeNextIndex())
    return false;
  if (!isListFreeHeadLoad(BasePtr, Obj))
    return false;
  Visited.insert(LI);
  return true;
}

// Returns true if "V" represents Prev field of Node.
// Ex:
//   NodePos->Prev
bool MemManageTransImpl::isNodePosPrev(Value *V, Value *NodePos) {
  auto Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getNodePrevIndex())
    return false;
  if (BasePtr != NodePos)
    return false;
  return true;
}

// Returns true if "V" represents Next field of Node.
// Ex:
//   NodePos->Next
bool MemManageTransImpl::isNodePosNext(Value *V, Value *NodePos) {
  auto Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getNodeNextIndex())
    return false;
  if (BasePtr != NodePos)
    return false;
  return true;
}

// Returns true if "V" represents load of Next field of Node.
// Ex:
//   NodePos->Next
bool MemManageTransImpl::isNodePosNextLoad(Value *V, Value *NodePos) {
  auto *LI = dyn_cast<LoadInst>(V);
  if (!LI)
    return false;
  if (!isNodePosNext(LI->getPointerOperand(), NodePos))
    return false;
  Visited.insert(LI);
  return true;
}

// Returns true if "V" represents load of Prev field of Node.
// Ex:
//   NodePos->Prev
bool MemManageTransImpl::isNodePosPrevLoad(Value *V, Value *NodePos) {
  auto *LI = dyn_cast<LoadInst>(V);
  if (!LI)
    return false;
  if (!isNodePosPrev(LI->getPointerOperand(), NodePos))
    return false;
  Visited.insert(LI);
  return true;
}

// Returns true if "V" represents Prev of Next field of Node.
// Ex:
//   NodePos->Next->Prev
bool MemManageTransImpl::isNodePosNextPrev(Value *V, Value *NodePos) {
  auto Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getNodePrevIndex())
    return false;
  if (!isNodePosNextLoad(BasePtr, NodePos))
    return false;
  return true;
}

// Returns true if "V" represents Next of Prev field of Node.
// Ex:
//   NodePos->Prev->Next
bool MemManageTransImpl::isNodePosPrevNext(Value *V, Value *NodePos) {
  auto Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getNodeNextIndex())
    return false;
  if (!isNodePosPrevLoad(BasePtr, NodePos))
    return false;
  return true;
}

// Returns true if "V" represents ReusableArenaBlock field of Node.
// Ex:
//   NodePos->ReusableArenaBlock
bool MemManageTransImpl::isNodePosReusableArenaBlock(Value *V, Value *NodePos) {
  auto Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getReusableArenaBlockIndex())
    return false;
  if (BasePtr != NodePos)
    return false;
  return true;
}

// Returns true if "V" represents address of destroyBlock in
// ReusableArenaAllocator. Ex:
//  foo(ReusableArenaAllocator *Obj) {
//     Obj->destroyBlock
//  }
bool MemManageTransImpl::isDestroyBlockFlagAddr(Value *V, Value *Obj) {
  auto Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getDestroyBlockFlagIndex())
    return false;
  if (BasePtr != Obj)
    return false;
  return true;
}

// Returns true if "V" is load of ListMemManager address.
// Ex:
//  foo(ReusableArenaAllocator *Obj) {
//     Obj->List.MemoryManager
//  }
//
//  or
//
//  foo(ReusableArenaAllocator *Obj) {
//    GetMemManager(Obj)
//  }
bool MemManageTransImpl::isListMemManagerLoad(Value *V, Value *Obj) {
  // We prove that GetMemManager function just returns "MemoryManager".
  // "GetMemManager(Obj)" call is also treated as "MemoryManager".
  if (auto *CB = dyn_cast<CallBase>(V)) {
    auto *CalledF = dtrans::getCalledFunction(*CB);
    assert(CalledF && "Unexpected indirect call");
    if (FunctionalityMap[GetMemManager] != CalledF)
      return false;
    assert(CB->arg_size() == 1 && "Unexpected arguments");
    if (!isArenaAllocatorAddr(CB->getArgOperand(0), Obj))
      return false;
    Visited.insert(CB);
    return true;
  }
  auto *LI = dyn_cast<LoadInst>(V);
  if (!LI)
    return false;
  Value *LoadAddr = LI->getPointerOperand();
  if (!isListMemManagerAddr(LoadAddr, Obj))
    return false;
  Visited.insert(LI);
  return true;
}

// Returns true if "V" represents address of Next of Head in List.
// Ex:
//  foo(ReusableArenaAllocator *Obj) {
//     Obj->ArenaAllocator.List.Head->Next
//  }
bool MemManageTransImpl::isNodeNextAddr(Value *V, Value *Obj) {
  auto Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getNodeNextIndex())
    return false;
  if (!isListHeadLoad(BasePtr, Obj))
    return false;
  return true;
}

// Returns true if "V" represents Load of Next of Head in List.
// Ex:
//  foo(ReusableArenaAllocator *Obj) {
//     Obj->ArenaAllocator.List.Head->Next
//  }
bool MemManageTransImpl::isListBegin(Value *V, Value *Obj) {
  auto *LI = dyn_cast<LoadInst>(V);
  if (!LI)
    return false;
  if (!isNodeNextAddr(LI->getPointerOperand(), Obj))
    return false;
  Visited.insert(LI);
  return true;
}

// Returns true if "V" represents the pattern below.
//
//  *((ReusableArenaBlock*)Obj->ArenaAllocator.List.Head->Next)
//   or
//  *((ArenaBlock*)Obj->ArenaAllocator.List.Head->Next)
//
bool MemManageTransImpl::isListFrontNodeArenaBlockAddr(Value *V, Value *Obj) {

  auto IsBitCastOkay = [this](Value *Ptr) {
    auto *PTy = dyn_cast<PointerType>(Ptr->getType());
    if (!PTy)
      return false;
    Type *ElemTy = PTy->getElementType();
    auto Cand = getCurrentCandidate();
    if (ElemTy != Cand->getBlockBaseType())
      return false;
    if (Cand->getReusableArenaBlockIndex() != 0 ||
        Cand->getBlockBaseObjIndex() != 0)
      return false;
    return true;
  };

  auto *LI = dyn_cast<LoadInst>(V);
  if (!LI)
    return false;
  Value *Ptr = LI->getPointerOperand();
  auto *BC = dyn_cast<BitCastInst>(Ptr);
  if (!BC)
    return false;
  if (!IsBitCastOkay(LI))
    return false;

  if (!isListBegin(BC->getOperand(0), Obj))
    return false;
  Visited.insert(BC);
  Visited.insert(LI);
  return true;
}

// Returns true if "V" represents the pattern below.
//
//  (((ArenaBlock*)Obj->ArenaAllocator.List.Head->Next))->ObjectCount
bool MemManageTransImpl::isFrontNodeObjectCountAddr(Value *V, Value *Obj) {
  auto Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getBlockObjectCountIndex())
    return false;
  if (!isListFrontNodeArenaBlockAddr(BasePtr, Obj))
    return false;
  return true;
}

// Returns true if "V" represents load of the pattern below.
//
//  (((ArenaBlock*)Obj->ArenaAllocator.List.Head->Next))->ObjectCount
//
bool MemManageTransImpl::isFrontNodeObjectCountLoad(Value *V, Value *Obj) {
  auto *LI = dyn_cast<LoadInst>(V);
  if (!LI)
    return false;
  if (!isFrontNodeObjectCountAddr(LI->getPointerOperand(), Obj))
    return false;
  Visited.insert(LI);
  return true;
}

// Returns true if "V" represents load of the pattern below.
//  (((ArenaBlock*)Obj->ArenaAllocator.List.Head->Next))->BlockSize
//
bool MemManageTransImpl::isFrontNodeBlockSizeLoad(Value *V, Value *Obj) {
  auto *LI = dyn_cast<LoadInst>(V);
  if (!LI)
    return false;
  auto Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(LI->getPointerOperand(), &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getBlockBlockSizeIndex())
    return false;
  if (!isListFrontNodeArenaBlockAddr(BasePtr, Obj))
    return false;
  Visited.insert(LI);
  return true;
}

// Returns true if "V" represents ArenaBlock in ReusableArenaBlock.
// Ex:
//     RABObj->ArenaBlock
bool MemManageTransImpl::isArenaBlockAddrFromRAB(Value *V, Value *RABObj) {
  auto Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getBlockBaseObjIndex())
    return false;
  if (BasePtr != RABObj)
    return false;
  return true;
}

// Returns true if "V" represents address of FirstFreeBlock in
// ReusableArenaBlock. Ex:
//     RABObj->FirstFreeBlock
bool MemManageTransImpl::isFirstFreeBlockAddrFromRAB(Value *V, Value *RABObj) {
  auto Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getFirstFreeBlockIndex())
    return false;
  if (BasePtr != RABObj)
    return false;
  return true;
}

// Returns true if "V" represents load of FirstFreeBlock in ReusableArenaBlock.
// Ex:
//     RABObj->FirstFreeBlock
bool MemManageTransImpl::isFirstFreeBlockLoadFromRAB(Value *V, Value *RABObj) {
  auto *LI = dyn_cast<LoadInst>(V);
  if (!LI)
    return false;
  if (!isFirstFreeBlockAddrFromRAB(LI->getPointerOperand(), RABObj))
    return false;
  Visited.insert(LI);
  return true;
}

// Returns true if "V" represents load of NextFreeBlock in ReusableArenaBlock.
// Ex:
//     RABObj->NextFreeBlock
bool MemManageTransImpl::isNextFreeBlockLoadFromRAB(Value *V, Value *RABObj) {
  auto *LI = dyn_cast<LoadInst>(V);
  if (!LI)
    return false;
  if (!isNextFreeBlockAddrFromRAB(LI->getPointerOperand(), RABObj))
    return false;
  Visited.insert(LI);
  return true;
}

// Returns true if "V" represents address of NextFreeBlock in
// ReusableArenaBlock.
// Ex:
//     RABObj->NextFreeBlock
bool MemManageTransImpl::isNextFreeBlockAddrFromRAB(Value *V, Value *RABObj) {
  auto Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getNextFreeBlockIndex())
    return false;
  if (BasePtr != RABObj)
    return false;
  return true;
}

// Returns true if "V" represents address of BasicAllocator in ArenaBlock.
// Ex:
//     RABObj->ArenaBlock.BasicAllocator
bool MemManageTransImpl::isAllocatorAddrFromRAB(Value *V, Value *RABObj) {
  auto Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getBasicAllocatorIndex())
    return false;
  if (!isArenaBlockAddrFromRAB(BasePtr, RABObj))
    return false;
  return true;
}

// Returns true if "V" represents address of ObjectCount in ArenaBlock.
// Ex:
//     RABObj->ArenaBlock.ObjectCount
bool MemManageTransImpl::isObjectCountAddrFromRAB(Value *V, Value *RABObj) {
  auto Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getBlockObjectCountIndex())
    return false;
  if (!isArenaBlockAddrFromRAB(BasePtr, RABObj))
    return false;
  return true;
}

// Returns true if "V" represents load of ObjectCount in ArenaBlock.
// Ex:
//     RABObj->ArenaBlock.ObjectCount
bool MemManageTransImpl::isObjectCountLoadFromRAB(Value *V, Value *RABObj) {
  auto *LI = dyn_cast<LoadInst>(V);
  if (!LI)
    return false;
  if (!isObjectCountAddrFromRAB(LI->getPointerOperand(), RABObj))
    return false;
  Visited.insert(LI);
  return true;
}

// Returns true if "V" represents load of BlockSize in ArenaBlock.
// Ex:
//     RABObj->ArenaBlock.BlockSize
bool MemManageTransImpl::isBlockSizeLoadFromRAB(Value *V, Value *RABObj) {
  auto *LI = dyn_cast<LoadInst>(V);
  if (!LI)
    return false;
  if (!isBlockSizeAddrFromRAB(LI->getPointerOperand(), RABObj))
    return false;
  Visited.insert(LI);
  return true;
}

// Returns true if "V" represents address of BlockSize in ArenaBlock.
// Ex:
//     RABObj->ArenaBlock.BlockSize
bool MemManageTransImpl::isBlockSizeAddrFromRAB(Value *V, Value *RABObj) {
  auto Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getBlockBlockSizeIndex())
    return false;
  if (!isArenaBlockAddrFromRAB(BasePtr, RABObj))
    return false;
  return true;
}

// Returns true if "V" represents address of ObjectBlock in ArenaBlock.
// Ex:
//     RABObj->ArenaBlock.ObjectBlock
bool MemManageTransImpl::isObjectBlockAddrFromRAB(Value *V, Value *RABObj) {
  auto Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getStringObjectIndex())
    return false;
  if (!isArenaBlockAddrFromRAB(BasePtr, RABObj))
    return false;
  return true;
}

// Returns true if "V" represents load of ObjectBlock in ArenaBlock.
// Ex:
//     RABObj->ArenaBlock.ObjectBlock
bool MemManageTransImpl::isObjectBlockLoadFromRAB(Value *V, Value *RABObj) {
  auto *LI = dyn_cast<LoadInst>(V);
  if (!LI)
    return false;
  if (!isObjectBlockAddrFromRAB(LI->getPointerOperand(), RABObj))
    return false;
  Visited.insert(LI);
  return true;
}

// Returns true if "V" represents address of MemManager in BasicAllocator.
// Ex:
//     RABObj->ArenaBlock.BasicAllocator.MemManager
bool MemManageTransImpl::isAllocatorMemAddrFromRAB(Value *V, Value *RABObj) {
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  // BasicAllocator has only one field.
  if (Idx != 0)
    return false;
  if (!isAllocatorAddrFromRAB(BasePtr, RABObj))
    return false;
  return true;
}

// Returns true if "BB" terminates with conditional BranchInst.
// Updates "LValue", "RValue", "TBlock", "FBlock" and "Predi".
// Ex:
//   %6 = icmp eq %"Node"* %5, null
//   br i1 %6, label %7, label %29
bool MemManageTransImpl::processBBTerminator(BasicBlock *BB, Value **LValue,
                                             Value **RValue,
                                             BasicBlock **TBlock,
                                             BasicBlock **FBlock,
                                             ICmpInst::Predicate *Predi) {
  auto *BI = dyn_cast<BranchInst>(BB->getTerminator());
  if (!BI || !BI->isConditional())
    return false;
  Value *BrCond = BI->getCondition();
  ICmpInst *IC = dyn_cast<ICmpInst>(BrCond);
  if (!IC)
    return false;
  *RValue = IC->getOperand(1);
  *LValue = IC->getOperand(0);
  *TBlock = BI->getSuccessor(0);
  *FBlock = BI->getSuccessor(1);
  *Predi = IC->getPredicate();
  Visited.insert(IC);
  Visited.insert(BI);
  return true;
}

// Returns single successor of "BB" if "BB" terminates with
// unconditional BranchInst.
// Ex:
//   br label %25
BasicBlock *MemManageTransImpl::getSingleSucc(BasicBlock *BB) {
  auto *BI = dyn_cast<BranchInst>(BB->getTerminator());
  if (!BI || BI->isConditional())
    return nullptr;
  Visited.insert(BI);
  return BI->getSuccessor(0);
}

// StoreInst in "BB" are collected in StoreVec.
void MemManageTransImpl::collectStoreInst(
    BasicBlock *BB, SmallVectorImpl<StoreInst *> &StoreVec) {
  for (auto &I : *BB) {
    auto *SI = dyn_cast<StoreInst>(&I);
    if (!SI)
      continue;
    StoreVec.push_back(SI);
  }
}

// Returns first NonDbg instruction (including PHI) in BB.
Instruction *MemManageTransImpl::getFirstNonDbg(BasicBlock *BB) {
  for (Instruction &I : *BB) {
    if (isa<DbgInfoIntrinsic>(I))
      continue;
    return &I;
  }
  return nullptr;
}

// Returns true if "V" looks like below. Updates "AddOp" when returns true.
//  "*AddOp" + 1
bool MemManageTransImpl::isIncrementByOne(Value *V, Value **AddOp) {
  auto *AddI = dyn_cast<Instruction>(V);
  if (!AddI || AddI->getOpcode() != Instruction::Add)
    return false;
  auto *Inc = dyn_cast<ConstantInt>(AddI->getOperand(1));
  if (!Inc || !Inc->isOne())
    return false;
  *AddOp = AddI->getOperand(0);
  Visited.insert(AddI);
  return true;
}

// Returns true if "V" looks like below. Updates "BaseAddr", "IndexAddr" and
// "Idx" when returns true.
// Ex:
// %GEP = getelementptr %"String", %"String"* %BaseAddr, i64 %IndexAddr
// %BC = bitcast %"String"* %GEP to %"NextBlock"*
// %Ptr = getelementptr %"NextBlock", %"NextBlock"* %BC, i64 0, i32 Idx
//
// When this routine is called first time, NextBlockTy is assigned to
// "NextBlock" type. The type should match for all other calls.
bool MemManageTransImpl::isNextBlockFieldAccess(Value *Ptr, Value **BaseAddr,
                                                Value **IndexAddr,
                                                int32_t *Idx) {
  Value *BasePtr = nullptr;
  if (!getGEPBaseAddrIndex(Ptr, &BasePtr, Idx))
    return false;
  auto *GEPTy = cast<GetElementPtrInst>(Ptr)->getSourceElementType();
  auto *StTy = dyn_cast<StructType>(GEPTy);
  if (!StTy)
    return false;

  auto *BC = dyn_cast<BitCastInst>(BasePtr);
  if (!BC)
    return false;

  auto *GEP = dyn_cast<GetElementPtrInst>(BC->getOperand(0));
  if (!GEP || GEP->getNumIndices() != 1)
    return false;

  // Get type of NextBlockTy when this routine is called first time.
  if (!NextBlockTy)
    NextBlockTy = StTy;
  else if (NextBlockTy != StTy)
    return false;
  *BaseAddr = GEP->getPointerOperand();
  *IndexAddr = GEP->getOperand(1);
  Visited.insert(BC);
  Visited.insert(GEP);
  return true;
}

// Returns true if "V" looks like below. Updates "BaseAddr", "IndexAddr" and
// "Idx" when returns true.
// %GEP = getelementptr %"String", %"String"* %BaseAddr, i64 %IndexAddr
// %BC = bitcast %"String"* %GEP to %"NextBlock"*
// %Val = getelementptr %"NextBlock", %"NextBlock"* %BC, i64 0, i32 Idx
// %Ptr = load i16, 16* %Val
//
bool MemManageTransImpl::isNextBlockFieldLoad(Value *Ptr, Value **BaseAddr,
                                              Value **IndexAddr, int32_t *Idx) {
  auto *LI = dyn_cast<LoadInst>(Ptr);
  if (!LI)
    return false;
  if (!isNextBlockFieldAccess(LI->getPointerOperand(), BaseAddr, IndexAddr,
                              Idx))
    return false;
  Visited.insert(LI);
  return true;
}

// Returns true if "BB" has checks related to devirtualization using
// "MemManager" from Obj->List.
// It updates CmpFn, TBlock and FBlock when returns true.
//
// Ex:
// BB:
//  %8 = getelementptr inbounds %List, %List* %3, i64 0, i32 0
//  %9 = load %"MemManager"*, %"MemManager"** %8, align 8
//  %10 = bitcast %"MemManager"* %9 to i8* (%"MemManager"*, i64)***
//  %11 = load i8* (%"MemManager"*, i64)**, i8* (%"MemManager"*, i64)*** %10
//  %12 = bitcast i8* (%"MemManager"*, i64)** %11 to i8*
//  %13 = tail call i1 @llvm.type.test(i8* %12, metadata !"VTable")
//  tail call void @llvm.assume(i1 %13)
//  %14 = getelementptr i8* (%"MemManager"*, i64)*, i8*
//                          (%"MemManager"*, i64)** %11, i64 2
//  %15 = load i8* (%"MemManager"*, i64)*, i8* (%"MemManager"*, i64)** %14
//  %16 = bitcast i8* (%"MemManager"*, i64)* %15 to i8*
//  %17 = bitcast i8* (%"MemManager"*, i64)* @CmpFn to i8*
//  %18 = icmp eq i8* %16, %17
//  br i1 %18, label %19, label %21
bool MemManageTransImpl::identifyDevirtChecks(BasicBlock *BB, Value *Obj,
                                              Function **CmpFn,
                                              BasicBlock **TBlock,
                                              BasicBlock **FBlock) {
  Value *LValue = nullptr;
  Value *RValue = nullptr;
  ICmpInst::Predicate Predi = CmpInst::ICMP_NE;
  if (!processBBTerminator(BB, &LValue, &RValue, TBlock, FBlock, &Predi))
    return false;
  if (Predi != ICmpInst::ICMP_EQ)
    return false;
  // Skip BitCastInst instructions.
  if (auto *BC = dyn_cast<BitCastInst>(RValue)) {
    Visited.insert(BC);
    RValue = BC->getOperand(0);
  }
  if (auto *BC = dyn_cast<BitCastInst>(LValue)) {
    Visited.insert(BC);
    LValue = BC->getOperand(0);
  }
  // Check compare function.
  auto F = dyn_cast<Function>(RValue->stripPointerCasts());
  if (!F)
    return false;
  auto *LI = dyn_cast<LoadInst>(LValue);
  if (!LI)
    return false;
  // Check for VTable load.
  auto *PTy = dyn_cast<PointerType>(LI->getType());
  if (!PTy || !PTy->getElementType()->isFunctionTy())
    return false;
  auto *GEP = dyn_cast<GetElementPtrInst>(LI->getPointerOperand());
  if (!GEP)
    return false;
  if (GEP->getNumIndices() != 1)
    return false;
  Value *V = GEP->getPointerOperand();
  auto *VTLoad = dyn_cast<LoadInst>(V);
  if (!VTLoad)
    return false;
  auto *MemI = VTLoad->getPointerOperand();
  if (auto *BC = dyn_cast<BitCastInst>(MemI)) {
    Visited.insert(BC);
    MemI = BC->getOperand(0);
  }
  // Check it is MemManager from Obj->List.
  if (!isListMemManagerLoad(MemI, Obj))
    return false;
  Visited.insert(VTLoad);
  Visited.insert(LI);
  Visited.insert(GEP);

  // Check for llvm.assume and llvm.type.test calls.
  if (VTLoad->getNumUses() != 2)
    return false;
  for (auto *UI : VTLoad->users()) {
    if (UI == GEP)
      continue;
    Value *UPtr = UI;
    if (auto *BC = dyn_cast<BitCastInst>(UPtr)) {
      Visited.insert(BC);
      if (!BC->hasOneUse())
        return false;
      UPtr = *BC->user_begin();
    }
    auto II = dyn_cast<IntrinsicInst>(UPtr);
    if (!II || II->getIntrinsicID() != Intrinsic::type_test)
      return false;
    if (!UPtr->hasOneUse())
      return false;
    auto III = dyn_cast<IntrinsicInst>(*UPtr->user_begin());
    if (!III || III->getIntrinsicID() != Intrinsic::assume)
      return false;
    Visited.insert(II);
    Visited.insert(III);
  }
  *CmpFn = F;
  return true;
}

// Finds common successor of Call1 and Call2 if it finds one.
// It handles the following two cases.
// For example 1, NSucc is updated with BB4 and USucc with nullptr.
// For example 2, NSucc is updated with BB4 and USucc with BB5.
//
// Ex 1:
//   BB1:
//       call Call1()
//       br label BB3
//
//   BB2:
//       call Call2()
//       br label BB3
//
//   BB3:
//       br label BB4
//
// Ex 2:
//   BB1:
//       invoke Call1()
//       to label BB3 unwind label BB5
//
//   BB2:
//       invoke Call2()
//       to label BB3 unwind label BB5
//
//   BB3:
//       br label BB4
bool MemManageTransImpl::getAllocDeallocCommonSucc(Instruction *Call1,
                                                   Instruction *Call2,
                                                   BasicBlock **NSucc,
                                                   BasicBlock **USucc) {
  assert(Call1->getParent() != Call2->getParent() && "Unexpected calls");
  BasicBlock *TSucc = getSingleSucc(Call1->getParent());
  BasicBlock *FSucc = getSingleSucc(Call2->getParent());
  BasicBlock *Succ = nullptr;
  BasicBlock *UnwindSucc = nullptr;
  if (TSucc && FSucc) {
    if (TSucc != FSucc)
      return false;
    Succ = getSingleSucc(TSucc);
  } else {
    auto *I1 = dyn_cast<InvokeInst>(Call1);
    auto *I2 = dyn_cast<InvokeInst>(Call2);
    if (!I1 || !I2)
      return false;
    Succ = I1->getNormalDest();
    if (Succ != I2->getNormalDest())
      return false;
    UnwindSucc = I1->getUnwindDest();
    if (UnwindSucc != I2->getUnwindDest())
      return false;
    if (!Succ)
      return false;
    Succ = getSingleSucc(Succ);
  }
  if (!Succ)
    return false;
  *NSucc = Succ;
  *USucc = UnwindSucc;
  return true;
}

// Returns true if "BB" has unreachable code.
//
// Ex:
//   %116 = landingpad { i8*, i32 }
//          catch i8* null
//   %117 = extractvalue { i8*, i32 } %116, 0
//   tail call void @__clang_call_terminate(i8* %117)
//   unreachable
bool MemManageTransImpl::isUnreachableOK(BasicBlock *BB) {
  BasicBlock *UnreachBB = BB;
  BasicBlock *Succ = getSingleSucc(BB);
  if (Succ)
    UnreachBB = Succ;
  Instruction *I = UnreachBB->getTerminator();
  if (!isa<UnreachableInst>(I))
    return false;
  auto *CallT = dyn_cast_or_null<CallBase>(I->getPrevNonDebugInstruction());
  if (!CallT)
    return false;
  Function *F = dtrans::getCalledFunction(*CallT);
  LibFunc LibF;
  auto &TLI = GetTLI(*CallT->getFunction());
  if (!F || !TLI.getLibFunc(*F, LibF) || !TLI.has(LibF))
    return false;
  if (LibF != LibFunc_clang_call_terminate)
    return false;

  auto *EVI = dyn_cast<ExtractValueInst>(CallT->getArgOperand(0));
  if (!EVI || EVI->getNumIndices() != 1 || *EVI->idx_begin() != 0)
    return false;
  auto *LPI = dyn_cast<LandingPadInst>(EVI->getOperand(0));
  if (!LPI || LPI->getNumClauses() != 1 || !LPI->isCatch(0))
    return false;
  Visited.insert(I);
  Visited.insert(CallT);
  Visited.insert(EVI);
  Visited.insert(LPI);
  return true;
}

// Returns true if "BB" is just a devirtualized Dealloc call like below.
//
// Ex:
//   BB0:
//       Check_Divirt_code
//       br i1 %C, label %BB1, label %BB2
//
//   BB1:
//       invoke DeallocCall()
//       to label BB3 unwind label BB5
//
//   BB2:
//       invoke DummyDeallocCall()
//       to label BB3 unwind label BB5
//
//   BB3:
//       br label BB4
//
//   BB5:
//      unreachable
bool MemManageTransImpl::identifyDeallocCall(BasicBlock *BB, Value *Obj,
                                             Value *Ptr, BasicBlock **Succ) {
  // Checks that "Obj->List->MemManager" and "Ptr" are passed as
  // arguments to "CB".
  auto IsArgsOkay = [this](CallBase *CB, Value *Obj, Value *Ptr) {
    if (CB->arg_size() != 2)
      return false;
    if (!isListMemManagerLoad(CB->getArgOperand(0), Obj))
      return false;
    if (Ptr != CB->getArgOperand(1))
      return false;
    return true;
  };
  // Returns true if "V" is a Dealloc call.
  auto IsDeallocCall = [this, &IsArgsOkay](Value *V, Value *Obj, Value *Ptr,
                                           Function *CmpF) {
    if (!V)
      return false;
    auto *CB = dyn_cast<CallBase>(V->stripPointerCasts());
    if (!CB)
      return false;
    auto *Info = DTInfo.getCallInfo(CB);
    if (!Info || Info->getCallInfoKind() != dtrans::CallInfo::CIK_Free)
      return false;
    if (!IsArgsOkay(CB, Obj, Ptr))
      return false;
    if (dtrans::getCalledFunction(*CB) != CmpF)
      return false;
    Visited.insert(CB);
    return true;
  };

  // Returns true "V" is a dummy call.
  auto IsDummyDeallocCall = [this, &IsArgsOkay](Value *V, Value *Obj,
                                                Value *Ptr) {
    if (!V)
      return false;
    auto *CB = dyn_cast<CallBase>(V->stripPointerCasts());
    if (!CB)
      return false;
    if (!isDummyFuncWithThisAndPtrArgs(CB, GetTLI(*CB->getFunction())))
      return false;
    if (!IsArgsOkay(CB, Obj, Ptr))
      return false;
    Visited.insert(CB);
    return true;
  };

  Function *CmpFn;
  BasicBlock *TBlock;
  BasicBlock *FBlock;
  if (!identifyDevirtChecks(BB, Obj, &CmpFn, &TBlock, &FBlock))
    return false;
  Instruction *DeCall = TBlock->getFirstNonPHIOrDbg();
  Instruction *DummyCall = FBlock->getFirstNonPHIOrDbg();
  if (!IsDeallocCall(DeCall, Obj, Ptr, CmpFn))
    return false;
  if (!IsDummyDeallocCall(DummyCall, Obj, Ptr))
    return false;
  BasicBlock *NSucc = nullptr;
  BasicBlock *USucc = nullptr;
  if (!getAllocDeallocCommonSucc(DeCall, DummyCall, &NSucc, &USucc))
    return false;
  if (!isUnreachableOK(USucc))
    return false;
  *Succ = NSucc;
  return true;
}

// Returns true if value of "SizeVal" is "SizeC".
bool MemManageTransImpl::checkConstantSize(Value *SizeVal, int64_t SizeC) {
  auto *ConstVal = dyn_cast<ConstantInt>(SizeVal);
  if (!ConstVal)
    return false;
  int64_t ASize = ConstVal->getLimitedValue();
  if (ASize != SizeC)
    return false;
  return true;
}

// Returns true if value of "SizeVal" is "SizeC * BlockSize".
bool MemManageTransImpl::checkSizeValue(Value *SizeVal, int64_t SizeC,
                                        Value *Obj) {
  auto *BO = dyn_cast<BinaryOperator>(SizeVal);
  if (!BO || BO->getOpcode() != Instruction::Mul)
    return false;
  if (!checkConstantSize(BO->getOperand(1), SizeC))
    return false;
  Value *V = BO->getOperand(0);
  if (auto *ZExt = dyn_cast<ZExtInst>(BO->getOperand(0))) {
    V = ZExt->getOperand(0);
    Visited.insert(ZExt);
  }
  if (!isAllocatorBlockSizeLoad(V, Obj))
    return false;
  Visited.insert(cast<Instruction>(BO));
  return true;
}

// Returns true if "BB" is just a devirtualized Alloc call like below.
// When returns true, it updates "NumOfElems" and "AllocPtr".
// It also sets "UnBB" if there is any Unwind targets.
//
// Ex:
//   BB0:
//       Check_Divirt_code
//       br i1 %C, label %BB1, label %BB2
//
//   BB1:
//       %t1 = call AllocCall()
//       br label BB3
//
//   BB2:
//       %t2 = call DummyAllocCall()
//       br label BB3
//
//   BB3:
//       %t3 = phi i8* [ %t1, %BB1 ], [ %t2, %BB2 ]
//       br label BB4
//
bool MemManageTransImpl::identifyAllocCall(BasicBlock *BB, Value *Obj,
                                           Value **AllocPtr, Value **NumOfElems,
                                           BasicBlock **UnBB) {

  // Returns true if it is a alloc call like below. It updates "SizeValue"
  // when returns true.
  //   AllocCall(Obj->List->MemManager, *SizeValue)
  auto IsAllocCall = [this](Value *V, Value *Obj, Value **SizeValue,
                            Function *CmpF) {
    if (!V)
      return false;
    auto *CB = dyn_cast<CallBase>(V->stripPointerCasts());
    if (!CB)
      return false;
    if (CB->arg_size() != 2)
      return false;
    auto *CallInfo = DTInfo.getCallInfo(CB);
    if (!CallInfo)
      return false;
    if (CallInfo->getCallInfoKind() != dtrans::CallInfo::CIK_Alloc)
      return false;
    auto AKind = cast<AllocCallInfo>(CallInfo)->getAllocKind();
    if (AKind != AK_Malloc && AKind != AK_UserMalloc)
      return false;

    if (!isListMemManagerLoad(CB->getArgOperand(0), Obj))
      return false;
    if (CmpF != dtrans::getCalledFunction(*CB))
      return false;
    *SizeValue = CB->getArgOperand(1);
    Visited.insert(CB);
    return true;
  };

  // Returns true if "V" is a dummy alloc call.
  auto IsDummyCall = [this](Value *V, Value *Obj, Value **SizeValue) {
    if (!V)
      return false;
    auto *CB = dyn_cast<CallBase>(V->stripPointerCasts());
    if (!CB)
      return false;
    if (CB->arg_size() != 2)
      return false;
    if (!isDummyFuncWithThisAndIntArgs(CB, GetTLI(*CB->getFunction())))
      return false;
    if (!isListMemManagerLoad(CB->getArgOperand(0), Obj))
      return false;
    *SizeValue = CB->getArgOperand(1);
    Visited.insert(CB);
    return true;
  };

  Function *CmpFn;
  BasicBlock *TBlock;
  BasicBlock *FBlock;
  if (!identifyDevirtChecks(BB, Obj, &CmpFn, &TBlock, &FBlock))
    return false;
  Instruction *ACall = TBlock->getFirstNonPHIOrDbg();
  Instruction *DCall = FBlock->getFirstNonPHIOrDbg();
  Value *ASizeValue = nullptr;
  if (!IsAllocCall(ACall, Obj, &ASizeValue, CmpFn))
    return false;
  Value *DSizeValue = nullptr;
  if (!IsDummyCall(DCall, Obj, &DSizeValue))
    return false;
  if (!ACall->hasOneUse() || !DCall->hasOneUse())
    return false;
  auto *PHI = dyn_cast_or_null<PHINode>(*ACall->user_begin());
  if (!PHI)
    return false;
  Visited.insert(PHI);
  if (*DCall->user_begin() != PHI)
    return false;
  BasicBlock *NSucc = nullptr;
  BasicBlock *USucc = nullptr;
  if (!getAllocDeallocCommonSucc(ACall, DCall, &NSucc, &USucc))
    return false;
  Instruction *PHIUser = NSucc->getFirstNonPHIOrDbg();
  if (!PHIUser)
    return false;
  auto *BC = dyn_cast<BitCastInst>(PHIUser);
  if (!BC || BC->getOperand(0) != PHI)
    return false;
  Visited.insert(BC);
  *UnBB = USucc;
  *AllocPtr = BC;
  *NumOfElems = ASizeValue;
  return true;
}

// Returns true if it identifies the pattern below.
// It updates "TrueB", "FalseB" and "NodePtr" when returns true.
// If "IsListHead" is true, this routine checks if ListHead is
// null. Otherwise, checks if ListFreeHead is null.
//
// if (Obj->List->listHead == nullptr) {
//   Node *ptr = alloc(sizeof(Node))
//   // Successor Block of this statement is considered as "FalseB"
//   // "NodePtr" is updated with "Ptr".
// } else {
//   // This Block is considered as "FalseB"
// }
//
bool MemManageTransImpl::identifyCheckAndAllocNode(BasicBlock *BB, Value *Obj,
                                                   BasicBlock **TrueB,
                                                   BasicBlock **FalseB,
                                                   Value **NodePtr,
                                                   bool IsListHead) {
  Value *LValue;
  Value *RValue;
  BasicBlock *TBlock;
  BasicBlock *FBlock;
  ICmpInst::Predicate Predi;
  if (!processBBTerminator(BB, &LValue, &RValue, &TBlock, &FBlock, &Predi))
    return false;
  if (Predi != ICmpInst::ICMP_EQ)
    return false;
  if (IsListHead) {
    if (!isListHeadLoad(LValue, Obj))
      return false;
  } else {
    if (!isListFreeHeadLoad(LValue, Obj))
      return false;
  }
  if (!isa<Constant>(RValue) || !cast<Constant>(RValue)->isNullValue())
    return false;
  Value *AllocPtr;
  Value *SizeVal;
  BasicBlock *UnBB = nullptr;
  if (!identifyAllocCall(TBlock, Obj, &AllocPtr, &SizeVal, &UnBB) || UnBB)
    return false;
  // Check size of memory allocation is equal to size of Node.
  auto Cand = getCurrentCandidate();
  StructType *NodeType = Cand->getListNodeType();
  assert(NodeType && "Unexpected Node Type");
  int64_t NodeSize = DL.getTypeAllocSize(NodeType);
  if (!checkConstantSize(SizeVal, NodeSize))
    return false;

  assert(isa<BitCastInst>(*AllocPtr) && "Expected BitCastInst");
  BasicBlock *InitBB = cast<BitCastInst>(AllocPtr)->getParent();
  *NodePtr = AllocPtr;
  *FalseB = FBlock;
  // If InitBB has single successor, treat the succssor as TrueB.
  BasicBlock *TBB = getSingleSucc(InitBB);
  if (!TBB)
    *TrueB = InitBB;
  else
    *TrueB = TBB;
  return true;
}

// Returns true if it identifies the pattern below.
// It updates "TrueB", "FalseB" and "NodePtr" when returns true.
//
// if (Obj->List->listHead == nullptr) {
//   Node *ptr = alloc(sizeof(Node))
//   Obj->list->listHead = Ptr;
//   Ptr->Next = Ptr;
//   Ptr->Prev = Ptr;
//   // Successor Block of this statement is considered as "FalseB"
//   // "NodePtr" is updated with "Ptr".
// } else {
//   // This Block is considered as "FalseB"
// }
//
bool MemManageTransImpl::identifyListHead(BasicBlock *BB, Value *Obj,
                                          BasicBlock **TrueB,
                                          BasicBlock **FalseB,
                                          Value **AllocPtr) {
  if (!identifyCheckAndAllocNode(BB, Obj, TrueB, FalseB, AllocPtr,
                                 /* IsListHead */ true))
    return false;

  assert(*AllocPtr && "Expected AllocPtr");
  assert(isa<BitCastInst>(*AllocPtr) && "Expected BitCastInst");
  BasicBlock *InitBB = cast<BitCastInst>(*AllocPtr)->getParent();
  // Check for the assignment statements below.
  //   Obj->list->listHead = Ptr;
  //   Ptr->Next = Ptr;
  //   Ptr->Prev = Ptr;
  unsigned NodePrevAssigned = 0;
  unsigned NodeNextAssigned = 0;
  unsigned NodeAssigned = 0;
  auto Cand = getCurrentCandidate();
  for (auto &I : *InitBB) {
    auto *SI = dyn_cast<StoreInst>(&I);
    if (!SI)
      continue;
    if (SI->getValueOperand() != *AllocPtr)
      return false;
    Value *PtrOp = SI->getPointerOperand();
    Value *BasePtr = nullptr;
    int32_t Idx = 0;
    if (!getGEPBaseAddrIndex(PtrOp, &BasePtr, &Idx))
      return false;
    if (BasePtr == *AllocPtr) {
      if (Idx == Cand->getNodePrevIndex()) {
        NodePrevAssigned++;
      } else if (Idx == Cand->getNodeNextIndex()) {
        NodeNextAssigned++;
      } else {
        return false;
      }
      Visited.insert(cast<Instruction>(PtrOp));
    } else if (isListHeadAddr(PtrOp, Obj)) {
      NodeAssigned++;
    } else {
      return false;
    }
    Visited.insert(SI);
  }
  if (NodePrevAssigned != 1 || NodeNextAssigned != 1 || NodeAssigned != 1)
    return false;
  return true;
}

// Returns true if it identifies the pattern below.
// It updates "EmptyBB" and "NotEmptyBB".
//
// if (Obj->List->listHead == nullptr) {
//   // allocate new Node and initialize it.
// } else {
//   // EmptyCheckBB
//   if ((begin() == end()) != 0) { // Check List is empty.
//     // EmptyBB
//   } else {
//     // NotEmptyBB
//   }
// }
bool MemManageTransImpl::identifyListEmpty(BasicBlock *BB, Value *Obj,
                                           BasicBlock **EmptyBB,
                                           BasicBlock **NotEmptyBB) {
  BasicBlock *CreatedHeadBB = nullptr;
  BasicBlock *EmptyCheckBB = nullptr;
  Value *NodePtr = nullptr;
  if (!identifyListHead(BB, Obj, &CreatedHeadBB, &EmptyCheckBB, &NodePtr))
    return false;
  Value *LValue = nullptr;
  Value *RValue = nullptr;
  ICmpInst::Predicate Predi = ICmpInst::ICMP_NE;
  if (!processBBTerminator(EmptyCheckBB, &LValue, &RValue, EmptyBB, NotEmptyBB,
                           &Predi))
    return false;
  if (Predi != ICmpInst::ICMP_EQ)
    return false;
  if (!isListHeadLoad(RValue, Obj))
    return false;
  if (!isListBegin(LValue, Obj))
    return false;
  if (*EmptyBB != CreatedHeadBB)
    return false;
  return true;
}

// Returns true if it identifies the pattern below.
// It updates "EmptyBB" and "NotEmptyBB".
//
// if (Obj->List->listHead->RAB.ArenaBlock.ObjectCount <
//     Obj->List->listHead->RAB.ArenaBlock.BlockSize) {
//     // AvailableBB
// } else {
//     // NotAvailableBB
// }
bool MemManageTransImpl::identifyBlockAvailable(BasicBlock *BB, Value *Obj,
                                                BasicBlock **AvailableBB,
                                                BasicBlock **NotAvailableBB) {
  Value *LValue = nullptr;
  Value *RValue = nullptr;
  ICmpInst::Predicate Predi = ICmpInst::ICMP_NE;
  if (!processBBTerminator(BB, &LValue, &RValue, AvailableBB, NotAvailableBB,
                           &Predi))
    return false;
  if (Predi != ICmpInst::ICMP_ULT)
    return false;
  if (!isFrontNodeBlockSizeLoad(RValue, Obj))
    return false;
  if (!isFrontNodeObjectCountLoad(LValue, Obj))
    return false;
  return true;
}

// Returns true if "F" is recognized as "GetMemManager".
// Ex:
//   MemManager *getMemManager(ArenaAllocatorTy *Obj) {
//     return Obj->List.MemoryManager;
//   }
bool MemManageTransImpl::recognizeGetMemManager(Function *F) {
  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANS, {
    dbgs() << "   Recognizing GetMemManager Functionality " << F->getName()
           << "\n";
    ;
  });
  if (F->size() != 1)
    return false;
  assert(F->arg_size() == 1 && "Unexpected arguments");
  auto *Ret = dyn_cast<ReturnInst>(F->getEntryBlock().getTerminator());
  assert(Ret && "Expected Ret instruction");
  Value *RetVal = Ret->getReturnValue();
  assert(RetVal && "Unexpected Return Value");
  Visited.clear();
  Argument *ThisObj = &*F->arg_begin();
  if (!isListMemManagerLoad(RetVal, ThisObj))
    return false;
  Visited.insert(Ret);
  if (!verifyAllInstsProcessed(F))
    return false;
  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANS, {
    dbgs() << "   Recognized GetMemManager: " << F->getName() << "\n";
    ;
  });
  return true;
}

// Returns true if "F" is recognized as "Constructor".
// Ex:
//  Ctor(ReusableArenaAllocator *Obj, MemManager *m, i16 bSize, i1 flag) {
//    Obj->ArenaAllocator.vtable = some_value;
//    Obj->ArenaAllocator.blockSize = bSize;
//    Obj->ArenaAllocator.List.MemManager = m;
//    Obj->ArenaAllocator.List.Head = null;
//    Obj->ArenaAllocator.List.FreeHead = null;
//    Obj->destroyBlock = 0;
//    return;
//  }
bool MemManageTransImpl::recognizeConstructor(Function *F) {
  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANS, {
    dbgs() << "   Recognizing Constructor Functionality " << F->getName()
           << "\n";
    ;
  });
  if (F->size() != 1)
    return false;
  assert(F->arg_size() == 4 && "Unexpected arguments");
  auto Cand = getCurrentCandidate();
  Argument *ThisObj = &*F->arg_begin();
  SmallPtrSet<StoreInst *, 8> SIList;
  Visited.clear();
  for (auto &I : instructions(F))
    if (auto *SI = dyn_cast<StoreInst>(&I))
      SIList.insert(SI);

  StructType *MemInterfaceType = Cand->getMemInterfaceType();
  StructType *ListNodeType = Cand->getListNodeType();
  unsigned BlockSizeAssigned = 0;
  unsigned DestroyObjFlagAssigned = 0;
  unsigned VTableAssigned = 0;
  unsigned MemManagerAssigned = 0;
  unsigned ListHeadAssigned = 0;
  unsigned ListFreeHeadAssigned = 0;
  for (auto *SI : SIList) {
    Visited.insert(SI);
    Value *PtrOp = SI->getPointerOperand();
    Value *ValOp = SI->getValueOperand();
    auto *SITy = ValOp->getType();
    if (SITy->isPointerTy()) {
      auto *ETy = cast<PointerType>(SITy)->getElementType();
      if (ETy == MemInterfaceType) {
        // Check "Obj->ArenaAllocator.List.MemManager = m;"
        if (!isListMemManagerAddr(PtrOp, ThisObj))
          return false;
        if (!isa<Argument>(ValOp))
          return false;
        MemManagerAssigned++;
      } else if (ETy == ListNodeType) {
        // Checking for the below:
        // Obj->ArenaAllocator.List.Head = null;
        // Obj->ArenaAllocator.List.FreeHead = null;
        if (!isa<Constant>(ValOp) || !cast<Constant>(ValOp)->isNullValue())
          return false;
        if (!isListHeadAddr(PtrOp, ThisObj))
          ListHeadAssigned++;
        else if (!isListFreeHeadAddr(PtrOp, ThisObj))
          ListFreeHeadAssigned++;
        else
          return false;
      } else if (isVFTablePointer(SITy)) {
        // Check "Obj->ArenaAllocator.vtable = some_value;"
        if (!isVTableAddrInReusableArenaAllocator(PtrOp, ThisObj))
          return false;
        VTableAssigned++;
      } else {
        return false;
      }
    } else if (SITy->isIntegerTy(8)) {
      // Check "Obj->destroyBlock = 0;"
      if (!isDestroyBlockFlagAddr(PtrOp, ThisObj))
        return false;
      if (!isa<ConstantInt>(ValOp) || !cast<ConstantInt>(ValOp)->isZeroValue())
        return false;
      DestroyObjFlagAssigned++;
    } else if (SITy->isIntegerTy(16)) {
      // Check "Obj->ArenaAllocator.blockSize = bSize;"
      if (!isa<Argument>(ValOp))
        return false;
      if (!isAllocatorBlockSizeAddr(PtrOp, ThisObj))
        return false;
      BlockSizeAssigned++;
      BlockSizeStoreInst = SI;
    } else {
      return false;
    }
  }
  if (DestroyObjFlagAssigned != 1 || BlockSizeAssigned != 1 ||
      MemManagerAssigned != 1 || VTableAssigned != 1 || ListHeadAssigned != 1 ||
      ListFreeHeadAssigned != 1)
    return false;
  if (!verifyAllInstsProcessed(F))
    return false;
  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANS, {
    dbgs() << "   Recognized Constructor: " << F->getName() << "\n";
    ;
  });
  return true;
}

// Returns true if "F" is recognized as "AllocateBlock".
//
// Ex:
//  AllocateBlock(ReusableArenaAllocator *Obj) {
//    if(Obj->m_blocks.empty()
//       || !Obj->m_blocks.front()->blockAvailable()) {
//      Obj->m_blocks.push_front(
//       ReusableArenaBlockType::create(Obj->getMemoryManager(),
//                                      Obj->m_blockSize));
//    }
//    return Obj->m_blocks.front()->allocateBlock();
//  }
bool MemManageTransImpl::recognizeAllocateBlock(Function *F) {
  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANS, {
    dbgs() << "   Recognizing AllocateBlock Functionality " << F->getName()
           << "\n";
  });
  Visited.clear();
  Argument *ThisObj = &*F->arg_begin();
  BasicBlock *CreateNewHeadBB = nullptr;
  BasicBlock *NotEmptyBB = nullptr;
  if (!identifyListEmpty(&F->getEntryBlock(), ThisObj, &CreateNewHeadBB,
                         &NotEmptyBB))
    return false;
  BasicBlock *BlockAvailableBB = nullptr;
  BasicBlock *NoBlockAvailableBB = nullptr;
  if (!identifyBlockAvailable(NotEmptyBB, ThisObj, &BlockAvailableBB,
                              &NoBlockAvailableBB))
    return false;
  if (CreateNewHeadBB != NoBlockAvailableBB)
    return false;
  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANS, {
    dbgs() << "   Recognized AllocateBlock partially: " << F->getName() << "\n";
  });
  return true;
}

// Check functionality of each interface function to prove that
// it is an allocator class.
bool MemManageTransImpl::recognizeFunctions(void) {

  // Check functionality of GetMemManager.
  Function *GetMemF = FunctionalityMap[GetMemManager];
  if (!GetMemF)
    return false;
  if (!recognizeGetMemManager(GetMemF))
    return false;

  // Check functionality of Constructor.
  Function *Ctor = FunctionalityMap[Constructor];
  if (!Ctor)
    return false;
  if (!recognizeConstructor(Ctor))
    return false;

  // Check functionality of AllocateBlock.
  Function *AllocBlock = FunctionalityMap[AllocateBlock];
  if (!AllocBlock)
    return false;
  if (!recognizeAllocateBlock(AllocBlock))
    return false;

  return false;
}

// Return false if any unvisited instruction is noticed in "F".
bool MemManageTransImpl::verifyAllInstsProcessed(Function *F) {
  for (auto &I : instructions(F)) {
    if (Visited.count(&I))
      continue;
    // Ignore ReturnInst with no operands.
    if (isa<ReturnInst>(&I) && I.getNumOperands() == 0)
      continue;

    if (isa<DbgInfoIntrinsic>(&I))
      continue;
    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANS, {
      dbgs() << "   Failed: Missed processing some instructions\n"
             << "   " << I << "\n";
    });
    // Return false if any instruction is not found in "visited".
    return false;
  }
  return true;
}

bool MemManageTransImpl::run(void) {
  // Collect candidates.
  if (!gatherCandidates())
    return false;

  // Check legality issues.
  if (!analyzeCandidates())
    return false;

  // Categorize functions based on signatures.
  if (!categorizeFunctions())
    return false;

  // Recognize functionality of each interface function.
  if (!recognizeFunctions()) {
    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANS, {
      dbgs() << "   Failed: Recognizing functionality\n";
      ;
    });
    return false;
  }

  return true;
}

bool MemManageTransPass::runImpl(Module &M, DTransAnalysisInfo &DTInfo,
                                 WholeProgramInfo &WPInfo, MemTLITy GetTLI) {

  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX2;
  if (!WPInfo.isWholeProgramSafe() || !WPInfo.isAdvancedOptEnabled(TTIAVX2))
    return false;

  if (!DTInfo.useDTransAnalysis())
    return false;

  MemManageTransImpl MemManageTransI(M, DTInfo, GetTLI, M.getDataLayout());
  return MemManageTransI.run();
}

PreservedAnalyses MemManageTransPass::run(Module &M,
                                          ModuleAnalysisManager &AM) {
  auto &DTransInfo = AM.getResult<DTransAnalysis>(M);
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function *>(&F)));
  };

  if (!runImpl(M, DTransInfo, WPInfo, GetTLI))
    return PreservedAnalyses::all();

  // TODO: We could add more preserved analyses.
  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  PA.preserve<DTransAnalysis>();
  return PA;
}

} // namespace dtrans

} // namespace llvm
