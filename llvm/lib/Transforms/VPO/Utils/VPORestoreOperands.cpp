#if INTEL_COLLAB
//===----- VPRestoreOperands.cpp - Restore Operands Pass for OpenMP -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements VPO Restore Operands pass to replace the renamed
/// clause operands created by the VPO Paropt Prepare pass, with the original
/// operands.
///
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/VPO/Utils/VPORestoreOperands.h"
#include "llvm/Analysis/DomTreeUpdater.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/IntrinsicUtils.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptUtils.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/VPO/VPOPasses.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-restore-operands"

// Return true if V is a pointer cast, address space cast, or zero-offset
// GEP; false otherwise.
bool VPOUtils::isPointerCastOrZeroOffsetGEP(Value *V) {

  assert(V && "isPointerCast: Null input value.");
  if (!V->getType()->isPointerTy())
    return false;

  if (auto *GEP = dyn_cast<GEPOperator>(V))
    return GEP->hasAllZeroIndices();

  if (Operator::getOpcode(V) == Instruction::BitCast ||
      Operator::getOpcode(V) == Instruction::AddrSpaceCast)
    return true;

  return false;
}

/// A helper class to track restoring of OPERAND_ADDR:SUBOBJ bundles.
class SubObjectHandler {
  CallInst *EntryDir = nullptr;
  SmallDenseMap<Value *, Value *, 8> ValueToInstructionMap;

public:
  SubObjectHandler(CallInst *EntryDir) : EntryDir(EntryDir){};

  /// Returns an Instruction for the constexpr or global \p V, inserted before
  /// the entry directive \p EntryDir. Returns nullptr if \p V is already an
  /// Instrucion, or not a GlobalVariable or ConstantExpr.
  Instruction *getConstExprPtrAsInstruction(Value *V) {
    assert(isa<PointerType>(V->getType()) && "V is not a pointer.");
    if (isa<Instruction>(V))
      return nullptr;

    auto Exiter = [&, FunctionName = __FUNCTION__, V](Instruction *I) {
      (void)FunctionName;
      LLVM_DEBUG(dbgs() << FunctionName << ": Converted '";
                 V->printAsOperand(dbgs()); dbgs() << "' into the instruction'";
                 I->printAsOperand(dbgs(), /*PrintType =*/false);
                 dbgs() << "'.\n");

      ValueToInstructionMap.insert({V, I});

      return I;
    };

    if (!isa<ConstantExpr>(V) && !isa<GlobalVariable>(V))
      return nullptr;

    if (auto It = ValueToInstructionMap.find(V);
        It != ValueToInstructionMap.end())
      return cast<Instruction>(It->second);

    if (isa<ConstantExpr>(V))
      return Exiter(cast<ConstantExpr>(V)->getAsInstruction(EntryDir));

    // Reaching here means V is a GlobalVariable.
    return Exiter(BitCastInst::CreateBitOrPointerCast(
        V, V->getType(), V->getName() + ".inst", EntryDir));
  }

  CallInst *updateSubObjClauseOperandsInDirective() {
    CallInst *UpdatedEntryDir =
        IntrinsicUtils::replaceFirstBundleOperandsInDirectiveIf(
            EntryDir, ValueToInstructionMap, [](const OperandBundleDef &B) {
              ClauseSpecifier ClauseInfo(B.getTag());
              return ClauseInfo.getIsSubObject();
            });

    EntryDir = UpdatedEntryDir;
    return UpdatedEntryDir;
  }
};

/// Restore the clause operands by undoing the renaming done in the prepare
/// pass. It looks at all OpenMP directive intrinsics for
/// `QUAL.OMP.OPERAND.ADDR(opnd, addr)` pairs, and:
/// * Replaces uses of the load (can be zero or one) from `addr` with `opnd`.
/// * Deletes any load/store/bitcasts/zero-offset GEPs/lifetime intrinsics on
/// `addr`.
/// * Deletes all `QUAL.OMP.OPERAND.ADDR` bundles from the intrinsics.
///
/// Some possible IR examples for before/after this function is
/// called are:
///
/// (A)
///
/// \code
///            Before                   |            After
///          ---------------------------+---------------------------
///   %y.addr = alloca i32*             |
///   store i32* %y, i32** %y.addr      |
///   %1 = begin_region[... %y...       |   %1 = begin_region[... %y...]
///                     "OPND.ADDR"     |
///                     (i32* %y,       |
///                     (i32** %y.addr)]|
///   %y1 = load i32*, i32** %y.addr    |
///                                     |
///   ...                               |   ...
///   <%y1 used inside the region>      |   <%y used inside the region>
///                                     |
///   end_region(%1)                    |   end_region(%1)
/// \endcode
///
/// (B)
///
/// \code
///            Before                   |            After
///          ---------------------------+---------------------------
///   %y.addr = alloca i32*             |
///   store i32* %y, i32** %y.addr      |
///   %1 = begin_region[... %y...       |   %1 = begin_region[... %y...]
///                     "OPND.ADDR"     |
///                     (i32* %y,       |
///                     (i32** %y.addr)]|
///                                     |
///   ...                               |   ...
///                      <%y not used inside the region>
///                                     |
///   end_region(%1)                    |   end_region(%1)
/// \endcode
///
/// (C)
///
/// \code
///            Before                   |            After
///          ---------------------------+---------------------------
///   %y = alloca i32                   |   %y = alloca i32
///   %y.cast = bitcast i32* %y to      |   %y.cast = bitcast i32* %y to
///             %struct.ty*             |             %struct.ty*
///   %y.addr = alloca %struct.ty*      |
///   %y.addr.cast = bitcast            |
///                  %struct.ty* %y.addr|
///                  to i32**           |
///   store i32* %y, i32** %y.addr.cast |
///   %1 = begin_region[... %y.cast...  |   %1 = begin_region[... %y.cast...]
///              "OPND.ADDR"            |
///              (%struct.ty* %y.cast,  |
///              (%struct.ty** %y.addr)]|
///   %y1 = load %struct.ty*,           |
///              %struct.ty** %y.addr   |
///   ...                               |   ...
///   <%y1 used inside the region>      |   <%y.cast used inside the region>
///                                     |
///   end_region(%1)                    |   end_region(%1)
/// \endcode
///
/// (D)
///
/// \code
///            Before                   |            After
///          ---------------------------+---------------------------
///   %y = alloca %struct.ty            |   %y = alloca %struct.ty
///   %y.addr = alloca %struct.ty*      |
///   store %struct.ty* %y,             |
///         %struct.ty** %y.addr        |
///   %1 = begin_region[... %y...       |   %1 = begin_region[... %y...]
///              "OPND.ADDR"            |
///              (%struct.ty* %y,       |
///              (%struct.ty** %y.addr)]|
///   %y.addr.cast = bitcast            |
///                  %struct.ty* %y.addr|
///                  to i8**            |
///   %y1 = load i8*, i8** %y.addr.cast |   %y.new = bitcast %struct.ty* %y
///                                     |            to i8*
///   ...                               |   ...
///   <%y1 used inside the region>      |  <%y.new used inside the region>
///                                     |
///   end_region(%1)                    |   end_region(%1)
/// \endcode
///
/// (E)
///
/// \code
///            Before                   |            After
///          ---------------------------+---------------------------
///   %y = alloca i32                   |   %y = alloca i32
///   %y.cast = bitcast i32* %y to      |   %y.cast = bitcast i32* %y to
///             %struct.ty*             |             %struct.ty*
///   %y.addr = alloca %struct.ty*      |
///   %y.addr.cast = bitcast            |
///                  %struct.ty* %y.addr|
///                  to i32**           |
///   store i32* %y, i32** %y.addr.cast |
///   %1 = begin_region[... %y.cast...  |   %1 = begin_region[... %y.cast...]
///              "OPND.ADDR"            |
///              (%struct.ty* %y.cast,  |
///              (%struct.ty** %y.addr)]|
///   %y1 = load i32*,                  |   %y.new = bitcast %struct.ty* %y.cast
///              i32** %y.addr.cast     |            to i32*
///   ...                               |   ...
///   <%y1 used inside the region>      |   <%y.new used inside the region>
///                                     |
///   end_region(%1)                    |   end_region(%1)
///
/// \endcode
///
/// (F)
///
/// \code
///            Before                   |            After
///          ---------------------------+---------------------------
///   %y = alloca i32                   |   %y = alloca i32
///   %y.cast = bitcast i32* %y to      |   %y.cast = bitcast i32* %y to
///             %struct.ty*             |             %struct.ty*
///   %y.addr = alloca %struct.ty*      |
///   %y.addr.cast = bitcast            |
///                  %struct.ty* %y.addr|
///                  to i32**           |
///   store i32* %y, i32** %y.addr.cast |
///   %1 = begin_region[... %y.cast...  |   %1 = begin_region[... %y.cast...]
///              "OPND.ADDR"            |
///              (%struct.ty* %y.cast,  |
///              (%struct.ty** %y.addr)]|
///   %y.addr.cast1 = bitcast           |
///                  %struct.ty* %y.addr|
///                  to i8**            |
///   %y1 = load i8*, i8** %y.addr.cast1|   %y.new = bitcast %struct.ty* %y.cast
///                                     |            to i8*
///   ...                               |   ...
///   <%y1 used inside the region>      |   <%y.new used inside the region>
///                                     |
///   end_region(%1)                    |   end_region(%1)
///
/// \endcode
///
/// (G)
///
/// \code
///            Before                   |            After
///          ---------------------------+---------------------------
///   %y.addr = alloca i32*             |
///                                     |
///   %y.addr.cst = bitcast %y.addr    (1)
///                          to i8*     |
///   @llvm.lifetime.start(%y.addr.cst)(2)
///                                     |
///   store i32* %y, i32** %y.addr      |
///   %1 = begin_region[... %y...       |   %1 = begin_region[... %y...]
///                     "OPND.ADDR"     |
///                     (i32* %y,       |
///                     (i32** %y.addr)]|
///   %y1 = load i32*, i32** %y.addr    |
///                                     |
///   ...                               |   ...
///   <%y1 used inside the region>      |   <%y used inside the region>
///                                     |
///   end_region(%1)                    |   end_region(%1)
///                                     |
///   %y.addr.cst1  = bitcast %y.addr  (3)
///                           to i8*    |
///   @llvm.lifetime.end(%y.addr.cst1) (4)
///   @llvm.lifetime.end(%y.addr.cst1) (5)
///   @llvm.lifetime.end(%y.addr.cst1) (6)
/// \endcode
///
/// (H)
///
/// \code
///            Before                   |            After
///          ---------------------------+---------------------------
///   %y.addr = alloca ptr              |
///                                     |
///   @llvm.lifetime.start(%y.addr)    (7)
///                                     |
///   store ptr %y, ptr %y.addr         |
///   %1 = begin_region[... %y...       |   %1 = begin_region[... %y...]
///                     "OPND.ADDR"     |
///                     (ptr %y,        |
///                     (ptr %y.addr)]  |
///   %y1 = load ptr, ptr %y.addr       |
///                                     |
///   ...                               |   ...
///   <%y1 used inside the region>      |   <%y used inside the region>
///                                     |
///   end_region(%1)                    |   end_region(%1)
///                                     |
///   @llvm.lifetime.end(%y.addr)      (8)
///   @llvm.lifetime.end(%y.addr)      (9)
///   @llvm.lifetime.end(%y.addr)      (10)
/// \endcode
///
///
/// (I)
///
/// \code
///            Before                   |            After
///          ---------------------------+---------------------------
///   %y1.addr = alloca ptr             |
///   %y2.addr = alloca ptr             |
///   %y3.addr = alloca ptr             |
///                                     |
///   store ptr @y, ptr %y1.addr        |
///   store ptr gep(@y, 4), ptr %y2.addr|
///   store ptr @y, ptr %y3.addr        |
///                                    (11) %y.0 = bitcast ptr @y to ptr
///                                    (12) %y.4 = gep(@y, 4)
///                                     |
///   %1 = begin_region[...             |   %1 = begin_region[...
///        "PRIVATE:SUBOBJ"(@y)        (13)      "PRIVATE:SUBOBJ"(%y.0)
///        "PRIVATE:SUBOBJ"(gep(@y, 4))(14)      "PRIVATE:SUBOBJ"(%y.4)
///        "SHARED"(@y)                 |        "SHARED"(@y) ]
///        "OPND.ADDR:SUBOBJ"(@y,       |
///                           %y1.addr) |
///        "OPND.ADDR:SUBOBJ"(gep(@y, 4)|
///                           %y2.addr) |
///        "OPND.ADDR"(ptr @y,          |
///                    ptr %y3.addr) ]  |
///                                     |
///   %y1 = load ptr, ptr %y1.addr      |
///   %y2 = load ptr, ptr %y2.addr      |
///   %y3 = load ptr, ptr %y3.addr      |
///                                     |
///   ...                               |   ...
///   <%y1 used inside the region>      |   <%y.0 used inside the region>
///   <%y2 used inside the region>      |   <%y.4 used inside the region>
///   <%y3 used inside the region>      |   <@y used inside the region>
///                                     |
///   end_region(%1)                    |   end_region(%1)
///                                     |
/// \endcode
/// TODO: OPAQUEPOINTER: After the type information is removed with opaque
/// pointers, bitcast instructions will be removed. It will make the possible
/// input IRs simpler. The code below needs to be revised appropriately.
// Lit tests for some of the above cases are:
// A: target_map_global_array.ll,rename_and_restore_nested.ll
// B: target_map_gep_fence2.ll
// C: [rename_and_]restore_struct_castoutside.ll
// D: [rename_and_]restore_int_addrcast.ll,
//    [rename_and_]restore_struct_castinside.ll
// E: [rename_and_]restore_struct_castsame.ll
// F: [rename_and_]restore_struct_castboth.ll
// G: restore_remove_lifetime_of_temps.ll
// H: restore_remove_lifetime_of_temps_nocast_opaqueptrs.ll
// I: rename_and_restore_subobj_geps_and_base_subobj.ll
bool VPOUtils::restoreOperands(Function &F) {
  LLVM_DEBUG(dbgs() << "VPO Restore Operands \n");

  bool Changed = false;

  SmallPtrSet<CallInst *, 8> DirectivesToRemoveOpndAddrsFrom;

  StringRef OperandAddrClauseString =
      VPOAnalysisUtils::getClauseString(QUAL_OMP_OPERAND_ADDR);
  std::string OperandAddrSubObjClauseString =
      (OperandAddrClauseString + ":SUBOBJ").str();

  for (Function::iterator B = F.begin(), BE = F.end(); B != BE; ++B)
    for (BasicBlock::iterator I = B->begin(), IE = B->end(); I != IE; ++I) {
      CallInst *CI = dyn_cast<CallInst>(&*I);
      if (!CI || !VPOAnalysisUtils::isOpenMPDirective(CI))
        continue;

      if (CI->getNumOperandBundles() == 0)
        continue;

      SubObjectHandler SubObjHandler(CI);
      bool CINeedsUpdates = false;

      for (unsigned I = 1; I < CI->getNumOperandBundles(); ++I) {
        OperandBundleUse BU = CI->getOperandBundleAt(I);
        if (!BU.getTagName().startswith(OperandAddrClauseString))
          continue;

        assert(BU.Inputs.size() == 2 && "Expected an operand-address pair in a "
                                        "'QUAL.OMP.OPERAND.ADDR' bundle.");

        bool IsSubObject = ClauseSpecifier(BU.getTagName()).getIsSubObject();

        Value *VOrig = BU.Inputs[0];
        AllocaInst *VAddr = cast_or_null<AllocaInst>(BU.Inputs[1]);

        assert(VOrig && "Null operand in 'QUAL.OMP.OPERAND.ADDR'.");
        assert(VAddr && "Null/non-alloca address in 'QUAL.OMP.OPERAND.ADDR'.");

        LoadInst *VRenamed = nullptr;
        StoreInst *VAddrStore = nullptr;
        Instruction *VAddrStoreCast = nullptr;
        Instruction *VAddrLoadCast = nullptr;
        SmallVector<Instruction *, 4> VAddrUsersInLifetimeMarkers;

        auto userIsLifetimeMarker = [&](User *U) -> bool {
          auto *II = dyn_cast<IntrinsicInst>(U);
          if (!II)
            return false;

          assert((II->getIntrinsicID() == Intrinsic::lifetime_start ||
                  II->getIntrinsicID() == Intrinsic::lifetime_end) &&
                 "Unexpected intrinsic using the ADDR operand (or a cast on "
                 "it) of 'QUAL.OMP.OPERAND.ADDR'.");
          return true;
        };

        auto collectLifetimeMarkerUsers = [&](User *U) -> bool {
          if (userIsLifetimeMarker(U)) {
            VAddrUsersInLifetimeMarkers.push_back(cast<IntrinsicInst>(U));
            return true;
          }
          return false;
        };

        for (User *U : VAddr->users()) {
          if (U == CI)
            continue;

          if (auto *LI = dyn_cast<LoadInst>(U)) {
            VRenamed = LI; // A, C: %y1
            continue;
          }

          if (auto *SI = dyn_cast<StoreInst>(U)) {
            assert(SI->getPointerOperand() == VAddr &&
                   "QUAL.OMP.OPERAND.ADDR addr operand is stored somewhere.");
            VAddrStore = SI; // A, B, D: store %y, %y.addr
            continue;
          }

          if (collectLifetimeMarkerUsers(U)) // H: (7)(8)(9)(10)
            continue;

          assert(VPOUtils::isPointerCastOrZeroOffsetGEP(U) &&
                 "Unexpected use of ADDR operand of 'QUAL.OMP.OPERAND.ADDR'.");
          // Reaching here means that U is a pointer cast / zero-offset GEP.

          assert((U->getNumUses() <=
                  llvm::count_if(U->users(), userIsLifetimeMarker) + 2) &&
                 "Unexpected number of uses of a bitcast on operand addr.");
          Instruction *CastI = cast<Instruction>(U);
          bool CastIsUsedInLifetimeMarkers = false;

          for (User *CastUser : CastI->users()) {
            if (isa<LoadInst>(CastUser)) {
              assert(!VRenamed && "Unexpected second load from operand addr.");
              VRenamed = cast<LoadInst>(CastUser); // D, E, F: %y1
              VAddrLoadCast = CastI; // D, E: %y.addr.cast; F: %y.addr.cast1
            } else if (auto *CastUserStore = dyn_cast<StoreInst>(CastUser)) {
              assert(CastUserStore->getPointerOperand() == CastI &&
                     "QUAL.OMP.OPERAND.ADDR addr operand is stored somewhere.");
              assert(!VAddrStore && "Unexpected second store to operand addr.");
              VAddrStoreCast = CastI;     // C, E, F: %y.addr.cast
              VAddrStore = CastUserStore; // C, E, F: store %y, %y.addr.cast
            } else if (collectLifetimeMarkerUsers(
                           CastUser)) {           // G: (2)(4)(5)(6)
              CastIsUsedInLifetimeMarkers = true; // G: (1)(3)
            } else
              llvm_unreachable("Unexpected use of a cast on ADDR operand of "
                               "'QUAL.OMP.OPERAND.ADDR'.");
          } // For all users of CastI
          if (CastIsUsedInLifetimeMarkers)
            VAddrUsersInLifetimeMarkers.push_back(CastI);
        }   // For all users of VAddr

        // We must see a store to VAddr, unless VOrig has been optimized away
        // and replaced with an "undef" (e.g. rename_and_restore_undef.ll).
        assert((VAddrStore || isa<UndefValue>(VOrig)) &&
               "No store found to OPERAND.ADDR opnd.");

        if (VRenamed) {
          LLVM_DEBUG(dbgs() << "RestoreOperands: Replacing "
                               "loads from 'ADDR' ('";
                     VAddr->printAsOperand(dbgs()); dbgs() << "'): '";
                     VRenamed->printAsOperand(dbgs());
                     dbgs() << "', with the 'OPERAND': `";
                     VOrig->printAsOperand(dbgs()); dbgs() << "'.\n");
          IRBuilder<> Builder(VRenamed);

          Instruction *VOrigAsInst =
              IsSubObject ? SubObjHandler.getConstExprPtrAsInstruction(
                                VOrig) //                     I: (11)(12)
                          : nullptr;

          Value *VOrigToRestore = VOrigAsInst ? VOrigAsInst : VOrig;

          Value *VOrigCast = Builder.CreateBitCast(
              VOrigToRestore, VRenamed->getType(), VOrigToRestore->getName());

          VRenamed->replaceAllUsesWith(VOrigCast);
          VRenamed->eraseFromParent();
        } else
          LLVM_DEBUG(dbgs() << "RestoreOperands: No load from 'ADDR' ('";
                     VAddr->printAsOperand(dbgs());
                     dbgs() << "'). Deleting it.\n");

        if (VAddrStore)
          VAddrStore->eraseFromParent();

        for (auto *VAU : VAddrUsersInLifetimeMarkers) {
          if (VAU != VAddrLoadCast && VAU != VAddrStoreCast)
            VAU->eraseFromParent();
        }
        if (VAddrStoreCast)
          VAddrStoreCast->eraseFromParent();
        if (VAddrLoadCast && VAddrLoadCast != VAddrStoreCast)
          VAddrLoadCast->eraseFromParent();

        assert(VAddr->hasOneUse() && "Only one use of addr operand (in the "
                                     "directive) is expected after restoring.");
        VAddr->replaceAllUsesWith(UndefValue::get(VAddr->getType()));
        VAddr->eraseFromParent();

        CINeedsUpdates = true;
      } // For all Bundles on CI

      if (!CINeedsUpdates)
        continue;

      // First, update clauses on CI that have the SUBOBJ modifier.
      CI = SubObjHandler.updateSubObjClauseOperandsInDirective(); //  I:(13)(14)
      I = CI->getIterator();

      // Then add it to the list of directives that need OPND.ADDRs removed.
      DirectivesToRemoveOpndAddrsFrom.insert(CI);

      Changed = true;
    } // For all instructons in BB

  for (CallInst *CI : DirectivesToRemoveOpndAddrsFrom)
    VPOUtils::removeOperandBundlesFromCall(
        CI, {OperandAddrClauseString, OperandAddrSubObjClauseString});

  return Changed;
}

/// Remove the auxillary branch from begin to end directive, which is added in
/// vpo-paropt-prepare phase to prevent deletion of unreachable `end.region`
/// directives.
///
/// \code
///            Before                   |            After
///          ---------------------------+---------------------------
///   ENTRY_BB:                         |   ENTRY_BB:
///                                     |
///   %t = alloca i1                    |
///   %t.cast = cast %t to i8*          |
///   lifetime.begin(%t.cast)           |
///                                     |
///   %1 = begin_region[...             |   %1 = begin_region[...]
///                     "JUMP.IF"       |
///                     (i1* %t)]       |
///                                     |
///   %t.load = load volatile i1, i1* %t|
///   %cmp = icmp ne i1 %.load, false   |
///   br i1 %cmp, %END, %CONTINUE       |   br %CONTINUE
///                                     |
///   CONTINUE:                         |   CONTINUE:
///   ...                               |   ...
///                                     |
///   END:                              |   END:
///   end_region(%1)                    |   end_region(%1)
///                                     |
///   %t.cast1 = cast %t to i8*         |
///   lifetime.begin(%t.cast1)          |
///                                     |
/// \endcode
///
/// Note: The function assumes that the conditional branch to the
/// `END` block is the terminator instruction of the BasicBlock containing
/// `%t.load`.
bool VPOUtils::removeBranchesFromBeginToEndDirective(
    Function &F, const TargetLibraryInfo *TLI, DominatorTree *DT) {
  LLVM_DEBUG(dbgs() << "VPO Restore WRegions \n");

  bool Changed = false;
  std::unique_ptr<DomTreeUpdater> DTU;

  if (DT) {
    // Use unique_ptr just for automatic destruction at exit.
    DTU = std::make_unique<DomTreeUpdater>(
        *DT, DomTreeUpdater::UpdateStrategy::Lazy);
  }

  SmallPtrSet<CallInst *, 8> DirectivesToUpdate;

  StringRef ClauseString =
      VPOAnalysisUtils::getClauseString(QUAL_OMP_JUMP_TO_END_IF);

  for (Function::iterator B = F.begin(), BE = F.end(); B != BE; ++B)
    for (BasicBlock::iterator I = B->begin(), IE = B->end(); I != IE; ++I) {
      CallInst *CI = dyn_cast<CallInst>(&*I); //                          (%1)
      if (!CI || !VPOAnalysisUtils::isOpenMPDirective(CI))
        continue;

      if (CI->getNumOperandBundles() == 0)
        continue;

      for (unsigned I = 0; I < CI->getNumOperandBundles(); ++I) {
        OperandBundleUse BU = CI->getOperandBundleAt(I);
        if (BU.getTagName() != ClauseString)
          continue;

        assert(BU.Inputs.size() == 1 && "Expected only one operand in a "
                                        "'QUAL.OMP.JUMP.TO.END.IF' bundle.");
        AllocaInst *VAddr = cast_or_null<AllocaInst>(BU.Inputs[0]); //    (%t)

        assert(VAddr &&
               "Null/non-alloca address in 'QUAL.OMP.JUMP.TO.END.IF'.");

        LoadInst *VLoad = nullptr; //                                (%t.load)

        // Now, users of VAddr may include VLoad, CI, and any bitcast
        // instructions used for lifetime begin/end markers, that the inliner
        // may have inserted. We collect users that need to be deleted here.
        SmallVector<Instruction *, 4> VAddrUsersToDelete;
        auto collectLifetimeMarkerUsers = [&](User *U) -> bool {
          if (auto *II = dyn_cast<IntrinsicInst>(U)) {
            assert((II->getIntrinsicID() == Intrinsic::lifetime_start ||
                    II->getIntrinsicID() == Intrinsic::lifetime_end) &&
                   "Unexpected intrinsic using the QUAL.OMP.JUMP.TO.END.IF "
                   "operand (or a cast on it).");
            VAddrUsersToDelete.push_back(II);
            return true;
          }
          return false;
        };
        bool DeleteVAddr = true;

        for (User *U : VAddr->users()) {
          if (U == CI)
            continue;

          if (isa<CallInst>(U) &&
              VPOAnalysisUtils::isOpenMPDirective(cast<CallInst>(U))) {
            if (DirectivesToUpdate.find(cast<CallInst>(U)) ==
                DirectivesToUpdate.end())
              // Some other unhandled directive uses the same operand. Don't
              // delete VAddr this time.
              DeleteVAddr = false;
            continue;
          }

          if (auto *LI = dyn_cast<LoadInst>(U)) {
            if (!VLoad) {
              // Delete one branch in one go. So if we've already found one load
              // from VAddr, don't collect any more as there might be multiple
              // branches associated with multiple directives, using the same
              // VAddr.
              VLoad = LI;
              VAddrUsersToDelete.push_back(VLoad);
            }
          } else if (collectLifetimeMarkerUsers(U)) {
            continue;
          } else if (auto *CastI = dyn_cast<CastInst>(U)) {
            for (User *CastUser : CastI->users()) {
              if (collectLifetimeMarkerUsers(CastUser))
                continue;
              assert(false &&
                     "Unexpected cast on 'QUAL.OMP.JUMP.TO.END.IF' operand.");
              VAddrUsersToDelete.push_back(cast<Instruction>(CastUser));
            }
            VAddrUsersToDelete.push_back(CastI);
          } else
            llvm_unreachable(
                "Unexpected user of QUAL.OMP.JUMP.TO.END.IF operand.");
        }

        assert(VLoad &&
               "Failed to find a load from 'QUAL.OMP.JUMP.TO.END.IF' operand.");

        assert((VLoad->hasNUses(0) || VLoad->hasOneUse()) &&
               "Expected zero/one use of load from 'QUAL.OMP.JUMP.TO.END.IF' "
               "operand.");

        BasicBlock *BlockToSimplify = VLoad->getParent(); //        (ENTRY_BB)

        IRBuilder<> Builder(CI);
        VLoad->replaceAllUsesWith(Builder.getInt1(0));

        for (Instruction *VAU : VAddrUsersToDelete)
          VAU->eraseFromParent();

        if (DeleteVAddr) {
          VAddr->replaceAllUsesWith(UndefValue::get(VAddr->getType()));
          VAddr->eraseFromParent();
        }

        llvm::SimplifyInstructionsInBlock(BlockToSimplify);
        llvm::ConstantFoldTerminator(BlockToSimplify,
                                     /*DeleteDeadConditions=*/false,
                                     TLI, DTU.get());

        Changed = true;
        DirectivesToUpdate.insert(CI);
      } // For all Bundles on CI
    }   // For all instructons in BB

  for (CallInst *CI : DirectivesToUpdate)
    VPOUtils::removeOperandBundlesFromCall(CI, {ClauseString});

  return Changed;
}

INITIALIZE_PASS_BEGIN(VPORestoreOperands, "vpo-restore-operands",
                      "VPO Restore Operands Function Pass", false, false)
INITIALIZE_PASS_DEPENDENCY(LoopSimplify)
INITIALIZE_PASS_END(VPORestoreOperands, "vpo-restore-operands",
                    "VPO Paropt Prepare Function Pass", false, false)

char VPORestoreOperands::ID = 0;

VPORestoreOperands::VPORestoreOperands() : FunctionPass(ID) {
  initializeVPORestoreOperandsPass(*PassRegistry::getPassRegistry());
}

bool VPORestoreOperands::runOnFunction(Function &F) {
  if (VPOAnalysisUtils::skipFunctionForOpenmp(F) && skipFunction(F))
    return false;

  return Impl.runImpl(F);
}

bool VPORestoreOperandsPass::runImpl(Function &F) {
  return VPOUtils::restoreOperands(F);
}

void VPORestoreOperands::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
}

PreservedAnalyses VPORestoreOperandsPass::run(Function &F,
                                              FunctionAnalysisManager &AM) {
  bool Changed = runImpl(F);

  if (!Changed)
    return PreservedAnalyses::all();

  return PreservedAnalyses::none();
}

FunctionPass *llvm::createVPORestoreOperandsPass() {
  return new VPORestoreOperands();
}
#endif // INTEL_COLLAB
