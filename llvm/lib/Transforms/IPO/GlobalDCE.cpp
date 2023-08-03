//===-- GlobalDCE.cpp - DCE unreachable internal functions ----------------===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021-2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This transform is designed to eliminate unreachable internal globals from the
// program.  It uses an aggressive algorithm, searching out globals that are
// known to be alive.  After it finds all of the globals which are needed, it
// deletes whatever is left over.  This allows it to delete recursive chunks of
// the program which are unreachable.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/GlobalDCE.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/TypeMetadataUtils.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Utils/CtorUtils.h"
#include "llvm/Transforms/Utils/GlobalStatus.h"

#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/Intel_WP.h"
#endif // INTEL_CUSTOMIZATION

using namespace llvm;

#define DEBUG_TYPE "globaldce"

static cl::opt<bool>
    ClEnableVFE("enable-vfe", cl::Hidden, cl::init(true),
                cl::desc("Enable virtual function elimination"));

STATISTIC(NumAliases  , "Number of global aliases removed");
STATISTIC(NumFunctions, "Number of functions removed");
STATISTIC(NumIFuncs,    "Number of indirect functions removed");
STATISTIC(NumVariables, "Number of global variables removed");
STATISTIC(NumVFuncs,    "Number of virtual functions removed");

/// Returns true if F is effectively empty.
static bool isEmptyFunction(Function *F) {
  // Skip external functions.
  if (F->isDeclaration())
    return false;
  BasicBlock &Entry = F->getEntryBlock();
  for (auto &I : Entry) {
    if (I.isDebugOrPseudoInst())
      continue;
    if (auto *RI = dyn_cast<ReturnInst>(&I))
      return !RI->getReturnValue();
    break;
  }
  return false;
}

/// Compute the set of GlobalValue that depends from V.
/// The recursion stops as soon as a GlobalValue is met.
void GlobalDCEPass::ComputeDependencies(Value *V,
                                        SmallPtrSetImpl<GlobalValue *> &Deps) {
  if (auto *I = dyn_cast<Instruction>(V)) {
    Function *Parent = I->getParent()->getParent();
    Deps.insert(Parent);
  } else if (auto *GV = dyn_cast<GlobalValue>(V)) {
    Deps.insert(GV);
  } else if (auto *CE = dyn_cast<Constant>(V)) {
    // Avoid walking the whole tree of a big ConstantExprs multiple times.
    auto Where = ConstantDependenciesCache.find(CE);
    if (Where != ConstantDependenciesCache.end()) {
      auto const &K = Where->second;
      Deps.insert(K.begin(), K.end());
    } else {
      SmallPtrSetImpl<GlobalValue *> &LocalDeps = ConstantDependenciesCache[CE];
      for (User *CEUser : CE->users())
        ComputeDependencies(CEUser, LocalDeps);
      Deps.insert(LocalDeps.begin(), LocalDeps.end());
    }
  }
}

void GlobalDCEPass::UpdateGVDependencies(GlobalValue &GV) {
  SmallPtrSet<GlobalValue *, 8> Deps;
  for (User *User : GV.users())
    ComputeDependencies(User, Deps);
  Deps.erase(&GV); // Remove self-reference.
  for (GlobalValue *GVU : Deps) {
    // If this is a dep from a vtable to a virtual function, and we have
    // complete information about all virtual call sites which could call
    // though this vtable, then skip it, because the call site information will
    // be more precise.
    if (VFESafeVTables.count(GVU) && isa<Function>(&GV)) {
      LLVM_DEBUG(dbgs() << "Ignoring dep " << GVU->getName() << " -> "
                        << GV.getName() << "\n");
      continue;
    }
    GVDependencies[GVU].insert(&GV);
  }
}

/// Mark Global value as Live
void GlobalDCEPass::MarkLive(GlobalValue &GV,
                             SmallVectorImpl<GlobalValue *> *Updates) {
  auto const Ret = AliveGlobals.insert(&GV);
  if (!Ret.second)
    return;

  if (Updates)
    Updates->push_back(&GV);
  if (Comdat *C = GV.getComdat()) {
    for (auto &&CM : make_range(ComdatMembers.equal_range(C))) {
      MarkLive(*CM.second, Updates); // Recursion depth is only two because only
                                     // globals in the same comdat are visited.
    }
  }
}

void GlobalDCEPass::ScanVTables(Module &M) {
  SmallVector<MDNode *, 2> Types;
  LLVM_DEBUG(dbgs() << "Building type info -> vtable map\n");

  for (GlobalVariable &GV : M.globals()) {
    Types.clear();
    GV.getMetadata(LLVMContext::MD_type, Types);
    if (GV.isDeclaration() || Types.empty())
      continue;

    // Use the typeid metadata on the vtable to build a mapping from typeids to
    // the list of (GV, offset) pairs which are the possible vtables for that
    // typeid.
    for (MDNode *Type : Types) {
      Metadata *TypeID = Type->getOperand(1).get();

      uint64_t Offset =
          cast<ConstantInt>(
              cast<ConstantAsMetadata>(Type->getOperand(0))->getValue())
              ->getZExtValue();

      TypeIdMap[TypeID].insert(std::make_pair(&GV, Offset));
    }

    // If the type corresponding to the vtable is private to this translation
    // unit, we know that we can see all virtual functions which might use it,
    // so VFE is safe.
    if (auto GO = dyn_cast<GlobalObject>(&GV)) {
      GlobalObject::VCallVisibility TypeVis = GO->getVCallVisibility();
      if (TypeVis == GlobalObject::VCallVisibilityTranslationUnit ||
          (InLTOPostLink &&
           TypeVis == GlobalObject::VCallVisibilityLinkageUnit)) {
        LLVM_DEBUG(dbgs() << GV.getName() << " is safe for VFE\n");
        VFESafeVTables.insert(&GV);
      }
    }
  }
}

void GlobalDCEPass::ScanVTableLoad(Function *Caller, Metadata *TypeId,
                                   uint64_t CallOffset) {
  for (const auto &VTableInfo : TypeIdMap[TypeId]) {
    GlobalVariable *VTable = VTableInfo.first;
    uint64_t VTableOffset = VTableInfo.second;

    Constant *Ptr =
        getPointerAtOffset(VTable->getInitializer(), VTableOffset + CallOffset,
                           *Caller->getParent(), VTable);
    if (!Ptr) {
      LLVM_DEBUG(dbgs() << "can't find pointer in vtable!\n");
      VFESafeVTables.erase(VTable);
      continue;
    }

    auto Callee = dyn_cast<Function>(Ptr->stripPointerCasts());
    if (!Callee) {
      LLVM_DEBUG(dbgs() << "vtable entry is not function pointer!\n");
      VFESafeVTables.erase(VTable);
      continue;
    }

    LLVM_DEBUG(dbgs() << "vfunc dep " << Caller->getName() << " -> "
                      << Callee->getName() << "\n");
    GVDependencies[Caller].insert(Callee);
  }
}

void GlobalDCEPass::ScanTypeCheckedLoadIntrinsics(Module &M) {
  LLVM_DEBUG(dbgs() << "Scanning type.checked.load intrinsics\n");
  Function *TypeCheckedLoadFunc =
      M.getFunction(Intrinsic::getName(Intrinsic::type_checked_load));
  Function *TypeCheckedLoadRelativeFunc =
      M.getFunction(Intrinsic::getName(Intrinsic::type_checked_load_relative));

  auto scan = [&](Function *CheckedLoadFunc) {
    if (!CheckedLoadFunc)
      return;

    for (auto *U : CheckedLoadFunc->users()) {
      auto CI = dyn_cast<CallInst>(U);
      if (!CI)
        continue;

      auto *Offset = dyn_cast<ConstantInt>(CI->getArgOperand(1));
      Value *TypeIdValue = CI->getArgOperand(2);
      auto *TypeId = cast<MetadataAsValue>(TypeIdValue)->getMetadata();

      if (Offset) {
        ScanVTableLoad(CI->getFunction(), TypeId, Offset->getZExtValue());
      } else {
        // type.checked.load with a non-constant offset, so assume every entry
        // in every matching vtable is used.
        for (const auto &VTableInfo : TypeIdMap[TypeId]) {
          VFESafeVTables.erase(VTableInfo.first);
        }
      }
    }
  };

  scan(TypeCheckedLoadFunc);
  scan(TypeCheckedLoadRelativeFunc);
}

void GlobalDCEPass::AddVirtualFunctionDependencies(Module &M) {
  if (!ClEnableVFE)
    return;

  // If the Virtual Function Elim module flag is present and set to zero, then
  // the vcall_visibility metadata was inserted for another optimization (WPD)
  // and we may not have type checked loads on all accesses to the vtable.
  // Don't attempt VFE in that case.
  auto *Val = mdconst::dyn_extract_or_null<ConstantInt>(
      M.getModuleFlag("Virtual Function Elim"));
  if (!Val || Val->isZero())
    return;

  ScanVTables(M);

  if (VFESafeVTables.empty())
    return;

  ScanTypeCheckedLoadIntrinsics(M);

  LLVM_DEBUG(
    dbgs() << "VFE safe vtables:\n";
    for (auto *VTable : VFESafeVTables)
      dbgs() << "  " << VTable->getName() << "\n";
  );
}

#if INTEL_CUSTOMIZATION
// Preprocess Auto CPU Dispatch (ACD) inserted dispatcher function uses
// to turn up more opportunities for DCE.
static bool PreprocessDispatcherFunctions(Module &M) {
  bool Changed = false;

  // After IPO passes are run, ACD generated multi-versioned functions may have
  // direct calls to other multi-versioned functions, defined in the same
  // module, through dispatcher functions (IFunc's on Linux, wrapper-based
  // dispatchers on Windows). Such calls can instead be made directly to the
  // appropriate target-specific callees matching thecaller's target.
  bool IterChanged = true;
  while (IterChanged) {
    IterChanged = false;

    // Here, we're going to transform the call below to bar():
    //      define foo.A
    //        call bar()    // bar() is a dispatcher function
    // to call to bar.A():
    //      define foo.A
    //        call bar.A()
    // This transformation may create dead dispatcher symbols/functions
    // for GlobalDCE to remove them.

    // Linux implementation uses IFunc dispatchers, thus here we preprocess them.
    for (GlobalIFunc &GIF : M.ifuncs()) {

      Function *Resolver = GIF.getResolverFunction();
      if (!Resolver || !Resolver->getName().endswith(".resolver"))
        continue;

      for (auto It = GIF.use_begin(), End = GIF.use_end(); It != End;) {
        Use &IFUse = *It;
        ++It;

        auto *CInst = dyn_cast<CallBase>(IFUse.getUser());
        if (CInst && CInst->isCallee(&IFUse)) {
          Function *Caller = CInst->getFunction();
          if (!Caller->hasMetadata("llvm.acd.clone"))
            continue;
          auto NameTargetPair = Caller->getName().rsplit('.');
          if (NameTargetPair.second.empty())
            continue;
          auto ReplacementName =
              GIF.getName().str() + "." + NameTargetPair.second.str();
          Function *Replacement = M.getFunction(ReplacementName);
          if (Replacement && Replacement->hasMetadata("llvm.acd.clone")) {
            CInst->setCalledFunction(Replacement);
            IterChanged = true;
          }
        }
      }
    }

    // Windows implementation uses wrapper-based dispatchers,
    // hence, we preprocess functions that carry llvm.acd.dispatcher metadata.
    for (Function &Dispatcher : M.functions()) {

      if (!Dispatcher.hasMetadata("llvm.acd.dispatcher"))
        continue;

      auto End = Dispatcher.use_end();
      for (auto It = Dispatcher.use_begin(); It != End;) {
        Use &DUse = *It;
        ++It;

        auto *CInst = dyn_cast<CallBase>(DUse.getUser());
        if (CInst && CInst->isCallee(&DUse)) {
          Function *Caller = CInst->getFunction();
          if (!Caller->hasMetadata("llvm.acd.clone"))
            continue;
          auto NameTargetPair = Caller->getName().rsplit('.');
          if (NameTargetPair.second.empty())
            continue;
          auto ReplacementName = Dispatcher.getName().str() + "." +
                                 NameTargetPair.second.str();
          Function *Replacement = M.getFunction(ReplacementName);
          if (Replacement && Replacement->hasMetadata("llvm.acd.clone")) {
            CInst->setCalledFunction(Replacement);
            IterChanged = true;
          }
        }
      }

      // If Dispatcher has local linkage and no remaining uses in IR,
      // drop all its references.
      if (Dispatcher.hasLocalLinkage() && Dispatcher.hasZeroLiveUses()) {
        Dispatcher.dropAllReferences();
      }
    }

    // Windows implementation uses wrapper-based dispatchers.
    // IPO passes may also inline wrapper-based dispatchers at callsites,
    // creating indirect calls to bar() as shown below:
    //      define foo.A
    //        %0 = load bar.ptr
    //        call %0()          // call to bar.A() through bar.ptr
    // Hence, transform such indirect calls to bar.A() to direct calls to bar.A():
    //      define foo.A
    //        call bar.A()
    // Process global function pointers that carry llvm.acd.dispatcher metadata.
    for (GlobalVariable &DispatchPtr : M.globals()) {

      if (!DispatchPtr.hasMetadata("llvm.acd.dispatcher"))
        continue;

      assert(DispatchPtr.getName().endswith(".ptr"));

      StringRef FnName = DispatchPtr.getName().drop_back(4);
      Function *ResolverFn = M.getFunction("__intel.acd.resolver");

      bool AllUsesAreInResolverFn = true;
      auto End = DispatchPtr.use_end();
      for (auto It = DispatchPtr.use_begin(); It != End;) {
        Use &PtrUse = *It;
        ++It;

        // Check if the function pointer is used in the resolver function.
        auto *Inst = dyn_cast<Instruction>(PtrUse.getUser());
        if (Inst && Inst->getFunction() == ResolverFn)
          continue;

        AllUsesAreInResolverFn = false;

        // Check if the use is a load instructions
        auto *LI = dyn_cast<LoadInst>(PtrUse.getUser());
        if (!LI)
          continue;

        // Iterate over the uses of the load instruction.
        auto End2 = LI->use_end();
        for (auto It2 = LI->use_begin(); It2 != End2;) {
          Use &LUse = *It2;
          ++It2;

          // If the use is the callee of a callsite, transform it to a direct call.
          auto *CInst = dyn_cast<CallBase>(LUse.getUser());
          if (CInst && CInst->isCallee(&LUse)) {
            Function *Caller = CInst->getFunction();
            if (!Caller->hasMetadata("llvm.acd.clone"))
              continue;
            auto NameTargetPair = Caller->getName().rsplit('.');
            if (NameTargetPair.second.empty())
              continue;
            auto ReplacementName = FnName.str() + "." + NameTargetPair.second.str();
            Function *Replacement = M.getFunction(ReplacementName);
            if (Replacement && Replacement->hasMetadata("llvm.acd.clone")) {
              CInst->setCalledFunction(Replacement);
              IterChanged = true;
            }
          }
        }

        // Remove load instruction, if it has no remaining uses in IR.
        if (LI->hasNUses(0)) {
          LI->eraseFromParent();
        }
      }

      // If all remaining uses of DispatchPtr are inside the resolver,
      // delete them as well in order to get rid of the dispatch pointer later.
      if (AllUsesAreInResolverFn) {
        for (auto It = DispatchPtr.use_begin(); It != End;) {
          Use &PtrUse = *It;
          ++It;
          auto *Inst = dyn_cast<Instruction>(PtrUse.getUser());
          // Assert that DispatchPtr use is in the resolver function.
          assert(Inst && Inst->getFunction() == ResolverFn);
          Inst->eraseFromParent();
        }
      }
    }
    Changed |= IterChanged;
  }
  return Changed;
}
#endif // INTEL_CUSTOMIZATION

PreservedAnalyses GlobalDCEPass::run(Module &M, ModuleAnalysisManager &MAM) {
  bool Changed = false;

#if INTEL_CUSTOMIZATION
  // Preprocess Auto CPU Dispatch (ACD) inserted dispatcher function uses
  // to turn up more opportunities for DCE.
  Changed |= PreprocessDispatcherFunctions(M);
#endif // INTEL_CUSTOMIZATION

  // The algorithm first computes the set L of global variables that are
  // trivially live.  Then it walks the initialization of these variables to
  // compute the globals used to initialize them, which effectively builds a
  // directed graph where nodes are global variables, and an edge from A to B
  // means B is used to initialize A.  Finally, it propagates the liveness
  // information through the graph starting from the nodes in L. Nodes note
  // marked as alive are discarded.

  // Remove empty functions from the global ctors list.
  Changed |= optimizeGlobalCtorsList(
      M, [](uint32_t, Function *F) { return isEmptyFunction(F); });

  // Collect the set of members for each comdat.
  for (Function &F : M)
    if (Comdat *C = F.getComdat())
      ComdatMembers.insert(std::make_pair(C, &F));
  for (GlobalVariable &GV : M.globals())
    if (Comdat *C = GV.getComdat())
      ComdatMembers.insert(std::make_pair(C, &GV));
  for (GlobalAlias &GA : M.aliases())
    if (Comdat *C = GA.getComdat())
      ComdatMembers.insert(std::make_pair(C, &GA));

  // Add dependencies between virtual call sites and the virtual functions they
  // might call, if we have that information.
  AddVirtualFunctionDependencies(M);

  // Loop over the module, adding globals which are obviously necessary.
  for (GlobalObject &GO : M.global_objects()) {
    GO.removeDeadConstantUsers();
    // Functions with external linkage are needed if they have a body.
    // Externally visible & appending globals are needed, if they have an
    // initializer.
    if (!GO.isDeclaration())
      if (!GO.isDiscardableIfUnused())
        MarkLive(GO);

    UpdateGVDependencies(GO);
  }

  // Compute direct dependencies of aliases.
  for (GlobalAlias &GA : M.aliases()) {
    GA.removeDeadConstantUsers();
    // Externally visible aliases are needed.
    if (!GA.isDiscardableIfUnused())
      MarkLive(GA);

    UpdateGVDependencies(GA);
  }

  // Compute direct dependencies of ifuncs.
  for (GlobalIFunc &GIF : M.ifuncs()) {
    GIF.removeDeadConstantUsers();
    // Externally visible ifuncs are needed.
    if (!GIF.isDiscardableIfUnused())
      MarkLive(GIF);

    UpdateGVDependencies(GIF);
  }

  // Propagate liveness from collected Global Values through the computed
  // dependencies.
  SmallVector<GlobalValue *, 8> NewLiveGVs{AliveGlobals.begin(),
                                           AliveGlobals.end()};
  while (!NewLiveGVs.empty()) {
    GlobalValue *LGV = NewLiveGVs.pop_back_val();
    for (auto *GVD : GVDependencies[LGV])
      MarkLive(*GVD, &NewLiveGVs);
  }

  // Now that all globals which are needed are in the AliveGlobals set, we loop
  // through the program, deleting those which are not alive.
  //

  // The first pass is to drop initializers of global variables which are dead.
  std::vector<GlobalVariable *> DeadGlobalVars; // Keep track of dead globals
  for (GlobalVariable &GV : M.globals())
    if (!AliveGlobals.count(&GV)) {
      DeadGlobalVars.push_back(&GV);         // Keep track of dead globals
      if (GV.hasInitializer()) {
        Constant *Init = GV.getInitializer();
        GV.setInitializer(nullptr);
        if (isSafeToDestroyConstant(Init))
          Init->destroyConstant();
      }
    }

  // The second pass drops the bodies of functions which are dead...
  std::vector<Function *> DeadFunctions;
  for (Function &F : M)
    if (!AliveGlobals.count(&F)) {
      DeadFunctions.push_back(&F);         // Keep track of dead globals
      if (!F.isDeclaration())
        F.deleteBody();
    }

  // The third pass drops targets of aliases which are dead...
  std::vector<GlobalAlias*> DeadAliases;
  for (GlobalAlias &GA : M.aliases())
    if (!AliveGlobals.count(&GA)) {
      DeadAliases.push_back(&GA);
      GA.setAliasee(nullptr);
    }

  // The fourth pass drops targets of ifuncs which are dead...
  std::vector<GlobalIFunc*> DeadIFuncs;
  for (GlobalIFunc &GIF : M.ifuncs())
    if (!AliveGlobals.count(&GIF)) {
      DeadIFuncs.push_back(&GIF);
      GIF.setResolver(nullptr);
    }

  // Now that all interferences have been dropped, delete the actual objects
  // themselves.
  auto EraseUnusedGlobalValue = [&](GlobalValue *GV) {
    GV->removeDeadConstantUsers();
    GV->eraseFromParent();
    Changed = true;
  };

  NumFunctions += DeadFunctions.size();
  for (Function *F : DeadFunctions) {
    if (!F->use_empty()) {
      // Virtual functions might still be referenced by one or more vtables,
      // but if we've proven them to be unused then it's safe to replace the
      // virtual function pointers with null, allowing us to remove the
      // function itself.
      ++NumVFuncs;

      // Detect vfuncs that are referenced as "relative pointers" which are used
      // in Swift vtables, i.e. entries in the form of:
      //
      //   i32 trunc (i64 sub (i64 ptrtoint @f, i64 ptrtoint ...)) to i32)
      //
      // In this case, replace the whole "sub" expression with constant 0 to
      // avoid leaving a weird sub(0, symbol) expression behind.
      replaceRelativePointerUsersWithZero(F);

      F->replaceNonMetadataUsesWith(ConstantPointerNull::get(F->getType()));
    }
    EraseUnusedGlobalValue(F);
  }

  NumVariables += DeadGlobalVars.size();
  for (GlobalVariable *GV : DeadGlobalVars)
    EraseUnusedGlobalValue(GV);

  NumAliases += DeadAliases.size();
  for (GlobalAlias *GA : DeadAliases)
    EraseUnusedGlobalValue(GA);

  NumIFuncs += DeadIFuncs.size();
  for (GlobalIFunc *GIF : DeadIFuncs)
    EraseUnusedGlobalValue(GIF);

  // Make sure that all memory is released
  AliveGlobals.clear();
  ConstantDependenciesCache.clear();
  GVDependencies.clear();
  ComdatMembers.clear();
  TypeIdMap.clear();
  VFESafeVTables.clear();

  if (!Changed)                         // INTEL
    return PreservedAnalyses::all();    // INTEL

  auto PA = PreservedAnalyses();        // INTEL
  PA.preserve<WholeProgramAnalysis>();  // INTEL
  PA.preserve<AndersensAA>();           // INTEL

  return PA;                            // INTEL
}

void GlobalDCEPass::printPipeline(
    raw_ostream &OS, function_ref<StringRef(StringRef)> MapClassName2PassName) {
  static_cast<PassInfoMixin<GlobalDCEPass> *>(this)->printPipeline(
      OS, MapClassName2PassName);
  if (InLTOPostLink)
    OS << "<vfe-linkage-unit-visibility>";
}
