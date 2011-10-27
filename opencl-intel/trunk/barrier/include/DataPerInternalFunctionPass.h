/*********************************************************************************************
 * TODO: add Copyright © 2011, Intel Corporation
 *********************************************************************************************/
#ifndef __DATA_PER_INTERNAL_FUNCTION_PASS_H__
#define __DATA_PER_INTERNAL_FUNCTION_PASS_H__

#include "BarrierUtils.h"
#include "DataPerValuePass.h"

#include "llvm/Pass.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Target/TargetData.h"

using namespace llvm;

namespace intel {

  /// @brief DataPerInternalFunction pass is a analysis module pass used to collect
  /// data on internal functions defined in the module that are called from the module
  class DataPerInternalFunction : public ModulePass {
  public:
    typedef std::vector<unsigned int> TCounterVector;
    typedef struct {
      unsigned int m_numberOfUses;
      TCounterVector m_argsInSpecialBuffer;
      bool m_needToBeFixed;
    } TInternalFunctionData;
    typedef std::map<Function*, TInternalFunctionData> TDataPerFunctionMap;
    typedef struct {
      TCounterVector m_argsOffsets;
    } TInternalCallData;
    typedef std::map<CallInst*, TInternalCallData> TDataPerCallMap;

  public:
    static char ID;

    static unsigned int m_badOffset;

    /// @brief C'tor
    DataPerInternalFunction();

    /// @brief D'tor
    ~DataPerInternalFunction() {}

    /// @brief Provides name of pass
    virtual const char *getPassName() const {
      return "Intel OpenCL DataPerInternalFunction";
    }

    /// @brief execute pass on given module
    /// @param M module to analyze
    /// @returns True if module was modified
    virtual bool runOnModule(Module &M);

    /// @brief Inform about usage/mofication/dependency of this pass
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<DataPerValue>();
      // Analysis pass preserve all
      AU.setPreservesAll();
    }

    /// @brief print data collected by the pass on the given module
    /// @param OS stream to print the info regarding the module into
    /// @param M pointer to the Module
    void print(raw_ostream &OS, const Module *M = 0) const;

    /// @brief return true if given function needs to be fixed
    /// @param pFunc pointer to Function
    /// @returns true if and only if pFunc needs to be fixed
    bool needToBeFixed(Function *pFunc) {
      if ( !m_dataPerFuncMap.count(pFunc) ) {
        //No collected data on this function, no need to fix it
        return false;
      }
      return m_dataPerFuncMap[pFunc].m_needToBeFixed;
    }

    /// @brief return in special buffer counters for all arguments of given function
    /// @param pFunc pointer to Function
    /// @param argIndex function argument index
    /// @returns container with in special buffer counters of pFunc arguments
    bool alwaysInSpecialBuffer(Function *pFunc, unsigned int argIndex) {
      assert( m_dataPerFuncMap[pFunc].m_numberOfUses >=
        m_dataPerFuncMap[pFunc].m_argsInSpecialBuffer[argIndex] &&
        "number of usages of pFunc is smaller than number of calls to pFunc!" );
      return ( m_dataPerFuncMap[pFunc].m_numberOfUses ==
        m_dataPerFuncMap[pFunc].m_argsInSpecialBuffer[argIndex] );
    }

    /// @brief return true if there is a call to given function with value
    ///  stored in special buffer and passed to given argument index
    /// @param pFunc pointer to Function
    /// @param argIndex function argument index
    /// @returns true if and only if there is a call to pFunc
    ///  with special buffer value passed to argIndex
    bool isInSpecialBuffer(Function *pFunc, unsigned int argIndex) {
      return ( 0 != m_dataPerFuncMap[pFunc].m_argsInSpecialBuffer[argIndex] );
    }

    /// @brief return offset in special buffer of given argument offset
    ///  of given function call instruction
    /// @param pCalInst pointer to CallInst
    /// @param argIndex function argument index
    /// @returns offset in special buffer, negative number for bad offset
    unsigned int getOffset(CallInst *pCallInst, unsigned int argIndex) {
      assert( m_dataPerCallMap.count(pCallInst) &&
        "No collected data for this pCallInst" );
      return m_dataPerCallMap[pCallInst].m_argsOffsets[argIndex];
    }

    /// @brief return true if given call instruction need to be fixed
    /// @param pCalInst pointer to CallInst
    /// @returns true if and only if given call instruction need to be fixed
    bool needToBeFixed(CallInst *pCallInst) {
      if ( !m_dataPerCallMap.count(pCallInst) ) {
        // No collected data for this pCallInst
        return false;
      }
      unsigned int numOfArgs = m_dataPerCallMap[pCallInst].m_argsOffsets.size();
      for ( unsigned int i = 0; i < numOfArgs; ++i ) {
        if ( m_badOffset != m_dataPerCallMap[pCallInst].m_argsOffsets[i] ) {
          //There is one offset that is not bad, need to fix this call instruction
          return true;
        }
      }
      return false;
    }

    /// @brief return ordered list of functions need to be fixed
    /// @returns ordered container with all functions to be fixed
    TFunctionVector& getOrderedFunctionsToFix() {
      return m_orderedFunctionsToFix;
    }

  private:
    /// @brief execute pass on given function
    /// @param F function to analyze
    /// @returns True if function was modified
    virtual bool runOnFunction(Function &F);

    /// @brief calculate the calling order of all functions need to be fixed
    void calculateCallingOrder();

  private:
    // Internal Data used to calculate user Analysis Data
    /// This is barrier utility class
    BarrierUtils m_util;
    /// This holds DataPerValue analysis pass
    DataPerValue *m_pDataPerValue;

    // Analysis Data for pass user
    /// This holds collected data per function in processed module
    TDataPerFunctionMap m_dataPerFuncMap;
    /// This holds collected data per call instruction in processed module
    TDataPerCallMap m_dataPerCallMap;
    /// This is a list of all functions to be fixed in processed module
    /// that are ordered according to call graph from leaf to root
    TFunctionVector m_orderedFunctionsToFix;

  };

} // namespace intel

#endif // __DATA_PER_INTERNAL_FUNCTION_PASS_H__

