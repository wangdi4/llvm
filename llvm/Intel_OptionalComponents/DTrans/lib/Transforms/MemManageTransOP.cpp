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

#define DTRANS_MEMMANAGETRANSOP "dtrans-memmanagetransop"

using namespace llvm;
using namespace dtransOP;

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
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

class MemManageTransImpl {

  constexpr static int MaxNumCandidates = 1;

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

  // Set of Interface functions and their users.
  SmallPtrSet<Function *, 32> RelatedFunctions;

  bool gatherCandidates(Module &M, FunctionTypeResolver &TypeResolver);
  bool analyzeCandidates(Module &M);
  bool categorizeFunctions();

  bool isUsedOnlyInUnusedVTable(Value *);
  bool checkInterfaceFunctions();
  bool checkCallSiteRestrictions();

  bool isStrObjPtrType(DTransType *Ty);
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
  }
  assert(FunctionalityMap.size() == NumInterfacefunctions &&
         "Unexpected functionality");
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
