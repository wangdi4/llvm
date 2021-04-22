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

  // This constant value is used to check if entry in a Block is
  // occupied or not
  constexpr static uint32_t ValidObjStamp = 0xffddffdd;

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
  bool recognizeCommitAllocation(Function *);
  bool recognizeDestructor(Function *F);
  bool recognizeReset(Function *F);
  bool checkConstantSize(Value *, int64_t);
  bool checkSizeValue(Value *, int64_t, Value *);
  bool processBBTerminator(BasicBlock *, Value **, Value **, BasicBlock **,
                           BasicBlock **, ICmpInst::Predicate *);
  BasicBlock *getSingleSucc(BasicBlock *);
  void collectStoreInst(BasicBlock *, SmallVectorImpl<StoreInst *> &);
  Instruction *getFirstNonDbg(BasicBlock *);
  LoadInst *getFirstLoadInst(BasicBlock *);
  bool checkInstructionInBlock(Value *, BasicBlock *);
  bool isUnreachableOK(BasicBlock *);
  bool isIncrementByOne(Value *, Value **);
  bool isNextBlockFieldAccess(Value *, Value **, Value **, int32_t *);
  bool isNextBlockFieldLoad(Value *, Value **, Value **, int32_t *);
  bool getAllocDeallocCommonSucc(Instruction *, Instruction *, BasicBlock **,
                                 BasicBlock **);
  bool identifyDevirtChecks(BasicBlock *, Value *, Function **, BasicBlock **,
                            BasicBlock **, Value *);
  bool identifyAllocCall(BasicBlock *, Value *, Value **, Value **,
                         BasicBlock **);
  bool identifyDeallocCall(BasicBlock *, Value *, Value *, BasicBlock **,
                           Value *);
  bool identifyCheckAndAllocNode(BasicBlock *, Value *, BasicBlock **,
                                 BasicBlock **, Value **, bool);

  bool identifyNodeInit(BasicBlock *, Value *, Value *);
  bool identifyListHead(BasicBlock *, Value *, BasicBlock **, BasicBlock **,
                        Value **);
  bool identifyGetListHead(BasicBlock *, Value *, BasicBlock **, Value **);
  bool identifyListEmpty(BasicBlock *, Value *, BasicBlock **, BasicBlock **);
  bool identifyBlockAvailable(BasicBlock *, Value *, BasicBlock **,
                              BasicBlock **, Value *);
  bool identifyCreate(BasicBlock *, Value *, BasicBlock **, Value **);
  bool identifyArenaBlockInit(BasicBlock *, Value *, Value *, Value *,
                              BasicBlock **);
  bool identifyRABAllocateBlock(BasicBlock *, Value *);
  bool identifyPushFront(BasicBlock *, Value *, Value *, BasicBlock *);
  bool identifyPushAtPos(BasicBlock *, Value *, Value *, Value *, Value *,
                         Value *);
  bool identifyBlockCommit(BasicBlock *, Value *, BasicBlock **, Value **);
  bool prepareForPopFront(BasicBlock *, Value *, BasicBlock **, Value **,
                          Value **, Value **);
  bool identifyFreeNode(BasicBlock *, Value *, Value *, Value *, Value *,
                        Value **);
  bool identifyPopFront(BasicBlock *, Value *, Value *, Value *, Value *,
                        Value **, Value **, Value **);
  bool identifyPushBack(BasicBlock *, Value *, Value *, Value *, Value *,
                        Value *, BasicBlock **);
  bool identifyResetCall(BasicBlock *, Value *, BasicBlock **, BasicBlock **);
  bool identifyListHeadPHINode(BasicBlock *, Value *, Value *, BasicBlock **,
                               Value **);
  bool identifyDestroyNodes(BasicBlock *, Value *, PHINode *, PHINode *,
                            BasicBlock **);
  bool identifyListHeadListHeadNext(BasicBlock *, Value *, BasicBlock **,
                                    BasicBlock **, PHINode **, PHINode **);
  bool identifyDestroyFreeNodes(BasicBlock *, Value *, BasicBlock **);
  bool identifyListDtor(BasicBlock *, Value *, bool);
  bool identifyCreateListHead(BasicBlock *, Value *, Value *, BasicBlock **,
                              BasicBlock **);
  bool identifyFreeNodeInLoop(BasicBlock *, Value *, Value *, BasicBlock **);
  bool identifyStrObjDtorCall(Instruction *, Value *, Value *);
  bool identifyRemoveStrObj(BasicBlock *, Value *, Value *, Value *, Value *,
                            Value **, Value **, Value **);
  bool identifyRABDtorInnerLoop(BasicBlock *, BasicBlock *, Value *, Value *,
                                Value *, BasicBlock **);
  bool identifyRABDtorOuterLoop(BasicBlock *, Value *, Value *, BasicBlock **);
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
  bool isListBeginNext(Value *V, Value *Obj);
  bool isListBeginPrev(Value *V, Value *Obj);
  bool isListFrontNodeArenaBlockAddr(Value *V, Value *Obj, Value *NodePtr);
  bool isFrontNodeObjectCountAddr(Value *V, Value *Obj, Value *NodePtr);
  bool isFrontNodeObjectCountLoad(Value *V, Value *Obj, Value *NodePtr);
  bool isFrontNodeBlockSizeLoad(Value *V, Value *Obj, Value *NodePtr);
  bool isNodePosNextPrev(Value *V, Value *NodePos);
  bool isNodePosPrevNext(Value *V, Value *NodePos);
  bool isNodePosPrevLoad(Value *V, Value *NodePos);
  bool isNodePosNextLoad(Value *V, Value *NodePos);
  bool isNodePosNext(Value *V, Value *NodePos);
  bool isNodePosPrev(Value *V, Value *NodePos);
  bool isNodePosReusableArenaBlockAddr(Value *V, Value *NodePos);
  bool isNodePosReusableArenaBlockLoad(Value *V, Value *NodePos);
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
  bool isAllocatorMemLoadFromRAB(Value *V, Value *Obj);
  bool isVTableAddrInReusableArenaAllocator(Value *V, Value *ThisObj);
  bool isVTableAddrInArenaAllocator(Value *V, Value *ThisObj);
  bool isFirstFreeBlockAddrFromNode(Value *V, Value *NodePtr);
  bool isFirstFreeBlockLoadFromNode(Value *V, Value *NodePtr);
  bool isNextFreeBlockAddrFromNode(Value *V, Value *NodePtr);
  bool isNextFreeBlockLoadFromNode(Value *V, Value *NodePtr);
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

// Returns true if "V" represents VTable in ArenaAllocator.
// VTable is always expected to be the first field.
// Ex:
//  foo(ReusableArenaAllocator *Obj) {
//     Obj->ArenaAllocator.VTable
//  }
bool MemManageTransImpl::isVTableAddrInArenaAllocator(Value *V, Value *Obj) {
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != 0)
    return false;
  if (!isArenaAllocatorAddr(BasePtr, Obj))
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

// Returns true if "V" represents address of ReusableArenaBlock field of Node.
// Ex:
//   NodePos->ReusableArenaBlock
bool MemManageTransImpl::isNodePosReusableArenaBlockAddr(Value *V,
                                                         Value *NodePos) {
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

// Returns true if "V" represents load of ReusableArenaBlock field of "NodePos".
// Ex:
//   NodePos->ReusableArenaBlock
bool MemManageTransImpl::isNodePosReusableArenaBlockLoad(Value *V,
                                                         Value *NodePos) {
  auto *LI = dyn_cast<LoadInst>(V);
  if (!LI)
    return false;
  if (!isNodePosReusableArenaBlockAddr(LI->getPointerOperand(), NodePos))
    return false;
  Visited.insert(LI);
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

// Returns true if "V" represents Next of Next of Head in List.
// Ex:
//  foo(ReusableArenaAllocator *Obj) {
//     Obj->ArenaAllocator.List.Head->Next->Next
//  }
bool MemManageTransImpl::isListBeginNext(Value *V, Value *Obj) {
  auto Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getNodeNextIndex())
    return false;
  if (!isListBegin(BasePtr, Obj))
    return false;
  return true;
}

// Returns true if "V" represents Prev of Next of Head in List.
// Ex:
//  foo(ReusableArenaAllocator *Obj) {
//     Obj->ArenaAllocator.List.Head->Next->Prev
//  }
bool MemManageTransImpl::isListBeginPrev(Value *V, Value *Obj) {
  auto Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getNodePrevIndex())
    return false;
  if (!isListBegin(BasePtr, Obj))
    return false;
  return true;
}

// Returns true if "V" represents the pattern below.
//
//  *((ArenaBlock*)Obj->ArenaAllocator.List.Head->Next)
//
// When NodePtr is not null, it checks if "V" represents the pattern below.
//
//  *((ArenaBlock*)NodePtr)
//
bool MemManageTransImpl::isListFrontNodeArenaBlockAddr(
    Value *V, Value *Obj, Value *NodePtr = nullptr) {

  auto IsBitCastOkay = [this](Value *Ptr) {
    auto *PTy = dyn_cast<PointerType>(Ptr->getType());
    if (!PTy)
      return false;
    Type *ElemTy = PTy->getElementType();
    auto Cand = getCurrentCandidate();
    // Makes sure ElemTy is ArenaBlockBase.
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

  Value *Op1 = BC->getOperand(0);

  if (NodePtr) {
    if (Op1 != NodePtr)
      return false;
  } else {
    if (!isListBegin(Op1, Obj))
      return false;
  }

  Visited.insert(BC);
  Visited.insert(LI);
  return true;
}

// Returns true if "V" represents the pattern below.
//
//  (((ArenaBlock*)Obj->ArenaAllocator.List.Head->Next))->ObjectCount
//
// When NodePtr is non-null, it checks if "V" represents the pattern below.
//  (((ArenaBlock*)NodePtr))->ObjectCount
//
bool MemManageTransImpl::isFrontNodeObjectCountAddr(Value *V, Value *Obj,
                                                    Value *NodePtr = nullptr) {
  auto Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getBlockObjectCountIndex())
    return false;
  if (!isListFrontNodeArenaBlockAddr(BasePtr, Obj, NodePtr))
    return false;
  return true;
}

// Returns true if "V" represents load of the pattern below.
//
//  (((ArenaBlock*)Obj->ArenaAllocator.List.Head->Next))->ObjectCount
//
// When NodePtr is non-null, it checks if "V" represents load of the pattern
// below.
//  (((ArenaBlock*)NodePtr))->ObjectCount
//
//
bool MemManageTransImpl::isFrontNodeObjectCountLoad(Value *V, Value *Obj,
                                                    Value *NodePtr = nullptr) {
  auto *LI = dyn_cast<LoadInst>(V);
  if (!LI)
    return false;
  if (!isFrontNodeObjectCountAddr(LI->getPointerOperand(), Obj, NodePtr))
    return false;
  Visited.insert(LI);
  return true;
}

// Returns true if "V" represents load of the pattern below.
//  (((ArenaBlock*)Obj->ArenaAllocator.List.Head->Next))->BlockSize
//
// When NodePtr is non-null, it checks if "V" represents load of the pattern
// below.
//  (((ArenaBlock*)NodePtr))->BlockSize
//
bool MemManageTransImpl::isFrontNodeBlockSizeLoad(Value *V, Value *Obj,
                                                  Value *NodePtr = nullptr) {
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
  if (!isListFrontNodeArenaBlockAddr(BasePtr, Obj, NodePtr))
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

// Returns true if "V" represents load of MemManager in BasicAllocator.
// Ex:
//     RABObj->ArenaBlock.BasicAllocator.MemManager
bool MemManageTransImpl::isAllocatorMemLoadFromRAB(Value *V, Value *RABObj) {
  auto *LI = dyn_cast<LoadInst>(V);
  if (!LI)
    return false;
  if (!isAllocatorMemAddrFromRAB(LI->getPointerOperand(), RABObj))
    return false;
  Visited.insert(LI);
  return true;
}

// Returns true if "V" represents address of FirstFreeBlock of
// ReusableArenaBlock in "NodePtr".
// Ex:
//     NodePtr->ReusableArenaBlock->FirstFreeBlock
bool MemManageTransImpl::isFirstFreeBlockAddrFromNode(Value *V,
                                                      Value *NodePtr) {
  auto Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getFirstFreeBlockIndex())
    return false;
  if (!isNodePosReusableArenaBlockLoad(BasePtr, NodePtr))
    return false;
  return true;
}

// Returns true if "V" represents address of NextFreeBlock of
// ReusableArenaBlock in "NodePtr".
// Ex:
//     NodePtr->ReusableArenaBlock->NextFreeBlock
bool MemManageTransImpl::isNextFreeBlockAddrFromNode(Value *V, Value *NodePtr) {
  auto Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getNextFreeBlockIndex())
    return false;
  if (!isNodePosReusableArenaBlockLoad(BasePtr, NodePtr))
    return false;
  return true;
}

// Returns true if "V" represents load of FirstFreeBlock of
// ReusableArenaBlock in "NodePtr".
// Ex:
//     NodePtr->ReusableArenaBlock->FirstFreeBlock
bool MemManageTransImpl::isFirstFreeBlockLoadFromNode(Value *V,
                                                      Value *NodePtr) {
  auto *LI = dyn_cast<LoadInst>(V);
  if (!LI)
    return false;
  if (!isFirstFreeBlockAddrFromNode(LI->getPointerOperand(), NodePtr))
    return false;
  Visited.insert(LI);
  return true;
}

// Returns true if "V" represents load of NextFreeBlock of
// ReusableArenaBlock in "NodePtr".
// Ex:
//     NodePtr->ReusableArenaBlock->NextFreeBlock
bool MemManageTransImpl::isNextFreeBlockLoadFromNode(Value *V, Value *NodePtr) {
  auto *LI = dyn_cast<LoadInst>(V);
  if (!LI)
    return false;
  if (!isNextFreeBlockAddrFromNode(LI->getPointerOperand(), NodePtr))
    return false;
  Visited.insert(LI);
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

// Returns true if "V" is an instruction in "BB".
bool MemManageTransImpl::checkInstructionInBlock(Value *V, BasicBlock *BB) {
  auto *I = dyn_cast<Instruction>(V);
  if (!I || I->getParent() != BB)
    return false;
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

// Returns first LoadInst in BB if there is one.
LoadInst *MemManageTransImpl::getFirstLoadInst(BasicBlock *BB) {
  LoadInst *BBFirstLI = nullptr;
  for (auto &I : *BB) {
    BBFirstLI = dyn_cast<LoadInst>(&I);
    if (BBFirstLI)
      break;
  }
  return BBFirstLI;
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
//
// If RABPtr is not nullptr, checks that MemManager is loaded from
// ReusableArenaBlock instead of Obj->List.
//
bool MemManageTransImpl::identifyDevirtChecks(BasicBlock *BB, Value *Obj,
                                              Function **CmpFn,
                                              BasicBlock **TBlock,
                                              BasicBlock **FBlock,
                                              Value *RABPtr = nullptr) {
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
  if (RABPtr) {
    if (!isAllocatorMemLoadFromRAB(MemI, RABPtr))
      return false;
  } else {
    // Check it is MemManager from Obj->List.
    if (!isListMemManagerLoad(MemI, Obj))
      return false;
  }
  Visited.insert(VTLoad);
  Visited.insert(LI);
  Visited.insert(GEP);

  // Check for llvm.assume and llvm.type.test calls in BB.
  //  %12 = bitcast i8* (%"MemManager"*, i64)** %VTLoad to i8*
  //  %13 = tail call i1 @llvm.type.test(i8* %12, metadata !"VTable")
  //  tail call void @llvm.assume(i1 %13)
  IntrinsicInst *TestI = nullptr;
  for (auto &I : *BB) {
    if (isa<DbgInfoIntrinsic>(&I))
      continue;
    TestI = dyn_cast<IntrinsicInst>(&I);
    if (TestI)
      break;
  }
  if (!TestI || TestI->getIntrinsicID() != Intrinsic::type_test)
    return false;
  if (!TestI->hasOneUse())
    return false;
  auto *AssumeI = dyn_cast<IntrinsicInst>(*TestI->user_begin());
  if (!AssumeI || AssumeI->getIntrinsicID() != Intrinsic::assume)
    return false;
  auto *BC = dyn_cast<BitCastInst>(TestI->getArgOperand(0));
  if (!BC || BC->getOperand(0) != VTLoad)
    return false;
  Visited.insert(BC);
  Visited.insert(TestI);
  Visited.insert(AssumeI);

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
//
//   or
//
// BB:
//   %LP1 = landingpad { i8*, i32 }
//          catch i8* null
//   br label UnreachBB
//
// UnreachBB:
//   %116 = phi { i8*, i32 } [ %LP1, %BB ], ...
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
  Value *Op1 = EVI->getOperand(0);
  if (auto *PN = dyn_cast<PHINode>(Op1)) {
    if (UnreachBB == BB)
      return false;
    Op1 = PN->getIncomingValueForBlock(BB);
    Visited.insert(PN);
  }
  auto *LPI = dyn_cast<LandingPadInst>(Op1);
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
//
// If RABPtr is not nullptr, checks that MemManager is loaded from
// ReusableArenaBlock instead of Obj->List.
//
bool MemManageTransImpl::identifyDeallocCall(BasicBlock *BB, Value *Obj,
                                             Value *Ptr, BasicBlock **Succ,
                                             Value *RABPtr = nullptr) {
  // Checks that "Obj->List->MemManager" and "Ptr" are passed as
  // arguments to "CB".
  auto IsArgsOkay = [this](CallBase *CB, Value *Obj, Value *Ptr,
                           Value *RABPtr) {
    if (CB->arg_size() != 2)
      return false;
    if (RABPtr) {
      if (!isAllocatorMemLoadFromRAB(CB->getArgOperand(0), RABPtr))
        return false;
    } else {
      if (!isListMemManagerLoad(CB->getArgOperand(0), Obj))
        return false;
    }
    // Check if Ptr is passed after Bitcast.
    auto *BC = dyn_cast<BitCastInst>(CB->getArgOperand(1));
    if (BC && BC->getOperand(0) == Ptr) {
      Visited.insert(BC);
      return true;
    }
    if (Ptr != CB->getArgOperand(1))
      return false;
    return true;
  };
  // Returns true if "V" is a Dealloc call.
  auto IsDeallocCall = [this, &IsArgsOkay](Value *V, Value *Obj, Value *Ptr,
                                           Function *CmpF, Value *RABPtr) {
    if (!V)
      return false;
    auto *CB = dyn_cast<CallBase>(V->stripPointerCasts());
    if (!CB)
      return false;
    auto *Info = DTInfo.getCallInfo(CB);
    if (!Info || Info->getCallInfoKind() != dtrans::CallInfo::CIK_Free)
      return false;
    if (!IsArgsOkay(CB, Obj, Ptr, RABPtr))
      return false;
    if (dtrans::getCalledFunction(*CB) != CmpF)
      return false;
    Visited.insert(CB);
    return true;
  };

  // Returns true "V" is a dummy call.
  auto IsDummyDeallocCall = [this, &IsArgsOkay](Value *V, Value *Obj,
                                                Value *Ptr, Value *RABPtr) {
    if (!V)
      return false;
    auto *CB = dyn_cast<CallBase>(V->stripPointerCasts());
    if (!CB)
      return false;
    if (!isDummyFuncWithThisAndPtrArgs(CB, GetTLI(*CB->getFunction())))
      return false;
    if (!IsArgsOkay(CB, Obj, Ptr, RABPtr))
      return false;
    Visited.insert(CB);
    return true;
  };

  Function *CmpFn;
  BasicBlock *TBlock;
  BasicBlock *FBlock;
  if (!identifyDevirtChecks(BB, Obj, &CmpFn, &TBlock, &FBlock, RABPtr))
    return false;
  Instruction *DeCall = TBlock->getFirstNonPHIOrDbg();
  Instruction *DummyCall = FBlock->getFirstNonPHIOrDbg();
  if (!IsDeallocCall(DeCall, Obj, Ptr, CmpFn, RABPtr))
    return false;
  if (!IsDummyDeallocCall(DummyCall, Obj, Ptr, RABPtr))
    return false;
  BasicBlock *NSucc = nullptr;
  BasicBlock *USucc = nullptr;
  if (!getAllocDeallocCommonSucc(DeCall, DummyCall, &NSucc, &USucc))
    return false;
  if (USucc && !isUnreachableOK(USucc))
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

// Check for the assignment statements below.
// InitBB:
//   Obj->list->listHead = AllocPtr;
//   AllocPtr->Next = AllocPtr;
//   AllocPtr->Prev = AllocPtr;
//
bool MemManageTransImpl::identifyNodeInit(BasicBlock *InitBB, Value *Obj,
                                          Value *AllocPtr) {
  unsigned NodePrevAssigned = 0;
  unsigned NodeNextAssigned = 0;
  unsigned NodeAssigned = 0;
  auto Cand = getCurrentCandidate();
  for (auto &I : *InitBB) {
    auto *SI = dyn_cast<StoreInst>(&I);
    if (!SI)
      continue;
    if (SI->getValueOperand() != AllocPtr)
      return false;
    Value *PtrOp = SI->getPointerOperand();
    Value *BasePtr = nullptr;
    int32_t Idx = 0;
    if (!getGEPBaseAddrIndex(PtrOp, &BasePtr, &Idx))
      return false;
    if (BasePtr == AllocPtr) {
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
  if (!identifyNodeInit(InitBB, Obj, *AllocPtr))
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
//
// When NodePtr is not null, it identifies the pattern below.
// if ((((ArenaBlock*)NodePtr))->ObjectCount <
//     (((ArenaBlock*)NodePtr))->BlockSize) {
//     // AvailableBB
// } else {
//     // NotAvailableBB
// }
bool MemManageTransImpl::identifyBlockAvailable(BasicBlock *BB, Value *Obj,
                                                BasicBlock **AvailableBB,
                                                BasicBlock **NotAvailableBB,
                                                Value *NodePtr = nullptr) {
  Value *LValue = nullptr;
  Value *RValue = nullptr;
  ICmpInst::Predicate Predi = ICmpInst::ICMP_NE;
  if (!processBBTerminator(BB, &LValue, &RValue, AvailableBB, NotAvailableBB,
                           &Predi))
    return false;
  if (Predi != ICmpInst::ICMP_ULT)
    return false;
  if (!isFrontNodeBlockSizeLoad(RValue, Obj, NodePtr))
    return false;
  if (!isFrontNodeObjectCountLoad(LValue, Obj, NodePtr))
    return false;
  return true;
}

// Returns true if it identified the pattern below.
//
// BB:
//  %RABAllocPtr = call AllocateMemoryForRAB(%RABSizeVal)
//  Initialize ArenaBlock (i.e using %RABAllocPtr)
//  BlockAllocPtr = invoke AllocateMemoryForStringObjects(BlockSizeVal)
//                   label %NBB unwind label %UnBB
// NBB:
//   Initialize ReusableArenaBlock
//   ...
// UnBB:
//   %LPI = LandingPad
//   invoke DeallocateCall(MemManager, %RABAllocPtr)
//          label %EndBB unwind label %UnReachableBB
// EndBB:
//   resume %LPI
//  ...
// UnReachableBB:
//   ...
//   unreachable
//
bool MemManageTransImpl::identifyCreate(BasicBlock *BB, Value *Obj,
                                        BasicBlock **ABlock,
                                        Value **RABAllocPtr) {
  // Check memory allocation for ReusableArenaBlock
  Value *RABSizeVal = nullptr;
  BasicBlock *UnBB = nullptr;
  if (!identifyAllocCall(BB, Obj, RABAllocPtr, &RABSizeVal, &UnBB) || UnBB)
    return false;
  auto Cand = getCurrentCandidate();
  StructType *RABType = Cand->getReusableArenaBlockType();
  assert(RABType && "Unexpected RABType Type");
  int64_t RABSize = DL.getTypeAllocSize(RABType);
  if (!checkConstantSize(RABSizeVal, RABSize))
    return false;
  auto *RABAllocBC = dyn_cast<BitCastInst>(*RABAllocPtr);
  assert(RABAllocBC && "Expected BitCastInst");
  auto *PTy = dyn_cast<PointerType>(RABAllocBC->getType());
  if (!PTy || PTy->getElementType() != RABType)
    return false;

  // Check memory allocation for StringObject
  BasicBlock *InitBB = RABAllocBC->getParent();
  Value *BlockSizeVal = nullptr;
  Value *BlockAllocPtr = nullptr;
  UnBB = nullptr;
  if (!identifyAllocCall(InitBB, Obj, &BlockAllocPtr, &BlockSizeVal, &UnBB))
    return false;
  if (!UnBB)
    return false;
  StructType *StrObjType = Cand->getStringObjectType();
  assert(StrObjType && "Unexpected StrObjType Type");
  int64_t StrObjSize = DL.getTypeAllocSize(StrObjType);
  if (!checkSizeValue(BlockSizeVal, StrObjSize, Obj))
    return false;
  auto *BlkAllocBC = dyn_cast<BitCastInst>(BlockAllocPtr);
  assert(BlkAllocBC && "Expected BitCastInst");
  PTy = dyn_cast<PointerType>(BlkAllocBC->getType());
  if (!PTy || PTy->getElementType() != StrObjType)
    return false;

  // Check EH code
  Instruction *I = UnBB->getFirstNonPHIOrDbg();
  auto *LPI = dyn_cast_or_null<LandingPadInst>(I);
  if (!LPI || LPI->getNumClauses() != 0 || !LPI->isCleanup())
    return false;
  BasicBlock *EndBB = nullptr;
  if (!identifyDeallocCall(UnBB, Obj, RABAllocBC->getOperand(0), &EndBB))
    return false;
  BasicBlock *Succ = getSingleSucc(EndBB);
  if (Succ)
    EndBB = Succ;
  auto *Res = dyn_cast<ResumeInst>(EndBB->getTerminator());
  if (!Res || Res->getValue() != LPI)
    return false;
  Visited.insert(LPI);
  Visited.insert(Res);

  // Check for initialization of ArenaBlock and ReusableArenaBlock
  if (!identifyArenaBlockInit(InitBB, Obj, *RABAllocPtr, BlockAllocPtr, ABlock))
    return false;

  return true;
}

// Checks initialization of ArenaBlock and ReusableArenaBlock.
//
// Returns true if it identifies the the pattern below.
// BB: (Initialization of ArenaBlock)
//   RABObj->ArenaBlock.BlockSize = Obj->ArenaAllocator.blockSize
//   RABObj->ArenaBlock.ObjectCount = 0
//   RABObj->ArenaBlock.Allocator.MemManager = Obj->ArenaAllocator.MemManager
//
// AllocBB:
//   RABObj->ObjectBlock = BlockAllocPtr;
//   RABObj->FirstFreeBlock = 0;
//   RABObj->NextFreeBlock = 0;
//   for (int I = 0; I < RABObj->ArenaBlock.BlockSize; I++) {
//     int J = I + 1;
//     *(i16*((BlockAllocPtr + I) + 0)) = J;
//     *(i32*((BlockAllocPtr + I) + 1)) = Some_constant;
//   }
//
bool MemManageTransImpl::identifyArenaBlockInit(BasicBlock *BB, Value *Obj,
                                                Value *RABObj,
                                                Value *BlockAllocPtr,
                                                BasicBlock **ABlock) {

  auto IsLoopCounter = [this](Value *V, Value *AddI) {
    auto *I = dyn_cast<TruncInst>(V);
    if (!I)
      return false;
    if (!I->getType()->isIntegerTy(16) || I->getOperand(0) != AddI)
      return false;
    Visited.insert(I);
    return true;
  };

  // Initialization of ArenaBlock
  //  RABObj->ArenaBlock.BlockSize = Obj->ArenaAllocator.blockSize
  //  RABObj->ArenaBlock.ObjectCount = 0
  //  RABObj->ArenaBlock.Allocator.MemManager = Obj->ArenaAllocator.MemManager
  SmallVector<StoreInst *, 4> StoreVec;

  collectStoreInst(BB, StoreVec);
  if (StoreVec.size() != 3)
    return false;
  StoreInst *ObjCountSI = nullptr;
  StoreInst *BlockSizeSI = nullptr;
  StoreInst *MemSI = nullptr;
  for (auto *SI : StoreVec) {
    Value *Ptr = SI->getPointerOperand();
    Value *Val = SI->getValueOperand();
    if (isAllocatorBlockSizeLoad(Val, Obj)) {
      if (!isBlockSizeAddrFromRAB(Ptr, RABObj))
        return false;
      if (BlockSizeSI)
        return false;
      BlockSizeSI = SI;
    } else if (isListMemManagerLoad(Val, Obj)) {
      if (!isAllocatorMemAddrFromRAB(Ptr, RABObj))
        return false;
      if (MemSI)
        return false;
      MemSI = SI;
    } else if (isa<ConstantInt>(Val) && cast<ConstantInt>(Val)->isZeroValue()) {
      if (!isObjectCountAddrFromRAB(Ptr, RABObj))
        return false;
      if (ObjCountSI)
        return false;
      ObjCountSI = SI;
    } else {
      return false;
    }
  }
  if (!ObjCountSI || !BlockSizeSI || !MemSI)
    return false;
  Visited.insert(ObjCountSI);
  Visited.insert(BlockSizeSI);
  Visited.insert(MemSI);

  assert(isa<BitCastInst>(BlockAllocPtr) && "Unexpected allocation pointer");
  BasicBlock *AllocBB = cast<BitCastInst>(BlockAllocPtr)->getParent();

  // Initialization of ReusableArenaBlock
  //  RABObj->ObjectBlock = BlockAllocPtr;
  //  RABObj->FirstFreeBlock = 0;
  //  RABObj->NextFreeBlock = 0;
  SmallVector<StoreInst *, 4> AllocStoreVec;
  collectStoreInst(AllocBB, AllocStoreVec);
  if (AllocStoreVec.size() != 3)
    return false;
  StoreInst *ObjBlockSI = nullptr;
  StoreInst *FirstFreeBlockSI = nullptr;
  StoreInst *NextFreeBlockSI = nullptr;
  for (auto *SI : AllocStoreVec) {
    Value *Ptr = SI->getPointerOperand();
    Value *Val = SI->getValueOperand();
    if (isObjectBlockAddrFromRAB(Ptr, RABObj)) {
      if (Val != BlockAllocPtr)
        if (ObjBlockSI)
          return false;
      ObjBlockSI = SI;
    } else if (isFirstFreeBlockAddrFromRAB(Ptr, RABObj)) {
      if (!isa<ConstantInt>(Val) || !cast<ConstantInt>(Val)->isZeroValue())
        return false;
      if (FirstFreeBlockSI)
        return false;
      FirstFreeBlockSI = SI;
    } else if (isNextFreeBlockAddrFromRAB(Ptr, RABObj)) {
      if (!isa<ConstantInt>(Val) || !cast<ConstantInt>(Val)->isZeroValue())
        return false;
      if (NextFreeBlockSI)
        return false;
      NextFreeBlockSI = SI;
    } else {
      return false;
    }
  }
  if (!ObjBlockSI || !FirstFreeBlockSI || !NextFreeBlockSI)
    return false;
  Visited.insert(ObjBlockSI);
  Visited.insert(FirstFreeBlockSI);
  Visited.insert(NextFreeBlockSI);

  // Check for the pattern below.
  //   for (int I = 0; I < RABObj->ArenaBlock.BlockSize; I++) {
  //     int J = I + 1;
  //     *(i16*((BlockAllocPtr + I) + 0)) = J;
  //     *(i32*((BlockAllocPtr + I) + 1)) = Some_constant;
  //   }
  //
  // Check ZTT for the loop
  Value *LValue = nullptr;
  Value *RValue = nullptr;
  ICmpInst::Predicate Predi = ICmpInst::ICMP_NE;
  BasicBlock *LoopEarlyExitBB = nullptr;
  BasicBlock *LoopHead = nullptr;
  if (!processBBTerminator(AllocBB, &LValue, &RValue, &LoopEarlyExitBB,
                           &LoopHead, &Predi))
    return false;
  if (Predi != ICmpInst::ICMP_EQ)
    return false;
  if (!isa<ConstantInt>(RValue) || !cast<ConstantInt>(RValue)->isZeroValue())
    return false;
  if (!isBlockSizeLoadFromRAB(LValue, RABObj))
    return false;

  // Check Loop
  BasicBlock *LoopBB = getSingleSucc(LoopHead);
  if (!LoopBB)
    return false;
  BasicBlock *LoopExitBB = nullptr;
  BasicBlock *TargetBB = nullptr;
  Value *AddI = nullptr;
  Value *BlockSizeI = nullptr;
  if (!processBBTerminator(LoopBB, &AddI, &BlockSizeI, &LoopExitBB, &TargetBB,
                           &Predi))
    return false;
  if (Predi != ICmpInst::ICMP_EQ)
    return false;
  auto *ZExt = dyn_cast<ZExtInst>(BlockSizeI);
  if (ZExt) {
    BlockSizeI = ZExt->getOperand(0);
    Visited.insert(ZExt);
  }
  if (TargetBB != LoopBB)
    return false;
  if (!isBlockSizeLoadFromRAB(BlockSizeI, RABObj))
    return false;

  Value *AddOp = nullptr;
  if (!isIncrementByOne(AddI, &AddOp))
    return false;
  auto *PHI = dyn_cast<PHINode>(AddOp);
  if (!PHI || PHI->getNumIncomingValues() != 2)
    return false;
  if (LoopBB != PHI->getParent())
    return false;
  Value *V1 = PHI->getIncomingValueForBlock(LoopHead);
  Value *V2 = PHI->getIncomingValueForBlock(LoopBB);
  ConstantInt *Init = dyn_cast<ConstantInt>(V1);
  if (!Init || !Init->isZero())
    return false;
  if (V2 != AddI)
    return false;
  Visited.insert(PHI);

  SmallVector<StoreInst *, 2> InitStoreVec;
  collectStoreInst(LoopBB, InitStoreVec);
  if (InitStoreVec.size() != 2)
    return false;
  StoreInst *NextFBlockSI = nullptr;
  StoreInst *VerifySI = nullptr;
  for (auto SI : InitStoreVec) {
    Value *Ptr = SI->getPointerOperand();
    Value *Val = SI->getValueOperand();
    Value *BasePtr = nullptr;
    Value *IndexPtr = nullptr;
    int32_t Idx = 0;
    if (!isNextBlockFieldAccess(Ptr, &BasePtr, &IndexPtr, &Idx))
      return false;
    if (BasePtr != BlockAllocPtr || IndexPtr != PHI)
      return false;
    // Check for special constant Value here.
    if (isa<ConstantInt>(Val) &&
        cast<ConstantInt>(Val)->getLimitedValue() == ValidObjStamp) {
      if (VerifySI)
        return false;
      if (!Val->getType()->isIntegerTy(32) || Idx != 1)
        return false;
      VerifySI = SI;
    } else if (IsLoopCounter(Val, AddI)) {
      if (NextFBlockSI)
        return false;
      if (Idx != 0)
        return false;
      NextFBlockSI = SI;
    } else {
      return false;
    }
  }
  if (!NextFBlockSI || !VerifySI)
    return false;
  if (LoopExitBB != LoopEarlyExitBB)
    return false;
  Visited.insert(NextFBlockSI);
  Visited.insert(VerifySI);
  *ABlock = LoopExitBB;
  return true;
}

// Returns true if IR matches the pattern below starting "BB".
// Updates TargetBB and NPtr when it returns true.
//
// BB:
//   if (Obj->ArenaBlock.List.ListHead == nullptr) {
//     // NodeBB:
//     NodePtr = allocNode();
//   } else {
//     // HasHeadBB:
//     NodeLI = Obj->ArenaBlock.List.ListHead->Next
//   }
//   // CreatedHeadBB:
//   PHI <- [NodePtr, CreatedHeadBB], [NodeLI, HasHeadBB]
//   *Nptr = PHI
bool MemManageTransImpl::identifyGetListHead(BasicBlock *BB, Value *Obj,
                                             BasicBlock **TargetB,
                                             Value **NPtr) {
  Value *NodePtr = nullptr;
  BasicBlock *CreatedHeadBB = nullptr;
  BasicBlock *HasHeadBB = nullptr;
  if (!identifyListHead(BB, Obj, &CreatedHeadBB, &HasHeadBB, &NodePtr))
    return false;
  BasicBlock *SuccBB = getSingleSucc(HasHeadBB);
  if (!SuccBB)
    return false;
  if (SuccBB != CreatedHeadBB)
    return false;
  auto *BI = dyn_cast<BranchInst>(HasHeadBB->getTerminator());
  assert(BI && " Expected BranchInst");
  auto *NodeLI = dyn_cast_or_null<LoadInst>(BI->getPrevNonDebugInstruction());
  if (!NodeLI)
    return false;
  if (!isListBegin(NodeLI, Obj))
    return false;
  auto *PHI = dyn_cast_or_null<PHINode>(getFirstNonDbg(SuccBB));
  if (!PHI)
    return false;
  if (NodeLI != PHI->getIncomingValueForBlock(HasHeadBB))
    return false;
  BasicBlock *NodeBB = cast<Instruction>(NodePtr)->getParent();
  if (PHI->getBasicBlockIndex(NodeBB) < 0 ||
      NodePtr != PHI->getIncomingValueForBlock(NodeBB))
    return false;
  *TargetB = CreatedHeadBB;
  *NPtr = PHI;
  Visited.insert(PHI);
  return true;
}

// Returns true if IR matches the pattern below starting with "BB".
//
// BB:
// if (Obj->ArenaBlock.List.ListHead == nullptr) {
//   CreteNode()
// } else {
//   GetHead
// }
// // CreatedHeadBB:
// PHI <- [NodePtr, CreatedHeadBB], [NodeLI, HasHeadBB]
// ListHeadPtr = PHI;
// if (Obj->ArenaBlock.List.FreeListHead == nullptr) {
//   // AllocBB:
//   FreeListHeadPtr = CreteNode()
//   // FreeListHeadNext is considered nullptr
// } else {
//   // HasFreeListHeadBB:
//   NewNode1 = GetFreeListHead
//   FreeListHeadNextLI = GetFreeListHeadNextLI
// }
// // CreateFreeListHeadBB:
// NewNodePHI = phi [NewNode1, HasFreeListHeadBB], [FreeListHeadPtr, AllocBB]
// NextFreeNodePHI = phi [FreeListHeadNextLI, HasFreeListHeadBB], [null,
// AllocBB]
// // Push BlockAllocPtr at front using NewNodePHI and NextFreeNodePHI.
// // AllocateBlock using RABPtr.
bool MemManageTransImpl::identifyPushFront(BasicBlock *BB, Value *Obj,
                                           Value *BlockAllocPtr,
                                           BasicBlock *BlockAvailableBB) {
  BasicBlock *ListHeadBlock = nullptr;
  Value *ListHeadPtr = nullptr;
  if (!identifyGetListHead(BB, Obj, &ListHeadBlock, &ListHeadPtr))
    return false;

  BasicBlock *CreateFreeListHeadBB = nullptr;
  BasicBlock *HasFreeListHeadBB = nullptr;
  Value *FreeListHeadPtr = nullptr;
  if (!identifyCheckAndAllocNode(ListHeadBlock, Obj, &CreateFreeListHeadBB,
                                 &HasFreeListHeadBB, &FreeListHeadPtr, false))
    return false;
  BasicBlock *PushBlock = CreateFreeListHeadBB;
  if (!PushBlock || PushBlock != getSingleSucc(HasFreeListHeadBB))
    return false;
  Instruction *LastI = HasFreeListHeadBB->getTerminator();
  auto *FreeListHeadNextLI =
      dyn_cast_or_null<LoadInst>(LastI->getPrevNonDebugInstruction());
  if (!FreeListHeadNextLI || !isListFreeHeadNextLoad(FreeListHeadNextLI, Obj))
    return false;
  // GEP should be FreeListHeadNext.
  auto *GEP =
      dyn_cast<GetElementPtrInst>(FreeListHeadNextLI->getPointerOperand());
  assert(GEP && "Expected GEP");
  // GEP->getPointerOperand() should be FreeListHead since FreeListHeadNextLI
  // is proved to be ListFreeHeadNextLoad.
  Value *NewNode1 = GEP->getPointerOperand();
  Value *NextFreeNode1 = FreeListHeadNextLI;
  Value *NewNode2 = FreeListHeadPtr;
  Value *NewNodePHI = nullptr;
  Value *NextFreeNodePHI = nullptr;
  BasicBlock *AllocBB = cast<BitCastInst>(FreeListHeadPtr)->getParent();
  if (PushBlock != getSingleSucc(AllocBB))
    return false;
  // Makes sure NextFreeNode and NewNode from HasFreeListHeadBB and AllocBB
  // are merged properly.
  for (auto &I : *PushBlock) {
    if (isa<DbgInfoIntrinsic>(&I))
      continue;
    auto *PHI = dyn_cast<PHINode>(&I);
    if (!PHI)
      break;
    Value *Ptr = PHI->getIncomingValueForBlock(HasFreeListHeadBB);
    if (Ptr == NewNode1) {
      if (NewNodePHI)
        return false;
      if (NewNode2 != PHI->getIncomingValueForBlock(AllocBB))
        return false;
      NewNodePHI = PHI;
    } else if (Ptr == NextFreeNode1) {
      if (NextFreeNodePHI)
        return false;
      Value *NextFreePtr = PHI->getIncomingValueForBlock(AllocBB);
      if (!isa<Constant>(NextFreePtr) ||
          !cast<Constant>(NextFreePtr)->isNullValue())

        return false;
      NextFreeNodePHI = PHI;
    } else {
      return false;
    }
  }
  if (!NextFreeNodePHI || !NewNodePHI)
    return false;
  Visited.insert(cast<Instruction>(NextFreeNodePHI));
  Visited.insert(cast<Instruction>(NewNodePHI));
  if (!identifyPushAtPos(PushBlock, Obj, ListHeadPtr, NewNodePHI,
                         NextFreeNodePHI, BlockAllocPtr))
    return false;
  BasicBlock *CreateHeadBB = nullptr;
  BasicBlock *HasHeadBB = nullptr;
  Value *HeadPtr = nullptr;
  if (!identifyListHead(PushBlock, Obj, &CreateHeadBB, &HasHeadBB, &HeadPtr))
    return false;

  // Check the Preds of CreateHeadBB: One coming from blockAvailable()
  BasicBlock *Succ = getSingleSucc(HasHeadBB);
  if (!Succ || Succ != CreateHeadBB)
    return false;
  if (BlockAvailableBB != HasHeadBB)
    return false;

  // Get PHI node that merge ListHead values.
  auto *NodePHI = dyn_cast_or_null<PHINode>(getFirstNonDbg(HasHeadBB));
  if (!NodePHI || NodePHI->getNumIncomingValues() != 2)
    return false;
  if (!isListHeadLoad(NodePHI->getIncomingValue(0), Obj) ||
      !isListHeadLoad(NodePHI->getIncomingValue(1), Obj))
    return false;
  Visited.insert(NodePHI);

  auto *PHI = dyn_cast_or_null<PHINode>(getFirstNonDbg(CreateHeadBB));
  if (!PHI)
    return false;
  auto *PredBB = cast<BitCastInst>(HeadPtr)->getParent();
  if (HeadPtr != PHI->getIncomingValueForBlock(PredBB))
    return false;
  Value *NextNodeVal = PHI->getIncomingValueForBlock(HasHeadBB);
  auto *NextNodeLI = dyn_cast<LoadInst>(NextNodeVal);
  if (!NextNodeLI || !isNodePosNextLoad(NextNodeLI, NodePHI))
    return false;

  // Check for the pattern below to get load of ReusableArenaBlock
  // from PHI node.
  //  %214 = phi %Node* [ %210, %209 ], [ %190, %187 ]
  //  %215 = getelementptr %Node, %Node* %214, i64 0, i32 0
  //  %216 = load %"ReusableArenaBlock"*, %"ReusableArenaBlock"** %215
  if (!PHI->hasOneUse())
    return false;
  auto *II = dyn_cast<GetElementPtrInst>(*PHI->user_begin());
  if (!II || !II->hasOneUse() || !isNodePosReusableArenaBlockAddr(II, PHI))
    return false;
  auto *RABPtr = dyn_cast<LoadInst>(*II->user_begin());
  if (!RABPtr)
    return false;
  Visited.insert(PHI);
  Visited.insert(II);
  Visited.insert(RABPtr);

  // Identify the functionality of ReusableArenaBlock's allocation.
  if (!identifyRABAllocateBlock(CreateHeadBB, RABPtr))
    return false;

  return true;
}

//
// BB:
//  ObjectType* theResult;
//  if ( this->m_objectCount == this->m_blockSize ) {
//    theResult =0;
//  } else {
//    // AllocInitBB:
//    if(this->m_firstFreeBlock != this->m_nextFreeBlock) {
//      // IncrementBB:
//      theResult = this->m_objectBlock + this->m_firstFreeBlock;
//      this->m_nextFreeBlock = NextBlock::cast(theResult)->next;
//      ++this->m_objectCount;
//    } else {
//      // NoIncrementBB:
//      theResult = this->m_objectBlock + this->m_firstFreeBlock;
//    }
//  }
//  // RetBB:
//  retun theResult;
//
bool MemManageTransImpl::identifyRABAllocateBlock(BasicBlock *BB,
                                                  Value *RABPtr) {

  // Returns true if "V" represents the pattern below.
  // %229 = zext i16 RABPtr->FirstFreeBlock to i64
  auto IsFirstFreeBlock = [this](Value *V, Value *RABPtr) {
    auto *ZExt = dyn_cast<ZExtInst>(V);
    if (!ZExt)
      return false;
    if (!isFirstFreeBlockLoadFromRAB(ZExt->getOperand(0), RABPtr))
      return false;
    Visited.insert(ZExt);
    return true;
  };

  // Returns true if "Val" represents the pattern below.
  // %2 = zext i16 RABPtr->FirstFreeBlock to i64
  // %3 = getelementptr %"SObj", %"SObj"* RABPtr->ArenaBlock.ObjBlock, i64 %2
  auto IsFirstFreeRABBlock = [this, &IsFirstFreeBlock](Value *Val,
                                                       Value *RABPtr) {
    auto *GEP = dyn_cast<GetElementPtrInst>(Val);
    if (!GEP || GEP->getNumIndices() != 1)
      return false;
    if (!isObjectBlockLoadFromRAB(GEP->getPointerOperand(), RABPtr))
      return false;
    if (!IsFirstFreeBlock(GEP->getOperand(1), RABPtr))
      return false;
    Visited.insert(GEP);
    return true;
  };

  // Check for RABPtr->m_objectCount == RABPtr->m_blockSize
  Value *LValue = nullptr;
  Value *RValue = nullptr;
  ICmpInst::Predicate Predi = ICmpInst::ICMP_NE;
  BasicBlock *AllocInitBB = nullptr;
  BasicBlock *RetBB = nullptr;
  if (!processBBTerminator(BB, &LValue, &RValue, &RetBB, &AllocInitBB, &Predi))
    return false;
  if (Predi != ICmpInst::ICMP_EQ)
    return false;
  if (!isObjectCountLoadFromRAB(LValue, RABPtr))
    return false;
  if (!isBlockSizeLoadFromRAB(RValue, RABPtr))
    return false;

  // Check for this->m_firstFreeBlock == this->m_nextFreeBlock
  BasicBlock *NoIncrementBB = nullptr;
  BasicBlock *IncrementBB = nullptr;
  if (!processBBTerminator(AllocInitBB, &LValue, &RValue, &IncrementBB,
                           &NoIncrementBB, &Predi))
    return false;
  if (Predi != ICmpInst::ICMP_EQ)
    return false;
  if (!isFirstFreeBlockLoadFromRAB(LValue, RABPtr))
    return false;
  if (!isNextFreeBlockLoadFromRAB(RValue, RABPtr))
    return false;

  // Check CFG here. Makes sure single successor of NoIncrementBB and
  // IncrementBB is RetBB
  BasicBlock *BB1 = getSingleSucc(NoIncrementBB);
  if (!BB1 || BB1 != RetBB)
    return false;
  BB1 = getSingleSucc(IncrementBB);
  if (!BB1 || BB1 != RetBB)
    return false;

  // Check for the following store instructions in IncrementBB.
  //
  // temp = RABPtr->objectBlock + RABPtr->firstFreeBlock;
  // RABPtr->NextFreeBlock = NextBlock::cast(temp)->next;
  // ++RABPtr->objectCount;
  //
  SmallVector<StoreInst *, 8> InitStoreVec;
  collectStoreInst(IncrementBB, InitStoreVec);
  if (InitStoreVec.size() != 2)
    return false;
  StoreInst *NewObjCountSI = nullptr;
  StoreInst *NewNextFreeBlockSI = nullptr;
  for (auto *SI : InitStoreVec) {
    Value *Ptr = SI->getPointerOperand();
    Value *Val = SI->getValueOperand();
    if (isObjectCountAddrFromRAB(Ptr, RABPtr)) {
      if (NewObjCountSI)
        return false;
      Value *AddOp = nullptr;
      if (!isIncrementByOne(Val, &AddOp))
        return false;
      if (!isObjectCountLoadFromRAB(AddOp, RABPtr))
        return false;
      NewObjCountSI = SI;
    } else if (isNextFreeBlockAddrFromRAB(Ptr, RABPtr)) {
      if (NewNextFreeBlockSI)
        return false;
      Value *BaseAddr = nullptr;
      Value *IndexAddr = nullptr;
      int32_t Idx = 0;
      if (!isNextBlockFieldLoad(Val, &BaseAddr, &IndexAddr, &Idx))
        return false;
      if (Idx != 0 || !isObjectBlockLoadFromRAB(BaseAddr, RABPtr) ||
          !IsFirstFreeBlock(IndexAddr, RABPtr))
        return false;
      NewNextFreeBlockSI = SI;
    } else {
      return false;
    }
  }
  if (!NewObjCountSI || !NewNextFreeBlockSI)
    return false;
  Visited.insert(NewObjCountSI);
  Visited.insert(NewNextFreeBlockSI);

  // BB:
  //
  // NoIncrementBB:
  //   %2 = zext i16 RABPtr->FirstFreeBlock to i64
  //   %3 = getelementptr %S, %S* RABPtr->ArenaBlock.ObjBlock, i64 %2
  //
  // IncrementBB:
  //   %4 = zext i16 RABPtr->FirstFreeBlock to i64
  //   %5 = getelementptr %S, %S* RABPtr->ArenaBlock.ObjBlock, i64 %4
  //
  // RetBB:
  //   %7 = phi %S* [ null, %BB ], [ %3, %NoIncrementBB ], [ %4, %IncrementBB ]
  //   ret %S* %7

  auto *Ret = dyn_cast<ReturnInst>(RetBB->getTerminator());
  assert(Ret && "Expected Ret instruction");
  Value *RetVal = Ret->getReturnValue();
  assert(RetVal && "Unexpected Return Value");
  auto *RetPHI = dyn_cast<PHINode>(RetVal);
  if (!RetPHI)
    return false;
  Value *IncPtr = RetPHI->getIncomingValueForBlock(NoIncrementBB);
  if (!IsFirstFreeRABBlock(IncPtr, RABPtr))
    return false;
  IncPtr = RetPHI->getIncomingValueForBlock(IncrementBB);
  if (!IsFirstFreeRABBlock(IncPtr, RABPtr))
    return false;
  IncPtr = RetPHI->getIncomingValueForBlock(BB);
  if (!isa<Constant>(IncPtr) || !cast<Constant>(IncPtr)->isNullValue())
    return false;
  Visited.insert(Ret);
  Visited.insert(RetPHI);
  return true;
}

// Returns true if BB has the following StoreInsts in the same order.
//
// newNode->ReusableArenaBlock = FullBlock;
// newNode->prev = NodePos.prev
// newNode->next = NodePos
// NodePos.prev->next = newNode;
// NodePos.prev = newNode;
// Obj->ArenaAllocator->List->freeListHeadPtr = nextFreeNode
//
bool MemManageTransImpl::identifyPushAtPos(BasicBlock *BB, Value *Obj,
                                           Value *NodePos, Value *NewNode,
                                           Value *NextFreeNode,
                                           Value *FullBlock) {
  SmallVector<StoreInst *, 8> StoreVec;

  collectStoreInst(BB, StoreVec);
  if (StoreVec.size() != 6)
    return false;
  StoreInst *SI;

  SI = StoreVec[0];
  if (SI->getValueOperand() != FullBlock)
    return false;
  if (!isNodePosReusableArenaBlockAddr(SI->getPointerOperand(), NewNode))
    return false;
  Visited.insert(SI);

  SI = StoreVec[1];
  if (!isNodePosPrevLoad(SI->getValueOperand(), NodePos))
    return false;
  if (!isNodePosPrev(SI->getPointerOperand(), NewNode))
    return false;
  Visited.insert(SI);

  SI = StoreVec[2];
  if (SI->getValueOperand() != NodePos)
    return false;
  if (!isNodePosNext(SI->getPointerOperand(), NewNode))
    return false;
  Visited.insert(SI);

  SI = StoreVec[3];
  if (SI->getValueOperand() != NewNode)
    return false;
  if (!isNodePosPrevNext(SI->getPointerOperand(), NodePos))
    return false;
  Visited.insert(SI);

  SI = StoreVec[4];
  if (SI->getValueOperand() != NewNode)
    return false;
  if (!isNodePosPrev(SI->getPointerOperand(), NodePos))
    return false;
  Visited.insert(SI);

  SI = StoreVec[5];
  if (SI->getValueOperand() != NextFreeNode)
    return false;
  if (!isListFreeHeadAddr(SI->getPointerOperand(), Obj))
    return false;
  Visited.insert(SI);

  return true;
}

// Returns true if IR matches the pattern below starting with "BB".
// Updates TargetBBPtr and NPtr when returns true.
// BB:
//   if (Obj->ArenaBlock.List.ListHead == nullptr) {
//     // NodeBB:
//     NodePtr = allocNode();
//   } else {
//     // HasHeadBB:
//     NodeLI = Obj->ArenaBlock.List.ListHead->Next
//   }
//   // TargetBB:
//   PHI <- [NodePtr, CreatedHeadBB], [NodeLI, HasHeadBB]
//   *NPtr = PHI
//   *NPtr->ReusableArenaBlock.FirstFreeBlock =
//                     *NPtr->ReusableArenaBlock.NextFreeBlock
//
bool MemManageTransImpl::identifyBlockCommit(BasicBlock *BB, Value *Obj,
                                             BasicBlock **TargetBBPtr,
                                             Value **NPtr) {

  if (!identifyGetListHead(BB, Obj, TargetBBPtr, NPtr))
    return false;

  // Check for *NPtr->ReusableArenaBlock.FirstFreeBlock =
  //                   *NPtr->ReusableArenaBlock.NextFreeBlock
  SmallVector<StoreInst *, 1> StoreVec;
  collectStoreInst(*TargetBBPtr, StoreVec);
  if (StoreVec.size() != 1)
    return false;
  StoreInst *SI = StoreVec[0];
  if (!isNextFreeBlockLoadFromNode(SI->getValueOperand(), *NPtr))
    return false;
  if (!isFirstFreeBlockAddrFromNode(SI->getPointerOperand(), *NPtr))
    return false;
  Visited.insert(SI);
  return true;
}

//
// Returns true if IR matches the pattern below starting with
// "NoBlockAvailableBB".
// Updates PopFrontBB, NodePtr, NodeNextPtr and NodePrevPtr when returns true.
//
// NoBlockAvailableBB:
//   if (Obj->ArenaAllocator.List.listHead == null) Goto NodeHeadLoadBB
//   else Goto NewHeadNodeBB
//
// NodeHeadLoadBB:
//   Begin = Obj->ArenaAllocator.List.listHead->Next;
//   BeginNext = Obj->ArenaAllocator.List.listHead->Next->Next;
//   BeginPrev = Obj->ArenaAllocator.List.listHead->Next->Prev;
//   Goto PopFrontBB
//
// NewHeadNodeBB:
//   NewNode = CreateNode()
//   Goto PopFrontBB
//
// PopFrontBB:
//  PHI1 = phi [Begin, NodeHeadLoadBB], [NewNode, NewHeadNodeBB]
//  PHI2 = phi [BeginNext, NodeHeadLoadBB], [NewNode, NewHeadNodeBB]
//  PHI3 = phi [BeginPrev, NodeHeadLoadBB], [NewNode, NewHeadNodeBB]
//  *NodePtr = PHI1;
//  *NodeNextPtr = PHI2;
//  *NodePrevPtr = PHI3;
//
bool MemManageTransImpl::prepareForPopFront(BasicBlock *NoBlockAvailableBB,
                                            Value *Obj, BasicBlock **PopFrontBB,
                                            Value **NodePtr,
                                            Value **NodeNextPtr,
                                            Value **NodePrevPtr) {

  BasicBlock *NodeHeadLoadBB = nullptr;
  Value *NewNode = nullptr;
  if (!identifyListHead(NoBlockAvailableBB, Obj, PopFrontBB, &NodeHeadLoadBB,
                        &NewNode))
    return false;

  BasicBlock *NewHeadNodeBB = cast<Instruction>(NewNode)->getParent();
  // Ensures NewHeadNodeBB and NodeHeadLoadBB are predecessors of *PopFrontBB.
  if (getSingleSucc(NewHeadNodeBB) != *PopFrontBB ||
      getSingleSucc(NodeHeadLoadBB) != *PopFrontBB)
    return false;

  // Get Begin, BeginNext and BeginPrev load from NodeHeadLoadBB.
  //   Begin = Obj->ArenaAllocator.List.listHead->Next;
  //   BeginNext = Obj->ArenaAllocator.List.listHead->Next->Next;
  //   BeginPrev = Obj->ArenaAllocator.List.listHead->Next->Prev;
  LoadInst *Begin = nullptr;
  LoadInst *BeginNext = nullptr;
  LoadInst *BeginPrev = nullptr;
  for (auto &I : *NodeHeadLoadBB) {
    auto *LI = dyn_cast<LoadInst>(&I);
    if (!LI)
      continue;
    Value *PtrOp = LI->getPointerOperand();
    if (isListBeginPrev(PtrOp, Obj)) {
      if (BeginPrev)
        return false;
      BeginPrev = LI;
    } else if (isListBeginNext(PtrOp, Obj)) {
      if (BeginNext)
        return false;
      BeginNext = LI;
    } else if (isListBegin(LI, Obj)) {
      if (Begin)
        return false;
      Begin = LI;
    } else {
      return false;
    }
  }
  if (!Begin || !BeginNext || !BeginPrev)
    return false;
  Visited.insert(Begin);
  Visited.insert(BeginNext);
  Visited.insert(BeginPrev);

  Value *NodePHI = nullptr;
  Value *NodeNextPHI = nullptr;
  Value *NodePrevPHI = nullptr;
  for (auto &I : **PopFrontBB) {
    if (isa<DbgInfoIntrinsic>(&I))
      continue;
    auto *PHI = dyn_cast<PHINode>(&I);
    if (!PHI)
      break;
    Value *Ptr = PHI->getIncomingValueForBlock(NodeHeadLoadBB);
    if (PHI->getIncomingValueForBlock(NewHeadNodeBB) != NewNode)
      return false;
    if (Ptr == Begin) {
      if (NodePHI)
        return false;
      NodePHI = PHI;
    } else if (Ptr == BeginNext) {
      if (NodeNextPHI)
        return false;
      NodeNextPHI = PHI;
    } else if (Ptr == BeginPrev) {
      if (NodePrevPHI)
        return false;
      NodePrevPHI = PHI;
    } else {
      return false;
    }
  }
  if (!NodePHI || !NodeNextPHI || !NodePrevPHI)
    return false;
  Visited.insert(cast<Instruction>(NodePHI));
  Visited.insert(cast<Instruction>(NodeNextPHI));
  Visited.insert(cast<Instruction>(NodePrevPHI));
  *NodePtr = NodePHI;
  *NodeNextPtr = NodeNextPHI;
  *NodePrevPtr = NodePrevPHI;
  return true;
}

// Returns true if BB has the following StoreInsts in the same order.
//
//  NodePrev->next = NodeNext;
//  Node.next->prev = NodePrev;
//  Node.prev = 0;
//  Node.next = FreeListHead;
//  FreeListHead = Node;
//
// Sets *PFNextFreeNodePtr to FreeListHead when returns.
//
bool MemManageTransImpl::identifyFreeNode(BasicBlock *BB, Value *Obj,
                                          Value *Node, Value *NodeNext,
                                          Value *NodePrev,
                                          Value **PFNextFreeNodePtr) {
  SmallVector<StoreInst *, 8> StoreVec;

  collectStoreInst(BB, StoreVec);
  if (StoreVec.size() != 5)
    return false;
  StoreInst *SI;

  SI = StoreVec[0];
  if (SI->getValueOperand() != NodeNext)
    return false;
  if (!isNodePosNext(SI->getPointerOperand(), NodePrev))
    return false;
  Visited.insert(SI);

  SI = StoreVec[1];
  if (SI->getValueOperand() != NodePrev)
    return false;
  if (!isNodePosNextPrev(SI->getPointerOperand(), Node))
    return false;
  Visited.insert(SI);

  SI = StoreVec[2];
  Value *ValOp = SI->getValueOperand();
  if (!isa<Constant>(ValOp) || !cast<Constant>(ValOp)->isNullValue())
    return false;
  if (!isNodePosPrev(SI->getPointerOperand(), Node))
    return false;
  Visited.insert(SI);

  SI = StoreVec[3];
  if (!isListFreeHeadLoad(SI->getValueOperand(), Obj))
    return false;
  if (!isNodePosNext(SI->getPointerOperand(), Node))
    return false;
  *PFNextFreeNodePtr = SI->getValueOperand();
  Visited.insert(SI);

  SI = StoreVec[4];
  if (SI->getValueOperand() != Node)
    return false;
  if (!isListFreeHeadAddr(SI->getPointerOperand(), Obj))
    return false;
  Visited.insert(SI);
  return true;
}

// Returns true if BB has the pattern below.
//
// BB:
//   Load of Node->ReusableArenaBlock;
//   Code for FreeNode
//   if (Obj->ArenaAllocator.List.listHead == null)
//
// When returns true, sets PopBlockPtr, PFNextFreeNodePtr and PFNodePtr.
//
bool MemManageTransImpl::identifyPopFront(BasicBlock *BB, Value *Obj,
                                          Value *Node, Value *NodeNext,
                                          Value *NodePrev, Value **PopBlockPtr,
                                          Value **PFNodePtr,
                                          Value **PFNextFreeNodePtr) {

  // Check for FreeNode
  if (!identifyFreeNode(BB, Obj, Node, NodeNext, NodePrev, PFNextFreeNodePtr))
    return false;

  // Makes sure first load of BB is Node->ReusableArenaBlock.
  // Sets it to *PopBlockPtr.
  LoadInst *BBFirstLI = getFirstLoadInst(BB);
  if (!BBFirstLI)
    return false;
  if (!isNodePosReusableArenaBlockAddr(BBFirstLI->getPointerOperand(), Node))
    return false;

  // Here, we are just interested in LValue of terminator condition of "BB"
  // to get value of "listHead". The actual processing of this condition
  // will be done later to verify.
  Value *LValue = nullptr;
  Value *RValue = nullptr;
  ICmpInst::Predicate Predi;
  BasicBlock *TBB = nullptr;
  BasicBlock *FBB = nullptr;
  if (!processBBTerminator(BB, &LValue, &RValue, &TBB, &FBB, &Predi))
    return false;

  *PFNodePtr = LValue;
  *PopBlockPtr = BBFirstLI;
  Visited.insert(BBFirstLI);
  return true;
}

// Returns true if IR matches the pattern below starting with "CreatedHeadBB".
// Update PushBackBBPtr when returns true.
//
//   // PFNodePos, PFNewNode and PFNextFreeNode are set by identifyPopFront.
//   br CreateFreeListHeadBB
//
// CreatedHeadBB:
//   if (listHead == null) then Goto CreatedHeadBB
//   else Goto PushBackBB
//
// CreatedHeadBB:
//    ListHeadPtr = CreateNode()
//    if (FreeListHead == null) Goto NewFreeListHeadBB
//    else Goto HasFreeListHeadBB
//
// HasFreeListHeadBB:
//   HasFreeListFreeHead = GetListHeadNode;
//   GEP = getelementptr %L, %L* HasFreeListFreeHead, 0, 2
//   HasFreeListFreeHeadNext = GetListHeadNext using GEP
//   Goto PushBackBB
//
// NewFreeListHeadBB:
//   FreeListHeadPtr = CreateNode()
//   Goto PushBackBB
//
// PushBackBB (CreateFreeListHeadBB):
//   PHI1 = [ListHeadPtr, NewFreeListHeadBB], [ListHeadPtr, HasFreeListHeadBB],
//          [PFNodePos, CreatedHeadBB]
//   PHI2 = [FreeListHeadPtr, NewFreeListHeadBB], [HasFreeListFreeHead,
//           HasFreeListHeadBB], [PFNewNode, CreatedHeadBB]
//   PHI3 = [null, NewFreeListHeadBB], [HasFreeListFreeHeadNext,
//           HasFreeListHeadBB], [PFNextFreeNode, CreatedHeadBB]
//   NodePos = PHI1;
//   NewNode = PHI2;
//   NextFreeNode = PHI3;
//   // PushBack the PopBlock
//
bool MemManageTransImpl::identifyPushBack(BasicBlock *CreatedHeadBB, Value *Obj,
                                          Value *PopBlock, Value *PFNodePos,
                                          Value *PFNewNode,
                                          Value *PFNextFreeNode,
                                          BasicBlock **PushBackBBPtr) {
  BasicBlock *CreateListHeadBB = nullptr;
  BasicBlock *PushBackBB = nullptr;
  Value *ListHeadPtr = nullptr;
  if (!identifyListHead(CreatedHeadBB, Obj, &CreateListHeadBB, &PushBackBB,
                        &ListHeadPtr))
    return false;
  BasicBlock *CreateFreeListHeadBB = nullptr;
  BasicBlock *HasFreeListHeadBB = nullptr;
  Value *FreeListHeadPtr = nullptr;
  if (!identifyCheckAndAllocNode(CreateListHeadBB, Obj, &CreateFreeListHeadBB,
                                 &HasFreeListHeadBB, &FreeListHeadPtr,
                                 /* IsListHead */ false))
    return false;
  BasicBlock *NewFreeListHeadBB =
      cast<Instruction>(FreeListHeadPtr)->getParent();
  if (CreateFreeListHeadBB != PushBackBB)
    return false;
  if (PushBackBB != getSingleSucc(NewFreeListHeadBB))
    return false;
  if (PushBackBB != getSingleSucc(HasFreeListHeadBB))
    return false;

  Instruction *LastI = HasFreeListHeadBB->getTerminator();
  auto *HasFreeListFreeHeadNext =
      dyn_cast_or_null<LoadInst>(LastI->getPrevNonDebugInstruction());
  if (!HasFreeListFreeHeadNext ||
      !isListFreeHeadNextLoad(HasFreeListFreeHeadNext, Obj))
    return false;
  auto *GEP =
      dyn_cast<GetElementPtrInst>(HasFreeListFreeHeadNext->getPointerOperand());
  assert(GEP && "Expected GEP");
  Value *HasFreeListFreeHead = GEP->getPointerOperand();

  Value *NodePos = nullptr;
  Value *NewNode = nullptr;
  Value *NextFreeNode = nullptr;
  for (auto &I : *PushBackBB) {
    if (isa<DbgInfoIntrinsic>(&I))
      continue;
    auto *PHI = dyn_cast<PHINode>(&I);
    if (!PHI)
      break;
    if (PHI->getBasicBlockIndex(CreatedHeadBB) < 0)
      return false;
    Value *Ptr = PHI->getIncomingValueForBlock(CreatedHeadBB);
    if (Ptr == PFNodePos) {
      if (NodePos)
        return false;
      if (ListHeadPtr != PHI->getIncomingValueForBlock(NewFreeListHeadBB))
        return false;
      if (ListHeadPtr != PHI->getIncomingValueForBlock(HasFreeListHeadBB))
        return false;
      NodePos = PHI;
    } else if (Ptr == PFNewNode) {
      if (NewNode)
        return false;
      if (FreeListHeadPtr != PHI->getIncomingValueForBlock(NewFreeListHeadBB))
        return false;
      if (HasFreeListFreeHead !=
          PHI->getIncomingValueForBlock(HasFreeListHeadBB))
        return false;
      NewNode = PHI;
    } else if (Ptr == PFNextFreeNode) {
      if (NextFreeNode)
        return false;
      if (HasFreeListFreeHeadNext !=
          PHI->getIncomingValueForBlock(HasFreeListHeadBB))
        return false;
      Value *NextFreeListNode =
          PHI->getIncomingValueForBlock(NewFreeListHeadBB);
      if (!isa<Constant>(NextFreeListNode) ||
          !cast<Constant>(NextFreeListNode)->isNullValue())
        return false;
      NextFreeNode = PHI;
    } else {
      return false;
    }
  }
  if (!NodePos || !NewNode || !NextFreeNode)
    return false;
  Visited.insert(cast<Instruction>(NodePos));
  Visited.insert(cast<Instruction>(NewNode));
  Visited.insert(cast<Instruction>(NextFreeNode));

  if (!identifyPushAtPos(PushBackBB, Obj, NodePos, NewNode, NextFreeNode,
                         PopBlock))
    return false;
  *PushBackBBPtr = PushBackBB;
  return true;
}

// Returns true if IR matches the pattern below for "BB".
// Update NDestBB when returns UnDestBB.
//
// BB:
//  store const, VTable
//  %3 = tail call i1 @llvm.type.test(const)
//  tail call void @llvm.assume(i1 %3)
//  invoke reset(%Obj)
//   to label %NDestBB unwind label %UnDestBB
//
bool MemManageTransImpl::identifyResetCall(BasicBlock *BB, Value *Obj,
                                           BasicBlock **NDestBB,
                                           BasicBlock **UnDestBB) {
  // Makes sure last instruction of "BB" is Reset call.
  auto *RCall = dyn_cast<InvokeInst>(BB->getTerminator());
  if (!RCall)
    return false;
  auto *CalledF = dtrans::getCalledFunction(*RCall);
  assert(CalledF && "Unexpected indirect call");
  if (FunctionalityMap[Reset] != CalledF)
    return false;
  if (RCall->getArgOperand(0) != Obj)
    return false;
  auto *Assume =
      dyn_cast_or_null<IntrinsicInst>(RCall->getPrevNonDebugInstruction());
  if (!Assume || Assume->getIntrinsicID() != Intrinsic::assume)
    return false;
  auto *Test = dyn_cast<IntrinsicInst>(Assume->getArgOperand(0));
  if (!Test || Test->getIntrinsicID() != Intrinsic::type_test)
    return false;
  if (!isa<Constant>(Test->getArgOperand(0)))
    return false;
  auto *SI = dyn_cast_or_null<StoreInst>(Test->getPrevNonDebugInstruction());
  if (!SI)
    return false;
  if (!isVTableAddrInArenaAllocator(SI->getPointerOperand(), Obj))
    return false;
  if (!isVFTablePointer(SI->getValueOperand()->getType()))
    return false;
  if (!isa<Constant>(SI->getValueOperand()))
    return false;
  Visited.insert(SI);
  Visited.insert(Assume);
  Visited.insert(Test);
  Visited.insert(RCall);
  *NDestBB = RCall->getNormalDest();
  *UnDestBB = RCall->getUnwindDest();
  return true;
}

// Returns true if IR matches the pattern below starting with "BB".
// Update HeadNotEmptyBB, HeadEmptyBB, ListHeadPtr and ListHeadNextPtr
// when returns true.
//
//   if (ListHead == nullptr) {
//     Goto TBlock (HeadEmptyBB):
//   } else {
//     // FBlock:
//     Load ListHeadNext
//     Goto LoopHeadBB:
//   }
//   LoopHeadBB (HeadNotEmptyBB):
//     PHI1 = phi [ListHead, FBlock], []
//     PHI2 = phi [ListHeadNext, FBlock], []
//     *ListHeadPtr = PHI1;
//     *ListHeadNextPtr = PHI2;
//
bool MemManageTransImpl::identifyListHeadListHeadNext(
    BasicBlock *BB, Value *Obj, BasicBlock **HeadNotEmptyBB,
    BasicBlock **HeadEmptyBB, PHINode **ListHeadPtr,
    PHINode **ListHeadNextPtr) {
  BasicBlock *TBlock = nullptr;
  BasicBlock *FBlock = nullptr;
  Value *LValue = nullptr;
  Value *RValue = nullptr;
  ICmpInst::Predicate Pr = ICmpInst::ICMP_NE;
  if (!processBBTerminator(BB, &LValue, &RValue, &TBlock, &FBlock, &Pr))
    return false;
  if (Pr != ICmpInst::ICMP_EQ)
    return false;
  if (!isListHeadLoad(LValue, Obj))
    return false;
  if (!isa<Constant>(RValue) || !cast<Constant>(RValue)->isNullValue())
    return false;
  BasicBlock *LoopHeadBB = getSingleSucc(FBlock);
  if (!LoopHeadBB)
    return false;
  LoadInst *BBFirstLI = getFirstLoadInst(FBlock);
  if (!BBFirstLI || !isListBegin(BBFirstLI, Obj))
    return false;
  PHINode *ListHead = nullptr;
  PHINode *ListHeadNext = nullptr;
  for (auto &I : *LoopHeadBB) {
    if (isa<DbgInfoIntrinsic>(&I))
      continue;
    auto *PHI = dyn_cast<PHINode>(&I);
    if (!PHI)
      break;
    Value *Ptr = PHI->getIncomingValueForBlock(FBlock);
    if (Ptr == BBFirstLI) {
      if (ListHeadNext)
        return false;
      ListHeadNext = PHI;
    } else if (Ptr == LValue) {
      if (ListHead)
        return false;
      ListHead = PHI;
    } else {
      return false;
    }
  }
  if (!ListHeadNext || !ListHead)
    return false;
  Visited.insert(ListHeadNext);
  Visited.insert(ListHead);
  Visited.insert(BBFirstLI);

  *HeadNotEmptyBB = LoopHeadBB;
  *HeadEmptyBB = TBlock;
  *ListHeadNextPtr = ListHeadNext;
  *ListHeadPtr = ListHead;
  return true;
}

// Returns true if IR matches the pattern below starting with "BB".
// Update TargetBB and HeadPHI when returns true.
//
//   BB:
//     ListHead = phi [ListHead, FBlock], []
//     if (ListHead == nullptr) {
//       // InitBB:
//       temp = CreateNode()
//       AllocPtr = bitcast i8* temp to Node*
//     }
//     // FB (TBB):
//     HeadPHI = phi [AllocPtr, InitBB], [ListHead, BB]
//
bool MemManageTransImpl::identifyListHeadPHINode(BasicBlock *BB, Value *Obj,
                                                 Value *ListHead,
                                                 BasicBlock **TargetBB,
                                                 Value **HeadPHI) {
  BasicBlock *TB = nullptr;
  BasicBlock *FB = nullptr;
  Value *LValue = nullptr;
  Value *RValue = nullptr;
  ICmpInst::Predicate Predi = ICmpInst::ICMP_NE;
  if (!processBBTerminator(BB, &LValue, &RValue, &TB, &FB, &Predi))
    return false;
  if (Predi != ICmpInst::ICMP_EQ)
    return false;
  if (ListHead != LValue)
    return false;
  if (!isa<Constant>(RValue) || !cast<Constant>(RValue)->isNullValue())
    return false;
  Value *AllocPtr;
  Value *SizeVal;
  BasicBlock *UnBB = nullptr;
  if (!identifyAllocCall(TB, Obj, &AllocPtr, &SizeVal, &UnBB))
    return false;
  // Check size of memory allocation is equal to size of Node.
  auto Cand = getCurrentCandidate();
  StructType *NodeType = Cand->getListNodeType();
  assert(NodeType && "Unexpected Node Type");
  int64_t NodeSize = DL.getTypeAllocSize(NodeType);
  if (!checkConstantSize(SizeVal, NodeSize))
    return false;

  if (!UnBB || !isUnreachableOK(UnBB))
    return false;

  assert(isa<BitCastInst>(AllocPtr) && "Expected BitCastInst");
  BasicBlock *InitBB = cast<BitCastInst>(AllocPtr)->getParent();
  if (!identifyNodeInit(InitBB, Obj, AllocPtr))
    return false;
  BasicBlock *TBB = getSingleSucc(InitBB);
  if (!TBB || TBB != FB)
    return false;
  auto *NodeNext = dyn_cast<PHINode>(getFirstNonDbg(TBB));
  if (!NodeNext)
    return false;
  if (AllocPtr != NodeNext->getIncomingValueForBlock(InitBB))
    return false;
  if (ListHead != NodeNext->getIncomingValueForBlock(BB))
    return false;
  Visited.insert(NodeNext);

  *TargetBB = TBB;
  *HeadPHI = NodeNext;
  return true;
}

// It identifies the loop below that destroys all Nodes in "listHead".
//
//   iterator pos = begin();
//   while (pos != end()) {
//     destroyNode(pos++.node());
//   }
//
// Returns true if IR matches the pattern below starting with "LoopHeadBB".
// Update TargetBB when returns true.
//
//   LoopHeadBB (HeadNotEmptyBB):
//     PHI1 = phi [ListHead, FBlock], [NewHeadLoad, EndBB]
//     PHI2 = phi [ListHeadNext, FBlock], [NewHeadNextLoad, EndBB]
//     ...
//     PHI3 = phi [PHI1, LoopHeadBB], []
//     if (PHI2 == PHI3) {
//        Goto LoopExit;
//     }
//     NewHeadNextLoad = Load ListHeadNext
//     Dealloc(PHI2);
//     NewHeadLoad = Load ListHead
//   EndBB:
//     Goto LoopHeadBB;
//   ...
//   LoopExit (TargetBB):
bool MemManageTransImpl::identifyDestroyNodes(BasicBlock *LoopHeadBB,
                                              Value *Obj, PHINode *ListHead,
                                              PHINode *ListHeadNext,
                                              BasicBlock **TargetBB) {

  Value *NodeNext = nullptr;
  BasicBlock *TBB = nullptr;
  if (!identifyListHeadPHINode(LoopHeadBB, Obj, ListHead, &TBB, &NodeNext))
    return false;

  BasicBlock *TBlock = nullptr;
  BasicBlock *FBlock = nullptr;
  Value *LValue = nullptr;
  Value *RValue = nullptr;
  ICmpInst::Predicate Predi = ICmpInst::ICMP_NE;
  if (!processBBTerminator(TBB, &LValue, &RValue, &TBlock, &FBlock, &Predi))
    return false;
  if (Predi != ICmpInst::ICMP_EQ)
    return false;
  if (LValue != ListHeadNext || RValue != NodeNext)
    return false;

  LoadInst *NewHeadNextLoad = getFirstLoadInst(FBlock);
  if (!NewHeadNextLoad)
    return false;
  if (!isNodePosNext(NewHeadNextLoad->getPointerOperand(), ListHeadNext))
    return false;

  BasicBlock *EndBB = nullptr;
  if (!identifyDeallocCall(FBlock, Obj, LValue, &EndBB))
    return false;
  if (EndBB->size() == 1) {
    EndBB = getSingleSucc(EndBB);
    if (!EndBB)
      return false;
  }
  BasicBlock *LoopHBB = getSingleSucc(EndBB);
  if (!LoopHBB || LoopHBB != LoopHeadBB)
    return false;
  Instruction *BI = EndBB->getTerminator();
  auto *NewHeadLoad =
      dyn_cast_or_null<LoadInst>(BI->getPrevNonDebugInstruction());
  if (!NewHeadLoad)
    return false;
  if (ListHead->getIncomingValueForBlock(EndBB) != NewHeadLoad ||
      ListHeadNext->getIncomingValueForBlock(EndBB) != NewHeadNextLoad)
    return false;
  Visited.insert(NewHeadLoad);
  Visited.insert(NewHeadNextLoad);

  *TargetBB = TBlock;
  return true;
}

// It identifies the loop below that deallocates all Nodes in "freeListHead".
//
//   Node * freeNode = freeListHeadPtr;
//   while (freeNode != 0) {
//     Node * nextNode = freeNode->next;
//     deallocate(freeNode);
//     freeNode = nextNode;
//   }
//
// Returns true if IR matches the pattern below starting with "BB".
// Update TargetBB when returns true.
//
// BB:
//   Load FreeHeadLoad;
//   Goto LoopHead;
//
// LoopHead:
//   FreeNode = phi [FreeHeadLoad, BB], [FreeNodeNext, EndBB]
//   if (FreeNode == nullptr) {
//     Goto LoopExit;
//   }
//   Load FreeNodeNext;
//   Dealloc(FreeNode)
// EndBB:
//   Goto LoopHead;
//
// LoopExit (TargetBB):
bool MemManageTransImpl::identifyDestroyFreeNodes(BasicBlock *BB, Value *Obj,
                                                  BasicBlock **TargetBB) {
  BasicBlock *LoopHead = getSingleSucc(BB);
  if (!LoopHead)
    return false;
  Instruction *BI = BB->getTerminator();
  auto *FreeHeadLoad =
      dyn_cast_or_null<LoadInst>(BI->getPrevNonDebugInstruction());
  if (!FreeHeadLoad)
    return false;
  if (!isListFreeHeadAddr(FreeHeadLoad->getPointerOperand(), Obj))
    return false;
  auto *FreeHead = dyn_cast<PHINode>(getFirstNonDbg(LoopHead));
  if (!FreeHead)
    return false;
  if (FreeHeadLoad != FreeHead->getIncomingValueForBlock(BB))
    return false;
  BasicBlock *TBB = nullptr;
  BasicBlock *FBB = nullptr;
  Value *LValue = nullptr;
  Value *RValue = nullptr;
  ICmpInst::Predicate Predi = ICmpInst::ICMP_NE;
  if (!processBBTerminator(LoopHead, &LValue, &RValue, &TBB, &FBB, &Predi))
    return false;
  if (Predi != ICmpInst::ICMP_EQ)
    return false;
  if (LValue != FreeHead)
    return false;
  if (!isa<Constant>(RValue) || !cast<Constant>(RValue)->isNullValue())
    return false;
  BasicBlock *LoopHBB = nullptr;
  if (!identifyDeallocCall(FBB, Obj, FreeHead, &LoopHBB))
    return false;
  if (LoopHBB != LoopHead)
    return false;
  LoadInst *FreeNodeNext = getFirstLoadInst(FBB);
  if (!FreeNodeNext)
    return false;
  if (!isNodePosNext(FreeNodeNext->getPointerOperand(), FreeHead))
    return false;
  // Prove that other incoming value of FreeNode is FreeNodeNext.
  if (FreeHead->getNumIncomingValues() != 2)
    return false;
  unsigned FreeNodeNextIdx = FreeHead->getBasicBlockIndex(BB) == 0 ? 1 : 0;
  if (FreeNodeNext != FreeHead->getIncomingValue(FreeNodeNextIdx))
    return false;
  Visited.insert(FreeNodeNext);
  Visited.insert(FreeHeadLoad);
  Visited.insert(FreeHead);

  *TargetBB = TBB;
  return true;
}

// It identifies the destructor of "List".
//
// if (listHead != 0) {
//   iterator pos = begin();
//   while (pos != end()) {
//     destroyNode(pos++.node());
//   }
//   Node * freeNode = freeListHeadPtr;
//   while (freeNode != 0) {
//     Node * nextNode = freeNode->next;
//     deallocate(freeNode);
//     freeNode = nextNode;
//   }
//   LdPtr = Address of listHead;
//   listHead = bitcast %LdPtr to i8**
//   deallocate(listHead);
// }
// return;
//
// If IsEHCode is true, make sures RetBB is unreachable block.
//
bool MemManageTransImpl::identifyListDtor(BasicBlock *BB, Value *Obj,
                                          bool IsEHCode) {

  BasicBlock *HeadNotEmptyBB = nullptr;
  BasicBlock *HeadEmptyBB = nullptr;
  PHINode *ListHeadPtr = nullptr;
  PHINode *ListHeadNextPtr = nullptr;
  if (!identifyListHeadListHeadNext(BB, Obj, &HeadNotEmptyBB, &HeadEmptyBB,
                                    &ListHeadPtr, &ListHeadNextPtr))
    return false;
  BasicBlock *EndDestroyBB = nullptr;
  if (!identifyDestroyNodes(HeadNotEmptyBB, Obj, ListHeadPtr, ListHeadNextPtr,
                            &EndDestroyBB))
    return false;

  BasicBlock *EndLoopBB = nullptr;
  if (!identifyDestroyFreeNodes(EndDestroyBB, Obj, &EndLoopBB))
    return false;

  LoadInst *ListHeadLI = getFirstLoadInst(EndLoopBB);
  if (!ListHeadLI)
    return false;
  Value *LdPtr = ListHeadLI->getPointerOperand();
  if (auto *BC1 = dyn_cast<BitCastInst>(LdPtr)) {
    LdPtr = BC1->getOperand(0);
    Visited.insert(BC1);
  }
  if (!isListHeadAddr(LdPtr, Obj))
    return false;
  BasicBlock *RetBB = nullptr;
  if (!identifyDeallocCall(EndLoopBB, Obj, ListHeadLI, &RetBB))
    return false;
  if (HeadEmptyBB != RetBB)
    return false;
  if (IsEHCode) {
    if (!isUnreachableOK(RetBB))
      return false;
  } else {
    auto *Ret = dyn_cast<ReturnInst>(RetBB->getTerminator());
    if (!Ret)
      return false;
    Visited.insert(Ret);
  }
  Visited.insert(ListHeadLI);
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
  BasicBlock *ABlock = nullptr;
  Value *RABAllocPtr = nullptr;
  if (!identifyCreate(NoBlockAvailableBB, ThisObj, &ABlock, &RABAllocPtr))
    return false;
  if (!identifyPushFront(ABlock, ThisObj, RABAllocPtr, BlockAvailableBB))
    return false;

  if (!verifyAllInstsProcessed(F))
    return false;
  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANS, {
    dbgs() << "   Recognized AllocateBlock: " << F->getName() << "\n";
  });
  return true;
}

// Returns true if "F" is recognized as "CommitAllocation".
//
// Ex:
//  commitAllocation(ReusableArenaAllocator *Obj, StrObj) {
//    Obj->m_blocks.front()->commitAllocation(theObject);
//    if(!Obj->m_blocks.front()->blockAvailable() ) {
//       ReusableArenaBlockType* popBlock = Obj->m_blocks.front();
//       Obj->m_blocks.pop_front();
//       Obj->m_blocks.push_back(popBlock );
//    }
//  }
bool MemManageTransImpl::recognizeCommitAllocation(Function *F) {
  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANS, {
    dbgs() << "   Recognizing CommitAllocation Functionality " << F->getName()
           << "\n";
  });
  Visited.clear();
  assert(F->arg_size() == 2 && "Unexpected arguments");
  Argument *ThisObj = &*F->arg_begin();
  // Check 2nd argument doesn't have any uses.
  Argument *StrObj = F->getArg(1);
  if (!StrObj->hasNUses(0))
    return false;
  BasicBlock *CommitBlock = nullptr;
  Value *NodePtr = nullptr;
  if (!identifyBlockCommit(&F->getEntryBlock(), ThisObj, &CommitBlock,
                           &NodePtr))
    return false;
  BasicBlock *BlockAvailableBB = nullptr;
  BasicBlock *NoBlockAvailableBB = nullptr;
  if (!identifyBlockAvailable(CommitBlock, ThisObj, &BlockAvailableBB,
                              &NoBlockAvailableBB, NodePtr))
    return false;

  auto *Ret = dyn_cast<ReturnInst>(BlockAvailableBB->getTerminator());
  if (!Ret)
    return false;

  BasicBlock *CreatedHeadBB = nullptr;
  Value *Node = nullptr;
  Value *NodeNext = nullptr;
  Value *NodePrev = nullptr;
  if (!prepareForPopFront(NoBlockAvailableBB, ThisObj, &CreatedHeadBB, &Node,
                          &NodeNext, &NodePrev))
    return false;
  Value *PFNewNode = Node;
  Value *PopBlock = nullptr;
  Value *PFNodePos = nullptr;
  Value *PFNextFreeNode = nullptr;
  if (!identifyPopFront(CreatedHeadBB, ThisObj, Node, NodeNext, NodePrev,
                        &PopBlock, &PFNodePos, &PFNextFreeNode))
    return false;

  BasicBlock *PushBackBB = nullptr;
  if (!identifyPushBack(CreatedHeadBB, ThisObj, PopBlock, PFNodePos, PFNewNode,
                        PFNextFreeNode, &PushBackBB))
    return false;
  if (BlockAvailableBB != getSingleSucc(PushBackBB))
    return false;

  if (!verifyAllInstsProcessed(F))
    return false;

  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANS, {
    dbgs() << "   Recognized CommitAllocation: " << F->getName() << "\n";
  });
  return true;
}

// Returns true if "F" is recognized as "Destructor".
//
// Ex:
// Entry:
//   invoke reset();
//   to label %NormalDest  unwind label %UnwindDest
// NormalDest:
//   ListDtor();
// UnwindDest:
//   ListDtor();
//
bool MemManageTransImpl::recognizeDestructor(Function *F) {
  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANS, {
    dbgs() << "   Recognizing Destructor Functionality " << F->getName()
           << "\n";
  });
  Visited.clear();
  Argument *ThisObj = &*F->arg_begin();
  BasicBlock *NDestBB = nullptr;
  BasicBlock *UnDestBB = nullptr;
  if (!identifyResetCall(&F->getEntryBlock(), ThisObj, &NDestBB, &UnDestBB))
    return false;
  if (!identifyListDtor(NDestBB, ThisObj, /* IsEHCode */ false))
    return false;
  if (!identifyListDtor(UnDestBB, ThisObj, /* IsEHCode */ true))
    return false;

  if (!verifyAllInstsProcessed(F))
    return false;

  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANS, {
    dbgs() << "   Recognized Destructor: " << F->getName() << "\n";
  });
  return true;
}

// This is similar to identifyListHeadPHINode function but there are
// some minor differences.
//
// Returns true if IR matches the pattern below starting with "BB".
// Update TargetBB and HeadPHI when returns true.
//
//   BB:
//     ListHead = phi [ListHead, FBlock], []
//     if (ListHead == nullptr) {
//       // InitBB:
//       temp = CreateNode()
//       AllocPtr = bitcast i8* temp to Node*
//       // TBlock
//     }
//     // FBlock
//
bool MemManageTransImpl::identifyCreateListHead(BasicBlock *BB, Value *Obj,
                                                Value *ListHead,
                                                BasicBlock **TBlock,
                                                BasicBlock **FBlock) {
  BasicBlock *TB = nullptr;
  BasicBlock *FB = nullptr;
  Value *LValue = nullptr;
  Value *RValue = nullptr;
  ICmpInst::Predicate Predi = ICmpInst::ICMP_NE;
  if (!processBBTerminator(BB, &LValue, &RValue, &TB, &FB, &Predi))
    return false;
  if (Predi != ICmpInst::ICMP_EQ)
    return false;
  if (ListHead != LValue)
    return false;
  if (!isa<Constant>(RValue) || !cast<Constant>(RValue)->isNullValue())
    return false;
  Value *AllocPtr;
  Value *SizeVal;
  BasicBlock *UnBB = nullptr;
  if (!identifyAllocCall(TB, Obj, &AllocPtr, &SizeVal, &UnBB))
    return false;
  // Check size of memory allocation is equal to size of Node.
  auto Cand = getCurrentCandidate();
  StructType *NodeType = Cand->getListNodeType();
  assert(NodeType && "Unexpected Node Type");
  int64_t NodeSize = DL.getTypeAllocSize(NodeType);
  if (!checkConstantSize(SizeVal, NodeSize))
    return false;

  if (UnBB)
    return false;

  assert(isa<BitCastInst>(AllocPtr) && "Expected BitCastInst");
  BasicBlock *InitBB = cast<BitCastInst>(AllocPtr)->getParent();
  if (!identifyNodeInit(InitBB, Obj, AllocPtr))
    return false;
  BasicBlock *Succ = getSingleSucc(InitBB);
  if (!Succ)
    return false;
  *TBlock = Succ;
  *FBlock = FB;
  return true;
}

// It identifies the loop below that does FreeNodes in "listHead".
//
//   iterator pos = begin();
//   while (pos != end()) {
//     FreeNode(pos++.node());
//   }
//
// Returns true if IR matches the pattern below starting with
// "FreeNodeLoopZTTBB".
//
// FreeNodeLoopZTTBB:
//   %EndPos
//   %BeginVal = %EndPos->Next
//   if (%BeginVal == %EndPos)
//   then Goto FreeNodeLEndBB
//   else {
//     FreeNodeLoopPreHead:
//        %FH = Obj->List->FreeHead
//        Goto FreeNodeLoopBB
//
//     FreeNodeLoopBB:
//       %FreeHead = phi [ %FreeHead, FreeNodeLoopPreHead ],
//                       [ Node, FreeNodeLoopBB ]
//       %Node = phi [ %BeginVal, FreeNodeLoopPreHead ],
//                   [ %NodeNext, FreeNodeLoopBB ]
//       %NodeNext = Node.next;
//       Node.prev->next = %NodeNext;
//       Node.next->prev = Node.prev;
//       Node.prev = 0;
//       Node.next = %FreeHead;
//       if (%NodeNext == EndPos)
//       then Goto SinkBB
//       else Goto FreeNodeLoopBB
//
//     SinkBB:
//       freeListHead = Node;
//       Goto FreeNodeLEndBB
//   }
//   FreeNodeLEndBB:
//
// Update EndLooppBB when returns true.
//
bool MemManageTransImpl::identifyFreeNodeInLoop(BasicBlock *FreeNodeLoopZTTBB,
                                                Value *Obj, Value *EndPos,
                                                BasicBlock **EndLooppBB) {
  Value *BeginVal = nullptr;
  Value *EndVal = nullptr;
  BasicBlock *FreeNodeLEndBB = nullptr;
  BasicBlock *FreeNodeLoopPreHead = nullptr;
  ICmpInst::Predicate Predic = ICmpInst::ICMP_NE;
  if (!processBBTerminator(FreeNodeLoopZTTBB, &BeginVal, &EndVal,
                           &FreeNodeLEndBB, &FreeNodeLoopPreHead, &Predic))
    return false;
  if (Predic != ICmpInst::ICMP_EQ)
    return false;
  if (!isNodePosNextLoad(BeginVal, EndPos) ||
      !checkInstructionInBlock(BeginVal, FreeNodeLoopZTTBB))
    return false;
  if (EndPos != EndVal)
    return false;

  BasicBlock *FreeNodeLoopBB = getSingleSucc(FreeNodeLoopPreHead);
  if (!FreeNodeLoopBB)
    return false;

  PHINode *Node = nullptr;
  PHINode *FreeHead = nullptr;
  for (auto &I : *FreeNodeLoopBB) {
    if (isa<DbgInfoIntrinsic>(&I))
      continue;
    auto *PHI = dyn_cast<PHINode>(&I);
    if (!PHI)
      break;
    Value *Ptr = PHI->getIncomingValueForBlock(FreeNodeLoopPreHead);
    if (BeginVal == Ptr) {
      if (Node)
        return false;
      Node = PHI;
    } else if (isListFreeHeadLoad(Ptr, Obj) &&
               checkInstructionInBlock(Ptr, FreeNodeLoopPreHead)) {
      if (FreeHead)
        return false;
      FreeHead = PHI;
    } else {
      return false;
    }
  }
  if (!Node || !FreeHead)
    return false;
  Visited.insert(Node);
  Visited.insert(FreeHead);

  Value *NodeNext;
  SmallVector<StoreInst *, 4> StoreVec;
  collectStoreInst(FreeNodeLoopBB, StoreVec);
  if (StoreVec.size() != 4)
    return false;
  StoreInst *SI;

  // All Next and Prev field accesses are from "Node" pointer, which is
  // a PHINode of "FreeNodeLoopBB".
  SI = StoreVec[0];
  if (!isNodePosPrevNext(SI->getPointerOperand(), Node))
    return false;
  if (!isNodePosNextLoad(SI->getValueOperand(), Node))
    return false;
  NodeNext = SI->getValueOperand();
  Visited.insert(SI);

  SI = StoreVec[1];
  if (!isNodePosPrevLoad(SI->getValueOperand(), Node))
    return false;
  if (!isNodePosNextPrev(SI->getPointerOperand(), Node))
    return false;
  Visited.insert(SI);

  SI = StoreVec[2];
  Value *ValOp = SI->getValueOperand();
  if (!isa<Constant>(ValOp) || !cast<Constant>(ValOp)->isNullValue())
    return false;
  if (!isNodePosPrev(SI->getPointerOperand(), Node))
    return false;
  Visited.insert(SI);

  SI = StoreVec[3];
  if (FreeHead != SI->getValueOperand())
    return false;
  if (!isNodePosNext(SI->getPointerOperand(), Node))
    return false;
  Visited.insert(SI);

  Value *LValue = nullptr;
  Value *RValue = nullptr;
  BasicBlock *SinkBB = nullptr;
  BasicBlock *FBlock = nullptr;
  ICmpInst::Predicate Predi = ICmpInst::ICMP_NE;
  if (!processBBTerminator(FreeNodeLoopBB, &LValue, &RValue, &SinkBB, &FBlock,
                           &Predi))
    return false;
  if (Predi != ICmpInst::ICMP_EQ)
    return false;
  if (NodeNext != LValue)
    return false;
  if (EndPos != RValue)
    return false;
  if (FBlock != FreeNodeLoopBB)
    return false;

  SmallVector<StoreInst *, 1> SinkStoreVec;
  collectStoreInst(SinkBB, SinkStoreVec);
  if (SinkStoreVec.size() != 1)
    return false;

  SI = SinkStoreVec[0];
  if (Node != SI->getValueOperand())
    return false;
  if (!isListFreeHeadAddr(SI->getPointerOperand(), Obj))
    return false;
  Visited.insert(SI);

  BasicBlock *Succ = getSingleSucc(SinkBB);
  if (!Succ)
    return false;
  if (Succ != FreeNodeLEndBB)
    return false;
  *EndLooppBB = Succ;
  return true;
}

// Returns true if IR matches the pattern below before "I" instruction.
//
//  %GEP = getelementptr %"ObjStr", %"ObjStr"* %ObjBlkPtr, i64 %LoopCountPHI
//  %BC2 = bitcast %"ObjStr"* %81 to void (%"ObjStr"*)***
//  %Ld = load void (%"ObjStr"*)**, void (%"ObjStr"*)*** %BC2, align 8
//  %BC1 = bitcast void (%"ObjStr"*)** %Ld to i8*
//  %TCall = tail call i1 @llvm.type.test(i8* %BC1, metadata !"some_data")
//  tail call void @llvm.assume(i1 %TCall)
//  %II = load void (%"ObjStr"*)*, void (%"ObjStr"*)** %BC2, align 8
//  tail call void @ObjDestCall(%"ObjStr"* %GEP)
bool MemManageTransImpl::identifyStrObjDtorCall(Instruction *I,
                                                Value *ObjBlkPtr,
                                                Value *LoopCountPHI) {
  auto *ObjDestCall =
      dyn_cast_or_null<CallInst>(I->getPrevNonDebugInstruction());
  if (!ObjDestCall)
    return false;
  Instruction *II = ObjDestCall->getPrevNonDebugInstruction();
  if (II && isa<LoadInst>(II) && II->hasNUses(0)) {
    // Ignore checking of PointerOPerand of II if II doesn't have any uses.
    Visited.insert(II);
    II = II->getPrevNonDebugInstruction();
  }
  auto *ACall = dyn_cast_or_null<IntrinsicInst>(II);
  if (!ACall || ACall->getIntrinsicID() != Intrinsic::assume)
    return false;
  Visited.insert(ACall);
  auto *TCall = dyn_cast<IntrinsicInst>(ACall->getArgOperand(0));
  if (!TCall || TCall->getIntrinsicID() != Intrinsic::type_test)
    return false;
  Visited.insert(TCall);
  auto *BC1 = dyn_cast<BitCastInst>(TCall->getArgOperand(0));
  if (!BC1)
    return false;
  Visited.insert(BC1);
  auto *Ld = dyn_cast<LoadInst>(BC1->getOperand(0));
  if (!Ld)
    return false;
  Visited.insert(Ld);
  auto *BC2 = dyn_cast<BitCastInst>(Ld->getPointerOperand());
  if (!BC2)
    return false;
  Visited.insert(BC2);
  auto *GEP = dyn_cast<GetElementPtrInst>(BC2->getOperand(0));
  if (!GEP || GEP->getNumIndices() != 1)
    return false;
  Visited.insert(GEP);
  if (ObjBlkPtr != GEP->getPointerOperand())
    return false;
  if (LoopCountPHI != GEP->getOperand(1))
    return false;
  if (ObjDestCall->arg_size() != 1)
    return false;
  Value *ArgOp = ObjDestCall->getArgOperand(0);
  if (ArgOp != GEP)
    return false;
  Type *ObjTy = nullptr;
  if (auto *PTy = dyn_cast<PointerType>(ArgOp->getType()))
    ObjTy = PTy->getElementType();
  auto Cand = getCurrentCandidate();
  StructType *StrObjType = Cand->getStringObjectType();
  if (!ObjTy || ObjTy != StrObjType)
    return false;

  // TODO: Makes sure ObjDestCall is a destructor.

  Visited.insert(ObjDestCall);
  return true;
}

// Returns true if IR matches the pattern below.
//  BB:
//     ...
//     tail call void @ObjDestCall(%"ObjStr"* %GEP)
//     %RemovedObjs = add nuw i16 %RemObjsPHI, 1
//     %BlkSize = load i16, i16* %RABPtr->BlockSize, align 2
//     %WideBlkSize = zext i16 %BlkSize to i64
//     br label SomeBB
//
bool MemManageTransImpl::identifyRemoveStrObj(
    BasicBlock *BB, Value *RABPtr, Value *ObjBlkPtr, Value *LoopCountPHI,
    Value *RemObjsPHI, Value **WideBlkSizePtr, Value **BlkSizePtr,
    Value **RemovedObjsPtr) {
  Instruction *BI = BB->getTerminator();
  auto *WideBlkSize =
      dyn_cast_or_null<ZExtInst>(BI->getPrevNonDebugInstruction());
  if (!WideBlkSize)
    return false;
  auto *BlkSize =
      dyn_cast_or_null<LoadInst>(WideBlkSize->getPrevNonDebugInstruction());
  if (!BlkSize || !isBlockSizeLoadFromRAB(BlkSize, RABPtr))
    return false;
  Instruction *RemovedObjs = BlkSize->getPrevNonDebugInstruction();
  Value *AddOp = nullptr;
  if (!RemovedObjs || !isIncrementByOne(RemovedObjs, &AddOp))
    return false;
  if (AddOp != RemObjsPHI)
    return false;
  if (!identifyStrObjDtorCall(RemovedObjs, ObjBlkPtr, LoopCountPHI))
    return false;
  Visited.insert(WideBlkSize);
  Visited.insert(BlkSize);
  Visited.insert(RemovedObjs);
  *WideBlkSizePtr = WideBlkSize;
  *BlkSizePtr = BlkSize;
  *RemovedObjsPtr = RemovedObjs;
  return true;
}

// Returns true if IR matches the pattern below starting with LoopH.
//
// PreHead:
//  br label %LoopH
//
// LoopH:
//  br label %Succ
//
// Succ:
//  %LoopCountPHI = phi i64 [ 0, %Pred ], [ %LatchLValue, %LoopH ]
//  %BlkSizePHI = phi i16 [ %BSize, %Pred ], [ %BSizePHI, %LoopH ]
//  %RemObjsPHI = phi i16 [ 0, %Pred ], [ %RemPHI, %LoopH ]
//  %BlockSize = load i16, i16* %RABPtr->BlockSize
//  %ic0 = icmp ult i16 %RemObjsPHI, %BlockSize
//  br i1 %ic0, label %CheckOneBB, label %LoopExitBB
//
// CheckOneBB:
//  %ObjBlkLoad = load %"ObjStr"*, %"ObjStr"** %RABPtr
//  %GEP = getelementptr %"ObjStr", %"ObjStr"* %ObjBlkLoad, i64 %LoopCountPHI
//  %BC1 = bitcast %"ObjStr"* %GEP to %"NextBlock"*
//  %CondOneRValue = zext i16 %BlkSizePHI to i64
//  %ic1 = icmp ult i64 %LoopCountPHI, %CondOneRValue
//  br i1 %ic1, label %CondOneTBB, label %CondThreeTBB
//
// CondOneTBB:
//  %GEP2 = getelementptr %"NextBlock", %"NextBlock"* %BC1, i64 0, i32 1
//  %CondTwoLValue = load i32, i32* %GEP2, align 4
//  %ic2 = icmp eq i32 %CondTwoLValue, -2228259
//  br i1 %ic2, label %CondTwoTBB, label %CondThreeTBB
//
// CondTwoTBB:
//  %GEP3 = getelementptr %"NextBlock", %"NextBlock"* %BC1, i64 0, i32 0
//  %CondThreeLValue = load i16, i16* %GEP3, align 4
//  %ic3 = icmp ugt i16 %CondThreeLValue, %BlkSizePHI
//  br i1 %ic3, label %CondThreeTBB, label %CondThreeFBB
//
// CondThreeTBB:
//  identifyRemoveStrObj(..., &WideBlkSize, &BlkSize, &RemovedObjs)
//  br label %CondThreeFBB
//
// CondThreeFBB:
//  %WideBSizePHI = phi [ %WideBlkSize, %CondThreeTBB ],
//                      [ %CondOneRValue, %CondTwoTBB ]
//  %BSizePHI = phi [ %BlkSize, %CondThreeTBB ], [ %BlkSizePHI, %CondTwoTBB ]
//  %RemPHI = phi [ %RemovedObjs, %CondThreeTBB ], [ %RemObjsPHI, %CondTwoTBB ]
//  %LatchLValue = add nuw nsw i64 %LoopCountPHI, 1
//  %ic4 = icmp ult i64 %LatchLValue, %WideBSizePHI
//  br i1 %ic4, label %LoopH, label %LoopExitBB

bool MemManageTransImpl::identifyRABDtorInnerLoop(BasicBlock *LoopH,
                                                  BasicBlock *PreHead,
                                                  Value *ThisObj, Value *BSize,
                                                  Value *RABPtr,
                                                  BasicBlock **LoopEndBB) {

  BasicBlock *Pred = PreHead;
  // Skip almost empty BB
  BasicBlock *Succ = getSingleSucc(LoopH);
  if (Succ) {
    Pred = LoopH;
    LoopH = Succ;
  }

  PHINode *LoopCountPHI = nullptr;
  PHINode *BlkSizePHI = nullptr;
  PHINode *RemObjsPHI = nullptr;
  for (auto &I : *LoopH) {
    if (isa<DbgInfoIntrinsic>(&I))
      continue;
    auto *PHI = dyn_cast<PHINode>(&I);
    if (!PHI)
      break;
    Value *Ptr = PHI->getIncomingValueForBlock(Pred);
    if (isa<ConstantInt>(Ptr) && cast<ConstantInt>(Ptr)->isZeroValue()) {
      Type *PHITy = PHI->getType();
      if (PHITy->isIntegerTy(16)) {
        if (RemObjsPHI)
          return false;
        RemObjsPHI = PHI;
      } else if (PHITy->isIntegerTy(64)) {
        if (LoopCountPHI)
          return false;
        LoopCountPHI = PHI;
      } else {
        return false;
      }
    } else if (BSize == Ptr) {
      if (BlkSizePHI)
        return false;
      BlkSizePHI = PHI;
    } else {
      return false;
    }
  }
  if (!BlkSizePHI || !LoopCountPHI || !RemObjsPHI)
    return false;
  Visited.insert(BlkSizePHI);
  Visited.insert(LoopCountPHI);
  Visited.insert(RemObjsPHI);

  Value *RemovedObj = nullptr;
  Value *BlockSize = nullptr;
  BasicBlock *CheckOneBB = nullptr;
  BasicBlock *LoopExitBB = nullptr;
  ICmpInst::Predicate LoopP = ICmpInst::ICMP_NE;
  if (!processBBTerminator(LoopH, &RemovedObj, &BlockSize, &CheckOneBB,
                           &LoopExitBB, &LoopP))
    return false;
  if (LoopP != ICmpInst::ICMP_ULT)
    return false;
  if (RemObjsPHI != RemovedObj)
    return false;
  if (!isObjectCountLoadFromRAB(BlockSize, RABPtr) ||
      !checkInstructionInBlock(BlockSize, LoopH))
    return false;

  LoadInst *ObjBlkLoad = getFirstLoadInst(CheckOneBB);
  if (!ObjBlkLoad || !isObjectBlockLoadFromRAB(ObjBlkLoad, RABPtr))
    return false;
  Value *CondOneLValue = nullptr;
  Value *CondOneRValue = nullptr;
  BasicBlock *CondOneTBB = nullptr;
  BasicBlock *CondOneFBB = nullptr;
  ICmpInst::Predicate CondOneP = ICmpInst::ICMP_NE;
  if (!processBBTerminator(CheckOneBB, &CondOneLValue, &CondOneRValue,
                           &CondOneTBB, &CondOneFBB, &CondOneP))
    return false;
  if (CondOneP != ICmpInst::ICMP_ULT)
    return false;
  // if (CondOneLValue)
  auto *ZExt = dyn_cast<ZExtInst>(CondOneRValue);
  if (!ZExt)
    return false;
  if (BlkSizePHI != ZExt->getOperand(0))
    return false;
  if (LoopCountPHI != CondOneLValue)
    return false;
  Visited.insert(ZExt);
  Visited.insert(ObjBlkLoad);

  Value *CondTwoLValue = nullptr;
  Value *CondTwoRValue = nullptr;
  BasicBlock *CondTwoTBB = nullptr;
  BasicBlock *CondTwoFBB = nullptr;
  ICmpInst::Predicate CondTwoP = ICmpInst::ICMP_NE;
  if (!processBBTerminator(CondOneTBB, &CondTwoLValue, &CondTwoRValue,
                           &CondTwoTBB, &CondTwoFBB, &CondTwoP))
    return false;
  if (CondTwoP != ICmpInst::ICMP_EQ)
    return false;
  auto *CInt = dyn_cast<ConstantInt>(CondTwoRValue);
  if (!CInt || CInt->getLimitedValue() != ValidObjStamp)
    return false;
  Value *BaseAddr = nullptr;
  Value *IndexAddr = nullptr;
  int32_t Idx = 0;
  if (!isNextBlockFieldLoad(CondTwoLValue, &BaseAddr, &IndexAddr, &Idx) ||
      !checkInstructionInBlock(CondTwoLValue, CondOneTBB))
    return false;
  if (Idx != 1 || BaseAddr != ObjBlkLoad || IndexAddr != LoopCountPHI)
    return false;

  Value *CondThreeLValue = nullptr;
  Value *CondThreeRValue = nullptr;
  BasicBlock *CondThreeTBB = nullptr;
  BasicBlock *CondThreeFBB = nullptr;
  ICmpInst::Predicate CondThreeP = ICmpInst::ICMP_NE;
  if (!processBBTerminator(CondTwoTBB, &CondThreeLValue, &CondThreeRValue,
                           &CondThreeTBB, &CondThreeFBB, &CondThreeP))
    return false;
  if (CondThreeP != ICmpInst::ICMP_UGT)
    return false;
  if (!isNextBlockFieldLoad(CondThreeLValue, &BaseAddr, &IndexAddr, &Idx) ||
      !checkInstructionInBlock(CondThreeLValue, CondTwoTBB))
    return false;
  if (Idx != 0 || BaseAddr != ObjBlkLoad || IndexAddr != LoopCountPHI)
    return false;
  if (CondThreeRValue != BlkSizePHI)
    return false;

  if (CondThreeTBB != CondOneFBB || CondThreeTBB != CondTwoFBB)
    return false;
  Value *WideBlkSize = nullptr;
  Value *BlkSize = nullptr;
  Value *RemovedObjs = nullptr;
  if (!identifyRemoveStrObj(CondThreeTBB, RABPtr, ObjBlkLoad, LoopCountPHI,
                            RemObjsPHI, &WideBlkSize, &BlkSize, &RemovedObjs))
    return false;
  BasicBlock *SSS = getSingleSucc(CondThreeTBB);
  if (!SSS || SSS != CondThreeFBB)
    return false;

  PHINode *WideBSizePHI = nullptr;
  PHINode *BSizePHI = nullptr;
  PHINode *RemPHI = nullptr;
  for (auto &I : *CondThreeFBB) {
    if (isa<DbgInfoIntrinsic>(&I))
      continue;
    auto *PHI = dyn_cast<PHINode>(&I);
    if (!PHI)
      break;
    Value *Op1 = PHI->getIncomingValueForBlock(CondTwoTBB);
    Value *Op2 = PHI->getIncomingValueForBlock(CondThreeTBB);
    if (Op1 == CondOneRValue) {
      if (WideBSizePHI)
        return false;
      if (Op2 != WideBlkSize)
        return false;
      WideBSizePHI = PHI;
    } else if (Op1 == BlkSizePHI) {
      if (BSizePHI)
        return false;
      if (Op2 != BlkSize)
        return false;
      BSizePHI = PHI;
    } else if (Op1 == RemObjsPHI) {
      if (RemPHI)
        return false;
      if (Op2 != RemovedObjs)
        return false;
      RemPHI = PHI;
    } else {
      return false;
    }
  }
  if (!WideBSizePHI || !BSizePHI || !RemPHI)
    return false;
  Visited.insert(WideBSizePHI);
  Visited.insert(BSizePHI);
  Visited.insert(RemPHI);

  Value *LatchLValue = nullptr;
  Value *LatchRValue = nullptr;
  BasicBlock *LatchTBB = nullptr;
  BasicBlock *LatchFBB = nullptr;
  ICmpInst::Predicate LatchP = ICmpInst::ICMP_NE;
  if (!processBBTerminator(CondThreeFBB, &LatchLValue, &LatchRValue, &LatchTBB,
                           &LatchFBB, &LatchP))
    return false;
  if (LatchP != ICmpInst::ICMP_ULT)
    return false;

  Value *AddOp = nullptr;
  if (!isIncrementByOne(LatchLValue, &AddOp) ||
      !checkInstructionInBlock(LatchLValue, CondThreeFBB))
    return false;
  if (AddOp != LoopCountPHI)
    return false;
  if (LatchRValue != WideBSizePHI)
    return false;
  if (LatchTBB != LoopH)
    return false;
  if (LatchFBB != LoopExitBB)
    return false;

  *LoopEndBB = LoopExitBB;
  return true;
}

// Returns true if IR matches the pattern below starting with PreHead.
//
// PreHead:
//  br label %LoopH
//
// LoopH:
//  %ListNodeNext = phi [ %ListBegin, %PreHead ], [ %NodeNextV, %LoopLatch ]
//  %GEP = getelementptr %"Node", %"Node"* %ListNodeNext, i64 0, i32 0
//  %RABPtr = load %GEP
//  %ic0 = icmp eq %RABPtr, null
//  br i1 %ic0, label %LoopLatch, label %BlkSizeCheckBB
//
// BlkSizeCheckBB:
//  %BSize = load i16, i16* %RABPtr->BlockSize
//  %ic1 = icmp eq i16 %BSize, 0
//  br i1 %ic1, label %DeallocBB, label %InnerLoop
//
// InnerLoop:
//  identifyRABDtorInnerLoop();
//  Goto DeallocBB
//
// DeallocBB:
//  %ObjBlk = load %"Str"*, %"Str"** %RABPtr->ObjBlk
//  %ic2 = icmp eq %"Str"* %ObjBlk, null
//  br i1 %ic2, label %RABDealloc, label %ObjDealloc
//
// ObjDealloc:
//  identifyDeallocCall(ObjBlk)
//  Goto EndBB
//
// RABDealloc:
//  identifyDeallocCall(RABPtr)
//  Goro LoopLatch
//
// LoopLatch:
//  %NodeNextV = load %Node"*, %"Node"** %ListNodeNext->Next
//  %ic3 = icmp eq %"Node"* %NodeNextV, %HeadV
//  br i1 %ic3, label %OuterLoopEndBB, label %LoopH
//
bool MemManageTransImpl::identifyRABDtorOuterLoop(BasicBlock *PreHead,
                                                  Value *Obj, Value *ListBegin,
                                                  BasicBlock **OuterLoopEndBB) {
  BasicBlock *LoopH = getSingleSucc(PreHead);
  if (!LoopH)
    return false;
  auto *ListNodeNext = dyn_cast<PHINode>(getFirstNonDbg(LoopH));
  if (!ListNodeNext)
    return false;
  if (ListBegin != ListNodeNext->getIncomingValueForBlock(PreHead))
    return false;

  // Check for "RABPtr == nullptr"
  Value *RABPtr = nullptr;
  Value *RValue = nullptr;
  BasicBlock *LoopLatch = nullptr;
  BasicBlock *BlkSizeCheckBB = nullptr;
  ICmpInst::Predicate Predi = ICmpInst::ICMP_NE;
  if (!processBBTerminator(LoopH, &RABPtr, &RValue, &LoopLatch, &BlkSizeCheckBB,
                           &Predi))
    return false;
  if (Predi != ICmpInst::ICMP_EQ)
    return false;
  if (!isa<Constant>(RValue) || !cast<Constant>(RValue)->isNullValue())
    return false;
  if (!isNodePosReusableArenaBlockLoad(RABPtr, ListNodeNext) ||
      !checkInstructionInBlock(RABPtr, LoopH))
    return false;
  Visited.insert(ListNodeNext);

  // Check for BlockSize == 0
  Value *BSize = nullptr;
  Value *RVal = nullptr;
  BasicBlock *DeallocBB = nullptr;
  BasicBlock *InnerLoop = nullptr;
  ICmpInst::Predicate Pred = ICmpInst::ICMP_NE;
  if (!processBBTerminator(BlkSizeCheckBB, &BSize, &RVal, &DeallocBB,
                           &InnerLoop, &Pred))
    return false;
  if (Pred != ICmpInst::ICMP_EQ)
    return false;
  if (!isa<ConstantInt>(RVal) || !cast<ConstantInt>(RVal)->isZeroValue())
    return false;
  if (!isBlockSizeLoadFromRAB(BSize, RABPtr) ||
      !checkInstructionInBlock(BSize, BlkSizeCheckBB))
    return false;

  // Check for inner loop.
  BasicBlock *LoopEndBB = nullptr;
  if (!identifyRABDtorInnerLoop(InnerLoop, BlkSizeCheckBB, Obj, BSize, RABPtr,
                                &LoopEndBB))
    return false;
  if (LoopEndBB != DeallocBB)
    return false;

  Value *ObjBlk = nullptr;
  Value *NullV = nullptr;
  BasicBlock *RABDealloc = nullptr;
  BasicBlock *ObjDealloc = nullptr;
  ICmpInst::Predicate PredO = ICmpInst::ICMP_NE;
  if (!processBBTerminator(DeallocBB, &ObjBlk, &NullV, &RABDealloc, &ObjDealloc,
                           &PredO))
    return false;
  if (PredO != ICmpInst::ICMP_EQ)
    return false;
  if (!isa<Constant>(NullV) || !cast<Constant>(NullV)->isNullValue())
    return false;
  if (!isObjectBlockLoadFromRAB(ObjBlk, RABPtr) ||
      !checkInstructionInBlock(ObjBlk, DeallocBB))
    return false;

  // Deallocate ObjBlk and RABPtr
  BasicBlock *EndBB = nullptr;
  if (!identifyDeallocCall(ObjDealloc, Obj, ObjBlk, &EndBB, RABPtr))
    return false;
  if (EndBB != RABDealloc)
    return false;
  BasicBlock *EndDealloc = nullptr;
  if (!identifyDeallocCall(EndBB, Obj, RABPtr, &EndDealloc))
    return false;
  // Skip empty block if there is any.
  BasicBlock *Succ = getSingleSucc(EndDealloc);
  if (Succ)
    EndDealloc = Succ;

  if (EndDealloc != LoopLatch)
    return false;
  // Check OuterLoop's Latch
  Value *NodeNextV = nullptr;
  Value *HeadV = nullptr;
  BasicBlock *LoopEnd = nullptr;
  BasicBlock *LoopBegin = nullptr;
  ICmpInst::Predicate Pre = ICmpInst::ICMP_NE;
  if (!processBBTerminator(LoopLatch, &NodeNextV, &HeadV, &LoopEnd, &LoopBegin,
                           &Pre))
    return false;
  if (Pre != ICmpInst::ICMP_EQ)
    return false;
  if (!isListHeadLoad(HeadV, Obj))
    return false;
  if (!isNodePosNextLoad(NodeNextV, ListNodeNext) ||
      !checkInstructionInBlock(NodeNextV, LoopLatch))
    return false;
  if (LoopBegin != LoopH)
    return false;
  if (NodeNextV != ListNodeNext->getIncomingValueForBlock(EndDealloc))
    return false;

  *OuterLoopEndBB = LoopEnd;
  return true;
}

// Returns true if "F" has the functionality below.
//
// foreach ReusableAreanaBlock  {
//   size_type removedObjects = 0;
//   for (size_type i = 0;
//       i < this->m_blockSize &&
//       removedObjects < this->m_objectCount; ++i) {
//     NextBlock* const pStruct = NextBlock::cast(&this->m_objectBlock[i]);
//     if ( isOccupiedBlock(pStruct) ) {
//       this->m_objectBlock[i].~ObjectType();  // Dtor of StrObj
//       ++removedObjects;
//     }
//   }
// }
// iterator pos = begin();
// while (pos != end()) {
//   freeNode(pos++.node());
// }
//
bool MemManageTransImpl::recognizeReset(Function *F) {
  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANS, {
    dbgs() << "   Recognizing Reset Functionality " << F->getName() << "\n";
  });
  Visited.clear();
  Argument *ThisObj = &*F->arg_begin();
  BasicBlock *CreatedHeadBB = nullptr;
  BasicBlock *RABDtorLoopZTTBB = nullptr;
  Value *NodePtr = nullptr;
  if (!identifyListHead(&F->getEntryBlock(), ThisObj, &CreatedHeadBB,
                        &RABDtorLoopZTTBB, &NodePtr))
    return false;

  // Check ZTT for Outerloop
  Value *ListBegin = nullptr;
  Value *LHead = nullptr;
  BasicBlock *EmptyBB = nullptr;
  BasicBlock *RABDtorLoopPreHead = nullptr;
  ICmpInst::Predicate Predi = ICmpInst::ICMP_NE;
  if (!processBBTerminator(RABDtorLoopZTTBB, &ListBegin, &LHead, &EmptyBB,
                           &RABDtorLoopPreHead, &Predi))
    return false;
  if (Predi != ICmpInst::ICMP_EQ)
    return false;
  if (!isListHeadLoad(LHead, ThisObj))
    return false;
  if (!isListBegin(ListBegin, ThisObj))
    return false;

  // Check for the 1st loop
  BasicBlock *RABDtorLoopEnd = nullptr;
  if (!identifyRABDtorOuterLoop(RABDtorLoopPreHead, ThisObj, ListBegin,
                                &RABDtorLoopEnd))
    return false;

  BasicBlock *Succ = getSingleSucc(RABDtorLoopEnd);
  if (!Succ)
    return false;
  if (Succ != EmptyBB)
    return false;
  // So, RABDtorLoopEnd and RABDtorLoopZTTBB are predecessors.
  // RABDtorLoopEnd:
  //   %V1 = load ThisObj->listHead
  //   Goto Succ;
  //
  // Succ:
  // %ListHeadPHI = phi [ %V1, %RABDtorLoopEnd ],
  //                    [ %ListBegin,  RABDtorLoopZTTBB ]
  auto *ListHeadPHI = dyn_cast<PHINode>(getFirstNonDbg(Succ));
  if (!ListHeadPHI)
    return false;
  if (ListBegin != ListHeadPHI->getIncomingValueForBlock(RABDtorLoopZTTBB))
    return false;
  Value *V1 = ListHeadPHI->getIncomingValueForBlock(RABDtorLoopEnd);
  if (!isListHeadLoad(V1, ThisObj) ||
      !checkInstructionInBlock(V1, RABDtorLoopEnd))
    return false;
  Visited.insert(ListHeadPHI);

  BasicBlock *RetBB = nullptr;
  BasicBlock *FreeNodeLoopZTTBB = nullptr;
  if (!identifyCreateListHead(Succ, ThisObj, ListHeadPHI, &RetBB,
                              &FreeNodeLoopZTTBB))
    return false;

  auto *Ret = dyn_cast<ReturnInst>(RetBB->getTerminator());
  if (!Ret)
    return false;

  // Succ and NodePtrBB are predecessors of FreeNodeLoopZTTBB.
  // FreeNodeLoopZTTBB:
  //   %NodePHI = phi [ %NodePtr, %NodePtrBB ], [ %ListHeadPHI, %Succ ]
  auto *NodePHI = dyn_cast<PHINode>(getFirstNonDbg(FreeNodeLoopZTTBB));
  if (!NodePHI)
    return false;
  if (CreatedHeadBB != FreeNodeLoopZTTBB)
    return false;
  if (ListHeadPHI != NodePHI->getIncomingValueForBlock(Succ))
    return false;
  assert(isa<BitCastInst>(NodePtr) && "Expected BitCastInst");
  BasicBlock *NodePtrBB = cast<BitCastInst>(NodePtr)->getParent();
  if (NodePtr != NodePHI->getIncomingValueForBlock(NodePtrBB))
    return false;
  Visited.insert(NodePHI);

  // Check for the 2nd loop
  BasicBlock *FreeNodeLoopEndBB = nullptr;
  if (!identifyFreeNodeInLoop(FreeNodeLoopZTTBB, ThisObj, NodePHI,
                              &FreeNodeLoopEndBB))
    return false;
  if (RetBB != FreeNodeLoopEndBB)
    return false;

  if (!verifyAllInstsProcessed(F))
    return false;

  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANS, {
    dbgs() << "   Recognized Reset: " << F->getName() << "\n";
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

  // Check functionality of CommitAllocation.
  Function *CAllocation = FunctionalityMap[CommitAllocation];
  if (!CAllocation)
    return false;
  if (!recognizeCommitAllocation(CAllocation))
    return false;

  // Check functionality of Destructor.
  Function *Dtor = FunctionalityMap[Destructor];
  if (!Dtor)
    return false;
  if (!recognizeDestructor(Dtor))
    return false;

  // Check functionality of Destructor.
  Function *ResetF = FunctionalityMap[Reset];
  if (!ResetF)
    return false;
  if (!recognizeReset(ResetF))
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
