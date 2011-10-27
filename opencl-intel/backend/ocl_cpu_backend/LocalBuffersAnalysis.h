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

File Name:  LocalBuffersAnalysis.h

\*****************************************************************************/

#ifndef __LOCAL_BUFFERS_ANALYSIS_H__
#define __LOCAL_BUFFERS_ANALYSIS_H__

#include "llvm/Pass.h"
#include "llvm/Module.h"
#include "llvm/ADT/SmallPtrSet.h"

#include <map>

using namespace llvm;

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  /// @brief  LocalBuffersAnalysis class used to provide information about the 
  ///         local values each function uses directly.
  ///         The analysis class goes over all local values and over all their direct 
  ///         users and maps between functions and the local values they uses.
  /// @Author Marina Yatsina
  class LocalBuffersAnalysis : public ModulePass {
  public:
    /// @brief  A set of local values used by a function
    typedef SmallPtrSet<llvm::GlobalValue*, 16> TUsedLocals;

  public:
    /// @brief  Pass identification, replacement for typeid
    static char ID;

  public:
      
    /// @brief  Constructor
    LocalBuffersAnalysis() : ModulePass(ID) {}

    /// @brief LLVM Module pass entry
    /// @param M Module to transform
    /// @returns True if changed
    bool runOnModule(Module &M);
    
    /// @brief Returns the set of local values used directly by the given function
    /// @param pFunc   A function for which should return the local values that were 
    ///                 used by it directly
    /// @returns The set of local values used directly by the given function
    const TUsedLocals& getDirectLocals(Function* pFunc) {
      return m_localUsageMap[pFunc];
    }

    /// @brief Returns the size of local buffer used directly by the given function
    /// @param pFunc   given function
    /// @returns The size of local buffer used directly by the given function
    size_t getDirectLocalsSize(Function* pFunc) {
      return m_directLocalSizeMap[pFunc];
    }

    /// @brief Returns the size of local buffer used by the given function
    /// @param pFunc   given function
    /// @returns The size of local buffer used by the given function
    size_t getLocalsSize(Function* pFunc) {
      return m_localSizeMap[pFunc];
    }

    /// @brief LLVM Interface
    /// @param AU Analysis
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      // Analysis pass preserve all
      AU.setPreservesAll();
    }
      
  protected:

    /// @brief  Adds the given local value to the set of used locals of all functions
    ///         that are using the given user directly. It recursively searches the first
    ///         useres (and users of a users) that are functions
    /// @param  pLocalVal   Local value (which is represented by a global value with address space 3)
    /// @param  user        Direct user of pLocalVal
    void updateLocalsMap(GlobalValue* pLocalVal, User * user);

    /// @brief  Goes over all local values in the module and over all their direct
    ///         users and maps between functions and the local values they use
    /// @param  M   The module which need to go over its local values
    void updateDirectLocals(Module &M);

    /// @brief  Goes over all local values used by the function and calculate
    ///         the total size needed to store all of them and add to that
    ///         the max size of local buffer needed by all called functions.
    /// @param  pFunc   The function which need to go over its local values
    /// @param  returns The total size of local buffer needed by given function
    size_t calculateLocalsSize(Function *pFunc);

  protected:
    /// @brief A mapping between function pointer and the set of local values 
    ///        the function uses directly
    typedef std::map<const llvm::Function*, TUsedLocals> TUsedLocalsMap;

    /// @brief A mapping between function pointer and the local buffer size
    ///         that the function uses
    typedef std::map<const llvm::Function*, size_t> TLocalSizeMap;

    /// @brief The llvm module this pass needs to update
    Module *m_pModule;

    /// @brief Map between function and the local values it uses directly
    TUsedLocalsMap m_localUsageMap;

    /// @brief Map between function and the local buffer size
    TLocalSizeMap m_localSizeMap;

    /// @brief Map between function and the local buffer size
    ///        for local values used directly by this function
    TLocalSizeMap m_directLocalSizeMap;

  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __LOCAL_BUFFERS_ANALYSIS_H__