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
//
// TODO: Add more examples later.
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
  void applyCtorTransformations();

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
void SOAToAOSPrepCandidateInfo::applyCtorTransformations() {

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
    return;
  }

  // Check if flag field is not modified anywhere except constructor.
  // Copy-Constructor is not handled for now.
  SmallPtrSet<LoadInst *, 8> FlagUseSet;
  if (!CollectAllFlagFieldUses(FlagUseSet, FlagFI)) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSPREPARE, {
      dbgs() << "   Not able to handle all uses of flag field\n";
    });
    return;
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
    return;
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

  CreateNewFunctionWithUnusedArgAsLast(CtorF, UnusedArgPos);

  CtorF->eraseFromParent();
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
  Candidate->applyCtorTransformations();

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
