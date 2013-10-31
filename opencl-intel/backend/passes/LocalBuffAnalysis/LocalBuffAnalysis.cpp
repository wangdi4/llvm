/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "LocalBuffAnalysis.h"
#include "CompilationUtils.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"
#include "common_dev_limits.h"

#include "llvm/Instructions.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Constant.h"
#include "llvm/Version.h"
#if LLVM_VERSION == 3425
#include "llvm/Target/TargetData.h"
#else
#include "llvm/DataLayout.h"
#endif

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

    // Update localSizeMap
    for ( Module::iterator fi = M.begin(), fe = M.end(); fi != fe; ++fi ) {
      calculateLocalsSize(&*fi);
    }

    return false;
  }

  void LocalBuffAnalysis::updateLocalsMap(GlobalValue* pLocalVal, User * user) {
    // llvm::Instruction, llvm::Operator and llvm::Constant are the only possible subtypes of llvm::user
    if ( isa<Instruction>(user) ) {
      Instruction *inst = cast<Instruction>(user);

      // declaring variables for debugging purposes shouldn't affect local buffers.
      if (MDNode *mdn = inst->getMetadata("dbg_declare_inst")) {
        if (cast<ConstantInt>(mdn->getOperand(0))->isAllOnesValue()) {
            return;
        }
      }
      // Parent of Instruction is BasicBlock
      // Parent of BasicBlock is Function
      Function* pFunc = inst->getParent()->getParent();
      // Add pLocalVal to the set of local values used by pFunc
      m_localUsageMap[pFunc].insert(pLocalVal);
    } else if ( isa<Constant>(user) ) {
      Value::use_iterator it;
      // Recursievly locate all users of the constant value
      for ( it = user->use_begin(); it != user->use_end(); ++it ) {
        updateLocalsMap(pLocalVal, *it);
      }
    }  else {
      // llvm::Operator is an internal llvm class, so we do not expect it to be a user of GlobalValue
      assert("Unexpected user type");
    }
  }

  void LocalBuffAnalysis::updateDirectLocals(Module &M) {
    // Get a list of all the global values in the module
    Module::GlobalListType& lstGlobals = M.getGlobalList();

    // Find globals that appear in the origin kernel as local variables and add update mapping accordingly
    for ( Module::GlobalListType::iterator it = lstGlobals.begin(), e = lstGlobals.end(); it != e; ++it ) {
      GlobalValue* pVal = it;

      const PointerType* TP = cast<PointerType>(pVal->getType());
      if(TP->getAddressSpace() != CompilationUtils::LOCL_VALUE_ADDRESS_SPACE) {
        // LOCL_VALUE_ADDRESS_SPACE = '3' is a magic number for global variables that were in origin local kernel variable!
        continue;
      }

      // If we reached here, then pVal is a global value that was originally a local value
      for ( GlobalValue::use_iterator ui = pVal->use_begin(), ue = pVal->use_end(); ui != ue; ++ui ) {
        updateLocalsMap(pVal, *ui);
      }
    } // Find globals done
  }

  size_t LocalBuffAnalysis::calculateLocalsSize(Function *pFunc) {

    if ( !pFunc || pFunc->isDeclaration () ) {
      // Not module function, no need for local buffer, return size zero
      return 0;
    }

    if ( m_localSizeMap.count(pFunc) ) {
      // local size of this function already calculated
      return m_localSizeMap[pFunc];
    }

#if LLVM_VERSION == 3425
    llvm::TargetData DL(m_pModule);
#else
    llvm::DataLayout DL(m_pModule);
#endif
    size_t localBufferSize = 0;

    for ( TUsedLocals::iterator li = m_localUsageMap[pFunc].begin(),
      le = m_localUsageMap[pFunc].end(); li != le; ++li ) {

        GlobalValue *pLclBuff = dyn_cast<GlobalValue>(*li);
        assert( pLclBuff && "locals container contains something other than GlobalValue!" );

        // Calculate required buffer size
        size_t uiArraySize = DL.getTypeSizeInBits(pLclBuff->getType()->getElementType())/8;
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

      size_t callLocalSize = calculateLocalsSize(pCall->getCalledFunction());
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

