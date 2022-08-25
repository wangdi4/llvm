//===------------ MemManageTransOP.cpp - DTransMemManageTransPass ---------===//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans Initial Memory Management Transformation
// for opaque pointers.
//
// Detects memory pool allocator class by analyzing member functions of the
// class like allocateBlock, destroyObject, commitAllocationObject, etc.
// Increases the size of block (i.e number of objects allocated each time) if
// there are no legality issues.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/MemManageTransOP.h"

#include "Intel_DTrans/Analysis/DTransAllocCollector.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#include "Intel_DTrans/Transforms/MemManageInfoOPImpl.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/IR/AssemblyAnnotationWriter.h"
#include "llvm/Support/FormattedStream.h"

#define DTRANS_MEMMANAGETRANSOP "dtrans-memmanagetransop"

using namespace llvm;
using namespace dtransOP;

// MemManageTrans is triggered only when SOAToAOS is triggered.
// This option is used to ignore SOAToAOS heuristic for testing.
static cl::opt<bool>
    MemManageIgnoreSOAHeur("dtrans-memmanageop-ignore-soa-heur",
                           cl::init(false), cl::ReallyHidden);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// This pass requires that several functions be matched in their entirety to
// trigger the transformation. This flag is used for LIT testing to run the
// subsequent 'recognize*' function, even if a prior 'recognize*' function
// failed to match the required function. This facilitates creating negative
// tests, which otherwise would need to be generated with a complete valid match
// of each prior function to generate the negative variant of a function being
// checked.
static cl::opt<bool>
    MemManageContinueAfterMismatch("dtrans-memmanageop-continue-after-mismatch",
                                   cl::init(false), cl::ReallyHidden);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Check whether a mismatch during function recognition should continue to check
// functions following one recognition failure. In production, this should
// always be 'false', but for testing negative tests this can help put multiple
// negative versions into a single LIT test.
static bool continueAfterMismatch() {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  return MemManageContinueAfterMismatch;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  return false;
}

namespace {
DTransFunctionType *getDTransFunctionType(TypeMetadataReader &MDReader,
                                          Function *F) {
  return dyn_cast_or_null<DTransFunctionType>(MDReader.getDTransTypeFromMD(F));
}

DTransType *getArgType(TypeMetadataReader &MDReader, Function *F,
                       uint32_t ArgIdx) {
  DTransFunctionType *DFnTy = getDTransFunctionType(MDReader, F);
  if (!DFnTy)
    return nullptr;

  if (ArgIdx >= DFnTy->getNumArgs())
    return nullptr;

  return DFnTy->getArgType(ArgIdx);
}

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

// Helper to annotate which instructions were visited when trying to recognize
// the functionality of one of the functions.
class VisitedAnnotationWriter : public AssemblyAnnotationWriter {
public:
  VisitedAnnotationWriter(std::set<Instruction *> &Visited)
      : Visited(Visited.begin(), Visited.end()) {}

  // printInfoComment - Emit a comment to the right of an instruction to
  // identify which instructions were visited during the recognition.
  void printInfoComment(const Value &V, formatted_raw_ostream &OS) override {
    if (auto *I = dyn_cast<Instruction>(&V))
      if (Visited.count(I))
        OS << " ; [Visited]";
      else
        OS << " ; [Not visited]";
  }

private:
  std::set<const Instruction *> Visited;
};

#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

class MemManageTransImpl {

  constexpr static int MaxNumCandidates = 1;

  // This constant value is used by the benchmark application to check if entry
  // in a Block is occupied or not
  constexpr static uint32_t ValidObjStamp = 0xffddffdd;

public:
  MemManageTransImpl(DTransTypeManager &TM, TypeMetadataReader &MDReader,
                     llvm::dtransOP::MemManageTransOPPass::MemTLITy GetTLI)
      : TM(TM), MDReader(MDReader), GetTLI(GetTLI), DTAC(MDReader, GetTLI){};

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

  bool run(Module &M);

private:
  DTransTypeManager &TM;
  TypeMetadataReader &MDReader;
  llvm::dtransOP::MemManageTransOPPass::MemTLITy GetTLI;
  DTransAllocCollector DTAC;

  // List of candidates. For now, only one candidate is allowed.
  SmallVector<MemManageCandidateInfo *, MaxNumCandidates> Candidates;

  // Map of MemManageFKind to Function. Only one function is allowed
  // for each MemManageFKind.
  DenseMap<unsigned, Function *> FunctionalityMap;

  // For each of the categorized functions, keep the DTransFunctionType of the
  // signature for use when matching the functionality of the Function.
  DenseMap<Function *, DTransFunctionType *> FunctionalityTypeMap;

  // Instruction that stores 'blockSize' variable in ArenaAllocator.
  // This instruction will be changed during the transformation
  // to change the value used for the block size.
  StoreInst *BlockSizeStoreInst = nullptr;

  // This set is used to track instructions that are processed. This needs to be
  // cleared at the start of each 'recognize*' function.
  std::set<Instruction *> Visited;

  // This StructType captured to this variable is not really used in interface
  // functions. It is just used as a GEP indexing type. NextBlockTy helps to
  // make sure that the same StructType is used by all interface functions.
  StructType *NextBlockTy = nullptr;

  // Set of Interface functions and their users.
  SmallPtrSet<Function *, 32> RelatedFunctions;

  bool gatherCandidates(Module &M, FunctionTypeResolver &TypeResolver);
  bool analyzeCandidates(Module &M);
  bool isUsedOnlyInUnusedVTable(Value *);
  bool checkInterfaceFunctions();
  bool checkCallSiteRestrictions();
  bool categorizeFunctions();
  bool recognizeFunctions();
  bool recognizeGetMemManager(Function *);
  bool recognizeConstructor(Function *);
  bool recognizeAllocateBlock(Function *);
  bool checkConstantSize(Value *, int64_t);
  bool checkSizeValue(Value *, int64_t, Value *);
  bool processBBTerminator(BasicBlock *, Value **, Value **, BasicBlock **,
                           BasicBlock **, ICmpInst::Predicate *);
  bool isStrObjPtrType(DTransType *Ty);
  BasicBlock *getSingleSucc(BasicBlock *BB);
  void collectStoreInst(BasicBlock *, SmallVectorImpl<StoreInst *> &);
  Instruction *getFirstNonDbg(BasicBlock *);
  bool checkInstructionInBlock(Value *, BasicBlock *);
  bool isUnreachableOK(BasicBlock *);
  bool isIncrementByOne(Value *, Value **);
  bool isNextBlockObjBlkAddress(Value *, GetElementPtrInst **, int32_t *);
  bool isNextBlockFieldAccess(Value *, Value **, Value **, int32_t *);
  bool isNextBlockFieldLoad(Value *, Value **, Value **, int32_t *);

  bool getAllocDeallocCommonSucc(Instruction *, Instruction *, BasicBlock **,
                                 BasicBlock **);
  bool identifyDevirtChecks(BasicBlock *, Value *, Function **, BasicBlock **,
                            BasicBlock **, Value *RABPtr = nullptr);
  bool identifyAllocCall(BasicBlock *BB, Value *Obj, Value **AllocPtr,
                         Value **NumOfElems, BasicBlock **UnBB);
  bool identifyDeallocCall(BasicBlock *, Value *, Value *, BasicBlock **,
                           Value *);
  bool identifyCheckAndAllocNode(BasicBlock *, Value *, BasicBlock **,
                                 BasicBlock **, Value **, Value **, bool);

  bool identifyNodeInit(BasicBlock *, Value *, Value *);
  bool identifyListHead(BasicBlock *, Value *, BasicBlock **, BasicBlock **,
                        Value **, Value **);
  bool identifyGetListHead(BasicBlock *, Value *, BasicBlock **, Value **);

  bool identifyListEmpty(BasicBlock *, Value *, BasicBlock **, BasicBlock **,
                         LoadInst **);
  bool identifyBlockAvailable(BasicBlock *, Value *, BasicBlock **,
                              BasicBlock **, Value *BB = nullptr);
  bool identifyCreate(BasicBlock *, Value *, BasicBlock **, Value **);
  bool identifyArenaBlockInit(BasicBlock *, Value *, Value *, Value *,
                              BasicBlock **);
  bool identifyRABAllocateBlock(BasicBlock *, Value *);
  bool identifyPushFront(BasicBlock *, Value *, Value *, BasicBlock *);
  bool identifyPushAtPos(SmallVectorImpl<StoreInst *> &, Value *, Value *,
                         Value *, Value *, Value *);

  bool identifyUncommittedBlock(BasicBlock *, Value *, Value **, Value **,
                                BasicBlock **, BasicBlock **);

  bool getGEPBaseAddrIndex(Value *V, Value **BaseOp, int32_t *Idx);
  bool isArenaAllocatorAddr(Value *V, Value *Obj);
  bool isGEPLessArenaAllocatorAddr(Value *V, Value *Obj);
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
  bool isListFrontNodeArenaBlockAddr(Value *V, Value *Obj, Value *NodePtr);
  bool isFrontNodeObjectCountAddr(Value *V, Value *Obj, Value *NodePtr);
  bool isFrontNodeObjectCountLoad(Value *V, Value *Obj, Value *NodePtr);
  bool isFrontNodeBlockSizeLoad(Value *V, Value *Obj, Value *NodePtr);

  bool isNodePosPrevLoad(Value *V, Value *NodePos);
  bool isNodePosNextLoad(Value *V, Value *NodePos);
  bool isNodePosNext(Value *V, Value *NodePos);
  bool isNodePosPrev(Value *V, Value *NodePos);
  bool isNodePosReusableArenaBlockAddr(Value *V, Value *NodePos);
  bool isNodePosReusableArenaBlockLoad(Value *V, Value *NodePos);

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
  bool isArenaBlockAddrFromNode(Value *V, Value *NodePtr);
  bool isArenaBlockAddrFromRAB(Value *V, Value *Obj);

  bool verifyAllInstsProcessed(Function *F);
};

// Collect candidates.
bool MemManageTransImpl::gatherCandidates(Module &M,
                                          FunctionTypeResolver &TypeResolver) {
  for (auto *Str : M.getIdentifiedStructTypes()) {
    if (!Str->hasName())
      continue;

    DTransStructType *StrType = TM.getStructType(Str->getName());
    if (!StrType)
      continue;

    std::unique_ptr<MemManageCandidateInfo> CInfo(
        new MemManageCandidateInfo(M));

    if (!CInfo->isCandidateType(StrType))
      continue;

    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP, {
      dbgs() << "  Considering candidate: ";
      Str->print(dbgs(), true, true);
      dbgs() << "\n";
    });

    if (!CInfo->collectMemberFunctions(TypeResolver, true)) {
      DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP,
                      { dbgs() << "  Failed: member function collection.\n"; });
      continue;
    }
    if (Candidates.size() >= MaxNumCandidates) {
      DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP, {
        dbgs() << "  Failed: Exceeding maximum candidate limit.\n";
      });
      return false;
    }
    Candidates.push_back(CInfo.release());
  }

  if (Candidates.empty()) {
    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP,
                    { dbgs() << "  Failed: No candidates found.\n"; });
    return false;
  }

  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP, {
    dbgs() << "  Possible candidate structs: \n";
    for (auto CInfo : Candidates) {
      DTransStructType *St = CInfo->getStringAllocatorType();
      dbgs() << "      " << St->getName() << "\n";
    }
  });

  return true;
}

// Returns true if we can prove that "U" is used only in GlobalVariables that
// are only accessed from interface functions.
bool MemManageTransImpl::isUsedOnlyInUnusedVTable(Value *U) {
  std::function<bool(Value *, SmallPtrSetImpl<GlobalVariable *> &)>
      FindAllGlobalVariableUsers;
  std::function<bool(Value *)> CheckAllUsesInInterfaceFuncs;

  // Recursively finds all GlobalVariable users of "V". Returns false if it
  // finds any non-GlobalVariable user.
  FindAllGlobalVariableUsers =
      [&FindAllGlobalVariableUsers](Value *V,
                                    SmallPtrSetImpl<GlobalVariable *> &GVSet) {
        if (auto *GV = dyn_cast<GlobalVariable>(V)) {
          GVSet.insert(GV);
          return true;
        }
        auto *CE = dyn_cast<Constant>(V);
        if (!CE)
          return false;
        for (User *CEUser : CE->users())
          if (!FindAllGlobalVariableUsers(CEUser, GVSet))
            return false;
        return true;
      };

  // Recursively finds all users of "V" and makes sure "V" is used only
  // in interface functions. Returns false if it finds any non-function
  // user.
  CheckAllUsesInInterfaceFuncs = [this,
                                  &CheckAllUsesInInterfaceFuncs](Value *V) {
    MemManageCandidateInfo *Cand = getCurrentCandidate();
    if (auto *I = dyn_cast<Instruction>(V)) {
      // Check if "I" is in interface functions.
      if (!Cand->isInterfaceFunction(I->getFunction()))
        return false;
    } else if (auto *CE = dyn_cast<Constant>(V)) {
      for (User *CEUser : CE->users())
        if (!CheckAllUsesInInterfaceFuncs(CEUser))
          return false;
    } else {
      // Don't allow any non-constant users.
      return false;
    }
    return true;
  };

  // Returns true if all uses of "GV" are in interface functions.
  auto IsGVUsedOnlyInInterfaceFunctions =
      [&CheckAllUsesInInterfaceFuncs](GlobalVariable *GV) {
        for (auto *UU : GV->users())
          if (!CheckAllUsesInInterfaceFuncs(UU))
            return false;
        return true;
      };

  SmallPtrSet<GlobalVariable *, 2> GVSet;
  if (!FindAllGlobalVariableUsers(U, GVSet))
    return false;
  if (GVSet.empty())
    return false;
  for (auto *GV : GVSet)
    if (!IsGVUsedOnlyInInterfaceFunctions(GV))
      return false;
  return true;
}

// Returns false if any interface function is not called from
// member functions of StringAllocator. It does check for the following
// two things.
// 1. All interface functions need to be called from either StringAllocator
//    or Interface functions.
// 2. If interface function is in VTable (i.e GlobalVariable), this makes
//    sure the all uses of VTable are in interface functions, which will be
//    analyzed later. In the analysis, it will be indirectly proved that
//    there is no real use of VTable.
bool MemManageTransImpl::checkInterfaceFunctions() {
  MemManageCandidateInfo *Cand = getCurrentCandidate();
  for (auto InterF : Cand->interface_functions()) {
    RelatedFunctions.insert(InterF);
    for (User *U : InterF->users()) {
      if (auto *CB = dyn_cast<CallBase>(U)) {
        auto *CalledF = CB->getFunction();
        if (!CalledF)
          return false;
        RelatedFunctions.insert(CalledF);
        if (!Cand->isStrAllocatorOrInterfaceFunction(CalledF) &&
            !isUsedOnlyInUnusedVTable(CalledF))
          return false;
      } else {
        // If "U" is not a call, check "U" is in unused VTables.
        if (!isUsedOnlyInUnusedVTable(U))
          return false;
      }
    }
  }
  return true;
}

// Check legality issues for candidates.
bool MemManageTransImpl::analyzeCandidates(Module &M) {

  // Returns true if "Call" is library function call.
  auto IsLibFunction = [](const CallBase *Call, const TargetLibraryInfo &TLI) {
    LibFunc LibF;
    auto *F = dyn_cast_or_null<Function>(Call->getCalledFunction());
    if (!F || !TLI.getLibFunc(*F, LibF) || !TLI.has(LibF))
      return false;
    return true;
  };

  MemManageCandidateInfo *Cand = getCurrentCandidate();
  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP,
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

    dtrans::AllocKind AK = DTAC.getAllocFnKind(CB, TLI);
    if (AK != dtrans::AK_NotAlloc)
      continue;

    dtrans::FreeKind FK = DTAC.getFreeFnKind(CB, TLI);
    if (FK != dtrans::FK_NotFree)
      continue;

    if (DTransAllocCollector::isDummyFuncWithThisAndIntArgs(CB, TLI,
                                                            MDReader) ||
        DTransAllocCollector::isDummyFuncWithThisAndInt8PtrArgs(CB, TLI,
                                                                MDReader))
      continue;

    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP, {
      dbgs() << "    Failed: Unexpected Inner call " << *CB << "\n";
    });
    return false;
  }

  if (!checkInterfaceFunctions()) {
    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP, {
      dbgs() << "   Failed: Unknown Interface function uses\n";
    });
    return false;
  }

  // TODO: Makes sure ReusableArenaAllocatorType and all other related
  // types are not escaped.

  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP,
                  { dbgs() << "  No issues found with candidate\n"; });

  return true;
}

// Returns true if "V" matches the 2nd GEP below. Updates "GEPPtr" and "Idx"
// when returning true:
//   %GEP = getelementptr %"String", ptr %BaseAddr, i64 %IndexAddr
//   %Ptr = getelementptr %"NextBlock", ptr %GEP, i64 0, i32 Idx
//
// When this routine is called first time, NextBlockTy is assigned to the
// "NextBlock" type. The type should match for all other calls.
bool MemManageTransImpl::isNextBlockObjBlkAddress(Value *Ptr,
                                                  GetElementPtrInst **GEPPtr,
                                                  int32_t *Idx) {
  Value *BasePtr = nullptr;
  if (!getGEPBaseAddrIndex(Ptr, &BasePtr, Idx))
    return false;
  auto *GEPTy = cast<GetElementPtrInst>(Ptr)->getSourceElementType();
  auto *StTy = dyn_cast<StructType>(GEPTy);
  if (!StTy)
    return false;

  auto *GEP = dyn_cast<GetElementPtrInst>(BasePtr);
  if (!GEP)
    return false;

  // Get type of NextBlockTy when this routine is called first time.
  if (!NextBlockTy)
    NextBlockTy = StTy;
  else if (NextBlockTy != StTy)
    return false;

  *GEPPtr = GEP;
  return true;
}

// Returns true if "V" looks like below. Updates "BaseAddr", "IndexAddr" and
// "Idx" when returning true.
// Ex:
//   %GEP = getelementptr %"String", ptr %BaseAddr, i64 %IndexAddr
//   %Ptr = getelementptr %"NextBlock", ptr %GEP, i64 0, i32 Idx
bool MemManageTransImpl::isNextBlockFieldAccess(Value *Ptr, Value **BaseAddr,
                                                Value **IndexAddr,
                                                int32_t *Idx) {
  GetElementPtrInst *GEP = nullptr;
  if (!isNextBlockObjBlkAddress(Ptr, &GEP, Idx))
    return false;

  if (!GEP || GEP->getNumIndices() != 1)
    return false;

  *BaseAddr = GEP->getPointerOperand();
  *IndexAddr = GEP->getOperand(1);
  Visited.insert(GEP);
  return true;
}

// Returns true if "V" looks like below. Updates "BaseAddr", "IndexAddr" and
// "Idx" when returning true.
//   %GEP = getelementptr %"String", ptr %BaseAddr, i64 %IndexAddr
//   %Val = getelementptr %"NextBlock",ptr %GEP, i64 0, i32 Idx
//   %Ptr = load i16, ptr %Val
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

// Returns true if "BB" has unreachable code.
//
// Ex:
//   %116 = landingpad { ptr, i32 }
//          catch ptr null
//   %117 = extractvalue { ptr, i32 } %116, 0
//   tail call void @__clang_call_terminate(ptr %117)
//   unreachable
//
//   or
//
// BB:
//   %LP1 = landingpad { ptr, i32 }
//          catch ptr null
//   br label UnreachBB
//
// UnreachBB:
//   %116 = phi { ptr, i32 } [ %LP1, %BB ], ...
//   %117 = extractvalue { ptr, i32 } %116, 0
//   tail call void @__clang_call_terminate(ptr %117)
//   unreachable
//
//   or
//
//  %98 = cleanuppad within none []
//  ...
// BB:
//  %189 = cleanuppad within %98 []
//  call void @__std_terminate()
//  unreachable
//
//  or
//
// BB:
//  call void @__std_terminate()
//  unreachable
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

  if (LibF == LibFunc_clang_call_terminate) {
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
    Visited.insert(EVI);
    Visited.insert(LPI);
  } else if (LibF == LibFunc_dunder_std_terminate) {
    Instruction *II = CallT->getPrevNonDebugInstruction();
    if (II) {
      auto *CPI = dyn_cast<CleanupPadInst>(II);
      if (!CPI)
        return false;
      auto *CleanupP = dyn_cast_or_null<CleanupPadInst>(CPI->getParentPad());
      if (!CleanupP && !isa<ConstantTokenNone>(CPI->getParentPad()))
        return false;
      Visited.insert(CPI);
      if (CleanupP)
        Visited.insert(CleanupP);
    }
  } else {
    return false;
  }

  Visited.insert(I);
  Visited.insert(CallT);
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
  auto CheckArgsOK = [this](CallBase *CB, Value *Obj, Value *Ptr,
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

    if (Ptr != CB->getArgOperand(1))
      return false;
    return true;
  };

  // Returns true if "V" is a Dealloc call.
  auto IsDeallocCall = [this, &CheckArgsOK](Value *V, Value *Obj, Value *Ptr,
                                            Function *CmpF, Value *RABPtr) {
    if (!V)
      return false;
    auto *CB = dyn_cast<CallBase>(V->stripPointerCasts());
    if (!CB)
      return false;

    auto TLI = GetTLI(*CB->getFunction());
    dtrans::FreeKind FKind = DTAC.getFreeFnKind(CB, TLI);
    if (FKind == dtrans::FK_NotFree)
      return false;

    if (!CheckArgsOK(CB, Obj, Ptr, RABPtr))
      return false;
    if (dtrans::getCalledFunction(*CB) != CmpF)
      return false;
    Visited.insert(CB);
    return true;
  };

  // Returns true "V" is a dummy call.
  auto IsDummyDeallocCall = [this, &CheckArgsOK](Value *V, Value *Obj,
                                                 Value *Ptr, Value *RABPtr) {
    if (!V)
      return false;
    auto *CB = dyn_cast<CallBase>(V->stripPointerCasts());
    if (!CB)
      return false;
    auto TLI = GetTLI(*CB->getFunction());
    if (!DTransAllocCollector::isDummyFuncWithThisAndInt8PtrArgs(CB, TLI,
                                                                 MDReader))
      return false;

    if (!CheckArgsOK(CB, Obj, Ptr, RABPtr))
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
// When it returns true, it updates "NumOfElems" to be the 'size' argument of
// the AllocCall and "AllocPtr" to be reult of the PHInode.
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
//       %t3 = phi ptr [ %t1, %BB1 ], [ %t2, %BB2 ]
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
    auto TLI = GetTLI(*CB->getFunction());
    dtrans::AllocKind AKind = DTAC.getAllocFnKind(CB, TLI);
    if (AKind != dtrans::AK_Malloc && !isUserAllocKind(AKind))
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
    auto TLI = GetTLI(*CB->getFunction());
    if (!DTransAllocCollector::isDummyFuncWithThisAndIntArgs(CB, TLI, MDReader))
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
  *UnBB = USucc;
  *AllocPtr = PHI;
  *NumOfElems = ASizeValue;
  return true;
}

// Returns true if it identifies the pattern below.
// It updates "TrueB", "FalseB" and "NodePtr" when returns true.
// If "IsListHead" is true, this routine checks if ListHead is
// null. Otherwise, checks if ListFreeHead is null.
// "CheckedPtr" is updated with "LValue".
//
// if (Obj->List->listHead == nullptr) {
//   Node *ptr = alloc(sizeof(Node))
//   // Successor Block of this statement is considered as "FalseB"
//   // "NodePtr" is updated with "Ptr".
// } else {
//   // This Block is considered as "FalseB"
// }
//
bool MemManageTransImpl::identifyCheckAndAllocNode(
    BasicBlock *BB, Value *Obj, BasicBlock **TrueB, BasicBlock **FalseB,
    Value **NodePtr, Value **CheckedPtr, bool IsListHead) {
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
  *CheckedPtr = LValue;
  Value *AllocPtr;
  Value *SizeVal;
  BasicBlock *UnBB = nullptr;
  if (!identifyAllocCall(TBlock, Obj, &AllocPtr, &SizeVal, &UnBB) || UnBB)
    return false;
  // Check size of memory allocation is equal to size of Node.
  MemManageCandidateInfo *Cand = getCurrentCandidate();
  DTransStructType *NodeType = Cand->getListNodeType();
  assert(NodeType && "Unexpected Node Type");
  int64_t NodeSize = BB->getModule()->getDataLayout().getTypeAllocSize(
      NodeType->getLLVMType());
  if (!checkConstantSize(SizeVal, NodeSize))
    return false;

  assert(isa<PHINode>(*AllocPtr) && "Expected PHINode");
  BasicBlock *InitBB = cast<PHINode>(AllocPtr)->getParent();

  // If the PHInode is in a block with just an unconditional branch to another
  // block, treat the target block as the one which will perform the
  // initialization.
  if (InitBB->size() == 2) {
    InitBB = getSingleSucc(InitBB);
    if (!InitBB)
      return false;
  }
  *NodePtr = AllocPtr;
  *FalseB = FBlock;
  // If InitBB has single successor, treat the successor as TrueB.
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
  MemManageCandidateInfo *Cand = getCurrentCandidate();
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
// It updates "TrueB", "FalseB", "AllocPtr" and CheckedPtr when returns true.
//
// if (Obj->List->listHead == nullptr) {
//   Node *ptr = alloc(sizeof(Node))
//   Obj->list->listHead = Ptr;
//   Ptr->Next = Ptr;
//   Ptr->Prev = Ptr;
//   // Successor Block of this statement is considered as "FalseB"
//   // "AllocPtr" is updated with "Ptr" and "CheckedPtr" is updated
//   // with "Obj->List->listHead".
// } else {
//   // This Block is considered as "FalseB"
// }
//
bool MemManageTransImpl::identifyListHead(BasicBlock *BB, Value *Obj,
                                          BasicBlock **TrueB,
                                          BasicBlock **FalseB, Value **AllocPtr,
                                          Value **CheckedPtr) {
  if (!identifyCheckAndAllocNode(BB, Obj, TrueB, FalseB, AllocPtr, CheckedPtr,
                                 /* IsListHead */ true))
    return false;

  assert(*AllocPtr && "Expected AllocPtr");
  assert(isa<Instruction>(*AllocPtr) && "Expected Instruction");
  BasicBlock *InitBB = cast<Instruction>(*AllocPtr)->getParent();

  // If the PHInode is in a block with just an unconditional branch to another
  // block, treat the target block as the one which will perform the
  // initialization.
  if (isa<PHINode>(*AllocPtr) && InitBB->size() == 2) {
    InitBB = getSingleSucc(InitBB);
    if (!InitBB)
      return false;
  }

  if (!identifyNodeInit(InitBB, Obj, *AllocPtr))
    return false;
  return true;
}

// Returns true if it identifies the pattern below.
// It updates "EmptyBB", "NotEmptyBB" and "BeginPtr".
//
// if (Obj->List->listHead == nullptr) {
//   // allocate new Node and initialize it.
// } else {
//   // EmptyCheckBB
//   if ((begin() == end()) != 0) { // Check List is empty.
//     // EmptyBB
//     // "BeginPtr" is updated with "begin()".
//   } else {
//     // NotEmptyBB
//   }
// }
bool MemManageTransImpl::identifyListEmpty(BasicBlock *BB, Value *Obj,
                                           BasicBlock **EmptyBB,
                                           BasicBlock **NotEmptyBB,
                                           LoadInst **BeginPtr) {
  BasicBlock *CreatedHeadBB = nullptr;
  BasicBlock *EmptyCheckBB = nullptr;
  Value *NodePtr = nullptr;
  Value *CheckedPtr = nullptr;
  if (!identifyListHead(BB, Obj, &CreatedHeadBB, &EmptyCheckBB, &NodePtr,
                        &CheckedPtr))
    return false;
  Value *LValue = nullptr;
  Value *RValue = nullptr;
  ICmpInst::Predicate Predi = ICmpInst::ICMP_NE;
  if (!processBBTerminator(EmptyCheckBB, &LValue, &RValue, EmptyBB, NotEmptyBB,
                           &Predi))
    return false;
  if (Predi != ICmpInst::ICMP_EQ)
    return false;
  // Check RValue as ListHead
  if (RValue != CheckedPtr)
    return false;
  // Check LValue as RValue->Next
  if (!isNodePosNextLoad(LValue, RValue) ||
      !checkInstructionInBlock(LValue, EmptyCheckBB))
    return false;

  if (*EmptyBB != CreatedHeadBB)
    return false;
  *BeginPtr = cast<LoadInst>(LValue);
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
                                                Value *NodePtr) {
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
  // Check that Value 'V' is being used as the Reusable Arena Block type.
  auto UsedAsBlockType = [](MemManageCandidateInfo *Cand, Value *V) {
    llvm::Type *BlockBaseType = Cand->getBlockBaseType()->getLLVMType();
    llvm::Type *RABType = Cand->getReusableArenaBlockType()->getLLVMType();
    unsigned RABIdx = Cand->getReusableArenaBlockIndex();
    for (auto *U : V->users()) {
      if (auto *GEP = dyn_cast<GetElementPtrInst>(U)) {
        llvm::Type *SrcType = GEP->getSourceElementType();
        if (SrcType == BlockBaseType)
          continue;

        // If structure is nested at element zero, allow the GEP to use that
        // structure type.
        if (RABIdx == 0 && SrcType == RABType)
          continue;

        return false;
      }

      // We skip over uses that are stores or calls because those will be
      // checked later.
      if (isa<StoreInst>(U)) {
        continue;
      } else if (isa<CallBase>(U)) {
        continue;
      } else {
        return false;
      }
    }
    return true;
  };

  // Check memory allocation for ReusableArenaBlock
  Value *RABSizeVal = nullptr;
  BasicBlock *UnBB = nullptr;
  if (!identifyAllocCall(BB, Obj, RABAllocPtr, &RABSizeVal, &UnBB) || UnBB)
    return false;

  auto &DL = BB->getModule()->getDataLayout();
  MemManageCandidateInfo *Cand = getCurrentCandidate();
  DTransStructType *RABType = Cand->getReusableArenaBlockType();
  assert(RABType && "Unexpected RABType Type");
  int64_t RABSize = DL.getTypeAllocSize(RABType->getLLVMType());
  if (!checkConstantSize(RABSizeVal, RABSize))
    return false;

  auto *RABAllocPHI = dyn_cast<PHINode>(*RABAllocPtr);
  assert(RABAllocPHI && "Expected PHINode");
  if (!UsedAsBlockType(Cand, RABAllocPHI))
    return false;

  // Check memory allocation for StringObject
  //
  // If the PHInode is in a block with just an unconditional branch to another
  // block, treat the target block as the one which will perform the
  // initialization.
  BasicBlock *InitBB = RABAllocPHI->getParent();
  if (InitBB->size() == 2) {
    InitBB = getSingleSucc(InitBB);
    if (!InitBB)
      return false;
  }

  Value *BlockSizeVal = nullptr;
  Value *BlockAllocPtr = nullptr;
  UnBB = nullptr;
  if (!identifyAllocCall(InitBB, Obj, &BlockAllocPtr, &BlockSizeVal, &UnBB))
    return false;
  if (!UnBB)
    return false;

  DTransStructType *StrObjType = Cand->getStringObjectType();
  assert(StrObjType && "Unexpected StrObjType Type");
  int64_t StrObjSize = DL.getTypeAllocSize(StrObjType->getLLVMType());
  if (!checkSizeValue(BlockSizeVal, StrObjSize, Obj))
    return false;

  // Check that the BlockAllocPtr matches the expectation. Later, inside of the
  // call to identifyArenaBlockInit(), this will be checked to verify it is
  // stored to field 3 (StringObjectIndex) of BlockBaseType.
  if (!isa<PHINode>(BlockAllocPtr) ||
      !isa<PointerType>(BlockAllocPtr->getType()))
    return false;

  // Check EH code
  Instruction *I = UnBB->getFirstNonPHIOrDbg();
  auto *LPI = dyn_cast_or_null<LandingPadInst>(I);
  auto *CleanupP = dyn_cast_or_null<CleanupPadInst>(I);
  BasicBlock *EndBB = nullptr;
  if (!identifyDeallocCall(UnBB, Obj, RABAllocPHI, &EndBB))
    return false;
  BasicBlock *Succ = getSingleSucc(EndBB);
  if (Succ)
    EndBB = Succ;
  auto *Res = dyn_cast<ResumeInst>(EndBB->getTerminator());
  auto *CRI = dyn_cast<CleanupReturnInst>(EndBB->getTerminator());
  if (Res) {
    // %i100 = landingpad { i8*, i32 }
    //     Cleanup
    //  ...
    // resume { i8*, i32 } %i100
    if (!LPI || LPI->getNumClauses() != 0 || !LPI->isCleanup())
      return false;
    if (Res->getValue() != LPI)
      return false;
    Visited.insert(LPI);
    Visited.insert(Res);
  } else if (CRI) {
    // %102 = cleanuppad within none []
    // ...
    // cleanupret from %102 unwind to caller
    if (!CleanupP)
      return false;
    if (CRI->getCleanupPad() != CleanupP)
      return false;
    Visited.insert(CRI);
    Visited.insert(CleanupP);
  } else {
    return false;
  }

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

  assert(isa<PHINode>(BlockAllocPtr) && "Unexpected allocation pointer");
  BasicBlock *AllocBB = cast<PHINode>(BlockAllocPtr)->getParent();
  // If the PHInode is in a block with just an unconditional branch to another
  // block, treat the target block as the one which will perform the
  // initialization.
  if (AllocBB->size() == 2) {
    AllocBB = getSingleSucc(AllocBB);
    if (!AllocBB)
      return false;
  }

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
  Value *CheckedPtr = nullptr;
  if (!identifyListHead(BB, Obj, &CreatedHeadBB, &HasHeadBB, &NodePtr,
                        &CheckedPtr))
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

  // If the NodePtr PHInode is in a block with just an unconditional branch to
  // another block, use the successor block for the block for the block of
  // interest that is expected to branch to the PHINode being examined.
  BasicBlock *NodeBB = cast<Instruction>(NodePtr)->getParent();
  if (NodeBB->size() == 2) {
    NodeBB = getSingleSucc(NodeBB);
    if (!NodeBB)
      return false;
  }

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
  Value *CheckedPtr = nullptr;
  if (!identifyCheckAndAllocNode(ListHeadBlock, Obj, &CreateFreeListHeadBB,
                                 &HasFreeListHeadBB, &FreeListHeadPtr,
                                 &CheckedPtr, false))
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
  BasicBlock *AllocBB = cast<PHINode>(FreeListHeadPtr)->getParent();
  // If the PHInode is in a block with just an unconditional branch to another
  // block, treat the target block as the one which will perform the
  // initialization.
  if (AllocBB->size() == 2) {
    AllocBB = getSingleSucc(AllocBB);
    if (!AllocBB)
      return false;
  }

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

  SmallVector<StoreInst *, 8> StoreVec;
  collectStoreInst(PushBlock, StoreVec);
  if (!identifyPushAtPos(StoreVec, Obj, ListHeadPtr, NewNodePHI,
                         NextFreeNodePHI, BlockAllocPtr))
    return false;
  BasicBlock *CreateHeadBB = nullptr;
  BasicBlock *HasHeadBB = nullptr;
  Value *HeadPtr = nullptr;
  if (!identifyListHead(PushBlock, Obj, &CreateHeadBB, &HasHeadBB, &HeadPtr,
                        &CheckedPtr))
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
  auto *PredBB = cast<PHINode>(HeadPtr)->getParent();
  // If the PHInode is in a block with just an unconditional branch to another
  // block, treat the target block as the one which will perform the
  // initialization.
  if (PredBB->size() == 2) {
    PredBB = getSingleSucc(PredBB);
    if (!PredBB)
      return false;
  }

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
  Value *FirstFreeBlock = nullptr;
  Value *NextFreeBlock = nullptr;
  if (!identifyUncommittedBlock(AllocInitBB, RABPtr, &FirstFreeBlock,
                                &NextFreeBlock, &IncrementBB, &NoIncrementBB))
    return false;

  BasicBlock *NoIncrementPredBB = nullptr;
  // Check CFG here. Makes sure single successor of NoIncrementBB and
  // IncrementBB is RetBB
  //
  BasicBlock *BB1 = getSingleSucc(NoIncrementBB);
  if (!BB1) {
    // AllocInitBB:
    //   %3 = getelementptr %S, %S* RABPtr->ArenaBlock.ObjBlock, i64 %2
    //   br i1 %cond1, label %IncrementBB, label %RetBB
    //
    // IncrementBB:
    //   br label %RetBB
    //
    // RetBB:
    //   phi X* [ null, BB ], [ %3, AllocInitBB ], [ %3, IncrementBB ]
    if (NoIncrementBB != RetBB)
      return false;
    NoIncrementPredBB = AllocInitBB;
  } else {
    // AllocInitBB:
    //   br i1 %cond1, label %IncrementBB, label %NoIncrementBB
    //
    // NoIncrementBB:
    //   %3 = getelementptr %S, %S* RABPtr->ArenaBlock.ObjBlock, i64 %2
    //   br label %RetBB
    //
    // IncrementBB:
    //   %5 = getelementptr %S, %S* RABPtr->ArenaBlock.ObjBlock, i64 %4
    //   br label %RetBB
    //
    // RetBB:
    //   phi X* [ null, BB ], [ %3, NoIncrementBB ], [ %5, IncrementBB ]
    NoIncrementPredBB = NoIncrementBB;
    if (BB1 != RetBB)
      return false;
  }
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
  Value *IncPtr = RetPHI->getIncomingValueForBlock(NoIncrementPredBB);
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

// Returns true if "StoreVec" has the following StoreInsts in the same order.
//
// newNode->ReusableArenaBlock = FullBlock;
// newNode->prev = NodePos.prev
// newNode->next = NodePos
// NodePos.prev->next = newNode;
// NodePos.prev = newNode;
// Obj->ArenaAllocator->List->freeListHeadPtr = nextFreeNode
//
bool MemManageTransImpl::identifyPushAtPos(
    SmallVectorImpl<StoreInst *> &StoreVec, Value *Obj, Value *NodePos,
    Value *NewNode, Value *NextFreeNode, Value *FullBlock) {
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
  Value *ValOp = SI->getValueOperand();
  if (ValOp != SI->getPrevNonDebugInstruction())
    return false;
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

  auto *ListPrev = dyn_cast<LoadInst>(SI->getNextNonDebugInstruction());
  if (!ListPrev || !isNodePosPrevLoad(ListPrev, NodePos))
    return false;
  SI = StoreVec[3];
  if (SI->getValueOperand() != NewNode)
    return false;
  if (!isNodePosNext(SI->getPointerOperand(), ListPrev))
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

bool MemManageTransImpl::isStrObjPtrType(DTransType *Ty) {
  if (!Ty || !Ty->isPointerTy())
    return false;

  MemManageCandidateInfo *Cand = getCurrentCandidate();
  DTransStructType *StrObjType = Cand->getStringObjectType();
  DTransType *ObjTy = Ty->getPointerElementType();
  return ObjTy == StrObjType;
}

// Categorize interface functions (AllocatorInterfaceFunctions) using
// return type and signature.
// ReusableArenaAllocatorType or ArenaAllocatorType is considered as
// allocator class type.
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

  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP,
                  { dbgs() << "   Categorize Interface Functions\n"; });

  MemManageCandidateInfo *Cand = getCurrentCandidate();
  DTransStructType *StringObjectType = Cand->getStringObjectType();
  DTransStructType *ReusableArenaAllocatorType =
      Cand->getReusableArenaAllocatorType();
  DTransStructType *ArenaAllocatorType = Cand->getArenaAllocatorType();
  DTransStructType *MemInterfaceType = Cand->getMemInterfaceType();

  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP, {
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
      if (Cand->isInterfaceFunction(CalledF))
        return true;
    }
    return false;
  };

  // Categorize "F" using return type and signature.
  auto CategorizeFunctionUsingSignature =
      [&, IsCalledFromInterfaceFunction](Function *F) -> MemManageFKind {
    bool NoReturn = false;
    bool ClassReturn = false;
    bool MemManagerReturn = false;
    bool StrObjReturn = false;
    bool IntReturn = false;
    int32_t MemInterfaceArgs = 0;
    int32_t ClassArgs = 0;
    int32_t IntArgs = 0;
    int32_t StrObjArgs = 0;

    DTransFunctionType *DFnTy = getDTransFunctionType(MDReader, F);
    if (!DFnTy)
      return UnKnown;

    // Analyze return type here.
    DTransType *RTy = DFnTy->getReturnType();
    switch (RTy->getLLVMType()->getTypeID()) {
    default:
      return UnKnown;

    case Type::VoidTyID:
      NoReturn = true;
      break;

    case Type::PointerTyID: {
      auto *PTy = cast<DTransPointerType>(RTy)->getPointerElementType();
      if (PTy == ReusableArenaAllocatorType || PTy == ArenaAllocatorType)
        ClassReturn = true;
      else if (PTy == StringObjectType)
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
    unsigned NumArgs = DFnTy->getNumArgs();
    for (unsigned Idx = 0; Idx < NumArgs; ++Idx) {
      DTransType *ArgTy = DFnTy->getArgType(Idx);
      switch (ArgTy->getLLVMType()->getTypeID()) {
      default:
        return UnKnown;

      case Type::PointerTyID: {
        auto *PTy = cast<DTransPointerType>(ArgTy)->getPointerElementType();
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
    if ((NoReturn || ClassReturn) && MemInterfaceArgs == 1 && ClassArgs == 1 &&
        IntArgs == 2 && ArgsSize == 4)
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
    DTransFunctionType *DFnTy = getDTransFunctionType(MDReader, F);
    if (!DFnTy) {
      DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP,
                      { dbgs() << "   Failed: Unknown functionality\n"; });
      return false;
    }
    MemManageFKind FKind = CategorizeFunctionUsingSignature(F);
    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP, {
      dbgs() << "   " << F->getName() << ": " << FKind << "\n";
    });
    if (FKind == UnKnown) {
      DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP,
                      { dbgs() << "   Failed: Unknown functionality\n"; });
      return false;
    }
    if (FunctionalityMap[FKind]) {
      DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP,
                      { dbgs() << "   Failed: Multiple functionality\n"; });
      return false;
    }
    FunctionalityMap[FKind] = F;
    FunctionalityTypeMap[F] = DFnTy;
  }
  assert(FunctionalityMap.size() == NumInterfacefunctions &&
         "Unexpected functionality");
  return true;
}

// Returns true if "V" represents address of VTable field in ArenaAllocator.
// Ex:
//   %i9 = getelementptr %ReusableArenaAllocator, ptr %This, i64 0, i32 0, i32 0
bool MemManageTransImpl::isVTableAddrInReusableArenaAllocator(Value *V,
                                                              Value *ThisObj) {
  auto *GEP = dyn_cast<GetElementPtrInst>(V);
  if (!GEP)
    return false;
  if (GEP->getNumIndices() != 3 || !GEP->hasAllZeroIndices())
    return false;
  llvm::Type *SrcTy = GEP->getSourceElementType();
  auto *StTy = dyn_cast<llvm::StructType>(SrcTy);
  if (!StTy || !StTy->hasName())
    return false;
  DTransStructType *DStTy = TM.getStructType(StTy->getName());
  if (!DStTy)
    return false;
  if (DStTy->getNumFields() == 0)
    return false;

  // Allow for nested structures at element 0
  DTransType *FirstFieldTy = DStTy->getFieldType(0);
  while (FirstFieldTy && FirstFieldTy->isStructTy()) {
    auto *FieldStTy = cast<DTransStructType>(FirstFieldTy);
    if (FieldStTy->getNumFields() == 0)
      return false;
    FirstFieldTy = FieldStTy->getFieldType(0);
  }

  if (!FirstFieldTy)
    return false;
  if (!isPtrToVFTable(FirstFieldTy))
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
//   %i8 = getelementptr %XalanList, ptr %i5, i64 0, i32 2
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
  MemManageCandidateInfo *Cand = getCurrentCandidate();
  // Check if type of "This pointer" is ArenaAllocator.
  // If it is ArenaAllocator, address of ArenaAllocator is "obj".
  if (auto *Arg = dyn_cast<Argument>(Obj)) {
    Function *F = Arg->getParent();
    DTransFunctionType *DFnTy = FunctionalityTypeMap[F];
    if (!DFnTy || DFnTy->getNumArgs() != F->arg_size())
      return false;

    DTransType *ObjTy = DFnTy->getArgType(Arg->getArgNo());
    if (!ObjTy->isPointerTy())
      return false;

    if (ObjTy->getPointerElementType() == Cand->getArenaAllocatorType()) {
      if (V != Obj)
        return false;
      return true;
    }
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

// This handles the case where the "this pointer" object type is
// "ReusableArenaAllocator", but the GEP access directly indexes the element
// zero structure type as "ArenaAllocator"
//
//   foo(ReusableArenaAllocator *Obj) {
//     Obj->ArenaAllocator
//   }
bool MemManageTransImpl::isGEPLessArenaAllocatorAddr(Value *V, Value *Obj) {
  MemManageCandidateInfo *Cand = getCurrentCandidate();
  if (auto *Arg = dyn_cast<Argument>(Obj)) {
    Function *F = Arg->getParent();
    DTransFunctionType *DFnTy = FunctionalityTypeMap[F];
    if (!DFnTy || DFnTy->getNumArgs() != F->arg_size())
      return false;

    DTransType *ObjTy = DFnTy->getArgType(Arg->getArgNo());
    if (!ObjTy->isPointerTy())
      return false;

    if (ObjTy->getPointerElementType() ==
            Cand->getReusableArenaAllocatorType() &&
        Cand->getArenaAllocatorObjectIndex() == 0 && V == Obj)
      return true;
  }

  return false;
}

// Returns true if "V" represents address of List in ArenaAllocator.
// Ex:
//  foo(ReusableArenaAllocator *Obj) {
//     Obj->ArenaAllocator.List
//  }
bool MemManageTransImpl::isListAddr(Value *V, Value *Obj) {
  MemManageCandidateInfo *Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getListObjectIndex())
    return false;
  if (!isArenaAllocatorAddr(BasePtr, Obj) &&
      !isGEPLessArenaAllocatorAddr(BasePtr, Obj))
    return false;
  return true;
}

// Returns true if "V" represents address of blockSize in ArenaAllocator.
// Ex:
//  foo(ReusableArenaAllocator *Obj) {
//     Obj->ArenaAllocator.blockSize
//  }
bool MemManageTransImpl::isAllocatorBlockSizeAddr(Value *V, Value *Obj) {
  MemManageCandidateInfo *Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getAllocatorBlockSizeIndex())
    return false;
  if (!isArenaAllocatorAddr(BasePtr, Obj) &&
      !isGEPLessArenaAllocatorAddr(BasePtr, Obj))
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

// Returns true if "V" represents address of 'MemManager' field in List.
// Ex:
//  foo(ReusableArenaAllocator *Obj) {
//     Obj->ArenaAllocator.List.MemManager
//  }
bool MemManageTransImpl::isListMemManagerAddr(Value *V, Value *Obj) {
  MemManageCandidateInfo *Cand = getCurrentCandidate();
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

// Returns true if "V" represents address of 'Head' field in List.
// Ex:
//  foo(ReusableArenaAllocator *Obj) {
//     Obj->ArenaAllocator.List.Head
//  }
bool MemManageTransImpl::isListHeadAddr(Value *V, Value *Obj) {
  MemManageCandidateInfo *Cand = getCurrentCandidate();
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

// Returns true if "V" represents address of 'FreeHead' field in List.
// Ex:
//  foo(ReusableArenaAllocator *Obj) {
//     Obj->ArenaAllocator.List.FreeHead
//  }
bool MemManageTransImpl::isListFreeHeadAddr(Value *V, Value *Obj) {
  MemManageCandidateInfo *Cand = getCurrentCandidate();
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
  MemManageCandidateInfo *Cand = getCurrentCandidate();
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
  MemManageCandidateInfo *Cand = getCurrentCandidate();
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
  MemManageCandidateInfo *Cand = getCurrentCandidate();
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

// Returns true if "V" represents address of ReusableArenaBlock field of Node.
// Ex:
//   NodePos->ReusableArenaBlock
bool MemManageTransImpl::isNodePosReusableArenaBlockAddr(Value *V,
                                                         Value *NodePos) {
  MemManageCandidateInfo *Cand = getCurrentCandidate();
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

// Returns true if "V" represents address of 'destroyBlock' field in
// ReusableArenaAllocator. Ex:
//  foo(ReusableArenaAllocator *Obj) {
//     Obj->destroyBlock
//  }
bool MemManageTransImpl::isDestroyBlockFlagAddr(Value *V, Value *Obj) {
  MemManageCandidateInfo *Cand = getCurrentCandidate();
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

// Returns true if "V" is load of 'List.MemManager' address.
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
    if (!isArenaAllocatorAddr(CB->getArgOperand(0), Obj) &&
        !isGEPLessArenaAllocatorAddr(CB->getArgOperand(0), Obj))
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
  MemManageCandidateInfo *Cand = getCurrentCandidate();
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
//  *((ArenaBlock*)Obj->ArenaAllocator.List.Head->Next)
//
// When NodePtr is not null, it checks if "V" represents the pattern below.
//
//  *((ArenaBlock*)NodePtr)
//  NodePtr->ReusableArenaBlock->ArenaBlock
//
bool MemManageTransImpl::isListFrontNodeArenaBlockAddr(
    Value *V, Value *Obj, Value *NodePtr = nullptr) {

  auto *LI = dyn_cast<LoadInst>(V);
  if (!LI) {
    if (isArenaBlockAddrFromNode(V, NodePtr))
      return true;
    return false;
  }

  // isListBegin() expects a LoadInst.
  //
  //   %28 = load ptr, ptr %27
  //   %31 = getelementptr %"Node", ptr %28, i64 0, i32 0
  // is equivalent to:
  //   %28 = load ptr, ptr %27
  Value *Ptr = LI->getPointerOperand();
  if (auto *GEP = dyn_cast<GetElementPtrInst>(Ptr))
    if (GEP->hasAllZeroIndices()) {
      Visited.insert(GEP);
      Ptr = GEP->getPointerOperand();
    }
  if (NodePtr) {
    if (Ptr != NodePtr)
      return false;
  } else {
    if (!isListBegin(Ptr, Obj))
      return false;
  }

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
  MemManageCandidateInfo *Cand = getCurrentCandidate();
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
  MemManageCandidateInfo *Cand = getCurrentCandidate();
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
//     (ReusableArenaBlock*)RABObj)->ArenaBlock
bool MemManageTransImpl::isArenaBlockAddrFromRAB(Value *V, Value *RABObj) {
  if (V != RABObj)
    return false;
  return true;
}

// Returns true if "V" represents address of FirstFreeBlock in
// ReusableArenaBlock. Ex:
//     RABObj->FirstFreeBlock
//     (ReusableArenaBlock*)RABObj)->FirstFreeBlock
bool MemManageTransImpl::isFirstFreeBlockAddrFromRAB(Value *V, Value *RABObj) {
  MemManageCandidateInfo *Cand = getCurrentCandidate();
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
//     (ReusableArenaBlock*)RABObj)->NextFreeBlock
bool MemManageTransImpl::isNextFreeBlockAddrFromRAB(Value *V, Value *RABObj) {
  MemManageCandidateInfo *Cand = getCurrentCandidate();
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
  MemManageCandidateInfo *Cand = getCurrentCandidate();
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
  MemManageCandidateInfo *Cand = getCurrentCandidate();
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
  MemManageCandidateInfo *Cand = getCurrentCandidate();
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
  MemManageCandidateInfo *Cand = getCurrentCandidate();
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

// Returns true if "V" represents address of ArenaBlock in
// ReusableArenaBlock in "NodePtr".
// Ex:
//     NodePtr->ReusableArenaBlock->ArenaBlock
bool MemManageTransImpl::isArenaBlockAddrFromNode(Value *V, Value *NodePtr) {
  MemManageCandidateInfo *Cand = getCurrentCandidate();
  Value *BasePtr = nullptr;
  int32_t Idx = 0;
  if (!getGEPBaseAddrIndex(V, &BasePtr, &Idx))
    return false;
  if (Idx != Cand->getBlockBaseObjIndex())
    return false;
  if (!isNodePosReusableArenaBlockLoad(BasePtr, NodePtr))
    return false;
  return true;
}

// Returns true if "BB" terminates with conditional BranchInst.
// Updates "LValue", "RValue", "TBlock", "FBlock" and "Predi".
// Ex:
//   %6 = icmp eq ptr %5, null
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
  *LValue = IC->getOperand(0);
  *RValue = IC->getOperand(1);
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

// Returns true if "BB" has checks related to devirtualization using
// "MemManager" from Obj->List.
// It updates CmpFn, TBlock and FBlock when returns true.
//
// Ex:
// BB:
//  %8 = getelementptr inbounds %List, ptr %3, i64 0, i32 0
//  %9 = load ptr, ptr %8     ; result as %"MemManager"*
//  %10 = getelementptr %"MemManager", ptr %9, i64 0, i32 0
//  %VTLoad = load ptr, ptr %10   ; result as a pointer to pointer to function
//  %13 = tail call i1 @llvm.type.test(ptr %VTLoad, metadata !"VTable")
//  tail call void @llvm.assume(i1 %13)
//  %14 = getelementptr ptr, ptr %VTLoad, i64 2
//  %15 = load ptr, ptr %14 ; result as a pointer to function type
//  ; Devirtualization may leave bitcast from 'ptr' to 'ptr'
//  (optional) %16 = bitcast ptr %15 to ptr
//  (optional) %17 = bitcast ptr @CmpFn to ptr
//  %18 = icmp eq ptr %16, %17
//  br i1 %18, label %19, label %21
//
// If RABPtr is not nullptr, checks that MemManager is loaded from
// ReusableArenaBlock instead of Obj->List.
//
bool MemManageTransImpl::identifyDevirtChecks(BasicBlock *BB, Value *Obj,
                                              Function **CmpFn,
                                              BasicBlock **TBlock,
                                              BasicBlock **FBlock,
                                              Value *RABPtr) {
  Value *LValue = nullptr;
  Value *RValue = nullptr;
  ICmpInst::Predicate Predi = CmpInst::ICMP_NE;
  if (!processBBTerminator(BB, &LValue, &RValue, TBlock, FBlock, &Predi))
    return false;
  if (Predi != ICmpInst::ICMP_EQ)
    return false;
  // Skip BitCastInst instructions. (Devirtualization may leave
  // 'bitcast ptr %x to ptr' instructions in the IR)
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

  // Check for VTable load by tracing back to the object the where this load
  // came from.
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

  //   %9 = load ptr, ptr %8
  //   %10 = getelementptr %"MemManager", ptr %9, i64 0, i32 0
  //   %VTLoad = load ptr, ptr %9
  // is equivalent to:
  //   %9 = load ptr, ptr %8
  //   %VTLoad = load ptr, ptr %9
  if (auto *MemGEP = dyn_cast<GetElementPtrInst>(MemI))
    if (MemGEP->hasAllZeroIndices()) {
      Visited.insert(MemGEP);
      MemI = MemGEP->getPointerOperand();
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
  //  %13 = tail call i1 @llvm.type.test(ptr %VTLoad, metadata !"VTable")
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
  if (TestI->getArgOperand(0) != VTLoad)
    return false;
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

// Check for the pattern below:
//   if (RABPtr->FirstFreeBlock == RABPtr->NextFreeBlock) {
//     // CommittedBB
//   } else {
//     // UncommittedBB
//   }
bool MemManageTransImpl::identifyUncommittedBlock(BasicBlock *BB, Value *RABPtr,
                                                  Value **FirstFreeBlock,
                                                  Value **NextFreeBlock,
                                                  BasicBlock **CommittedBB,
                                                  BasicBlock **UncommittedBB) {

  ICmpInst::Predicate Predi = ICmpInst::ICMP_NE;
  if (!processBBTerminator(BB, FirstFreeBlock, NextFreeBlock, CommittedBB,
                           UncommittedBB, &Predi))
    return false;
  if (Predi != ICmpInst::ICMP_EQ)
    return false;
  if (!isFirstFreeBlockLoadFromRAB(*FirstFreeBlock, RABPtr))
    return false;
  if (!isNextFreeBlockLoadFromRAB(*NextFreeBlock, RABPtr))
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

// Returns first NonDbg instruction (including PHINodes) in BB.
Instruction *MemManageTransImpl::getFirstNonDbg(BasicBlock *BB) {
  return &*skipDebugIntrinsics(BB->begin()->getIterator());
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

// Check functionality of each interface function to prove that
// it is an allocator class.
bool MemManageTransImpl::recognizeFunctions() {
  // Check functionality of GetMemManager.
  Function *GetMemF = FunctionalityMap[GetMemManager];
  if (!GetMemF)
    return false;
  if (!recognizeGetMemManager(GetMemF)) {
    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP, {
      dbgs() << "Failed to recognize GetMemManager function\n";
      VisitedAnnotationWriter Annotator(Visited);
      GetMemF->print(dbgs(), &Annotator);
    });
    if (!continueAfterMismatch())
      return false;
  }

  // Check functionality of Constructor.
  Function *Ctor = FunctionalityMap[Constructor];
  if (!Ctor)
    return false;
  if (!recognizeConstructor(Ctor)) {
    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP, {
      dbgs() << "Failed to recognize Constructor function\n";
      VisitedAnnotationWriter Annotator(Visited);
      Ctor->print(dbgs(), &Annotator);
    });
    if (!continueAfterMismatch())
      return false;
  }

  // Check functionality of AllocateBlock.
  Function *AllocBlock = FunctionalityMap[AllocateBlock];
  if (!AllocBlock)
    return false;
  if (!recognizeAllocateBlock(AllocBlock)) {
    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP, {
      dbgs() << "Failed to recognize AllocateBlock function\n";
      VisitedAnnotationWriter Annotator(Visited);
      AllocBlock->print(dbgs(), &Annotator);
    });
    if (!continueAfterMismatch())
      return false;
  }

#if 0 // TODO: Match the rest
  // Check functionality of CommitAllocation.
  Function *CAllocation = FunctionalityMap[CommitAllocation];
  if (!CAllocation)
    return false;
  if (!recognizeCommitAllocation(CAllocation)) {
    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP, {
      dbgs() << "Failed to recognize CommitAllocation function\n";
      VisitedAnnotationWriter Annotator(Visited);
      CAllocation->print(dbgs(), &Annotator);
    });
    if (!continueAfterMismatch())
      return false;
  }

  // Check functionality of Destructor.
  Function *Dtor = FunctionalityMap[Destructor];
  if (!Dtor)
    return false;
  if (!recognizeDestructor(Dtor)) {
    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP, {
      dbgs() << "Failed to recognize Destructor function\n";
      VisitedAnnotationWriter Annotator(Visited);
      Dtor->print(dbgs(), &Annotator);
    });
    if (!continueAfterMismatch())
      return false;
  }

  // Check functionality of Reset.
  Function *ResetF = FunctionalityMap[Reset];
  if (!ResetF)
    return false;
  if (!recognizeReset(ResetF)) {
    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP, {
      dbgs() << "Failed to recognize Reset function\n";
      VisitedAnnotationWriter Annotator(Visited);
      ResetF->print(dbgs(), &Annotator);
    });
    if (!continueAfterMismatch())
      return false;
  }

  // Check functionality of DestroyObject.
  Function *ObjDtorF = FunctionalityMap[DestroyObject];
  if (!ObjDtorF)
    return false;
  if (!recognizeDestroyObject(ObjDtorF)) {
    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP, {
      dbgs() << "Failed to recognize DestroyObject function\n";
      VisitedAnnotationWriter Annotator(Visited);
      ObjDtorF->print(dbgs(), &Annotator);
    });
    return false;
  }
#endif

  return true;
}

// Return false if any unvisited instruction that can affect the behavior of a
// function being matched is found in "F".
bool MemManageTransImpl::verifyAllInstsProcessed(Function *F) {
  for (auto &I : instructions(F)) {
    if (Visited.count(&I))
      continue;

    // Ignore ReturnInst with no operands.
    if (isa<ReturnInst>(&I) && I.getNumOperands() == 0)
      continue;

    if (isa<DbgInfoIntrinsic>(&I))
      continue;

    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP, {
      dbgs() << "   Failed: Missed processing some instructions\n"
             << "   " << I << "\n";
    });
    // Return false if any instruction is not found in "visited".
    return false;
  }

  return true;
}

// This routine checks that the calls are in the order given here:
//   AllocateBlock();
//   StrObjCtor();
//   CommitBlock();
//
// Check for the following callsite restrictions:
// 1. Only one callsite is allowed for AllocateBlock and CommitBlock.
// 2. StrObjCtor should be called after AllocateBlock.
// 3. CommitBlock is called after StrObjCtor.
// 4. Only instructions without side effects are allowed in between calls.
bool MemManageTransImpl::checkCallSiteRestrictions() {

  // Skip instructions without side effects. Returns next instruction.
  auto GetNextInstWithSideEffects = [this](Instruction *I) -> Instruction * {
    BasicBlock::iterator EndIt = I->getParent()->end();
    BasicBlock::iterator It = I->getIterator();
    // Get next instruction.
    It++;
    for (; It != EndIt; It++) {
      Instruction *II = &*It;
      if (!II->mayWriteToMemory())
        continue;
      if (auto *CB = dyn_cast<CallBase>(II)) {
        auto *CalledF = dtrans::getCalledFunction(*CB);
        assert(CalledF && "Unexpected indirect call");
        // Allow GetMemManager call as it doesn't have any side effects.
        if (FunctionalityMap[GetMemManager] == CalledF)
          continue;
      }
      return II;
    }
    return nullptr;
  };

  // Returns callsite for "F" if it has a single callsite.
  // Otherwise, returns nullptr.
  auto GetSingleCallSite = [](Function *F) -> CallBase * {
    CallBase *CallS = nullptr;
    for (auto *U : F->users()) {
      auto *CB = dyn_cast<CallBase>(U);
      // Ignore non-callbase uses as we already proved that all other
      // uses are related to VTable.
      if (!CB)
        continue;
      if (CallS)
        return nullptr;
      CallS = CB;
    }
    return CallS;
  };

  CallBase *AllocCall = GetSingleCallSite(FunctionalityMap[AllocateBlock]);
  CallBase *CommitCall = GetSingleCallSite(FunctionalityMap[CommitAllocation]);
  if (!CommitCall || !AllocCall)
    return false;

  // Check that both calls are in same BasicBlock.
  if (CommitCall->getParent() != AllocCall->getParent())
    return false;

  // Check that the "StrObjCtor" call follows the "AllocateBlock" call.
  auto *StrObjCtor =
      dyn_cast_or_null<CallBase>(GetNextInstWithSideEffects(AllocCall));
  if (!StrObjCtor)
    return false;
  if (StrObjCtor->arg_size() < 1)
    return false;

  // Check that StrObjCtor is a direct call to a constructor.
  Function *Ctor = dtrans::getCalledFunction(*StrObjCtor);
  if (!Ctor)
    return false;
  if (!Ctor->hasFnAttribute("intel-mempool-constructor"))
    return false;

  // Check that the first argument is a "StringObjectType"
  if (!isStrObjPtrType(getArgType(MDReader, Ctor, 0)))
    return false;

  // Check that the next call is the "CommitBlock"
  Instruction *NextCall = GetNextInstWithSideEffects(StrObjCtor);
  if (!NextCall || NextCall != CommitCall)
    return false;
  return true;
}

// Returns true if "F" is recognized as "GetMemManager".
// Ex:
//   MemManager *getMemManager(ArenaAllocatorTy *Obj) {
//     return Obj->List.MemoryManager;
//   }
//
// Based on the interface categorization, we know the following about F on
// input:
//   Return type: "MemInterfaceType"
//   Inputs: 1 argument of type "ReusableArenaAllocatorType" or
//   "ArenaAllocatorType"
//
// Check that the returned value is from loading a "MemoryManager" field from a
// "ListType"
bool MemManageTransImpl::recognizeGetMemManager(Function *F) {
  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP,
                  dbgs() << "   Recognizing GetMemManager Functionality "
                         << F->getName() << "\n";);
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
  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP,
                  dbgs() << "   Recognized GetMemManager: " << F->getName()
                         << "\n";);
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
//
// This function will check that there is one store instruction for each of the
// above.
bool MemManageTransImpl::recognizeConstructor(Function *F) {
  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP,
                  dbgs() << "   Recognizing Constructor Functionality "
                         << F->getName() << "\n";);
  if (F->size() != 1)
    return false;
  assert(F->arg_size() == 4 && "Unexpected arguments");

  Visited.clear();
  MemManageCandidateInfo *Cand = getCurrentCandidate();
  Argument *ThisObj = &*F->arg_begin();

  // On windows, constructor returns "this" object.
  auto *RI = dyn_cast<ReturnInst>(F->getEntryBlock().getTerminator());
  if (!RI)
    return false;
  Value *RetVal = RI->getReturnValue();
  if (RetVal && RetVal != ThisObj)
    return false;
  Visited.insert(RI);

  // Collect the store instructions, then check that there is one store with the
  // expected value for each field.
  SmallVector<StoreInst *, 8> SIList;
  for (auto &I : instructions(F))
    if (auto *SI = dyn_cast<StoreInst>(&I))
      SIList.push_back(SI);

  unsigned BlockSizeAssigned = 0;
  unsigned DestroyObjFlagAssigned = 0;
  unsigned VTableAssigned = 0;
  unsigned MemManagerAssigned = 0;
  unsigned ListHeadAssigned = 0;
  unsigned ListFreeHeadAssigned = 0;

  for (auto *SI : SIList) {
    Visited.insert(SI);
    Value *ValOp = SI->getValueOperand();
    Value *PtrOp = SI->getPointerOperand();
    llvm::Type *ValTy = ValOp->getType();
    if (ValTy->isPointerTy()) {
      // Check for: Obj->ArenaAllocator.vtable = some_value;
      //
      // This may be using the GEP-less form for storing to the first field of a
      // structure, or a structure GEP of the form:
      //   GEP %class.ReusableArenaAllocator, ptr %0, i64 0, i32 0, i32 0
      if (PtrOp == ThisObj ||
          isVTableAddrInReusableArenaAllocator(PtrOp, ThisObj)) {
        // The stored value should be something with !types attached
        // to it to indicate it is a vtable object.
        auto *GV = dyn_cast_or_null<GlobalVariable>(
            ValOp->stripInBoundsConstantOffsets());
        if (!GV)
          return false;

        SmallVector<MDNode *, 2> Types;
        GV->getMetadata(LLVMContext::MD_type, Types);
        if (Types.empty())
          return false;

        VTableAssigned++;
      }

      // Check for: Obj->ArenaAllocator.List.MemManager = m;
      else if (auto *Arg = dyn_cast<Argument>(ValOp)) {
        DTransType *ArgTy = getArgType(MDReader, F, Arg->getArgNo());
        // Because the ArgTy is coming from the DTrans metadata, ensure that
        // the lookup returned a pointer type.
        if (!ArgTy->isPointerTy())
          return false;
        if (ArgTy->getPointerElementType() != Cand->getMemInterfaceType())
          return false;
        if (!isListMemManagerAddr(PtrOp, ThisObj))
          return false;

        MemManagerAssigned++;
      }

      // Check for:  Obj->ArenaAllocator.List.Head = null;
      // Check for: Obj->ArenaAllocator.List.FreeHead = null;
      else if (isa<Constant>(ValOp) && cast<Constant>(ValOp)->isNullValue()) {
        if (isListHeadAddr(PtrOp, ThisObj))
          ListHeadAssigned++;
        else if (isListFreeHeadAddr(PtrOp, ThisObj))
          ListFreeHeadAssigned++;
        else
          return false;
      } else {
        return false;
      }
    } else if (ValTy->isIntegerTy()) {
      // Check for: Obj->ArenaAllocator.blockSize = bSize;
      if (ValTy->isIntegerTy(16) && isAllocatorBlockSizeAddr(PtrOp, ThisObj)) {
        // Only allow 'bSize' to be an input argument or compile time constant.
        if (!isa<Argument>(ValOp) && !isa<ConstantInt>(ValOp))
          return false;
        BlockSizeAssigned++;

        // Capture the StoreInst that store the blockSize because this is the
        // instruction that is going to be modified to during the
        // transformation.
        BlockSizeStoreInst = SI;
      }
      // Check for: Obj->destroyBlock = 0;
      else if (ValTy->isIntegerTy(8) &&
               cast<ConstantInt>(ValOp)->isZeroValue() &&
               isDestroyBlockFlagAddr(PtrOp, ThisObj)) {
        DestroyObjFlagAssigned++;
      } else {
        return false;
      }
    }
  }

  if (DestroyObjFlagAssigned != 1 || BlockSizeAssigned != 1 ||
      MemManagerAssigned != 1 || VTableAssigned != 1 || ListHeadAssigned != 1 ||
      ListFreeHeadAssigned != 1)
    return false;

  if (!verifyAllInstsProcessed(F))
    return false;

  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP,
                  dbgs() << "   Recognized Constructor: " << F->getName()
                         << "\n";);
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
  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP, {
    dbgs() << "   Recognizing AllocateBlock Functionality " << F->getName()
           << "\n";
  });
  Visited.clear();
  Argument *ThisObj = &*F->arg_begin();
  BasicBlock *CreateNewHeadBB = nullptr;
  BasicBlock *NotEmptyBB = nullptr;
  LoadInst *Begin = nullptr;

  if (!identifyListEmpty(&F->getEntryBlock(), ThisObj, &CreateNewHeadBB,
                         &NotEmptyBB, &Begin))
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

  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP, {
    dbgs() << "   Recognized AllocateBlock: " << F->getName() << "\n";
  });
  return true;
}

bool MemManageTransImpl::run(Module &M) {
  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP,
                  { dbgs() << "MemManageTransOP transformation: \n"; });

  // Collect candidates.
  DTransLibraryInfo DTransLibInfo(TM, GetTLI);
  DTransLibInfo.initialize(M);
  FunctionTypeResolver TypeResolver(MDReader, DTransLibInfo);
  if (!gatherCandidates(M, TypeResolver))
    return false;

  DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP, {
    if (MemManageCandidateInfo *Cand = getCurrentCandidate())
      Cand->dump();
  });

  DTAC.populateAllocDeallocTable(M);
  if (!analyzeCandidates(M))
    return false;

  // Categorize functions based on signatures.
  if (!categorizeFunctions())
    return false;

  // Check callsite restrictions for AllocateBlock and CommitBlock.
  if (!checkCallSiteRestrictions()) {
    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP,
                    { dbgs() << "   Failed: Callsite restrictions\n"; });
    return false;
  }

  // Recognize functionality of each interface function.
  if (!recognizeFunctions()) {
    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP,
                    { dbgs() << "   Failed: Recognizing functionality\n"; });
    return false;
  }

  // TODO: Implement the pass.
  return false;
}

} // end anonymous namespace

namespace llvm {

namespace dtransOP {

bool MemManageTransOPPass::runImpl(Module &M, WholeProgramInfo &WPInfo,
                                   MemManageTransOPPass::MemTLITy GetTLI) {
  if (!dtrans::shouldRunOpaquePointerPasses(M)) {
    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP,
                    dbgs() << "MemManageTransOP inhibited: opaque pointer "
                              "passes NOT in use\n");
    return false;
  }

  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX2;
  if (!WPInfo.isWholeProgramSafe() || !WPInfo.isAdvancedOptEnabled(TTIAVX2)) {
    DEBUG_WITH_TYPE(DTRANS_MEMMANAGETRANSOP,
                    dbgs() << "MemManageTransOP inhibited: "
                           << "Not whole-program-safe or not AVX2\ns");
    return false;
  }

  DTransTypeManager TM(M.getContext());
  TypeMetadataReader MDReader(TM);
  if (!MDReader.initialize(M))
    return false;

  MemManageTransImpl MemManageTransI(TM, MDReader, GetTLI);
  return MemManageTransI.run(M);
}

PreservedAnalyses MemManageTransOPPass::run(Module &M,
                                            ModuleAnalysisManager &AM) {
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function *>(&F)));
  };

  if (!runImpl(M, WPInfo, GetTLI))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

} // namespace dtransOP

} // namespace llvm
