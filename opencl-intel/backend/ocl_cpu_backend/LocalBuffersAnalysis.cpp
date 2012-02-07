/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  LocalBuffersAnalysis.cpp

\*****************************************************************************/

#include "LocalBuffersAnalysis.h"
#include "CompilationUtils.h"

#include "cpu_dev_limits.h"

#include "llvm/Instructions.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Constant.h"
#include "llvm/Target/TargetData.h"


namespace Intel { namespace OpenCL { namespace DeviceBackend {
  
  /// @brief Creates new LocalBuffersAnalysis module pass
  /// @returns new LocalBuffersAnalysis module pass
  ModulePass* createLocalBuffersAnalysisPass() {
    return new LocalBuffersAnalysis();
  }
    
  // Need to register analysis pass, otherwise, the passes that use this analysis cannot get this pass' info 
  static RegisterPass<LocalBuffersAnalysis>
    LocalBuffersAnalysisPass("LocalBuffersAnalysis", "LocalBuffersAnalysis provides local values analysis info");

  char LocalBuffersAnalysis::ID = 0;

  bool LocalBuffersAnalysis::runOnModule(Module &M) {

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

  void LocalBuffersAnalysis::updateLocalsMap(GlobalValue* pLocalVal, User * user) {
    // llvm::Instruction, llvm::Operator and llvm::Constant are the only possible subtypes of llvm::user
    if ( isa<Instruction>(user) ) {
      // Parent of Instruction is BasicBlock
      // Parent of BasicBlock is Function
      Function* pFunc = cast<Instruction>(user)->getParent()->getParent();
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

  void LocalBuffersAnalysis::updateDirectLocals(Module &M) {
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

  size_t LocalBuffersAnalysis::calculateLocalsSize(Function *pFunc) {

    if ( !pFunc || pFunc->isDeclaration () ) {
      // Not module function, no need for local buffer, return size zero
      return 0;
    }

    if ( m_localSizeMap.count(pFunc) ) {
      // local size of this function already calculated
      return m_localSizeMap[pFunc];
    }

    llvm::TargetData TD(m_pModule);
    size_t localBufferSize = 0;

    for ( TUsedLocals::iterator li = m_localUsageMap[pFunc].begin(),
      le = m_localUsageMap[pFunc].end(); li != le; ++li ) {

        GlobalValue *pLclBuff = dyn_cast<GlobalValue>(*li);
        assert( pLclBuff && "locals container contains something other than GlobalValue!" );
          
        // Calculate required buffer size
        size_t uiArraySize = TD.getTypeSizeInBits(pLclBuff->getType()->getElementType())/8;
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


}}}

