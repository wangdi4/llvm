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

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "TransformLibraryCalls"

static LoopVPlanDumpControl
    TransformLibraryCallsDumpsControl("transformed-library-calls",
                                      "transforming library calls");

void VPTransformLibraryCalls::transform() {
  transformSincosCalls();

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
  //   %sin.value = soa-extract-value {double, double} %res, 0
  //   store double %sin.value double* %sin.out
  //   %cos.value = soa-extract-value {double, double} %res, 1
  //   store double %cos.value double* %cos.out
  //
  for (auto &SincosCall : Calls) {
    LLVM_DEBUG(dbgs() << "Transforming call:\n" << *SincosCall << '\n';);

    Builder.setInsertPoint(SincosCall);
    Builder.setCurrentDebugLocation(SincosCall->getDebugLocation());

    // First, insert a transform-lib-call with the new (transformed) return type
    // and arguments.
    Type *ArgTy = SincosCall->getArgOperand(0)->getType();

    auto *RetTy = StructType::create({ArgTy, ArgTy}, ".vplan.sincos",
                                     /*isPacked*/ false);
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
      DA.markDivergent(*Value);
      ReturnValues.push_back(Value);
    }

    // Lastly, store the return values back to their respective storage (the
    // 'sin.out' and 'cos.out' arg ptrs to the original 'sincos' call).
    for (auto It : enumerate(ReturnValues)) {
      auto *Store = Builder.createStore(
          It.value(), SincosCall->getArgOperand(It.index() + 1));
      Store->setAlignment(Plan.getDataLayout()->getPrefTypeAlign(ArgTy));
      DA.markDivergent(*Store);
    }

    SincosCall->getParent()->eraseInstruction(SincosCall);
  }
}
