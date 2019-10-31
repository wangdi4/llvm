//===------ SOAToAOSPrepare.cpp - SOAToAOSPreparePass ---------------------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans SOAToAOS prepare pass to perform
// transformations that enable SOATOSOA for more candidates.
//
// Prepare layout information for candidates.
//
// In the below example, SOAToAOS transformation considers
// first two fields as candidates since they look like
// vector classes and rejects 3rd field as candidate. This
// pass converts the class of 3rd field to simple vector
// class (i.e similar to first two fields) by eliminating
// VTable pointer and removing wrapper class after doing
// legality checks.
//
// %class.FieldValueMap = type {
//    %class.vector*,
//    %class.vector.0*,
//    %class.refvector*,
//    MemoryManager*
// }
// %class.vector = type {
//    i8, i32, i32,
//    %class.elem**,
//    MemoryManager*
// }
// %class.vector.0 = type {
//    i8, i32, i32,
//    %class.elem0**,
//    MemoryManager*
// }
// %class.refvector = type { %class.basevector }
// %class.basevector = type {
//    Vtable *,
//    i8, i32, i32,
//    %class.elem2**,
//    MemoryManager*
// }
//
// This pass also fix member function calls, combine multiple
// calls into single call etc so that 3rd field can be considered
// as candidate by SOAToAOS transformation. This pass helps to
// avoid adding a lot of workarounds to SOAToAOS implementation.
//
// Here are transformations:
//  0th transformation: There are some dead instructions after Devirt
//  transformation. Delete them if there are any.
//
//  1st transformation: %class.refvector may be used by other parts of
//  the application in addition to %class.FieldValueMap. So, replicate
//  %class.refvector class and the member functions of the class that
//  are called in %class.FieldValueMap so that the replicated class
//  is only used by %class.FieldValueMap.
//
//   Ex: New types %class.refvector_1 and %class.basevector_1 with
//   same exact layout of original types are created and the member
//   functions are cloned.
//      %class.refvector_1 = type { %class.basevector_1 }
//      %class.basevector_1 = type {
//        Vtable *, i8, i32, i32, %class.elem2**, MemoryManager* }
//   Clone member functions of %class.refvector and %class.basevector and
//   fix member functions of %class.FieldValueMap by replacing original
//   member function calls with the cloned member function calls.
//
//  2nd transformation: Now, simplify the classes. During the analysis of
//  member functions of class, we already proved that there is no use of
//  Vtable and derived class. So, eliminate them to simplify and fix
//  member functions.
//
//   Ex:
//      %class.new_vector = { i8, i32, i32, %class.elem2**, MemoryManager* }
//      %class.FieldValueMap_1 = type {
//         %class.vector*,
//         %class.vector.0*,
//         %class.new_vector*,
//         MemoryManager* }
//
//  3rd transformation: Due to class inheritance, there are wrapper routines.
//  Simplify them by inlining wrapper routines.
//
//  Ex: Derived class Constructor just calls base class constructor.
//
// TODO: Add more examples later.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/SOAToAOSPrepare.h"
#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/Analysis/DTransAnnotator.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/DTransOptUtils.h"
#include "Intel_DTrans/Transforms/SOAToAOSExternal.h"

#include "SOAToAOSClassInfo.h"

#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/GlobalStatus.h"
#include "llvm/Transforms/Utils/Local.h"

#define DTRANS_SOATOAOSPREPARE "dtrans-soatoaos-prepare"

using namespace llvm;
using namespace dtrans;
using namespace soatoaos;

namespace llvm {
namespace dtrans {
namespace soatoaos {

// Index value to indicate that the field is deleted.
constexpr static unsigned DeletedField = ((unsigned)-1);

class SOAToAOSPrepCandidateInfo {

public:
  SOAToAOSPrepCandidateInfo(Module &M, const DataLayout &DL,
                            DTransAnalysisInfo &DTInfo, MemGetTLITy GetTLI,
                            MemInitDominatorTreeType GetDT)
      : M(M), DL(DL), DTInfo(DTInfo), GetTLI(GetTLI), GetDT(GetDT){};

  ~SOAToAOSPrepCandidateInfo() {
    if (CandI)
      delete CandI;
    if (ClassI)
      delete ClassI;
  }
  bool isCandidateField(Type *, unsigned);
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printCandidateInfo();
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  // Returns candidate struct type.
  StructType *getStructTy() { return CandI->getStructTy(); }

  // Returns replicated DerivedTy.
  StructType *getReplicatedDTy() { return ReplicatedDTy; }

  // Returns replicated BaseTy.
  StructType *getReplicatedBTy() { return ReplicatedBTy; }

  // Returns new struct type that is created during transformation.
  StructType *getNewStructTy() { return NewStructTy; }

  // Returns new vector field class that is created during transformation.
  StructType *getNewElemTy() { return NewElemTy; }

  // Returns position of candidate vector field class.
  int32_t getCandidateField() { return ClassI->getFieldIdx(); }

  void removeDevirtTraces();
  void replicateEntireClass();
  void simplifyCalls();
  void cleanupClonedFunctions(Function &);
  unsigned getNewIndex(Value *);
  void prepareTypes(LLVMContext &, StringRef);
  void populateTypes(LLVMContext &, SmallVector<Type *, 6> &);
  void processFunction(Function &);
  void postprocessFunction(Function &, Function &);

private:
  Module &M;
  const DataLayout &DL;
  DTransAnalysisInfo &DTInfo;
  MemGetTLITy GetTLI;
  MemInitDominatorTreeType GetDT;

  // ClassInfo for the candidate field class.
  ClassInfo *ClassI = nullptr;

  // Info of candidate Struct.
  MemInitCandidateInfo *CandI = nullptr;

  // Candidate field class which is derived from BaseTy.
  StructType *DerivedTy = nullptr;

  // Base type of candidate field class.
  StructType *BaseTy = nullptr;

  // Replicated DerivedTy
  StructType *ReplicatedDTy = nullptr;

  // Replicated BaseTy
  StructType *ReplicatedBTy = nullptr;

  // Transformed StructTy
  StructType *NewStructTy = nullptr;

  // New candidate field type after transforming DerivedTy and BaseTy.
  StructType *NewElemTy = nullptr;

  DenseMap<Function *, Function *> OrigToNewFuncMap;

  // Virtual table pointer field will be removed in BaseTy. This helps to
  // map old offsets of fields in BaseTy to offsets of fields in NewElemTy.
  SmallVector<uint32_t, 8> NewIndices;

  // Original wrapper for Ctor
  Function *CtorWrapper = nullptr;

  // Replicated wrapper for Ctor
  Function *ReplicatedCtorWrapper = nullptr;

  // New wrapper for Ctor after complete TypeRemap
  Function *NewCtorWrapper = nullptr;

  // Original wrapper for Dtor
  Function *DtorWrapper = nullptr;

  // Replicated wrapper for Dtor
  Function *ReplicatedDtorWrapper = nullptr;

  // New wrapper for Dtor after complete TypeRemap
  Function *NewDtorWrapper = nullptr;

  void replicateTypes();
  void replicateMemberFunctions();
  void removeUsers(Value *);
};

// This routine analyzes that field of Ty at Offset is potential candidate
// vector class.
//  Ex: %class.refvector will be considered as candidate.
//    %class.refvector = type { %class.basevector }
//    %class.basevector = type {
//       Vtable *, i8, i32, i32, %class.elem2**, MemoryManager* }
//
bool SOAToAOSPrepCandidateInfo::isCandidateField(Type *Ty, unsigned Offset) {

  std::unique_ptr<MemInitCandidateInfo> CandD(new MemInitCandidateInfo());

  // Check if it is a candidate field.
  Type *DTy = CandD->isSimpleDerivedVectorType(Ty, Offset);
  if (!DTy)
    return false;
  DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
    dbgs() << "  SOAToAOSPrepare: Candidate selected for more analysis\n";
    dbgs() << "    Candidate struct: " << getStructName(CandD->getStructTy());
    dbgs() << "    FieldOff: " << Offset << "\n";
  });

  // Check if member functions are okay.
  if (!CandD->collectMemberFunctions(M))
    return false;

  // Collect Derived and Base types.
  CandI = CandD.release();
  Type *BTy = getMemInitSimpleBaseType(DTy);
  assert(BTy && "Unexpected Base Type");
  DerivedTy = dyn_cast<StructType>(DTy);
  BaseTy = dyn_cast<StructType>(BTy);
  assert(DerivedTy && BaseTy && "Unexpected Derived and Base Types");

  // Analyze member functions to make sure it is vector class.
  std::unique_ptr<ClassInfo> ClassD(
      new ClassInfo(DL, DTInfo, GetTLI, GetDT, CandI, Offset, false));
  if (!ClassD->analyzeClassFunctions()) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
      dbgs() << "  Candidate failed after functionality analysis.\n";
    });
    return false;
  }

  // Makes sure it has valid CtorWrapper and DtorWrapper.
  CtorWrapper = ClassD->getCtorWrapper();
  DtorWrapper = ClassD->getDtorWrapper();
  if (!CtorWrapper || !DtorWrapper) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
      dbgs() << "  Candidate failed due to missing ctor/dtor.\n";
    });
    return false;
  }

  ClassI = ClassD.release();
  return true;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void SOAToAOSPrepCandidateInfo::printCandidateInfo() {
  CandI->printCandidateInfo();
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Remove dead instructions if there are any. There may be load/bitcast/
// getelementptr dead instructions due to Devirt.
void SOAToAOSPrepCandidateInfo::removeDevirtTraces() {
  SmallVector<Instruction *, 4> DeadInsts;

  DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE,
                  { dbgs() << "  Transformations 0: \n"; });

  // Collect dead instructions.
  for (auto *StructF : CandI->struct_functions()) {
    for (auto &I : instructions(StructF))
      if (isInstructionTriviallyDead(&I)) {
        DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE,
                        { dbgs() << "    Recursively Delete " << I << "\n"; });
        DeadInsts.push_back(&I);
      }
    if (!DeadInsts.empty())
      RecursivelyDeleteTriviallyDeadInstructions(DeadInsts);
  }
}

// Remove all users of "V" and then "V".
void SOAToAOSPrepCandidateInfo::removeUsers(Value *V) {

  std::function<void(Value * V, SmallSetVector<Value *, 8> & AllUsersSet)>
      CollectAllUsers;

  // Collect all uses of "V" recursively.
  CollectAllUsers =
      [&CollectAllUsers](Value *V, SmallSetVector<Value *, 8> &AllUsersSet) {
        if (!AllUsersSet.insert(V))
          return;
        for (Value *U : V->users())
          CollectAllUsers(U, AllUsersSet);
      };

  SmallSetVector<Value *, 8> AllUsersSet;
  CollectAllUsers(V, AllUsersSet);

  for (auto *U : reverse(AllUsersSet)) {
    if (auto *I = dyn_cast<Instruction>(U)) {
      I->eraseFromParent();
    } else if (auto *C = dyn_cast<Constant>(U)) {
      if (isSafeToDestroyConstant(C))
        C->destroyConstant();
    }
  }
}

// Create new derived and base types with exact same layout of DerivedTy
// and BaseTy.
void SOAToAOSPrepCandidateInfo::replicateTypes() {
  assert(BaseTy && DerivedTy && "Unexpected Base and Derived types");

  ReplicatedBTy = StructType::create(
      M.getContext(),
      (Twine("") + "_REP_" + cast<StructType>(BaseTy)->getName()).str());

  SmallVector<Type *, 6> StructElems(BaseTy->element_begin(),
                                     BaseTy->element_end());
  ReplicatedBTy->setBody(StructElems);

  ReplicatedDTy = StructType::create(
      M.getContext(),
      (Twine("") + "_REP_" + cast<StructType>(DerivedTy)->getName()).str());
  SmallVector<Type *, 6> Elems;
  Elems.push_back(ReplicatedBTy);
  ReplicatedDTy->setBody(Elems);

  DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
    dbgs() << "    New DerivedTy " << *ReplicatedDTy << "\n";
    dbgs() << "    New BaseTy " << *ReplicatedBTy << "\n";
  });
}

// This routine does the following:
//
// 1. Clone member functions of DerivedTy and BaseTy with type mapping
//    DerivedTy/ReplicatedDTy and BaseTy/ReplicatedBTy.
// 2. For all struct member functions:  Replace original member function
//    calls of DerivedTy/BaseTy with the cloned member functions.
// 3. For all struct member functions: Replace DerivedTy and BaseTy with
//    ReplicatedDTy and ReplicatedBTy using ValueMapper.
// 4. Fix CallInfo in all struct member functions and class member functions.
//
void SOAToAOSPrepCandidateInfo::replicateMemberFunctions() {

  std::function<void(Function * F,
                     SmallPtrSet<Function *, 32> & ProcessedFuncs)>
      UpdateCGWithClonedMemberFunctions;

  // Recursive function to replace original field member function with
  // cloned functions in "F" using OrigToNewFuncMap map.
  UpdateCGWithClonedMemberFunctions =
      [this, &UpdateCGWithClonedMemberFunctions](
          Function *F, SmallPtrSet<Function *, 32> &ProcessedFunctions) {
        if (!F || F->isDeclaration())
          return;
        if (!ProcessedFunctions.insert(F).second)
          return;
        for (auto &I : instructions(F))
          if (auto *CB = dyn_cast<CallBase>(&I)) {
            auto *OrigF = dtrans::getCalledFunction(*CB);
            if (OrigF && ClassI->isCandidateMemberFunction(OrigF)) {
              Function *NewF = OrigToNewFuncMap[OrigF];
              assert(NewF && "Expected cloned function");
              CB->setCalledFunction(NewF);
              UpdateCGWithClonedMemberFunctions(NewF, ProcessedFunctions);
            } else {
              UpdateCGWithClonedMemberFunctions(OrigF, ProcessedFunctions);
            }
          }
      };

  // Collect CallInfos in "F".
  auto CollectCallInfoInFunction =
      [this](Function *F, SmallPtrSet<CallInfo *, 16> &CallInfos) {
        for (Instruction &I : instructions(F)) {
          auto *CB = dyn_cast<CallBase>(&I);
          if (!CB)
            continue;
          auto *CInfo = DTInfo.getCallInfo(CB);
          if (!CInfo)
            continue;
          CallInfos.insert(CInfo);
        }
      };

  // Fix pointer type of "CInfo" using "TypeRemapper".
  auto FixCInfoPointerType = [](CallInfo *CInfo,
                                DTransTypeRemapper &TypeRemapper) {
    dtrans::PointerTypeInfo &PTI = CInfo->getPointerTypeInfoRef();
    size_t Num = PTI.getNumTypes();
    for (size_t i = 0; i < Num; ++i)
      PTI.setType(i, TypeRemapper.remapType(PTI.getType(i)));
  };

  ValueToValueMapTy VMap;
  DTransTypeRemapper TypeRemapper;

  assert(ReplicatedDTy && ReplicatedBTy && "Unexpected cloned types");

  // Type mapping DerivedTy/ReplicatedDTy and BaseTy/ReplicatedBTy.
  TypeRemapper.addTypeMapping(DerivedTy, ReplicatedDTy);
  TypeRemapper.addTypeMapping(BaseTy, ReplicatedBTy);
  TypeRemapper.setAllTypeMappingsAdded();

  SmallPtrSet<CallInfo *, 16> FieldClassCallInfos;
  for (auto *OrigF : ClassI->field_member_functions())
    CollectCallInfoInFunction(OrigF, FieldClassCallInfos);

  // Clone all field member functions.
  for (auto *OrigF : ClassI->field_member_functions()) {
    Type *FuncTy = OrigF->getType();
    Type *ReplTy = TypeRemapper.remapType(FuncTy);
    assert(ReplTy != FuncTy && "Unexpected cloning");
    Function *NewF =
        Function::Create(cast<FunctionType>(ReplTy->getPointerElementType()),
                         OrigF->getLinkage(), OrigF->getName(), &M);
    NewF->copyAttributesFrom(OrigF);
    VMap[OrigF] = NewF;
    OrigToNewFuncMap[OrigF] = NewF;
    Function::arg_iterator DestI = NewF->arg_begin();
    for (Argument &I : OrigF->args()) {
      DestI->setName(I.getName());
      VMap[&I] = &*DestI++;
    }

    SmallVector<ReturnInst *, 8> Returns;
    ClonedCodeInfo CodeInfo;
    CloneFunctionInto(NewF, OrigF, VMap, true, Returns, "", &CodeInfo,
                      &TypeRemapper);

    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
      dbgs() << "    Cloning " << OrigF->getName() << "\n";
      dbgs() << "    After cloning \n" << *NewF << "\n";
    });
  }

  // Fix CallInfo in all field member functions.
  for (auto *CInfo : FieldClassCallInfos) {
    DTInfo.replaceCallInfoInstruction(
        CInfo, cast<Instruction>(VMap[CInfo->getInstruction()]));
    FixCInfoPointerType(CInfo, TypeRemapper);
  }

  SmallPtrSet<CallInfo *, 16> StructCallInfos;
  for (auto *OrigF : CandI->struct_functions())
    CollectCallInfoInFunction(OrigF, StructCallInfos);

  // Fix call-graph to replace original calls with cloned calls.
  SmallPtrSet<Function *, 32> ProcessedFunctions;
  for (auto *StructF : CandI->struct_functions())
    UpdateCGWithClonedMemberFunctions(StructF, ProcessedFunctions);

  // Replace DerivedTy and BaseTy types with ReplicatedDTy andReplicatedBTy.
  // After this transformation, IR for these member functions is NOT
  // completely valid since the candidate struct (%class.FieldValueMap)
  // is not updated yet. This could be fixed this but I think it is okay
  // since it will be fixed in the later part of the transformation.
  for (auto *StructF : CandI->struct_functions()) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
      dbgs() << "    Typemapping " << StructF->getName() << "\n";
    });

    ValueMapper(VMap, RF_IgnoreMissingLocals, &TypeRemapper)
        .remapFunction(*StructF);

    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
      dbgs() << "    After Typemapping \n" << *StructF << "\n";
    });
  }

  // Fix CallInfo in all field member functions. Here, CallInfoInstruction
  // is not replaced in CInfo since struct member functions are
  // not really cloned.
  for (auto *CInfo : StructCallInfos)
    FixCInfoPointerType(CInfo, TypeRemapper);
}

// Replicate BaseTy and DerivedTy classes and their member functions that
// are used in candidate struct methods.
void SOAToAOSPrepCandidateInfo::replicateEntireClass() {
  DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE,
                  { dbgs() << "  Transformations 1: \n"; });
  replicateTypes();
  replicateMemberFunctions();

  assert(CtorWrapper && DtorWrapper && " Ctor/Dtor Wrappers missing");
  ReplicatedCtorWrapper = OrigToNewFuncMap[CtorWrapper];
  ReplicatedDtorWrapper = OrigToNewFuncMap[DtorWrapper];
}

// Inline cloned versions of CtorWrapper and DtorWrapper calls
// to simplify.
void SOAToAOSPrepCandidateInfo::simplifyCalls() {

  // Inline callsite "CB".
  auto InlineCS = [](CallBase *CB) {
    InlineFunctionInfo IFI;
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE,
                    { dbgs() << "  Inlining Call: " << *CB << "\n"; });
    bool InlineStatus = InlineFunction(CB, IFI);
    assert(InlineStatus && "inline must succeed");
    (void)InlineStatus;
  };

  // Inline all callsites of F.
  auto InlineFunction = [InlineCS](Function *F) {
    for (auto *U : F->users()) {
      auto *CB = dyn_cast<CallBase>(U);
      assert(CB && "Unexpected call");
      InlineCS(CB);
    }
  };

  DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE,
                  { dbgs() << "  Transformations 3: \n"; });

  assert(NewCtorWrapper && NewDtorWrapper && "Expected valid wrapper");

  InlineFunction(NewCtorWrapper);
  InlineFunction(NewDtorWrapper);
}

// Delete unnecessary BitCastInst. This helps downstream ClassInfo
// analysis to identify functionalities of member functions.
void SOAToAOSPrepCandidateInfo::cleanupClonedFunctions(Function &Func) {
  SmallPtrSet<BitCastInst *, 32> DelSet;
  for (auto &I : instructions(Func)) {
    auto *BC = dyn_cast<BitCastInst>(&I);
    if (!BC)
      continue;
    if (BC->getType() != BC->getSrcTy())
      continue;
    DelSet.insert(BC);
  }
  for (auto *BC : DelSet) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
      dbgs() << "    Delete unnecessary BitCastInst: " << *BC << "\n";
    });
    BC->replaceAllUsesWith(BC->getOperand(0));
    BC->eraseFromParent();
  }
}

// Returns new index for the given "Idx".
unsigned SOAToAOSPrepCandidateInfo::getNewIndex(Value *Idx) {
  auto *FIdx = dyn_cast<ConstantInt>(Idx);
  assert(FIdx && "Unexpected GEP Index");
  unsigned OrigIdx = FIdx->getLimitedValue();
  return NewIndices[OrigIdx];
}

// Create new types for candidate struct (%class.FieldValueMap) and field
// type (%class.refvector and %class.basevector).
void SOAToAOSPrepCandidateInfo::prepareTypes(LLVMContext &Context,
                                             StringRef DepTypePrefix) {
  // Create new candidate struct class.
  StructType *Struct = CandI->getStructTy();
  NewStructTy = StructType::create(
      Context, (Twine(DepTypePrefix) + Struct->getName()).str());

  // Create new field class
  NewElemTy = StructType::create(
      Context, (Twine(DepTypePrefix) + ReplicatedDTy->getName()).str());
}

// Fill/Fix new struct and field types.
//   Field type before:
//     %class.refvector = type { %class.basevector }
//     %class.basevector = type { Vtable *, i8, i32, i32, %class.elem2**,
//                                MemoryManager*  }
//
//   Field type after (Vtable pointer is deleted. Base type is peeled):
//     %class.refvector_new = type { i8, i32, i32, %class.elem2**,
//                                MemoryManager*  }
//
//   Struct type before:
//     %class.FieldValueMap = type { %class.vector*, %class.vector.0*,
//        %class.refvector*, MemoryManager* }
//
//   Struct type after (Only candidate field pointer is updated):
//     %class.FieldValueMap_new = type { %class.vector*, %class.vector.0*,
//        %class.refvector_new*, MemoryManager* }
//
void SOAToAOSPrepCandidateInfo::populateTypes(
    LLVMContext &Context, SmallVector<Type *, 6> &RemappedFields) {
  SmallVector<Type *, 6> ElemFields;

  // Fill new field type.
  unsigned IdxCount = 0;
  unsigned I = 0;
  for (auto *ETy : ReplicatedBTy->elements()) {
    // Ignore VTable pointer field.
    if (isPtrToVFTable(ETy)) {
      NewIndices.push_back(DeletedField);
    } else {
      // Remapped type is used.
      ElemFields.push_back(RemappedFields[I]);
      NewIndices.push_back(IdxCount++);
    }
    I++;
  }
  NewElemTy->setBody(ElemFields);

  // Fix the candidate field of the struct with new field type pointer.
  StructType *Struct = CandI->getStructTy();
  SmallVector<Type *, 6> StructFields(Struct->element_begin(),
                                      Struct->element_end());
  StructFields[getCandidateField()] = NewElemTy->getPointerTo(0);
  NewStructTy->setBody(StructFields);

  DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
    dbgs() << "    New Field type: " << *NewElemTy << "\n";
    dbgs() << "    New Struct type: " << *NewStructTy << "\n";
  });
}

// This routine processes GetElementPtrInsts in field member functions
// of cloned field class. There is no need to process instructions
// in candidate struct methods since there is no change in layout of it.
// All deleted fields and derived class references are handled in this
// routine.
//
//  ReplicatedBTy:
//     case 1 (NumIndices = 1): Nothing to change.
//
//     case 2 (NumIndices = 2):
//             Remove GEPs and their uses related to deleted fields.
//
//     case 3 (NumIndices > 2): Not expected to have more than 2.
//
//  ReplicatedDTy:
//     case 1 (NumIndices = 1): Nothing to change.
//
//     case 2 (NumIndices = 2): Since it has only one field (i.e Base
//             class object), it is basically accessing it. Just
//             convert the GEP to Bitcast.
//             Ex:
//               Before:
//                 %8 = getelementptr %"refvector_new", %"refvector_new"* %4,
//                         i64 0, i32 0
//               After:
//                 %7 = bitcast %"refvector_new"* %4 to %"basevector_new"*
//
//     case 3 (NumIndices = 3): This is basically accessing fields in base
//            class.
//             a. Deleted fields: Remove GEPs and their uses related to them.
//             b. Other fields: Fix the GEP to eliminate references through
//                derived class.
//                Ex:
//                  Before:
//                     %2 = getelementptr %"refvector_new",
//                               %"refvector_new"* %0, i64 0, i32 0, i32 1
//                  After:
//                     %2 = bitcast %"refvector_new"* %0 to %"basevector_new"*
//                     %3 = getelementptr %"basevector_new",
//                               %"basevector_new"* %2, i64 0, i32 1
//
//     case 4 (NumIndices > 3): Not expected to have more than 3.
//
void SOAToAOSPrepCandidateInfo::processFunction(Function &Func) {
  SmallVector<GetElementPtrInst *, 2> GEPsToRemove;
  SmallVector<GetElementPtrInst *, 2> GEPsToVoid;

  for (auto &I : instructions(Func)) {
    auto *GEP = dyn_cast<GetElementPtrInst>(&I);
    if (!GEP)
      continue;
    Type *StTy = GEP->getSourceElementType();

    unsigned NumIndices = GEP->getNumIndices();
    if (NumIndices == 1)
      continue;
    if (StTy == ReplicatedDTy) {
      if (NumIndices == 2) {
        assert(GEP->hasAllZeroIndices() && "Expected all zero indices");
        GEPsToVoid.push_back(GEP);
      } else {
        assert(NumIndices == 3 && "Unexpected GEP indices");
        if (getNewIndex(GEP->getOperand(3)) == DeletedField) {
          GEPsToRemove.push_back(GEP);
        } else {
          Type *DstTy = ReplicatedBTy->getPointerTo();
          Value *Src = GEP->getPointerOperand();
          CastInst *BC = CastInst::CreateBitOrPointerCast(Src, DstTy, "", GEP);

          SmallVector<Value *, 2> Indices;
          Indices.push_back(GEP->getOperand(1));
          Indices.push_back(GEP->getOperand(3));
          GetElementPtrInst *NGEP =
              GetElementPtrInst::Create(ReplicatedBTy, BC, Indices, "", GEP);
          NGEP->setIsInBounds(GEP->isInBounds());

          DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
            dbgs() << "    GEP is replaced with: " << *GEP << "\n";
          });

          GEP->replaceAllUsesWith(NGEP);
          NGEP->takeName(GEP);
          GEPsToRemove.push_back(GEP);
          DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
            dbgs() << "    BC:  " << *BC << "\n";
            dbgs() << "    New GEP:  " << *NGEP << "\n";
          });
        }
      }
    } else if (StTy == ReplicatedBTy) {
      assert(NumIndices == 2 && "Unexpected GEP indices");
      if (getNewIndex(GEP->getOperand(2)) == DeletedField)
        GEPsToRemove.push_back(GEP);
    }
  }

  for (auto *GEP : GEPsToVoid) {
    Instruction *Res = CastInst::CreateBitOrPointerCast(
        GEP->getPointerOperand(), GEP->getType(), "", GEP);
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
      dbgs() << "    GEP: " << *GEP << "\n";
      dbgs() << "       replaced with \n";
      dbgs() << "    BC: " << *Res << "\n";
    });
    GEP->replaceAllUsesWith(Res);
    GEP->eraseFromParent();
  }
  for (auto *GEP : GEPsToRemove) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE,
                    { dbgs() << "    GEP (field) deleted: " << *GEP << "\n"; });
    removeUsers(GEP);
  }
}

// This basically handles GetElementPtrInst and CallInfo instructions.
//
//  Alloc/Mem Calls: Nothing to be done for struct candidate since there
//  is no change in layout.
//  Ex:
//   Before:
//     %24 = call i8* @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 40,
//                               %"MemoryManager"* %23)
//
//   After:
//     %24 = call i8* @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 32,
//                               %"MemoryManager"* %23)
//
//  GetElementPtrInst:
//     case 1 (NumIndices = 1): Nothing to change.
//
//     case 2 (NumIndices = 2):
//            Deleted fields are not expected here. Just fix indices for
//            other fields of GEPs.
//            Ex:
//             Before:
//              %3 = getelementptr %"refvector_new", %"refvector_new"* %2,
//                                   i64 0, i32 1
//             After:
//              %3 = getelementptr %"refvector_new", %"refvector_new"* %2,
//                                   i64 0, i32 0
//
//     case 3 (NumIndices > 2): Not expected to have more than 2.
//
void SOAToAOSPrepCandidateInfo::postprocessFunction(Function &F,
                                                    Function &OrigF) {

  // Get Cloned CtorWrapper and DtorWrapper.
  if (ReplicatedCtorWrapper == &OrigF)
    NewCtorWrapper = &F;
  else if (ReplicatedDtorWrapper == &OrigF)
    NewDtorWrapper = &F;

  for (auto &I : instructions(F)) {
    if (auto *GEP = dyn_cast<GetElementPtrInst>(&I)) {
      Type *ElementTy = GEP->getSourceElementType();
      unsigned NumIndices = GEP->getNumIndices();
      if (NumIndices == 1 || ElementTy != NewElemTy)
        continue;
      assert(NumIndices == 2 && " Unexpected indices for GEP");
      unsigned NewIdx = getNewIndex(GEP->getOperand(2));
      assert(NewIdx != DeletedField && " Delete field shouldn't be here");
      DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE,
                      { dbgs() << "    GEP before: " << *GEP << "\n"; });
      // Expected NewIdx is different from original index since VTable
      // pointer, which is usually the first element, is deleted.
      Value *IdxValue = ConstantInt::get(GEP->getOperand(2)->getType(), NewIdx);
      GEP->setOperand(2, IdxValue);
      DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE,
                      { dbgs() << "    GEP after: " << *GEP << "\n"; });
    } else if (auto *CB = dyn_cast<CallBase>(&I)) {
      auto *CInfo = DTInfo.getCallInfo(CB);
      if (!CInfo || isa<dtrans::FreeCallInfo>(CInfo))
        continue;

      for (auto *PTy : CInfo->getPointerTypeInfoRef().getTypes()) {
        Type *StTy = PTy->getPointerElementType();
        if (StTy != NewElemTy)
          continue;
        DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE,
                        { dbgs() << "    Call before: " << *CB << "\n"; });
        const TargetLibraryInfo &TLI = GetTLI(F);
        updateCallSizeOperand(&I, CInfo, ReplicatedDTy, StTy, TLI);
        DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE,
                        { dbgs() << "    Call after: " << *CB << "\n"; });
      }
    }
  }
  cleanupClonedFunctions(F);
}

class SOAToAOSPrepareTransImpl : public DTransOptBase {
public:
  SOAToAOSPrepareTransImpl(DTransAnalysisInfo &DTInfo, LLVMContext &Context,
                           const DataLayout &DL, MemGetTLITy GetTLI,
                           StringRef DepTypePrefix,
                           DTransTypeRemapper *TypeRemapper,
                           SOAToAOSPrepCandidateInfo *CandI)
      : DTransOptBase(&DTInfo, Context, DL, GetTLI, DepTypePrefix,
                      TypeRemapper),
        CandI(CandI) {}
  ~SOAToAOSPrepareTransImpl() {}

private:
  SOAToAOSPrepCandidateInfo *CandI;

  SOAToAOSPrepareTransImpl(const SOAToAOSPrepareTransImpl &) = delete;
  SOAToAOSPrepareTransImpl &
  operator=(const SOAToAOSPrepareTransImpl &) = delete;

  bool prepareTypes(Module &M) override;
  void populateTypes(Module &M) override;
  void processFunction(Function &OrigFunc) override;
  void postprocessFunction(Function &OrigFunc, bool isCloned) override;
};

// Creates new types needed for the candidate and maps old and new
// types.
bool SOAToAOSPrepareTransImpl::prepareTypes(Module &M) {
  // Map original struct type to new struct type.
  CandI->prepareTypes(M.getContext(), DepTypePrefix);
  TypeRemapper->addTypeMapping(CandI->getStructTy(), CandI->getNewStructTy());

  // Map ReplicatedDTy/ReplicatedBTy to new field type.
  StructType *NewElemTy = CandI->getNewElemTy();
  TypeRemapper->addTypeMapping(CandI->getReplicatedDTy(), NewElemTy);
  TypeRemapper->addTypeMapping(CandI->getReplicatedBTy(), NewElemTy);
  return true;
}

// Add elements to new field and struct classes.
void SOAToAOSPrepareTransImpl::populateTypes(Module &M) {
  // Get remapped type for each element of field class here since
  // TypeRemapper is not available in SOAToAOSPrepCandidateInfo.
  SmallVector<Type *, 6> RemappedFields;
  StructType *RType = CandI->getReplicatedBTy();
  for (auto *ETy : RType->elements())
    RemappedFields.push_back(TypeRemapper->remapType(ETy));

  CandI->populateTypes(M.getContext(), RemappedFields);
}

void SOAToAOSPrepareTransImpl::processFunction(Function &Func) {
  CandI->processFunction(Func);
}

void SOAToAOSPrepareTransImpl::postprocessFunction(Function &Func,
                                                   bool isCloned) {
  Function *F = isCloned ? OrigFuncToCloneFuncMap[&Func] : &Func;
  CandI->postprocessFunction(*F, Func);
}

class SOAToAOSPrepareImpl {
public:
  SOAToAOSPrepareImpl(Module &M, const DataLayout &DL,
                      DTransAnalysisInfo &DTInfo, MemGetTLITy GetTLI,
                      MemInitDominatorTreeType GetDT)
      : M(M), DL(DL), DTInfo(DTInfo), GetTLI(GetTLI), GetDT(GetDT) {}

  ~SOAToAOSPrepareImpl() {
    for (auto *Cand : Candidates) {
      delete Cand;
    }
    Candidates.clear();
  }

  bool run(void);

private:
  constexpr static int MaxNumCandidates = 1;
  constexpr static int MaxNumPotentialArrs = 1;

  Module &M;
  const DataLayout &DL;
  DTransAnalysisInfo &DTInfo;
  MemGetTLITy GetTLI;
  MemInitDominatorTreeType GetDT;
  SmallPtrSet<SOAToAOSPrepCandidateInfo *, MaxNumCandidates> Candidates;

  bool gatherCandidateInfo(void);
};

bool SOAToAOSPrepareImpl::gatherCandidateInfo() {
  for (dtrans::TypeInfo *TI : DTInfo.type_info_entries()) {
    dtrans::soatoaos::SOAToAOSCFGInfo Info;
    Type *Ty = TI->getLLVMType();

    if (!Info.populateLayoutInformation(Ty))
      continue;
    if (DTInfo.testSafetyData(TI, dtrans::DT_SOAToAOS))
      continue;
    bool FieldSafetyCheck = true;
    for (auto *FI : Info.fields()) {
      auto *FInfo = DTInfo.getTypeInfo(FI);
      if (!FInfo || DTInfo.testSafetyData(FInfo, dtrans::DT_SOAToAOS)) {
        FieldSafetyCheck = false;
        break;
      }
    }
    if (!FieldSafetyCheck)
      continue;
    if (!Info.populateCFGInformation(M, true, true))
      continue;
    if (Info.getNumPotentialArrays() != MaxNumPotentialArrs)
      continue;
    auto *ArrOffsetIt = Info.potential_arr_fields().begin();

    std::unique_ptr<SOAToAOSPrepCandidateInfo> Candidate(
        new SOAToAOSPrepCandidateInfo(M, DL, DTInfo, GetTLI, GetDT));

    if (!Candidate->isCandidateField(Ty, *ArrOffsetIt))
      continue;

    Candidates.insert(Candidate.release());
  }
  if (Candidates.empty()) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
      dbgs() << "SOAToAOSPrepare Failed: No candidates found.\n";
    });
    return false;
  }
  return true;
}

bool SOAToAOSPrepareImpl::run(void) {
  if (!gatherCandidateInfo())
    return false;
  if (Candidates.size() != MaxNumCandidates) {
    dbgs() << "SOAToAOSPrepare: Candidate found\n";
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
      dbgs() << "SOAToAOSPrepare Failed: More candidates found.\n";
    });
    return false;
  }
  auto *Candidate = *Candidates.begin();
  DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
    dbgs() << "    Candidate Passed Analysis.\n";
    Candidate->printCandidateInfo();
  });

  // 0th transform.
  Candidate->removeDevirtTraces();

  // 1st transform.
  Candidate->replicateEntireClass();

  // 2nd transform.
  DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE,
                  { dbgs() << "  Transformations 2: \n"; });
  DTransTypeRemapper TypeRemapper;
  SOAToAOSPrepareTransImpl Transformer(DTInfo, M.getContext(),
                                       M.getDataLayout(), GetTLI, "_DPRE_",
                                       &TypeRemapper, Candidate);
  Transformer.run(M);

  // 3rd transform: Inline CtorWrapper and DtorWrapper calls
  Candidate->simplifyCalls();

  // TODO: Add more code here.

  return true;
}

} // end namespace soatoaos

bool SOAToAOSPreparePass::runImpl(Module &M, DTransAnalysisInfo &DTInfo,
                                  MemGetTLITy GetTLI, WholeProgramInfo &WPInfo,
                                  dtrans::MemInitDominatorTreeType &GetDT) {
  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasAVX2;
  if (!WPInfo.isWholeProgramSafe() || !WPInfo.isAdvancedOptEnabled(TTIAVX2))
    return false;

  if (!DTInfo.useDTransAnalysis())
    return false;

  auto &DL = M.getDataLayout();

  SOAToAOSPrepareImpl PrepareImpl(M, DL, DTInfo, GetTLI, GetDT);

  return PrepareImpl.run();
}

PreservedAnalyses SOAToAOSPreparePass::run(Module &M,
                                           ModuleAnalysisManager &AM) {
  auto &WP = AM.getResult<WholeProgramAnalysis>(M);
  auto &DTransInfo = AM.getResult<DTransAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  MemInitDominatorTreeType GetDT = [&FAM](Function &F) -> DominatorTree & {
    return FAM.getResult<DominatorTreeAnalysis>(F);
  };
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function *>(&F)));
  };

  bool Changed = runImpl(M, DTransInfo, GetTLI, WP, GetDT);

  if (!Changed)
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

} // end namespace dtrans
} // end namespace llvm

namespace {
class DTransSOAToAOSPrepareWrapper : public ModulePass {
private:
  dtrans::SOAToAOSPreparePass Impl;

public:
  static char ID;

  DTransSOAToAOSPrepareWrapper() : ModulePass(ID) {
    initializeDTransSOAToAOSPrepareWrapperPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;

    auto &DTAnalysisWrapper = getAnalysis<DTransAnalysisWrapper>();
    DTransAnalysisInfo &DTInfo = DTAnalysisWrapper.getDTransInfo(M);
    dtrans::MemInitDominatorTreeType GetDT =
        [this](Function &F) -> DominatorTree & {
      return this->getAnalysis<DominatorTreeWrapperPass>(F).getDomTree();
    };
    auto GetTLI = [this](const Function &F) -> const TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };

    bool Changed =
        Impl.runImpl(M, DTInfo, GetTLI,
                     getAnalysis<WholeProgramWrapperPass>().getResult(), GetDT);
    if (Changed)
      DTAnalysisWrapper.setInvalidated();
    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DTransAnalysisWrapper>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};

} // namespace

char DTransSOAToAOSPrepareWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransSOAToAOSPrepareWrapper, "dtrans-soatoaos-prepare",
                      "DTrans soatoaos prepare", false, false)
INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_END(DTransSOAToAOSPrepareWrapper, "dtrans-soatoaos-prepare",
                    "DTrans soatoaos prepare", false, false)

ModulePass *llvm::createDTransSOAToAOSPrepareWrapperPass() {
  return new DTransSOAToAOSPrepareWrapper();
}
