/*********************************************************************************************
 * TODO: add Copyright © 2011, Intel Corporation
 *********************************************************************************************/
#ifndef __DATA_PER_VALUE_PASS_H__
#define __DATA_PER_VALUE_PASS_H__

#include "DataPerBarrierPass.h"
#include "WIRelatedValuePass.h"

#include "llvm/Pass.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Target/TargetData.h"

using namespace llvm;

namespace intel {

  /// @brief DataPerValue pass is a analysis module pass used to collect
  /// data on Values (instructions) that needs special handling
  class DataPerValue : public ModulePass {
  public:
    typedef std::map<Function*, TValueVector> TValuesPerFunctionMap;
    typedef std::map<Value*, unsigned int> TValueToOffsetMap;

  public:
    static char ID;

    /// @brief C'tor
    DataPerValue();

    /// @brief D'tor
    ~DataPerValue() {}

    /// @brief Provides name of pass
    virtual const char *getPassName() const {
      return "Intel OpenCL DataPerValue";
    }

    /// @brief execute pass on given module
    /// @param M module to analyze
    /// @returns True if module was modified
    virtual bool runOnModule(Module &M);

    /// @brief Inform about usage/mofication/dependency of this pass
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<DataPerBarrier>();
      AU.addRequired<WIRelatedValue>();
      // Analysis pass preserve all
      AU.setPreservesAll();
    }

    /// @brief print data collected by the pass on the given module
    /// @param OS stream to print the info regarding the module into
    /// @param M pointer to the Module
    void print(raw_ostream &OS, const Module *M = 0) const;

    /// @brief return all values to handle in given function
    /// @param pFunc pointer to Function
    /// @returns container of values to handle in pFunc
    TValueVector& getValuesToHandle(Function *pFunc) {
      return m_specialValuesPerFuncMap[pFunc];
    }

    /// @brief return all alloca values to handle in given function
    /// @param pFunc pointer to Function
    /// @returns container of alloca values to handle in pFunc
    TValueVector& getAllocaValuesToHandle(Function *pFunc) {
      return m_allocaValuesPerFuncMap[pFunc];
    }

    /// @brief return all uniform values to handle in given function
    /// @param pFunc pointer to Function
    /// @returns container of uniform values to handle in pFunc
    TValueVector& getUniformValuesToHandle(Function *pFunc) {
      return m_crossBarrierValuesPerFuncMap[pFunc];
    }

    /// @brief return offset of given value relatively
    ///  to all other values in the special buffer structure
    /// @param pVal pointer to Value
    /// @returns offset of given value
    unsigned int getOffset(Value *pVal) {
      assert( m_valueToOffsetMap.count(pVal) &&
        "requiring offset of non special value!" );
      return m_valueToOffsetMap[pVal];
    }

    /// @brief return true if given value has offset
    ///  in the special buffer structure
    /// @param pVal pointer to Value
    /// @returns true if given value has offset
    bool hasOffset(Value *pVal) {
      return ( 0 != m_valueToOffsetMap.count(pVal) );
    }

    /// @brief return total size in bytes of special buffer per one WI
    ///  after allocating place for all alloca and special values
    /// @returns special buffer total size in bytes
    unsigned int getTotalSize() {
      return m_bufferTotalSize;
    }

    /// @brief return stride size per work item in special buffer
    /// @returns stride size per work item in special buffer
    unsigned int getStrideSize() {
      return m_bufferTotalSize;
    }

  private:
    /// @brief execute pass on given function
    /// @param F function to analyze
    /// @returns True if function was modified
    virtual bool runOnFunction(Function &F);

    typedef enum {
      SPECIAL_VALUE_TYPE_NONE,
      SPECIAL_VALUE_TYPE_A,
      SPECIAL_VALUE_TYPE_B1,
      SPECIAL_VALUE_TYPE_B2,
      SPECIAL_VALUE_TYPE_NUM
    } SPECIAL_VALUE_TYPE;

    /// @brief return type of given value Group-B.1, Group-B.2 or None
    /// @param pVal pointer to Value
    /// @param isWIRelated true if value depends on WI id, otherwise false
    /// @returns SPECIAL_VALUE_TYPE - speciality type of given value
    SPECIAL_VALUE_TYPE isSpecialValue(Value *pVal, bool isWIRelated);

    /// @brief return true if there is a barrier in one of the pathes between
    ///  pValBB and pValUsageBB basic blocks
    /// @param pValUsageBB basic block to start searching the path
    ///   according to its predecessors
    /// @param pValBB basic block to stop searching the path when reach it
    /// @returns true if and only if find a barrier in one of the searched pathes
    bool isCrossedByBarrier(BasicBlock *pValUsageBB, BasicBlock *pValBB);

    /// @brief calculates offsets of all values in Group-A and Group-B.1
    /// @param F function to process its values
    void calculateOffsets(Function &F);

    /// @brief return offset of given Type in special buffer stride
    /// @param pType pointer to Type
    /// @param allocaAlignment alignment of alloca instruction (0 if it is not alloca)
    /// @returns offset of given Type in special buffer stride
    unsigned int getValueOffset(const Type *pType, unsigned int allocaAlignment);

  private:
    // Internal Data used to calculate user Analysis Data
    /// This holds DataPerBarrier analysis pass
    DataPerBarrier *m_pDataPerBarrier;
    /// This holds container of synchronize instructions in processed module
    TInstructionSet *m_pSyncInstructions;
    /// This holds WIRelatedValue analysis pass
    WIRelatedValue *m_pWIRelatedValue;
    /// This holds TargetData of processed module
    TargetData *m_pTD;

    /// This will indecates the next empty offset in the buffer structure.
    unsigned int m_currentOffset;
    /// The allignment of buffer structure depends on
    /// largest alignment needed by any type in the buffer.
    unsigned int m_maxAlignment;

    // Analysis Data for pass user
    /// This holds a map between function and its values of Group-A
    TValuesPerFunctionMap m_allocaValuesPerFuncMap;
    /// This holds a map between function and its values of Group-B.1
    TValuesPerFunctionMap m_specialValuesPerFuncMap;
    /// This holds a map between function and its values of Group-B.2
    TValuesPerFunctionMap m_crossBarrierValuesPerFuncMap;
    /// This holds a map between value and it offset in Special Buffer structure
    TValueToOffsetMap     m_valueToOffsetMap;

    /// Special buffer total size in bytes per one WI
    unsigned int m_bufferTotalSize;

  };

} // namespace intel

#endif // __DATA_PER_VALUE_PASS_H__

