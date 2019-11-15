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
//  4th transformation: Apply multiple transformations on constructor. Since
//  the constructor is now localized to %class.FieldValueMap_1, new
//  transformations like constant propagation can be applied. The main reason to
//  do the below transformations is to make constructor calls (and the
//  definition) look like constructor calls of other vector classes in the
//  candidate structure. These transformations help SOAToAOS to compare
//  constructor calls of all three vectors easily. Currently, no heuristics are
//  used to do these transformations. If needed, we could apply these
//  transformations based on constructor calls of other vector classes in the
//  candidate struct.
//
//   Ex: After CtorWrapper is inlined, the code will look like below.
//
//    Ctor(ptr1, 0, true, Mem); // Call 1
//    Ctor(ptr2, C, true, Mem); // Call 2
//
//   Callee (After constant propagation):
//    Ctor(%0, int %1, bool %2, Mem* %3) {
//     ...
//     store true, %flag_field;  // Value is propagated.
//     ...
//    }
//
//    Now, We know the value of flag (i.e true). Propagate the value
//    wherever the flag is used in all member functions of vector class.
//
//    Once value of the flag is propagated, there is no real use of
//    the flag any more. It doesn't matter even if we change the value
//    of flag since there are no uses.
//
//    Change the value of the flag from true to false.
//
//    Ctor(ptr1, 0, false, Mem); // Call 1
//    Ctor(ptr2, C, false, Mem); // Call 2
//
//    Ctor(%0, int %1, bool %2, Mem* %3) {
//      ...
//      store false, %flag_field;  // Value changed from true to false
//      ...
//    }
//
//    2nd argument of Ctor is dead. Move this dead argument to the end
//    of the argument list by doing Cloning and then fix callsites.
//
//    Ctor(ptr1, 0, Mem, false); // Call 1
//    Ctor(ptr2, C, Mem, false); // Call 2
//
//    Ctor(%0, int %1, Mem* %2, bool %3) {
//      ...
//      store false, %flag_field;  // Value changed from true to false
//      ...
//    }
//
//  5th transformation: If the vector class doesn't have CCtor member
//  function, detect if there is any code that is semantically equivalent
//  to cctor. If it finds one, creates CCtor/SimpleSetElem functions and
//  just replaces the code with CCtor/SimpleSetElem calls. This
//  transformation helps to combine the CCtor call with CCtor calls of
//  other vectors.
//
//  Ex:
//  Before: This is sematically equivalent to CCtor.
//     size = Src->size();
//     Ctor(Dest, capacity, flag, Mem);
//     for (i = 0; i < size; i++) {
//       Elem = GetElem(Src, i);
//       AppendElem(Dest, Elem);
//     }
//
//  We could create complete CCtor constructor that copies all fields and
//  array elements from Src object to Dest object. But, decided to go
//  with another approach like below due to implementation complexity.
//  Two new functions will be created.
//   1. Simple CCtor function: Just copies all fields from Src to Dest.
//   It doesn't copy array element from Src to Dest. Replace Ctor call
//   with CCtor call. This CCtor call will be combined with CCtor calls
//   of other vectors.
//   2. Simple SetElem function. It just sets given element at given
//   index. This is not expected to be combined with other vector calls.
//   AppendElem call will be replaced with this simple SetElem.
//
//  After:
//     size = Src->size();
//     CCtor(Dest, Src);
//     for (i = 0; i < size; i++) {
//       Elem = GetElem(Src, i);
//       SimpleSetElem(Dest, Elem, i);
//     }
//
//  6th transformation: Reverse argument promotion for AppendFunc by
//  converting pointer argument to pointer-to-pointer argument and then
//  fix callsite. The main reason for the transformation is to make
//  it similar to AppendFunc of other vector classes in the candidate
//
//  Ex:
//  Before:
//     AppendElem(this, I16* elem);
//
//     AppendElem(this, I16* el) {
//       (this + i) = el;
//     }
//
//  After:
//     %a = alloca I16*
//      ...
//     store elem, %a
//     AppendElem(this, I16** %a);
//
//     AppendElem(this, I16** el) {
//       (this + i) = *el;
//     }
//
// TODO: Need to investigate how the debug info is impacted by this
// transformation and then fix it.
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
<<<<<<< HEAD
#include "llvm/InitializePasses.h"
=======
#include "llvm/IR/IRBuilder.h"
>>>>>>> 99df7bb2efc1bbb15db6eccf1946bcb5f4760cfc
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
    if (NewClassI)
      delete NewClassI;
    if (NewCandI)
      delete NewCandI;
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
  bool computeUpdatedCandidateInfo();
  Function *applyCtorTransformations();
  void convertCtorToCCtor(Function *);
  void reverseArgPromote();

private:
  Module &M;
  const DataLayout &DL;
  DTransAnalysisInfo &DTInfo;
  MemGetTLITy GetTLI;
  MemInitDominatorTreeType GetDT;

  // ClassInfo for the candidate field class.
  ClassInfo *ClassI = nullptr;

  // This ClassInfo is recomputed for the candidate field after 3rd
  // transformation is done since layout of the types are completely
  // changed.
  ClassInfo *NewClassI = nullptr;

  // Info of candidate Struct.
  MemInitCandidateInfo *CandI = nullptr;

  // Candidate Struct info is also recomputed for the candidate field after
  // 3rd transformation is done since layout of the types are completely
  // changed.
  MemInitCandidateInfo *NewCandI = nullptr;

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

  // Indicate that constant prop is applied and flag field
  // is constant.
  bool ConstantPropApplied = false;

  bool isSafeCallForAppend(Function *);
  void updateCallBase(CallBase *, AttributeList, Function *,
                      std::vector<Value *> &);
  void removeDeadInsts(Function *);
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
  Type *DTy = CandD->isSimpleVectorType(Ty, Offset, /*AllowOnlyDerived*/ true);
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
  DtorWrapper = ClassD->getSingleMemberFunction(DestructorWrapper);
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

void SOAToAOSPrepCandidateInfo::updateCallBase(CallBase *CB,
                                               AttributeList NewPAL,
                                               Function *NewF,
                                               std::vector<Value *> &NewArgs) {
  FunctionType *NFTy = NewF->getFunctionType();
  CallBase *NewCB;
  DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE,
                  { dbgs() << "  Before CB: " << *CB << "\n"; });
  if (InvokeInst *II = dyn_cast<InvokeInst>(CB)) {
    NewCB = InvokeInst::Create(NewF, II->getNormalDest(), II->getUnwindDest(),
                               NewArgs, None, "", CB->getParent());
  } else {
    NewCB = CallInst::Create(NFTy, NewF, NewArgs, None, "", CB);
    cast<CallInst>(NewCB)->setTailCallKind(
        cast<CallInst>(CB)->getTailCallKind());
  }
  NewCB->setCallingConv(CB->getCallingConv());
  NewCB->setDebugLoc(CB->getDebugLoc());
  NewCB->setAttributes(NewPAL);
  if (!CB->use_empty() || CB->isUsedByMetadata()) {
    CB->replaceAllUsesWith(NewCB);
  }
  DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE,
                  { dbgs() << "  After CB: " << *NewCB << "\n"; });
  CB->eraseFromParent();
}

void SOAToAOSPrepCandidateInfo::removeDeadInsts(Function *F) {
  SmallVector<Instruction *, 4> DeadInsts;

  for (auto &I : instructions(F))
    if (isInstructionTriviallyDead(&I)) {
      DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE,
                      { dbgs() << "    Recursively Delete " << I << "\n"; });
      DeadInsts.push_back(&I);
    }
  if (!DeadInsts.empty())
    RecursivelyDeleteTriviallyDeadInstructions(DeadInsts);
}

// Remove dead instructions if there are any. There may be load/bitcast/
// getelementptr dead instructions due to Devirt.
void SOAToAOSPrepCandidateInfo::removeDevirtTraces() {
  DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE,
                  { dbgs() << "  Transformations 0: \n"; });

  // Collect dead instructions.
  for (auto *StructF : CandI->struct_functions())
    removeDeadInsts(StructF);
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

  // Remove NewCtorWrapper and NewDtorWrapper that are not used anymore.
  NewCtorWrapper->eraseFromParent();
  NewDtorWrapper->eraseFromParent();
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

// Compute Candidate/Class Info (NewCandI and  NewClassI)  after applying
// initial transformations to types and member functions. NewStructTy is
// used as candidate struct.
bool SOAToAOSPrepCandidateInfo::computeUpdatedCandidateInfo() {

  int32_t Off = getCandidateField();
  std::unique_ptr<MemInitCandidateInfo> NewCandD(new MemInitCandidateInfo());
  if (!NewCandD->isSimpleVectorType(NewStructTy, Off,
                                    /*AllowOnlyDerived*/ false))
    return false;

  if (!NewCandD->collectMemberFunctions(M))
    return false;

  NewCandI = NewCandD.release();
  std::unique_ptr<ClassInfo> NewClassD(new ClassInfo(
      DL, DTInfo, GetTLI, GetDT, NewCandI, Off, /*RecognizeAll*/ false));

  if (!NewClassD->analyzeClassFunctions()) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
      dbgs() << "  2nd time functionality analysis failed.\n";
    });
    return false;
  }
  NewClassI = NewClassD.release();
  return true;
}

// Apply multiple transformations for Ctor.
Function *SOAToAOSPrepCandidateInfo::applyCtorTransformations() {

  // Returns true if "G" is valid GEP that accesses "FlagField"
  auto IsFlagGEP = [this](GetElementPtrInst *G, int32_t FlagField) {
    if (!G || G->getSourceElementType() != NewElemTy)
      return false;
    if (G->getNumIndices() != 2)
      return false;
    int32_t Idx = cast<ConstantInt>(G->getOperand(2))->getLimitedValue();
    if (FlagField != Idx)
      return false;
    return true;
  };

  // Returns the store instruction that writes data to Flag field in "F", which
  // is expected to be a constructor. Only one store instruction to the flag
  // field is expected.
  auto GetFlagFieldStoreInstInCtor =
      [IsFlagGEP](Function *F, int32_t FlagField) -> StoreInst * {
    StoreInst *SI = nullptr;

    for (Instruction &I : instructions(*F)) {
      auto *G = dyn_cast<GetElementPtrInst>(&I);
      if (!IsFlagGEP(G, FlagField))
        continue;
      if (!G->hasOneUse())
        return nullptr;
      if (SI)
        return nullptr;
      SI = dyn_cast<StoreInst>(G->user_back());
      if (!SI || SI->getPointerOperand() != G)
        return nullptr;
    }
    return SI;
  };

  // Collect all uses of flag field in all member functions of vector
  // class except constructor. In constructor, we already proved that
  // there are no accesses to flag field except the store instruction
  // to the flag field. Returns false if any other instructions except
  // reading value of the flag field is noticed in any member function.
  //
  auto CollectAllFlagFieldUses =
      [this, IsFlagGEP](SmallPtrSetImpl<LoadInst *> &FlagUseSet,
                        int32_t FlagField) {
        Function *CtorF = NewClassI->getCtorFunction();
        for (auto *F : NewClassI->field_member_functions()) {
          if (F == CtorF)
            continue;
          for (Instruction &I : instructions(F)) {
            auto *G = dyn_cast<GetElementPtrInst>(&I);
            if (!IsFlagGEP(G, FlagField))
              continue;
            for (auto *U : G->users()) {
              auto *LI = dyn_cast<LoadInst>(U);
              if (!LI)
                return false;
              FlagUseSet.insert(LI);
            }
          }
        }
        return true;
      };

  // Moves the argument of "F" at "UnusedArgPos" to the end of
  // argument list. It also fixes all callsites accordingly.
  //
  //  Ex:
  //    Before:
  //    Ctor(ptr1, 0, false, Mem); // Call 1
  //    Ctor(ptr2, C, false, Mem); // Call 2
  //
  //    Ctor(%0, int %1, Mem* %2, bool %3) {
  //      ...
  //      store false, %flag_field;
  //      ...
  //    }
  //
  //    After:
  //    Ctor(ptr1, 0, Mem, false); // Call 1
  //    Ctor(ptr2, C, Mem, false); // Call 2
  //
  //    Ctor(%0, int %1, Mem* %2, bool %3) {
  //      ...
  //      store false, %flag_field;
  //      ...
  //    }
  //
  auto CreateNewFunctionWithUnusedArgAsLast = [](Function *F,
                                                 unsigned UnusedArgPos) {
    std::vector<Type *> Params;
    SmallVector<AttributeSet, 8> ArgAttrVec;
    FunctionType *FTy = F->getFunctionType();
    const AttributeList &ParamAL = F->getAttributes();

    // Collect Param/Attr for all arguments except for the argument at
    // UnusedArgPos.
    unsigned Pos = 0;
    for (Argument &I : F->args()) {
      if (UnusedArgPos != Pos) {
        Params.push_back(I.getType());
        ArgAttrVec.push_back(ParamAL.getParamAttributes(Pos));
      }
      Pos++;
    }
    // Place the UnusedArg at the end of the list.
    Argument *UnusedArg = F->getArg(UnusedArgPos);
    Params.push_back(UnusedArg->getType());
    ArgAttrVec.push_back(ParamAL.getParamAttributes(UnusedArgPos));

    // Create New function with the new params/Atts.
    AttributeList NewParamAL =
        AttributeList::get(F->getContext(), ParamAL.getFnAttributes(),
                           ParamAL.getRetAttributes(), ArgAttrVec);
    FunctionType *NFTy =
        FunctionType::get(FTy->getReturnType(), Params, FTy->isVarArg());
    Function *NF =
        Function::Create(NFTy, F->getLinkage(), F->getAddressSpace());
    NF->copyAttributesFrom(F);
    NF->setComdat(F->getComdat());
    NF->setAttributes(NewParamAL);
    F->getParent()->getFunctionList().insert(F->getIterator(), NF);
    NF->takeName(F);

    // Fix callsites accordingly
    std::vector<Value *> Args;
    for (auto *U : F->users()) {
      auto *CB = dyn_cast<CallBase>(U);
      assert(CB && "Expected function call");
      ArgAttrVec.clear();
      const AttributeList &CallParamAL = CB->getAttributes();
      // Build Args/Attr list for the call.
      auto *I = CB->arg_begin();
      Pos = 0;
      for (unsigned e = FTy->getNumParams(); Pos != e; ++I, ++Pos) {
        if (UnusedArgPos != Pos) {
          Args.push_back(*I);
          AttributeSet Attrs = CallParamAL.getParamAttributes(Pos);
          ArgAttrVec.push_back(Attrs);
        }
      }
      // Place the UnusedArg at the end of the list.
      Args.push_back(CB->getArgOperand(UnusedArgPos));
      ArgAttrVec.push_back(CallParamAL.getParamAttributes(UnusedArgPos));

      AttributeList NewCallParamAL =
          AttributeList::get(F->getContext(), CallParamAL.getFnAttributes(),
                             CallParamAL.getRetAttributes(), ArgAttrVec);

      CallBase *NewCB;

      if (InvokeInst *II = dyn_cast<InvokeInst>(CB)) {
        NewCB = InvokeInst::Create(NF, II->getNormalDest(), II->getUnwindDest(),
                                   Args, None, "", CB->getParent());
      } else {
        NewCB = CallInst::Create(NFTy, NF, Args, None, "", CB);
        cast<CallInst>(NewCB)->setTailCallKind(
            cast<CallInst>(CB)->getTailCallKind());
      }
      NewCB->setCallingConv(CB->getCallingConv());
      NewCB->setAttributes(NewCallParamAL);
      NewCB->setDebugLoc(CB->getDebugLoc());

      Args.clear();
      ArgAttrVec.clear();

      // Since the function is constructor, there will be no normal uses
      // for the call. But, just keep this code if it is used by others
      // like Metadata.
      if (!CB->use_empty() || CB->isUsedByMetadata()) {
        CB->replaceAllUsesWith(NewCB);
        NewCB->takeName(CB);
      }
      CB->eraseFromParent();
    }

    // Get function body for NF.
    NF->getBasicBlockList().splice(NF->begin(), F->getBasicBlockList());

    // Fix argument usages since dead arg is moved at the end of param list,
    Pos = 0;
    for (Function::arg_iterator I = F->arg_begin(), E = F->arg_end(),
                                I2 = NF->arg_begin();
         I != E; ++I, ++Pos) {
      if (UnusedArgPos != Pos) {
        I->replaceAllUsesWith(&*I2);
        I2->takeName(&*I);
        ++I2;
      }
    }
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
      dbgs() << "  After all Ctor transformations: \n";
      dbgs() << "   New Ctor:  " << *NF << "\n";
      dbgs() << "      Callsites of New Ctor: \n";
      for (auto *U : NF->users())
        dbgs() << "      " << *U << "\n";
    });
    return NF;
  };

  DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE,
                  { dbgs() << "  Transformations 4: \n"; });
  assert(NewClassI && NewCandI && "Expected updated class info");
  Function *CtorF = NewClassI->getCtorFunction();
  assert(CtorF && "Expected valid Ctor");
  int32_t FlagFI = NewClassI->getFlagField();
  assert(FlagFI != -1 && "Unexpected Flag field");

  StoreInst *FlagSI = GetFlagFieldStoreInstInCtor(CtorF, FlagFI);
  assert(FlagSI && "Expected Store for flag field");

  // If non-constant value is stored to flag field, there is nothing
  // we can do.
  auto *FlagConst = dyn_cast<Constant>(FlagSI->getValueOperand());
  if (!FlagConst) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
      dbgs() << "    Non-Constant value is stored to flag field.\n";
    });
    return CtorF;
  }

  // Check if flag field is not modified anywhere except constructor.
  // Copy-Constructor is not handled for now.
  SmallPtrSet<LoadInst *, 8> FlagUseSet;
  if (!CollectAllFlagFieldUses(FlagUseSet, FlagFI)) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
      dbgs() << "   Not able to handle all uses of flag field\n";
    });
    return CtorF;
  }
  // Replace all uses of flag field with the constant.
  for (auto *LI : FlagUseSet) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE,
                    { dbgs() << "  Replacing use of flag: " << *LI << "\n"; });
    LI->replaceAllUsesWith(FlagConst);
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE,
                    { dbgs() << "  with constant: " << *FlagConst << "\n"; });
  }
  // Delete dead load instructions.
  // TODO: After replacing uses of load with constant, we have opportunities
  // to do constant-folding.
  for (auto *LI : FlagUseSet) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE,
                    { dbgs() << " Deleting dead uses: " << *LI << "\n"; });
    RecursivelyDeleteTriviallyDeadInstructions(LI);
  }

  // Since there are no real uses of the flag field anymore, store zero
  // value to the flag field.
  Value *ZeroValue = ConstantInt::get(FlagSI->getOperand(0)->getType(), 0);
  DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
    dbgs() << "  Flag Store Inst in Ctor before: " << *FlagSI << "\n";
  });
  FlagSI->setOperand(0, ZeroValue);
  DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
    dbgs() << "  Flag Store Inst in Ctor after: " << *FlagSI << "\n";
  });

  // Detect unused flag fields after constant propagation.
  unsigned UnusedArgCount = 0;
  unsigned UnusedArgPos = 0;
  unsigned Pos = 0;
  for (auto &Arg : CtorF->args()) {
    if (Arg.use_empty() && Arg.getType()->isIntegerTy(1)) {
      UnusedArgPos = Pos;
      UnusedArgCount++;
    }
    Pos++;
  }
  if (UnusedArgCount != 1) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE,
                    { dbgs() << " No unused args found \n"; });
    return CtorF;
  }

  Value *FalseValue = ConstantInt::getFalse(FlagSI->getContext());
  for (auto *U : CtorF->users()) {
    auto *CB = dyn_cast<CallBase>(U);
    assert(CB && "Unexpected Call");
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE,
                    { dbgs() << "  Ctor call before: " << *CB << "\n"; });
    CB->setArgOperand(UnusedArgPos, FalseValue);
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE,
                    { dbgs() << "  Ctor call after: " << *CB << "\n"; });
  }

  Function *NewCtor = CreateNewFunctionWithUnusedArgAsLast(CtorF, UnusedArgPos);

  CtorF->eraseFromParent();

  // Indicate that constant prop is applied.
  ConstantPropApplied = true;

  return NewCtor;
}

// Verifies that function "F" is safe to apply transformations
// on member functions of vector class. We basically need to
// prove that the function doesn't change fields of vector class.
// Return false if F have any instruction that may write to memory
// (like Store etc) except:
//   1. Alloc instructions
//   2. MemCpy if first argument is allocated memory in the routine.
//
bool SOAToAOSPrepCandidateInfo::isSafeCallForAppend(Function *F) {

  // Returns true if "I" is Alloc instruction.
  auto IsAllocCall = [this](Instruction *I) {
    auto *CB = dyn_cast_or_null<CallBase>(I);
    if (!CB)
      return false;
    if (isDummyFuncWithThisAndIntArgs(CB, GetTLI(*CB->getFunction())))
      return true;
    auto *CallInfo = DTInfo.getCallInfo(CB);
    if (CallInfo && CallInfo->getCallInfoKind() == dtrans::CallInfo::CIK_Alloc)
      return true;
    return false;
  };

  // Returns true if "V" is return value of any alloc call.
  auto IsAllocPtr = [&IsAllocCall](Value *V) {
    auto *PN = dyn_cast<PHINode>(V);
    if (!PN)
      return IsAllocCall(PN);
    for (unsigned I = 0, E = PN->getNumIncomingValues(); I < E; I++) {
      if (!IsAllocCall(dyn_cast<Instruction>(PN->getIncomingValue(I))))
        return false;
    }
    return true;
  };

  for (Instruction &I : instructions(F)) {
    if (isa<DbgInfoIntrinsic>(&I))
      continue;
    if (IsAllocCall(&I))
      continue;
    auto *Memcpy = dyn_cast<MemCpyInst>(&I);
    // Just check 1st argument is coming from alloc call. No need
    // to check for size.
    if (Memcpy && IsAllocPtr(Memcpy->getArgOperand(0)))
      continue;
    if (I.mayWriteToMemory())
      return false;
  }
  return true;
}

// This detects if any part of code that is semantically doing
// copy-ctor functionality, just replace the code with CCtor/SetElem
// calls.
//
// Ex:
//    unsigned int valuesSize = other.fValues->size();
//    fValues = RefArray_Ctor(Capacity, true, fMemoryManager);
//    for (unsigned int i=0; i<valuesSize; i++) {
//      fValues->addElement(replicate(other.fValues->elementAt(i)),
//                          fMemoryManager));
//    }
//
//    Here, new RefArray_Ctor constructor is created and then add all
//    elements of "other" vector in a loop after doing processing of
//    the elements by calling "replicate" function.
//    This is basically the functionality of copy-ctor.
//
//    The above will be converted as below. Two new member functions
//    will be created.
//    1. Simple CCtor: Copies all fields except array elements. Replace
//    RefArray_Ctor call with the newly created CCtor.
//    2. Simple SetElem: Set element at given location Replace addElement
//    call with the newly created SetElem.
//
//    After:
//    unsigned int valuesSize = other.fValues->size();
//    fValues = RefArray_CCtor(Capacity, other.fValues);
//    for (unsigned int i=0; i<valuesSize; i++) {
//      fValues->SimpleSetElem(replicate(other.fValues->elementAt(i)),
//                          fMemoryManager), i);
//    }
//
//
//
void SOAToAOSPrepCandidateInfo::convertCtorToCCtor(Function *NewCtor) {

  // Check Loop "L" has zero trip count test. Return loop count
  // if it has valid zero trip count test.
  // Ex:
  //     store %"RefArray"* %35, %"RefArray"** %5
  //     %40 = icmp eq i32 %18, 0
  //     br i1 %40, label %93, label %LoopPreHead
  //
  // LoopPreHead:
  //
  // Returns %18 for above example.
  //
  auto GetLoopCountFromZeroTripCheck = [](Loop *L) -> Value * {
    BasicBlock *PH = L->getLoopPreheader();
    if (!PH)
      return nullptr;
    auto *EntryBI = dyn_cast<BranchInst>(PH->getTerminator());
    if (!EntryBI || EntryBI->isConditional() ||
        EntryBI != PH->getFirstNonPHIOrDbg())
      return nullptr;

    auto *PreCondBB = PH->getSinglePredecessor();
    if (!PreCondBB)
      return nullptr;
    auto *BI = dyn_cast<BranchInst>(PreCondBB->getTerminator());
    if (!BI || !BI->isConditional())
      return nullptr;

    ICmpInst *Cond = dyn_cast<ICmpInst>(BI->getCondition());
    if (!Cond)
      return nullptr;

    ConstantInt *CmpZero = dyn_cast<ConstantInt>(Cond->getOperand(1));
    if (!CmpZero || !CmpZero->isZero())
      return nullptr;

    ICmpInst::Predicate Pred = Cond->getPredicate();
    if (Pred != ICmpInst::ICMP_EQ ||
        BI->getSuccessor(1) != L->getLoopPreheader())
      return nullptr;
    return Cond->getOperand(0);
  };

  // Returns loop count if the given "L" is simple loop like below.
  // Otherwise, returns nullptr. "Phi" represents loop index (i.e %60).
  //
  // Ex:
  //    59:                                               ; preds = %68, %41
  //      %60 = phi i32 [ 0, %41 ], [ %69, %68 ]
  //
  //      Loop ...
  //
  //    68:                                               ; preds = %67
  //      %69 = add nuw i32 %60, 1
  //      %70 = icmp eq i32 %69, %18
  //      br i1 %70, label %93, label %59
  //
  // Returns %18 for the example.
  //
  auto GetLoopCountFromLoopLatch =
      [&GetLoopCountFromZeroTripCheck](Value *Phi, Loop *L) -> Value * {
    auto *PN = dyn_cast<PHINode>(Phi);
    if (!PN || PN->getNumIncomingValues() != 2)
      return nullptr;
    BasicBlock *Latch = L->getLoopLatch();
    BasicBlock *PreHead = L->getLoopPreheader();
    if (!PreHead || !Latch)
      return nullptr;
    Value *V1 = PN->getIncomingValueForBlock(PreHead);
    Value *V2 = PN->getIncomingValueForBlock(Latch);
    ConstantInt *Init = dyn_cast<ConstantInt>(V1);
    if (!Init || !Init->isZero())
      return nullptr;
    BranchInst *BI = dyn_cast<BranchInst>(Latch->getTerminator());
    if (!BI || !BI->isConditional())
      return nullptr;

    ICmpInst *Cond = dyn_cast<ICmpInst>(BI->getCondition());
    if (!Cond)
      return nullptr;
    ICmpInst::Predicate Pred = Cond->getPredicate();
    if (Pred != ICmpInst::ICMP_EQ || BI->getSuccessor(1) != L->getHeader())
      return nullptr;
    Value *CmpOp0 = Cond->getOperand(0);
    Value *CmpOp1 = Cond->getOperand(1);
    if (V2 != CmpOp0)
      return nullptr;
    auto *AddI = dyn_cast<Instruction>(CmpOp0);
    if (!AddI || AddI->getOpcode() != Instruction::Add)
      return nullptr;
    if (AddI->getOperand(0) != PN)
      return nullptr;
    ConstantInt *Inc = dyn_cast<ConstantInt>(AddI->getOperand(1));
    if (!Inc || !Inc->isOne())
      return nullptr;
    Value *LoopCount = GetLoopCountFromZeroTripCheck(L);
    if (!LoopCount || LoopCount != CmpOp1)
      return nullptr;
    return CmpOp1;
  };

  // Make sure GEP is accessing a field from function argument at "ArgNo".
  auto IsValidGEPFromArg = [](GetElementPtrInst *GEP, unsigned ArgNo) {
    if (GEP->getNumIndices() != 2)
      return false;
    Function *F = GEP->getFunction();
    if (GEP->getPointerOperand() != F->getArg(ArgNo))
      return false;
    if (!isa<ConstantInt>(GEP->getOperand(2)))
      return false;
    return true;
  };

  // Detect copy-ctor functionality for given call of AppendElem (CB)
  // function.
  //
  // Ex:
  //    %18 = GetSize(%15)   // Get size of source vector
  //    Invoke Ctor(%35, ...);
  // 39:
  //    store  %"RefArray"* %35, %"RefArray"** %5
  //    %40 = icmp eq i32 %18, 0
  //    br i1 %40, label %93, label %41
  // 41:
  //   br label %59
  // 59:
  //   %60 = phi i32 [ 0, %41 ], [ %69, %68 ]
  //   %61 = load %"RefArray"*, %"RefArray"** %5
  //   %62 = load %"RefArray"*, %"RefArray"** %15
  //   %63 = GetElem(%62, %60)    // Get element from source vector
  //   %66 = replicate(%63)
  //   AppendElem(%61, %66)    // Append element to dest vector
  //   %69 = add nuw i32 %60, 1
  //   %70 = icmp eq i32 %69, %18
  //    br i1 %70, label %93, label %59
  //
  // Also, Detect and set SrcGEPPtr, LoopIdxPtr, CtorCBPtr.
  //
  auto CheckCopyCtor = [this, &GetLoopCountFromLoopLatch,
                        &IsValidGEPFromArg](CallBase *CB, Value **LoopIdxPtr,
                                            GetElementPtrInst **SrcGEPPtr,
                                            CallBase **CtorCBPtr) {
    Function *Caller = CB->getCaller();
    // Check if caller is potential copy-ctor.
    if (Caller->arg_size() != 2 || CB->arg_size() != 2)
      return false;
    Argument *FromArg = Caller->getArg(1);
    Argument *ToArg = Caller->getArg(0);
    if (ToArg->getType() != FromArg->getType())
      return false;

    // Make sure the call is in loop.
    LoopInfo LI((GetDT)(*Caller));
    Loop *L = LI.getLoopFor(CB->getParent());
    if (!L)
      return false;

    SmallPtrSet<CallBase *, 4> CallsInLoop;
    CallsInLoop.insert(CB);

    // Get dest vector and element from Append call.
    Value *DstPtr = CB->getArgOperand(0);
    Value *Elem = CB->getArgOperand(1);
    auto *DstPtrLd = dyn_cast<LoadInst>(DstPtr);
    if (!DstPtrLd)
      return false;
    auto *DstGEP = dyn_cast<GetElementPtrInst>(DstPtrLd->getPointerOperand());
    if (!DstGEP || !IsValidGEPFromArg(DstGEP, 0))
      return false;

    // Check if element is processed by some auxiliary safe function.
    auto *AuxCB = dyn_cast<CallBase>(Elem);
    if (AuxCB) {
      Function *AuxF = getCalledFunction(*AuxCB);
      if (!AuxF)
        return false;
      if (!NewClassI->isCandidateMemberFunction(AuxF)) {
        if (!isSafeCallForAppend(AuxF))
          return false;
        Elem = AuxCB->getArgOperand(0);
        CallsInLoop.insert(AuxCB);
      }
    }

    // Check where the element is coming from GetElem of other
    // vector.
    auto *GetElemCB = dyn_cast<CallBase>(Elem);
    if (!GetElemCB)
      return false;
    CallsInLoop.insert(GetElemCB);
    Function *GetElemF = getCalledFunction(*GetElemCB);
    if (!GetElemF)
      return false;
    if (NewClassI->getFinalFuncKind(GetElemF) != GetElem)
      return false;
    Value *GetArg0 = GetElemCB->getArgOperand(0);
    Value *GetArg1 = GetElemCB->getArgOperand(1);
    auto *SrcPtrLd = dyn_cast<LoadInst>(GetArg0);
    if (!SrcPtrLd)
      return false;
    auto *SrcGEP = dyn_cast<GetElementPtrInst>(SrcPtrLd->getPointerOperand());
    if (!SrcGEP || !IsValidGEPFromArg(SrcGEP, 1))
      return false;
    // Make sure 2nd argument of GetElem call is loop counter.
    Value *LatchCount = GetLoopCountFromLoopLatch(GetArg1, L);
    auto *SizeCB = dyn_cast_or_null<CallBase>(LatchCount);
    if (!SizeCB)
      return false;
    // Make sure loop counter is size of source vector.
    Function *SizeF = getCalledFunction(*SizeCB);
    if (!SizeF)
      return false;
    if (NewClassI->getFinalFuncKind(SizeF) != GetSize)
      return false;
    Value *SizeArg0 = SizeCB->getArgOperand(0);
    auto *SizeLd = dyn_cast<LoadInst>(SizeArg0);
    if (!SizeLd)
      return false;
    // Make sure all elements of source vector are copied to dest vector.
    auto SizeGEP = dyn_cast<GetElementPtrInst>(SizeLd->getPointerOperand());
    if (!SizeGEP)
      return false;
    if (SizeGEP != SrcGEP)
      return false;

    // Try to find instructions related Ctor just before the loop.
    BasicBlock *PH = L->getLoopPreheader();
    BasicBlock *PreCondBB = PH->getSinglePredecessor();
    assert(PreCondBB && " Expected valid SinglePredecessor");
    Instruction *TInst = PreCondBB->getTerminator();
    assert(TInst && "Expected Terminator");
    Instruction *CInst = TInst->getPrevNonDebugInstruction();
    if (!CInst || !isa<ICmpInst>(CInst))
      return false;
    auto *SI = dyn_cast_or_null<StoreInst>(CInst->getPrevNonDebugInstruction());
    if (!SI || PreCondBB->getFirstNonPHIOrDbg() != SI)
      return false;
    // Make sure the store is instruction that saves newly constructed
    // dest vector.
    Value *StoreValue = SI->getValueOperand();
    Value *StorePtr = SI->getPointerOperand();
    if (StorePtr != DstGEP)
      return false;
    CallBase *CtorCB = nullptr;
    for (auto *U : StoreValue->users()) {
      if (U == SI)
        continue;
      // Make sure dest vector doesn't have any other uses except CtorCB
      // and the store instruction.
      if (CtorCB)
        return false;
      CtorCB = dyn_cast<CallBase>(U);
      if (!CtorCB)
        return false;
    }
    if (!CtorCB)
      return false;
    if (CtorCB->getArgOperand(0) != StoreValue)
      return false;

    // Make sure there are no other instructions between constructor
    // call and the loop.
    BasicBlock *PredBB = PreCondBB->getSinglePredecessor();
    if (!PredBB)
      return false;
    auto *BIt = dyn_cast_or_null<BranchInst>(PredBB->getFirstNonPHIOrDbg());
    if (BIt && BIt->isUnconditional())
      PredBB = PredBB->getSinglePredecessor();
    if (!PredBB || CtorCB->getParent() != PredBB)
      return false;
    // Walk back PredBB to make sure there are no side effects between
    // CtorCB and the loop.
    Instruction *FrontI = &PredBB->front();
    for (Instruction *Inst = PredBB->getTerminator(); Inst != FrontI;
         Inst = Inst->getPrevNode()) {
      if (Inst == CtorCB)
        break;
      if (Inst->mayWriteToMemory())
        return false;
    }

    // Make sure there are no other stores/calls etc in loop.
    BasicBlock *Latch = L->getLoopLatch();
    for (auto *BB : L->blocks()) {
      for (auto &II : *BB) {
        auto *LCB = dyn_cast<CallBase>(&II);
        // Ignore calls already processed.
        if (LCB && CallsInLoop.count(LCB))
          continue;
        if (II.mayWriteToMemory())
          return false;
      }
      // Make sure CFG is linear.
      if (Latch == BB)
        continue;
      Instruction *II = BB->getTerminator();
      if (!II || !isa<InvokeInst>(II))
        return false;
    }
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE,
                    { dbgs() << "Transformations 5: \n"; });
    *SrcGEPPtr = SrcGEP;
    *CtorCBPtr = CtorCB;
    *LoopIdxPtr = GetArg1;
    return true;
  };

  // Create simple SetElem function.
  //   SimpleSetElem(Thisptr, Elem, Idx) {
  //     *(ThisPtr->baseptr + Idx) = Elem
  //     return;
  //   }
  //
  // Note that it doesn't check for "Idx < Size" as we know this is
  // always true since this routine will be used to replace AppendElem.
  //
  auto CreateSimpleSetElementFunction = [this](Function *SetFunc) {
    // Create New function with same signature as SetFunc.
    FunctionType *FTy = SetFunc->getFunctionType();
    Function *NF = Function::Create(FTy, SetFunc->getLinkage(),
                                    SetFunc->getName(), SetFunc->getParent());
    NF->setAttributes(SetFunc->getAttributes());
    NF->setCallingConv(SetFunc->getCallingConv());

    // Add instructions to set an element at given position.
    //
    //   (ThisPtr->baseptr + Idx) = Elem
    //
    BasicBlock *BB = BasicBlock::Create(M.getContext(), "entry", NF);
    IRBuilder<> IRB(BB);

    Value *ThisPtr = NF->getArg(0);
    Value *Elem = NF->getArg(1);
    Value *Idx = NF->getArg(2);
    int32_t BaseArrayIdx = NewClassI->getArrayField();
    assert(BaseArrayIdx != -1 && "Expected valid Array Index");
    SmallVector<Value *, 2> Indices;
    Indices.push_back(IRB.getInt64(0));
    Indices.push_back(IRB.getInt32(BaseArrayIdx));
    // Load base array of vector
    Value *GEP = IRB.CreateInBoundsGEP(NewElemTy, ThisPtr, Indices, "");
    unsigned Align = DL.getABITypeAlignment(Elem->getType());
    LoadInst *Load =
        IRB.CreateAlignedLoad(Elem->getType()->getPointerTo(0), GEP, Align, "");
    Value *NewIdx = IRB.CreateZExtOrTrunc(Idx, IRB.getInt64Ty());
    // Store Elem into base array at Idx.
    Indices.clear();
    Indices.push_back(NewIdx);
    Value *ElemGEP = IRB.CreateInBoundsGEP(Elem->getType(), Load, Indices, "");
    IRB.CreateAlignedStore(Elem, ElemGEP, Align);
    IRB.CreateRetVoid();
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
      dbgs() << "  New simple SetElem function created: \n";
      NF->dump();
    });
    return NF;
  };

  // Replace AppendElem call with simple SetElem call.
  //
  //   Before:
  //        AppendElem(ThisPtr, Elem);
  //   After:
  //        SimpleSetElem(ThisPtr, Elem, LoopCounter);
  //
  auto ReplaceAppendElemWithSetElemCall =
      [this](Function *SimpleSetElem, CallBase *AppendCB, Value *LoopCounter) {
        std::vector<Value *> NewArgs;
        auto NFPAL = SimpleSetElem->getAttributes();
        AttributeList Attrs = AppendCB->getAttributes();
        SmallVector<AttributeSet, 4> NewArgAttrs;
        auto *E = AppendCB->arg_end();
        auto *I = AppendCB->arg_begin();
        unsigned ArgIdx = 0;
        for (; I != E; I++) {
          NewArgs.push_back(*I);
          NewArgAttrs.push_back(NFPAL.getParamAttributes(ArgIdx));
          ArgIdx++;
        }
        NewArgAttrs.push_back(NFPAL.getParamAttributes(ArgIdx));
        NewArgs.push_back(LoopCounter);
        FunctionType *NFTy = SimpleSetElem->getFunctionType();
        AttributeList NewPAL =
            AttributeList::get(NFTy->getContext(), Attrs.getFnAttributes(),
                               Attrs.getRetAttributes(), NewArgAttrs);

        DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
          dbgs() << "  Replacing Append call with SetElemcall: \n";
        });
        updateCallBase(AppendCB, NewPAL, SimpleSetElem, NewArgs);
      };

  // Create simple CCtor using Ctor function body.
  //   Before:
  //     Ctor(This, c, flag ..) {
  //       capacity = c;
  //       size = 0;
  //       ...
  //       base = (S**)malloc(capacity * sizeof(S*));
  //       memset(base, 0, capacity * sizeof(S*));
  //     }
  //
  //   After:
  //     CCtor(Dest, Src) {  // Newly created function. No change to the Ctor.
  //       Dest.capacity = Src.capacity;
  //       Dest.size = Src.size;
  //       ...
  //       base = (S**)malloc(Src.capacity * sizeof(S*));
  //       memset(base, 0, Src.capacity * sizeof(S*));
  //
  //       // Note that no array elements are copied here.
  //     }
  //
  //  This transformation is implemented by cloning Ctor two times like
  //  below.
  //
  //    1st time clone: Just add "SrcPtr" argument and map other
  //    argument to the original arguments of Ctor.
  //       Ctor1(This, SrcPtr, c, flag...) {
  //       }
  //
  //   For each Store instruction in cloned routine (except base array field)
  //   is converted as copy instruction.
  //   Ex:
  //     Ctor1(this, SrcPtr, c, flag...) {
  //      ...
  //      this->capacity = c;
  //      ...
  //     }
  //
  //     to
  //
  //     Ctor1(this, SrcPtr, c, flag...) {
  //      ...
  //      this->capacity = SrcPtr->capacity;
  //      ...
  //     }
  //  Array base store instruction (nullptr and memory allocation) are not
  //  changed.
  //
  //  2nd time cloning: Now, clone Ctor1 again to remove all dead arguments.
  //  Ctor1 will be deleted.
  //     Ctor2(this, SrcPtr) {
  //      ...
  //      this->capacity = SrcPtr->capacity;
  //      ...
  //     }
  //
  auto CreateSimpleCCtorFunction = [this](Function *CtorF) {
    FunctionType *CtorFTy = CtorF->getFunctionType();
    std::vector<Type *> NewParams;
    // Add new argument for SrcPtr
    Type *ArgType = CtorF->getArg(0)->getType();
    NewParams.push_back(ArgType);
    for (auto I = CtorF->arg_begin(), E = CtorF->arg_end(); I != E; ++I) {
      Argument *A = &*I;
      NewParams.push_back(A->getType());
    }
    // 1st time cloning.
    FunctionType *NewFTy = FunctionType::get(Type::getVoidTy(M.getContext()),
                                             NewParams, CtorFTy->isVarArg());
    Function *NewF =
        Function::Create(NewFTy, CtorF->getLinkage(), CtorF->getName(), &M);
    ValueToValueMapTy VMap1;
    auto A = CtorF->arg_begin();
    unsigned Pos = 0;
    for (auto I = NewF->arg_begin(); I != NewF->arg_end(); ++I, Pos++) {
      if (Pos == 1) {
        continue;
      }
      VMap1[&*A] = &*I;
      A++;
    }
    SmallVector<ReturnInst *, 8> Rets;
    CloneFunctionInto(NewF, CtorF, VMap1, true, Rets);

    // Fix store instructions in the cloned routines.
    Argument *CopyArg = NewF->getArg(1);
    for (Instruction &I : instructions(NewF)) {
      auto *SI = dyn_cast<StoreInst>(&I);
      if (!SI)
        continue;
      auto *GEP = dyn_cast<GetElementPtrInst>(SI->getPointerOperand());
      assert(GEP && "Expected GEP");
      DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
        dbgs() << "  Processing : " << *GEP << "\n";
        dbgs() << "             " << *SI << "\n";
      });
      auto *CInt = dyn_cast<ConstantInt>(GEP->getOperand(2));
      assert(CInt && "Expected constant GEP index");
      int32_t Idx = CInt->getLimitedValue();
      // Ignore base array field stores.
      if (NewClassI->getArrayField() == Idx)
        continue;
      SmallVector<Value *, 8> Indices;
      Indices.append(GEP->idx_begin(), GEP->idx_end());
      auto *NewGEP = GetElementPtrInst::Create(GEP->getSourceElementType(),
                                               CopyArg, Indices, "", GEP);
      auto *Ld = new LoadInst(NewGEP, "", GEP);
      if (isa<Argument>(SI->getOperand(0))) {
        Value *V = SI->getValueOperand();
        V->replaceAllUsesWith(Ld);
      } else {
        SI->setOperand(0, Ld);
      }
      DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
        dbgs() << "  Transformed to : " << *NewGEP << "\n";
        dbgs() << "                   " << *Ld << "\n";
        dbgs() << "                   " << *SI << "\n";
      });
    }

    // 2nd time cloning
    ValueToValueMapTy VMap2;
    // Delete all arguments except the first two.
    for (unsigned I = 2, E = NewF->arg_size(); I < E; I++) {
      Argument *A = NewF->getArg(I);
      VMap2[A] = Constant::getNullValue(A->getType());
    }
    Function *CCtorF = CloneFunction(NewF, VMap2);
    NewF->eraseFromParent();
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
      dbgs() << "  New simple CCtor function created: \n";
      CCtorF->dump();
    });
    return CCtorF;
  };

  // Replace Ctor call with simple CCtor call.
  //
  // Before:
  //    Ctor(DestPtr, ...);
  //
  // After:
  //    %G = GetElementPtrInst  //  Address of
  //    SrcPtr = Load %G
  //    NewCCtor(DestPtr, SrcPtr);
  //
  auto FixCtorFunctionCall = [this](Function *NewCCtor, CallBase *CtorCB,
                                    GetElementPtrInst *GEP) {
    GetElementPtrInst *NewGEP = cast<GetElementPtrInst>(GEP->clone());
    NewGEP->insertBefore(CtorCB);
    LoadInst *NewLd = new LoadInst(NewGEP, "", CtorCB);
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
      dbgs() << "  Replacing Ctor call with CCtor call: \n";
      dbgs() << " GEP: " << *NewGEP << "\n";
      dbgs() << " Load: " << *NewLd << "\n";
    });
    std::vector<Value *> NewArgs;
    auto NFPAL = NewCCtor->getAttributes();
    AttributeList Attrs = CtorCB->getAttributes();
    SmallVector<AttributeSet, 4> NewArgAttrs;
    auto *I = CtorCB->arg_begin();
    NewArgs.push_back(*I);
    NewArgAttrs.push_back(NFPAL.getParamAttributes(0));
    NewArgs.push_back(NewLd);
    NewArgAttrs.push_back(NFPAL.getParamAttributes(1));
    FunctionType *NFTy = NewCCtor->getFunctionType();
    AttributeList NewPAL =
        AttributeList::get(NFTy->getContext(), Attrs.getFnAttributes(),
                           Attrs.getRetAttributes(), NewArgAttrs);
    updateCallBase(CtorCB, NewPAL, NewCCtor, NewArgs);
  };

  // Get the position of MemInterfaceTy for "CtorF".
  auto GetMemInterArgumentPos = [this](Function *CtorF) {
    StructType *MemInterTy = NewCandI->getMemInterfaceType();
    int32_t Pos = 0;
    for (auto &Arg : CtorF->args()) {
      auto *PTy = dyn_cast<PointerType>(Arg.getType());
      if (PTy && PTy->getElementType() == MemInterTy)
        return Pos;
      Pos++;
    }
    return -1;
  };

  // Returns true if the same candidate struct field value is
  // passed as MemInterface argument to all callsites of CtorF.
  auto IsMemInterFieldSame = [this, &GetMemInterArgumentPos](Function *CtorF) {
    int32_t Pos = GetMemInterArgumentPos(CtorF);
    if (Pos == -1)
      return false;
    for (auto *U : CtorF->users()) {
      auto *CB = dyn_cast<CallBase>(U);
      assert(CB && " Expected valid call");
      Value *Arg = CB->getArgOperand(Pos);
      auto *Ld = dyn_cast<LoadInst>(Arg);
      if (!Ld)
        return false;
      auto *GEP = dyn_cast<GetElementPtrInst>(Ld->getPointerOperand());
      if (!GEP)
        return false;
      // We know there is only one MemInterfaceType field in the candidate
      // struct. So, no need to check the field index.
      if (GEP->getSourceElementType() != getNewStructTy())
        return false;
      // TODO: Member function analysis proves that MemInterfaceType field
      // in vector class is not modified. Need to check if SOAToAOS is
      // proving that MemInterfaceType field is not modified in the candidate
      // struct.
    }
    return true;
  };

  // First, make sure the class doesn't have its own CCtor
  if (NewClassI->getSingleMemberFunction(CopyConstructor))
    return;
  assert(NewCtor && "Expected valid Ctor");
  // Check if AppendElem and SetElem are defined for the vector class.
  Function *AppendFunc = NewClassI->getSingleMemberFunction(AppendElem);
  Function *SetFunc = NewClassI->getSingleMemberFunction(SetElem);
  if (!AppendFunc || !SetFunc)
    return;

  // Check it is okay to convert Ctor to CCtor. Make sure there is
  // no change in values. Check flag field is same. Okay Size field
  // since AppendElem function is called in the immediate loop. Capacity
  // field can be ignored as it only impacts memory allocation.
  if (!ConstantPropApplied)
    return;
  if (!IsMemInterFieldSame(NewCtor))
    return;

  DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE,
                  { dbgs() << "Transformations 5: \n"; });
  Value *LoopCounter = nullptr;
  GetElementPtrInst *SrcGEP = nullptr;
  CallBase *AppendCB = nullptr;
  CallBase *CtorCB = nullptr;
  // Detect if any callsite of AppendElem function can be converted to
  // CCtor.
  for (auto *U : AppendFunc->users()) {
    auto *CB = dyn_cast<CallBase>(U);
    assert(CB && "Expected call");
    if (!CheckCopyCtor(CB, &LoopCounter, &SrcGEP, &CtorCB)) {
      DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
        dbgs() << "  CopyCtor detection failed: " << *CB << "\n";
      });
      continue;
    }
    if (AppendCB) {
      DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE,
                      { dbgs() << "  More than one CopyCtor detected \n"; });
      return;
    }
    AppendCB = CB;
  }
  if (!AppendCB)
    return;
  DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE,
                  { dbgs() << "  CopyCtor detected: " << *AppendCB << "\n"; });
  assert(SetFunc->arg_size() == 3 && "Unexpected SetElem function");
  // Make sure types of the loop counter and 3rd argument of SetElem
  // are same.
  // Expects last argument of SetFunc as integer argument.
  Type *SetFunIntArgTy = SetFunc->getArg(2)->getType();
  if (!SetFunIntArgTy->isIntegerTy() ||
      SetFunIntArgTy != LoopCounter->getType())
    return;

  // Create new simple SetElem function and replace AppendElem call
  // with the simple SetElem call.
  Function *CallerF = AppendCB->getFunction();
  Function *SimpleSetElem = CreateSimpleSetElementFunction(SetFunc);
  ReplaceAppendElemWithSetElemCall(SimpleSetElem, AppendCB, LoopCounter);

  // Create new simple CCtor function and replace Ctor call with
  // the simple CCtor call.
  Function *SimpleCCtor = CreateSimpleCCtorFunction(NewCtor);
  FixCtorFunctionCall(SimpleCCtor, CtorCB, SrcGEP);
  // Remove dead instructions.
  removeDeadInsts(CallerF);
}

// Reverse argument promotion for AppendFunc by converting pointer
// argument to pointer-to-pointer argument and then fix callsite.
//
//  Ex:
//  Before:
//     {
//       ...
//       AppendElem(this, %56);
//       ...
//     }
//
//     AppendElem(this, i16* %1) {
//      ...
//      store i16* %1, i16** %8
//      ...
//     }
//
//  After:
//     {
//       %5 = alloca i16*
//      ...
//       store i16* %56, i16** %5
//       AppendElem(this, i16** %5);
//     }
//
//     AppendElem(this, i16** %1) {
//      ...
//      %9 = load i16*, i16** %1
//      store i16* %9, i16** %8
//      ...
//     }
//
void SOAToAOSPrepCandidateInfo::reverseArgPromote() {
  auto IsReverseArgPromoteEligible = [](Function *F, unsigned Pos) {
    Argument *Arg = F->getArg(Pos);
    if (!Arg->hasOneUse())
      return false;
    auto *SI = dyn_cast<StoreInst>(Arg->user_back());
    if (!SI)
      return false;
    if (Arg != SI->getOperand(0))
      return false;
    return true;
  };

  Function *AppendFunc = NewClassI->getSingleMemberFunction(AppendElem);
  if (!AppendFunc || !AppendFunc->hasOneUse())
    return;
  DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE,
                  { dbgs() << "Transformations 6: \n"; });
  assert(AppendFunc->arg_size() == 2 && "Unexpected Append function");
  auto *CB = dyn_cast<CallBase>(AppendFunc->user_back());
  if (!CB || !IsReverseArgPromoteEligible(AppendFunc, 1)) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE,
                    { dbgs() << " AppendFunc is not eligible \n"; });
    return;
  }

  std::vector<Type *> Params;
  FunctionType *FTy = AppendFunc->getFunctionType();

  // No Change in 1st arg.
  Params.push_back(AppendFunc->getArg(0)->getType());
  // Make 2nd argument as pointer to the original arg type.
  Params.push_back(AppendFunc->getArg(1)->getType()->getPointerTo());
  FunctionType *NFTy =
      FunctionType::get(FTy->getReturnType(), Params, FTy->isVarArg());
  Function *NF = Function::Create(NFTy, AppendFunc->getLinkage(),
                                  AppendFunc->getAddressSpace());
  NF->copyAttributesFrom(AppendFunc);
  NF->setComdat(AppendFunc->getComdat());
  AppendFunc->getParent()->getFunctionList().insert(AppendFunc->getIterator(),
                                                    NF);
  NF->takeName(AppendFunc);

  // Fix CallBase by passing address of original param.
  Function *CallerF = CB->getParent()->getParent();
  std::vector<Value *> NewArgs;
  auto NFPAL = NF->getAttributes();
  AttributeList Attrs = CB->getAttributes();
  SmallVector<AttributeSet, 4> NewArgAttrs;
  auto *I = CB->arg_begin();
  NewArgs.push_back(*I);
  NewArgAttrs.push_back(NFPAL.getParamAttributes(0));
  AllocaInst *Alloca =
      new AllocaInst(AppendFunc->getArg(1)->getType(), 0, nullptr, "",
                     &*(CallerF->getEntryBlock().getFirstInsertionPt()));
  StoreInst *StoreI = new StoreInst(CB->getArgOperand(1), Alloca, CB);
  (void)StoreI;
  DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
    dbgs() << "  Created Alloca: " << *Alloca << "\n";
    dbgs() << "  Created Store: " << *StoreI << "\n";
  });
  NewArgs.push_back(Alloca);
  NewArgAttrs.push_back(NFPAL.getParamAttributes(1));

  AttributeList NewPAL =
      AttributeList::get(AppendFunc->getContext(), Attrs.getFnAttributes(),
                         Attrs.getRetAttributes(), NewArgAttrs);

  updateCallBase(CB, NewPAL, NF, NewArgs);

  NF->getBasicBlockList().splice(NF->begin(), AppendFunc->getBasicBlockList());

  // Update argument uses.
  unsigned Pos = 0;
  for (Function::arg_iterator I = AppendFunc->arg_begin(),
                              E = AppendFunc->arg_end(), I2 = NF->arg_begin();
       I != E; ++I) {
    if (Pos == 1) {
      // Generate load instruction to get element.
      Argument *Arg = &*I;
      auto *SI = cast<StoreInst>(Arg->user_back());
      Value *LI = new LoadInst(&*I2, "", SI);
      SI->setOperand(0, LI);
    } else {
      I->replaceAllUsesWith(&*I2);
    }
    I2->takeName(&*I);
    ++I2;
    Pos++;
  }
  AppendFunc->eraseFromParent();
  DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
    dbgs() << "  Updated AppendFunc: \n";
    NF->dump();
  });
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

  // After transforming types and member functions, Candidate/Class Info
  // is not valid. So, recompute the info again to do more transformations.
  if (!Candidate->computeUpdatedCandidateInfo()) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
      dbgs() << "SOAToAOSPrepare failed in the middle: Updated class Info.\n";
    });
    return false;
  }

  // 4th transform: Apply Ctor transformations.
  Function *NewCtor = Candidate->applyCtorTransformations();

  // 5th transform: Create CopyCtor from combination of "Ctor" and "add".
  Candidate->convertCtorToCCtor(NewCtor);

  // 6th transform: Reverse arg promotion for AppendFunc.
  Candidate->reverseArgPromote();

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
