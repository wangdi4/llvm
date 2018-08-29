// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#include "GenericAddressDynamicResolution.h"

#include <OCLPassSupport.h>
#include <NameMangleAPI.h>
#include <CompilationUtils.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/ValueMap.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <set>
#include <assert.h>


using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend::Passes::GenericAddressSpace;
using namespace Intel::OpenCL::DeviceBackend;

extern "C" {
  /// @brief  Creates new GenericAddressDynamicResolution module pass
  /// @returns new GenericAddressDynamicResolution module pass
  llvm::ModulePass *createGenericAddressDynamicResolutionPass() {
    return new intel::GenericAddressDynamicResolution();
  }
}

namespace intel {

  char GenericAddressDynamicResolution::ID = 0;

  OCL_INITIALIZE_PASS(GenericAddressDynamicResolution, "generic-addr-dynamic-resolution", "Resolves generic address space pointers to named ones", false, false)

  GenericAddressDynamicResolution::GenericAddressDynamicResolution() : ModulePass(ID) {
  }

  GenericAddressDynamicResolution::~GenericAddressDynamicResolution() {
  }

  bool GenericAddressDynamicResolution::runOnModule(Module &M) {
    bool changed = false;
    m_pModule = &M;
    m_functionsToHandle.clear();

    // Sort all functions in call-graph order
    sortFunctionsInCGOrder(m_pModule, m_functionsToHandle, false);

    // Iterate through functions sorted in the function list
    for (TFunctionList::iterator func_it = m_functionsToHandle.begin(),
                                 func_it_end = m_functionsToHandle.end();
                                 func_it != func_it_end; func_it++) {
      m_BiPoints.clear();
      m_IntrinPoints.clear();
      m_AddrQualifierBICalls.clear();

      Function *pFunc = *func_it;
      // Collect per-function GAS pointers
      analyzeGASPointers(pFunc);
      // Resolved usages of GAS pointers to 'named' address space
      changed |= resolveGASUsages(pFunc);
    }

    return changed;
  }

  void GenericAddressDynamicResolution::analyzeGASPointers(Function *pFunc) {

    // Collect calls with GAS pointers in the function (together with their immediate defs')
    for (inst_iterator inst_it = inst_begin(pFunc),
                       inst_it_end = inst_end(pFunc);
                       inst_it != inst_it_end; inst_it++) {

      Instruction *pInstr = &(*inst_it);

      // Filter-out all other instructions but calls
      CallInst *pCallInstr = dyn_cast<CallInst>(pInstr);
      if (!pCallInstr) {
        continue;
      }

      // Analyze call instruction
      const Function *pCallee = pCallInstr->getCalledFunction();
      assert(pCallee && "Call instruction doesn't have a callee!");
      if (isAddressQualifierBI(pCallee)) {
        // Address Qualifier BI function call - start BOTTOM-UP data flow
        // analysis in search for definition point of its (single) parameter
        const Value *pSrcVal = pCallInstr->getArgOperand(0);
        const PointerType *pSrcType = dyn_cast<PointerType>(pSrcVal->getType());
        if (!pSrcType || !IS_ADDR_SPACE_GENERIC(pSrcType->getAddressSpace())) {
          assert(0 && "Address Qualifier BI function must have GAS pointer as a parameter!");
        }
        m_AddrQualifierBICalls.push_back(pCallInstr);
      } else if (isGenericAddrBI(pCallee)) {
        // BI call accepting GAS pointer arguments(s) - if needed, schedule for conversion
        // to named-space parameters
        analyzeBIorIntrinsicCall(pCallInstr, CallBuiltIn);
      } else if (pCallee->isIntrinsic()) {
        // Intrinsic call - if needed, schedule for conversion to named-space parameters
        analyzeBIorIntrinsicCall(pCallInstr, CallIntrinsic);
      }
    }

  }

  void GenericAddressDynamicResolution::analyzeBIorIntrinsicCall(
      CallInst *pCallInstr, FuncCallType category) {

    assert((category == CallBuiltIn || category == CallIntrinsic) &&
           "Unexpected function category!");

    Function *pCalledFunc = pCallInstr->getCalledFunction();
    assert(pCalledFunc && "Unexpected indirect function invocation");
    StringRef funcName = pCalledFunc->getName();
    // Skip BIs which do not need processing
    if (needToSkipResolution(pCalledFunc)) return;

    // Address space enforcement: highest priority is 'global' for BI and
    // 'private' for intrinsic [we would like to have 'private' for all, however
    // not all BIs have  prototype with 'private']

    // If we resolve "wait_group_events" call we should resolve second argument(
    // event_t** ) to private because by the spec event_t may be only in private
    // address space qualifiers.
    OCLAddressSpace::spaces hiPriSpace =
        (category == CallBuiltIn &&
         !CompilationUtils::isWaitGroupEvents(funcName.str()))
            ? OCLAddressSpace::Global
            : OCLAddressSpace::Private;

    // Check for pointer arguments
    for (unsigned idx = 0; idx < pCallInstr->getNumArgOperands(); idx++) {
      if (const PointerType *pSrcType = dyn_cast<PointerType>(
              pCallInstr->getArgOperand(idx)->getType())) {
        // Check for pointer address space
        if (IS_ADDR_SPACE_GENERIC(pSrcType->getAddressSpace())) {
          // Set default to the highest priority target space
          if (category == CallBuiltIn) {
            m_BiPoints.push_back(TBiIntrinPair(pCallInstr, hiPriSpace));
          } else {
            m_IntrinPoints.push_back(TBiIntrinPair(pCallInstr, hiPriSpace));
          }
          break;
        }
      }
    }
  }

  bool GenericAddressDynamicResolution::resolveGASUsages(Function *pFunc) {

    bool changed = false;

    // Resolve Calls to Addr Space Qualifier BIs
    for (unsigned idx = 0; idx < m_AddrQualifierBICalls.size(); idx++) {
      // Inlining the function call to instructions which calculate the function result
      resolveAddrSpaceQualifierBICall(m_AddrQualifierBICalls[idx]);
      changed = true;
    }
    // Resolve BI function calls with GAS pointers
    for (unsigned idx = 0; idx < m_BiPoints.size(); idx++) {
     // Enforcing named address space to the function call
      resolveBIorIntrinsicCall(m_BiPoints[idx].first, CallBuiltIn, m_BiPoints[idx].second);
      changed = true;
    }
    // Resolve Intrinsic function calls with GAS pointers
    for (unsigned idx = 0; idx < m_IntrinPoints.size(); idx++) {
      // Enforcing named address space to the function call
      resolveBIorIntrinsicCall(m_IntrinPoints[idx].first, CallIntrinsic, m_IntrinPoints[idx].second);
      changed = true;
    }

    return changed;
  }

} // namespace intel
