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

#include "LocalBuffAnalysis.h"
#include "CompilationUtils.h"
#include "InitializePasses.h"
#include "OCLPassSupport.h"
#include "common_dev_limits.h"

#include "llvm/IR/Constant.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

using namespace DPCPPKernelMetadataAPI;
using namespace Intel::OpenCL::DeviceBackend;

namespace intel{

  /// @brief Creates new LocalBuffAnalysis module pass
  /// @returns new LocalBuffAnalysis module pass
  ModulePass* createLocalBuffersAnalysisPass() {
    return new LocalBuffAnalysis();
  }

  char LocalBuffAnalysis::ID = 0;

  OCL_INITIALIZE_PASS(LocalBuffAnalysis, "LocalBuffAnalysis", "LocalBuffAnalysis provides local values analysis info", false, false)

  bool LocalBuffAnalysis::runOnModule(Module &M) {

    m_pModule = &M;
    m_localUsageMap.clear();
    m_localSizeMap.clear();
    m_directLocalSizeMap.clear();

    // Initialize localUsageMap
    updateDirectLocals(M);

    for ( Module::iterator fi = M.begin(), fe = M.end(); fi != fe; ++fi ) {
      auto FMD = FunctionMetadataAPI(&*fi);
      bool isRecursive =
          FMD.RecursiveCall.hasValue() && FMD.RecursiveCall.get();
      unsigned maxDepth = isRecursive ? MAX_RECURSION_DEPTH : UINT_MAX;

      // Update localSizeMap
      calculateLocalsSize(&*fi, maxDepth);
    }

    return false;
  }

  void LocalBuffAnalysis::updateLocalsMap(GlobalValue* pLocalVal, User * user) {
    // llvm::Instruction, llvm::Operator and llvm::Constant are the only possible subtypes of llvm::user
    if ( isa<Instruction>(user) ) {
      Instruction *inst = cast<Instruction>(user);

      // declaring variables for debugging purposes shouldn't affect local buffers.
      if (MDNode *mdn = inst->getMetadata("dbg_declare_inst")) {
        if (mdconst::extract<ConstantInt>(mdn->getOperand(0))->isAllOnesValue()) {
            return;
        }
      }
      Function *pFunc = inst->getFunction();
      // Add pLocalVal to the set of local values used by pFunc
      m_localUsageMap[pFunc].insert(pLocalVal);
    } else if ( isa<Constant>(user) ) {
      // Recursievly locate all users of the constant value
      for ( Value::user_iterator it = user->user_begin(); it != user->user_end(); ++it ) {
        updateLocalsMap(pLocalVal, *it);
      }
    }  else {
      // llvm::Operator is an internal llvm class, so we do not expect it to be a user of GlobalValue
      llvm_unreachable("Unexpected user type");
    }
  }

  void LocalBuffAnalysis::updateDirectLocals(Module &M) {
    // Get a list of all the global values in the module
    Module::GlobalListType& lstGlobals = M.getGlobalList();

    // Find globals that appear in the origin kernel as local variables and add update mapping accordingly
    for ( Module::GlobalListType::iterator it = lstGlobals.begin(), e = lstGlobals.end(); it != e; ++it ) {
      GlobalValue* pVal = &*it;

      const PointerType* TP = cast<PointerType>(pVal->getType());
      if(TP->getAddressSpace() != CompilationUtils::LOCL_VALUE_ADDRESS_SPACE) {
        // LOCL_VALUE_ADDRESS_SPACE = '3' is a magic number for global variables that were in origin local kernel variable!
        continue;
      }

      // If we reached here, then pVal is a global value that was originally a local value
      for ( GlobalValue::user_iterator ui = pVal->user_begin(), ue = pVal->user_end(); ui != ue; ++ui ) {
        updateLocalsMap(pVal, *ui);
      }
    } // Find globals done
  }

  size_t LocalBuffAnalysis::calculateLocalsSize(Function *pFunc,
                                                unsigned maxDepth) {
    --maxDepth;

    if (!pFunc || pFunc->isDeclaration() || !maxDepth) {
      // Not module function, no need for local buffer, return size zero
      return 0;
    }

    if ( m_localSizeMap.count(pFunc) ) {
      // local size of this function already calculated
      return m_localSizeMap[pFunc];
    }

    llvm::DataLayout DL(m_pModule);
    size_t localBufferSize = 0;

    for ( TUsedLocals::iterator li = m_localUsageMap[pFunc].begin(),
      le = m_localUsageMap[pFunc].end(); li != le; ++li ) {

        GlobalValue *pLclBuff = dyn_cast<GlobalValue>(*li);
        assert( pLclBuff && "locals container contains something other than GlobalValue!" );

        // Calculate required buffer size
        size_t uiArraySize = DL.getTypeAllocSize(pLclBuff->getType()->getElementType());
        assert ( 0 != uiArraySize && "local buffer size is zero!" );

        // Advance total implicit size
        localBufferSize += ADJUST_SIZE_TO_MAXIMUM_ALIGN(uiArraySize);
    }

    // Update direct local size of this function
    m_directLocalSizeMap[pFunc] = localBufferSize;

    size_t extraLocalBufferSize = 0;
    //look for calls to other kernels
    for ( inst_iterator ii = inst_begin(pFunc), ie = inst_end(pFunc); ii != ie; ++ii ) {
      CallInst *pCall = dyn_cast<CallInst>(&*ii);
      if ( !pCall ) {
        continue;
      }
      // Call instruction

      size_t callLocalSize =
          calculateLocalsSize(pCall->getCalledFunction(), maxDepth);
      if ( extraLocalBufferSize < callLocalSize ) {
        // Found Function that needs more local size,
        // update max extraLocalBufferSize
        extraLocalBufferSize = callLocalSize;
      }
    }

    localBufferSize += extraLocalBufferSize;

    // Update the local size of this function
    m_localSizeMap[pFunc] = localBufferSize;
    return localBufferSize;
  }
}

