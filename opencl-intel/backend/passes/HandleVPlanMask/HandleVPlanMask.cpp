// INTEL CONFIDENTIAL
//
// Copyright 2012-2020 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "HandleVPlanMask.h"

#include "CompilationUtils.h"
#include "OCLPassSupport.h"

#include <llvm/ADT/SmallSet.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>

using namespace llvm;

namespace intel {

using namespace Intel::OpenCL::DeviceBackend;

char HandleVPlanMask::ID = 0;

OCL_INITIALIZE_PASS(
    HandleVPlanMask, "convert-vplan-mask",
    "HandleVPlanMask pass - convert vplan style mask to volcano style", false,
    false)

bool HandleVPlanMask::runOnModule(Module &M) {

  Type *Int32Type = IntegerType::get(M.getContext(), 32);
  SmallSet<Function *, 4> FuncToBeRemoved;
  SmallSet<Function *, 8> FuncNeedRemoveAttrs;

  for (auto &F : M) {
    if (!F.isDeclaration() || (F.arg_size() < 1) || F.user_empty())
      continue;

    // All users of built-in functions should be CallInst.
    CallInst *Call = dyn_cast<CallInst>(*F.user_begin());
    if (!Call)
      continue;

    // Check "has_vplan_mask" and "call_params_num" attr to decide
    // whether the call is masked.
    bool HasVPlanMask = false;
    HasVPlanMask |= Call->hasFnAttr(CompilationUtils::ATTR_HAS_VPLAN_MASK);
    if (!HasVPlanMask) {
      if (Call->hasFnAttr("call-params-num")) {
        FuncNeedRemoveAttrs.insert(&F);
        const Attribute &Attr = Call->getFnAttr("call-params-num");
        StringRef AttrStr = Attr.getValueAsString();
        int ArgSize;
        bool status = AttrStr.getAsInteger<int>(10, ArgSize);
        (void)status;
        assert(!status && "Unexpected call-params-num attribute");
        HasVPlanMask |= (Call->arg_size() - 1 == (unsigned)ArgSize);
      }
    } else {
      FuncNeedRemoveAttrs.insert(&F);
    }
    if (!HasVPlanMask)
      continue;

    // Check the type of mask parameter.
    FunctionType *FnType = F.getFunctionType();
    unsigned LastArgIdx = F.arg_size() - 1;
    auto *MaskType = cast<VectorType>(FnType->getParamType(LastArgIdx));
    unsigned VF = MaskType->getNumElements();
    auto *ExpectMaskType = VectorType::get(Int32Type, VF);
    auto *MaskElementType = dyn_cast<IntegerType>(MaskType->getElementType());

    // VPlan uses characteristic data type as the element type of mask arg.
    // So the element type may be non-int.
    VectorType *IntMaskType = nullptr;
    if (!MaskElementType) {
      IntMaskType = VectorType::getInteger(MaskType);
      MaskElementType = cast<IntegerType>(IntMaskType->getElementType());
    }
    unsigned ElemBitWidth = MaskElementType->getBitWidth();
    // The mask type is already <VF x i32>, skip this function.
    if (ElemBitWidth == 32 && !IntMaskType)
      continue;

    // Create a new function with expected mask type.
    StringRef FnName = F.getName();
    F.setName(FnName + "_before");
    SmallVector<Type *, 4> NewParams;
    for (unsigned idx = 0; idx < LastArgIdx; ++idx)
      NewParams.push_back(FnType->getParamType(idx));
    NewParams.push_back(ExpectMaskType);
    auto *NewFnType = FunctionType::get(FnType->getReturnType(), NewParams,
                                        FnType->isVarArg());
    auto *NewFn = Function::Create(NewFnType, F.getLinkage(), FnName, M);
    NewFn->setAttributes(F.getAttributes());
    FuncToBeRemoved.insert(&F);
    FuncNeedRemoveAttrs.erase(&F);
    FuncNeedRemoveAttrs.insert(NewFn);

    // Generate new mask arg for all calls.
    DenseMap<CallInst *, Value *> NewMaskForCalls;
    for (auto *User : F.users()) {
      Call = dyn_cast<CallInst>(User);
      assert(Call && "Unexpected use of OpenCL Builtins");
      Value *MaskArg = Call->getArgOperand(LastArgIdx);
      // Cast the mask arg to int vector type;
      if (IntMaskType)
        MaskArg = new BitCastInst(MaskArg, IntMaskType, "mask.cast.i.", Call);

      // Generate new mask arg.
      // For i8, i16, i64: signed extend or trunc the mask arg to <VF x i32>
      // type.
      // For i32: bitcasted value is ok.
      Value *NewMask = nullptr;
      switch (ElemBitWidth) {
      case 8:
      case 16:
        NewMask = new SExtInst(MaskArg, ExpectMaskType, "mask.i32.", Call);
        break;
      case 32:
        NewMask = MaskArg;
        break;
      case 64:
        NewMask = new TruncInst(MaskArg, ExpectMaskType, "mask.i32.", Call);
        break;
      default:
        llvm_unreachable("Unexpectd mask argument type");
      }
      NewMaskForCalls[Call] = NewMask;
    }

    // Set called function and mask operand.
    for (auto &MaskMap : NewMaskForCalls) {
      (MaskMap.first)->setCalledFunction(NewFn);
      (MaskMap.first)->setArgOperand(LastArgIdx, MaskMap.second);
    }
  }

  // Remove "has-vplan-mask" and "call-params-num" attrs.
  for (auto *Func : FuncNeedRemoveAttrs) {
    for (auto *User : Func->users()) {
      auto *Call = dyn_cast<CallInst>(User);
      assert(Call && "Unexpected use of OpenCL Built-ins");
      Call->removeAttribute(AttributeList::FunctionIndex,
                            CompilationUtils::ATTR_HAS_VPLAN_MASK);
      Call->removeAttribute(AttributeList::FunctionIndex, "call-params-num");
    }
  }

  for (auto *Func : FuncToBeRemoved) {
    assert(Func->user_empty() && "Unexpected use of OpenCL Built-ins");
    Func->eraseFromParent();
  }
  return !(FuncToBeRemoved.empty() && FuncNeedRemoveAttrs.empty());
}
} // namespace intel

extern "C" llvm::ModulePass *createHandleVPlanMaskPass() {
  return new intel::HandleVPlanMask();
}
