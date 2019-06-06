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

#include "llvm/IR/Function.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Transforms/VPO/Paropt/VPOParoptUtils.h"
#include "llvm/Transforms/VPO/Utils/VPORestoreOperands.h"
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

/// Restore the clause operands by undoing the renaming done in the prepare
/// pass. It looks at all OpenMP directive intrinsics for
/// `QUAL.OMP.OPERAND.ADDR(opnd, addr)` pairs, and:
/// * Replaces uses of the load (can be zero or one) from `addr` with `opnd`.
/// * Deletes any load/store/bitcasts/zero-offset GEPs on `addr`.
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
// Lit tests for some of the above cases are:
// A: target_map_global_array.ll,rename_and_restore_nested.ll
// B: target_map_gep_fence2.ll
// C: rename_and_restore_struct_castoutside.ll
// D: rename_and_restore_int_addrcast.ll,rename_and_restore_struct_castinside.ll
// E: rename_and_restore_struct_castsame.ll
// F: rename_and_restore_struct_castboth.ll
bool VPOUtils::restoreOperands(Function &F) {
  LLVM_DEBUG(dbgs() << "VPO Restore Operands \n");

  bool Changed = false;

  SmallPtrSet<CallInst *, 8> DirectivesToUpdate;

  StringRef OperandAddrClauseString =
      VPOAnalysisUtils::getClauseString(QUAL_OMP_OPERAND_ADDR);

  for (Function::iterator B = F.begin(), BE = F.end(); B != BE; ++B)
    for (BasicBlock::iterator I = B->begin(), IE = B->end(); I != IE; ++I) {
      CallInst *CI = dyn_cast<CallInst>(&*I);
      if (!CI || !VPOAnalysisUtils::isOpenMPDirective(CI))
        continue;

      if (CI->getNumOperandBundles() == 0)
        continue;

      for (unsigned I = 0; I < CI->getNumOperandBundles(); ++I) {
        OperandBundleUse BU = CI->getOperandBundleAt(I);
        if (BU.getTagName() != OperandAddrClauseString)
          continue;

        assert(BU.Inputs.size() == 2 && "Expected an operand-address pair in a "
                                        "'QUAL.OMP.OPERAND.ADDR' bundle.");
        Value *VOrig = BU.Inputs[0];
        AllocaInst *VAddr = cast_or_null<AllocaInst>(BU.Inputs[1]);

        assert(VOrig && "Null operand in 'QUAL.OMP.OPERAND.ADDR'.");
        assert(VAddr && "Null/non-alloca address in 'QUAL.OMP.OPERAND.ADDR'.");

        LoadInst *VRenamed = nullptr;
        StoreInst *VAddrStore = nullptr;
        Instruction *VAddrStoreCast = nullptr;
        Instruction *VAddrLoadCast = nullptr;
        assert((VAddr->hasNUses(3) ||  // store, directive and load
                VAddr->hasNUses(2)) && // store and directive (no load)
               "Unexpected number of users of QUAL.OMP.OPERAND.ADDR operand.");

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

          assert(VPOUtils::isPointerCastOrZeroOffsetGEP(U) &&
                 "Unexpected use of ADDR operand of 'QUAL.OMP.OPERAND.ADDR'.");
          // Reaching here means that U is a pointer cast / zero-offset GEP.

          assert((U->hasOneUse() || U->hasNUses(2)) &&
                 "Unexpected number of uses of a bitcast on operand addr.");
          Instruction *CastI = cast<Instruction>(U);

          for (User *CastUser : CastI->users()) {
            if (isa<LoadInst>(CastUser)) {
              VRenamed = cast<LoadInst>(CastUser); // D, E, F: %y1
              VAddrLoadCast = CastI; // D, E: %y.addr.cast; F: %y.addr.cast1
            } else if (auto *CastUserStore = dyn_cast<StoreInst>(CastUser)) {
              assert(CastUserStore->getPointerOperand() == CastI &&
                     "QUAL.OMP.OPERAND.ADDR addr operand is stored somewhere.");
              VAddrStoreCast = CastI;     // C, E, F: %y.addr.cast
              VAddrStore = CastUserStore; // C, E, F: store %y, %y.addr.cast
            } else
              llvm_unreachable("Unexpected use of a cast on ADDR operand of "
                               "'QUAL.OMP.OPERAND.ADDR'.");
          } // For all users of CastI
        }   // For all users of VAddr

        assert(VAddrStore && "No store found to OPERAND.ADDR opnd.");

        if (VRenamed) {
          LLVM_DEBUG(dbgs() << "RestoreOperands: Replacing "
                               "loads from 'ADDR' ('";
                     VAddr->printAsOperand(dbgs()); dbgs() << "'): '";
                     VRenamed->printAsOperand(dbgs());
                     dbgs() << "', with the 'OPERAND': `";
                     VOrig->printAsOperand(dbgs()); dbgs() << "'.\n");
          IRBuilder<> Builder(VRenamed);
          Value *VOrigCast = Builder.CreateBitCast(VOrig, VRenamed->getType(),
                                                   VOrig->getName());
          VRenamed->replaceAllUsesWith(VOrigCast);
          VRenamed->eraseFromParent();
        } else
          LLVM_DEBUG(dbgs() << "RestoreOperands: No load from 'ADDR' ('";
                     VAddr->printAsOperand(dbgs());
                     dbgs() << "'). Deleting it.\n");

        VAddrStore->eraseFromParent();

        if (VAddrStoreCast)
          VAddrStoreCast->eraseFromParent();
        if (VAddrLoadCast && VAddrLoadCast != VAddrStoreCast)
          VAddrLoadCast->eraseFromParent();

        assert(VAddr->hasOneUse() && "Only one use of addr operand (in the "
                                     "directive) is expected after restoring.");
        VAddr->replaceAllUsesWith(UndefValue::get(VAddr->getType()));
        VAddr->eraseFromParent();

        Changed = true;
        DirectivesToUpdate.insert(CI);
      } // For all Bundles on CI
    }   // For all instructons in BB

  for (CallInst *CI : DirectivesToUpdate)
    VPOParoptUtils::removeOperandBundlesFromCall(CI, {OperandAddrClauseString});

  return Changed;
}

INITIALIZE_PASS_BEGIN(VPORestoreOperands, "vpo-restore-operands",
                      "VPO Restore Operands Function Pass", false, false)
INITIALIZE_PASS_DEPENDENCY(LoopSimplify)
INITIALIZE_PASS_END(VPORestoreOperands, "vpo-restore-operands",
                    "VPO Paropt Prepare Function Pass", false, false)

char VPORestoreOperands::ID = 0;

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
