/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __LOCAL_BUFF_ANALYSIS_H__
#define __LOCAL_BUFF_ANALYSIS_H__

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/ADT/SmallPtrSet.h"

#include <map>

using namespace llvm;

namespace intel{

  /// @brief  LocalBuffAnalysis class used to provide information about the
  ///         local values each function uses directly.
  ///         The analysis class goes over all local values and over all their direct
  ///         users and maps between functions and the local values they uses.
  class LocalBuffAnalysis : public ModulePass {
  public:
    /// @brief  A set of local values used by a function
    typedef SmallPtrSet<llvm::GlobalValue*, 16> TUsedLocals;

  public:
    /// @brief  Pass identification, replacement for typeid
    static char ID;

  public:

    /// @brief  Constructor
    LocalBuffAnalysis() : ModulePass(ID) {}

    /// @brief Provides name of pass
    virtual llvm::StringRef getPassName() const {
      return "LocalBuffAnalysis";
    }

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

} // namespace intel

#endif // __LOCAL_BUFF_ANALYSIS_H__