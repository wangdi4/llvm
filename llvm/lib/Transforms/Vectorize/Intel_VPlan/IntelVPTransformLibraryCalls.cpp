//===- IntelVPTransformLibraryCalls.cpp --------------------------------------===//
//
//   Copyright (C) 2022 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===---------------------------------------------------------------------===//
///
/// \file
/// This file implements the VPTransformLibraryCalls class.
//===---------------------------------------------------------------------===//
#include "IntelVPTransformLibraryCalls.h"
#include "HIR/IntelVPlanInstructionDataHIR.h"
#include "IntelVPlan.h"
#include "IntelVPlanUtils.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "TransformLibraryCalls"

static LoopVPlanDumpControl
    TransformLibraryCallsDumpsControl("transformed-library-calls",
                                      "transforming library calls");

void VPTransformLibraryCalls::transform() {
  transformSincosCalls();
  transformCallsWithArgRepacking();

  VPLAN_DUMP(TransformLibraryCallsDumpsControl, Plan);
}

void VPTransformLibraryCalls::transformSincosCalls() {
  LLVM_DEBUG(
      dbgs() << "(" << Plan.getName()
             << ") Checking for library calls to 'sincos' be transformed...\n");

  // First, collect calls to be transformed (calls with library
  // vectorization scenario and whose LibFunc is 'sincos')
  auto IsSincosLibCall = [this](const VPInstruction &Inst) {
    auto *Call = dyn_cast<VPCallInstruction>(&Inst);
    if (!Call || !Call->getCalledFunction())
      return false;

    if (Call->getVectorizationScenario() !=
        VPCallInstruction::CallVecScenariosTy::LibraryFunc) {
      LLVM_DEBUG(dbgs() << "Not transforming call to "
                        << Call->getCalledFunction()->getName()
                        << ": not library vectorization scenario\n");
      return false;
    }

    LibFunc CallF;
    if (!TLI.getLibFunc(*Call->getCalledFunction(), CallF)) {
      LLVM_DEBUG(dbgs() << "Not transforming call to "
                        << Call->getCalledFunction()->getName()
                        << ": not in TLI\n");
      return false;
    }

    return CallF == LibFunc_sincos || CallF == LibFunc_sincosf;
  };
  SmallVector<VPCallInstruction *, 2> Calls(
      map_range(make_filter_range(vpinstructions(&Plan), IsSincosLibCall),
                [](VPInstruction &I) { return cast<VPCallInstruction>(&I); }));

  // Then replace 'sincos' calls with a transformed library call representing
  // the vectorized call, and add post-processing to handle the signature
  // mis-match. For example,
  //
  //   call double %x double* %sin.out double* %cos.out @sincos(double, double*, double*)
  //
  // ==>
  //
  //   {double, double} %res = transform-library-call double %x @__svml_sincos
  //   %sincos.sin = soa-extract-value {double, double} %res, 0
  //   %sincos.cos = soa-extract-value {double, double} %res, 1
  //   store double %sincos.sin double* %sin.out
  //   store double %sincos.cos double* %cos.out
  //
  for (auto &SincosCall : Calls) {
    LLVM_DEBUG(dbgs() << "Transforming call:\n" << *SincosCall << '\n';);

    Builder.setInsertPoint(SincosCall);
    Builder.setCurrentDebugLocation(SincosCall->getDebugLocation());

    // First, insert a transform-lib-call with the new (transformed) return type
    // and arguments.
    Type *ArgTy = SincosCall->getArgOperand(0)->getType();

    auto *RetTy = StructType::create({ArgTy, ArgTy}, ".vplan.sincos");
    auto *TransformedFnTy =
        FunctionType::get(RetTy, {ArgTy}, /*IsVarArg=*/false);
    auto *TransformedCall = Builder.create<VPTransformLibraryCall>(
        "transformed", *SincosCall, TransformedFnTy,
        SincosCall->getArgOperand(0));
    DA.markDivergent(*TransformedCall);

    // Then, extract the two return values from the aggregate returned from the
    // transformed call, using SOAExtractValue since SVML sincos returns an
    // aggregate of vectors (SOA).
    SmallVector<VPValue *, 2> ReturnValues;
    for (unsigned Idx : {0, 1}) {
      auto *IdxTy = Type::getInt64Ty(*Plan.getLLVMContext());
      auto *IdxValue = Plan.getVPConstant(ConstantInt::get(IdxTy, Idx));
      auto *Value = Builder.createNaryOp(VPInstruction::SOAExtractValue, ArgTy,
                                         {TransformedCall, IdxValue});
      Value->setName(Idx == 0 ? "sincos.sin" : "sincos.cos");
      DA.markDivergent(*Value);
      ReturnValues.push_back(Value);
    }

    // Lastly, store the return values back to their respective storage (the
    // 'sin.out' and 'cos.out' arg ptrs to the original 'sincos' call).
    for (const auto &It : enumerate(ReturnValues)) {
      auto *ArgValue = SincosCall->getArgOperand(It.index() + 1);
      auto *Store = Builder.createStore(It.value(), ArgValue);
      Store->setAlignment(Plan.getDataLayout()->getPrefTypeAlign(ArgTy));
      DA.markDivergent(*Store);

      if (auto *Subscript = dyn_cast<VPSubscriptInst>(ArgValue)) {
        // In the HIR path, our operands should always be subscript insts.
        Store->HIR().setGepRefSpecifics(*Subscript);
      }
    }

    SincosCall->getParent()->eraseInstruction(SincosCall);
  }
}

void VPTransformLibraryCalls::transformCallsWithArgRepacking() {
  LLVM_DEBUG(dbgs() << "(" << Plan.getName()
                    << ") Checking for library calls that need argument "
                       "repacking transform...\n");

  // First, collect calls to be transformed i.e. calls with library
  // vectorization scenario and has 'NeedsArgRepacking' attribute set in its
  // VecDesc.
  auto CallNeedsArgRepacking = [this](const VPInstruction &Inst) {
    auto *Call = dyn_cast<VPCallInstruction>(&Inst);
    if (!Call || !Call->getCalledFunction())
      return false;

    StringRef FnName = Call->getCalledFunction()->getName();
    if (Call->getVectorizationScenario() !=
        VPCallInstruction::CallVecScenariosTy::LibraryFunc) {
      LLVM_DEBUG(dbgs() << "Not transforming call to " << FnName
                        << ": not library vectorization scenario\n");
      return false;
    }

    return TLI.doesVectorFuncNeedArgRepacking(FnName);
  };
  SmallVector<VPCallInstruction *, 2> Calls(
      map_range(make_filter_range(vpinstructions(&Plan), CallNeedsArgRepacking),
                [](VPInstruction &I) { return cast<VPCallInstruction>(&I); }));

  // Then replace the scalar calls with a transformed library call representing
  // the vectorized call, and add needed pre/post-processing to handle the
  // signature mis-match. For example,
  //
  //   {double, double} %res = call double %arg.real double %arg.imag ptr @cexp
  //   double %res.real = extractvalue {double, double} %res, 0
  //   double %res.imag = extractvalue {double, double} %res, 1
  //
  // ==>
  //   %arg.0 = insertelement <2 x double> poison, double %arg.real, i64 0
  //   %arg.1 = insertelement <2 x double> %arg.0, double %arg.imag, i64 1
  //   <2 x double> %res = transform-library-call %arg.1 @__svml_cexp
  //   double %res.real = extractelement <double, double> %res, 0
  //   double %res.imag = extractelement <double, double> %res, 1
  //
  for (auto &ArgRepackCall : Calls) {
    LLVM_DEBUG(dbgs() << "Transforming call:\n" << *ArgRepackCall << "\n");

    Builder.setInsertPoint(ArgRepackCall);
    Builder.setCurrentDebugLocation(ArgRepackCall->getDebugLocation());

    // Compute type of the transformed library call's argument as -
    // ArgTy = <NumArgs x OrigArgTy>
    Type *OrigArgTy = ArgRepackCall->getArgOperand(0)->getType();
    assert(llvm::all_of(ArgRepackCall->arg_operands(),
                        [OrigArgTy](VPValue *Op) {
                          return Op->getType() == OrigArgTy;
                        }) &&
           "All arguments must be same type for arg repacking.");
    unsigned OrigNumArgs = ArgRepackCall->getNumArgOperands();
    Type *NewArgTy = FixedVectorType::get(OrigArgTy, OrigNumArgs);

    // Construct sequence of insertelements to pack all arguments into a single
    // vector register -
    // %vp0 = insertelement NewArgTy poison, %call.op0, 0
    // %vp1 = insertelement NewArgTy %vp0, %call.op1, 1
    // ... (number of args times)
    VPValue *NewArg = Plan.getPoison(NewArgTy);
    for (unsigned I = 0; I < OrigNumArgs; ++I) {
      auto *IdxTy = Type::getInt64Ty(*Plan.getLLVMContext());
      auto *Idx = Plan.getVPConstant(ConstantInt::get(IdxTy, I));
      NewArg =
          Builder.createNaryOp(Instruction::InsertElement, NewArgTy,
                               {NewArg, ArgRepackCall->getArgOperand(I), Idx},
                               nullptr /*UnderlyingI*/, "cwar.arg." + Twine(I));
    }

    // Insert a transform-lib-call with the new (transformed) return type and
    // arguments.
    auto *TransformedFnTy =
        FunctionType::get(NewArgTy, {NewArgTy}, false /*IsVarArg*/);
    auto *TransformedCall = Builder.create<VPTransformLibraryCall>(
        "transformed", *ArgRepackCall, TransformedFnTy, NewArg);
    DA.markDivergent(*TransformedCall);

    // Replace all extractvalue users of the original call with equivalent
    // extractelement operating now on the transformed call instead.
    SmallVector<VPInstruction *, 2> ExtractValUsersToRemove;
    for (auto *U : ArgRepackCall->users()) {
      auto *ExtractVal = cast<VPInsertExtractValue>(U);
      assert(ExtractVal->getOpcode() == Instruction::ExtractValue &&
             "Call that needs argument repacking expected to be used by "
             "extractvalue instructions only.");

      // Obtain index of extractvalue instruction.
      assert(ExtractVal->getNumIndices() == 1 &&
             "extractvalue expected to have only one index.");
      SmallVector<unsigned, 1> ExtractIdxs(ExtractVal->getIndices());

      // Generate equivalent extractelement instruction using transformed call
      // and index.
      auto *IdxTy = Type::getInt64Ty(*Plan.getLLVMContext());
      auto *Idx = Plan.getVPConstant(ConstantInt::get(IdxTy, ExtractIdxs[0]));
      VPInstruction *ExtractElem = Builder.createNaryOp(
          Instruction::ExtractElement, OrigArgTy, {TransformedCall, Idx});
      assert(ExtractElem->getType() == ExtractVal->getType() &&
             "extractvalue and extractelement expected to produce same types.");
      // Replace all uses of the extractvalue and mark it for removal from CFG.
      ExtractVal->replaceAllUsesWith(ExtractElem);
      ExtractValUsersToRemove.push_back(ExtractVal);
    }

    // Erase original call and all extractvalue users of the call.
    for (auto *I : ExtractValUsersToRemove)
      I->getParent()->eraseInstruction(I);
    ArgRepackCall->getParent()->eraseInstruction(ArgRepackCall);
  }
}
