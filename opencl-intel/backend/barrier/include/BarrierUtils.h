/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __BARRIER_UTILS_H__
#define __BARRIER_UTILS_H__

#include "CL/cl.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"

#include "llvm/ADT/SetVector.h"

//Forward declaration
namespace llvm {
  class Module;
  class Function;
  class BasicBlock;
  class Instruction;
  class Value;
  class Type;
  class StringRef;
  class Twine;
}

#include <map>
#include <vector>
using namespace llvm;

namespace intel {

  /// @brief OpenCL-specific barrier function names
  #define FIBER_FUNC_NAME         "fiber."
  #define DUMMY_BARRIER_FUNC_NAME "dummybarrier."

  #define GET_SPECIAL_BUFFER "get_special_buffer."
  #define GET_BASE_GID "get_base_global_id."

  #define CLK_LOCAL_MEM_FENCE (CL_LOCAL)
  #define CLK_GLOBAL_MEM_FENCE (CL_GLOBAL)

  #define SPECIAL_BUFFER_ADDR_SPACE 0
  #define CURR_WI_ADDR_SPACE 0

  typedef enum {
    SYNC_TYPE_NONE,
    SYNC_TYPE_BARRIER,
    SYNC_TYPE_DUMMY_BARRIER,
    SYNC_TYPE_FIBER,
    SYNC_TYPE_NUM
  } SYNC_TYPE;

  typedef enum {
    CALL_BI_TYPE_WG,
    CALL_BI_TYPE_WG_ASYNC_OR_PIPE, // work-group async_copy and pipe built-ins
    CALL_BI_NUM
  } CALL_BI_TYPE;

  typedef std::vector<Value*> TValueVector;
  typedef std::vector<Instruction*> TInstructionVector;
  typedef std::vector<BasicBlock*> TBasicBlockVector;
  typedef std::vector<Function*> TFunctionVector;

  typedef SetVector<Instruction*> TInstructionSet;
  typedef SetVector<Function*> TFunctionSet;
  typedef SetVector<BasicBlock*> TBasicBlockSet;

  typedef std::map<const Function*, unsigned> TFunctionToUnsigned;

  static std::string AppendWithDimension(std::string S, int Dimension) {
    if (Dimension >= 0)
      S += '0' + Dimension;
    else
      S += "var";
    return S;
  }
  static std::string AppendWithDimension(std::string S, const Value *Dimension) {
    int D = -1;
    if (const ConstantInt *C = dyn_cast<ConstantInt>(Dimension))
      D = C->getZExtValue();
    return AppendWithDimension(S, D);
  }
  /// @brief BarrierUtils is utility class that collects several data
  /// and processes several functionality on a given module
  class BarrierUtils {
  public:
    /// @brief C'tor
    BarrierUtils();

    /// @brief D'tor
    ~BarrierUtils() {}

    /// @brief Initialize Barrier Utils with given module to process data on
    /// @param M module to process
    void init(Module *pModule);

    /// @brief return BasicBlock of pUserInst (if it is not a PHINode)
    ///  Otherwise, return the prevBB of pUserInst with respect to pVal
    /// @param pVal value that pUserInst is using
    /// @param pUserInst instruction that is using pInst value
    /// @returns BasicBlock of usage instruction with respect to value it is using
    static BasicBlock* findBasicBlockOfUsageInst(Value *pVal, Instruction *pUserInst);

    /// @brief return synchronize type of given instruction
    /// @param pInst instruction to observe its synchronize type
    /// @returns given instruction synchronize type
    ///  {barrier, fiber, dummyBarrier or none}
    SYNC_TYPE getSynchronizeType(Instruction *pInst);

    /// @brief return synchronize type of given basic block
    /// @param pBB basic block to observe its synchronize type
    /// @returns given basic block synchronize type
    ///  {barrier, fiber, dummyBarrier or none}
    SYNC_TYPE getSynchronizeType(BasicBlock *pBB);

    /// @brief return all synchronize instructions in the module
    /// @returns container with all synchronize instructions
    TInstructionVector& getAllSynchronizeInstructuons();

    /// @brief return all synchronize basic blocks in the module
    /// @returns container with all synchronize basic blocks
    //TBasicBlockVector& getAllSynchronizeBasicBlocks();

    /// @brief Find all functions  in the module
    ///  that contain synchronize instructions
    /// @returns TInstructionSet container with found functions
    TFunctionSet& getAllFunctionsWithSynchronization();

    /// @brief Find calls to WG functions upon their type
    /// @param type - type of WG functions to find
    /// @returns container with calls to WG functions of requested type
    TInstructionVector& getWGCallInstructions(CALL_BI_TYPE type);

    /// @brief Find all kernel functions in the module
    /// @returns TFunctionVector container with found functions
    TFunctionVector& getAllKernelFunctions();

    unsigned getKernelVectorizationWidth(const Function *F) const;

    Instruction *createGetLocalSize(unsigned dim, Instruction *pInsertBefore);
    Instruction *createGetBaseGlobalId(Value* dim, Instruction *pInsertBefore);

    /// @brief Create new call instruction to barrier()
    /// @param pInsertBefore instruction to insert new call instruction before
    /// @returns new created call instruction
    Instruction* createBarrier(Instruction *pInsertBefore = 0);

    /// @brief Checks whether call instruction calls barrier()
    /// @param pCallInstr instruction of interest
    /// @returns true if this is barrier() call and false otherwise
    bool isBarrierCall(CallInst *pCallInstr);

    /// @brief Create new call instruction to dummyBarrier()
    /// @param pInsertBefore instruction to insert new call instruction before
    /// @returns new created call instruction
    Instruction* createDummyBarrier(Instruction *pInsertBefore = 0);

    /// @brief Checks whether call instruction calls dummyBarrier()
    /// @param pCallInstr instruction of interest
    /// @returns true if this is dummyBarrier() call and false otherwise
    bool isDummyBarrierCall(CallInst *pCallInstr);

    /// @brief Create new call instruction to __mm_mfence()
    /// @param pAtEnd basic block to insert new call at its end
    /// @returns new created call instruction
    Instruction* createMemFence(IRBuilder<>& B);

    /// @brief Create new call instruction to get_special_buffer()
    /// @param pInsertBefore instruction to insert new call instruction before
    /// @returns new created call instruction
    Instruction* createGetSpecialBuffer(Instruction *pInsertBefore = 0);

    /// @brief Create new call instruction to get_iter_count()
    /// @param pInsertBefore instruction to insert new call instruction before
    /// @returns new created call instruction
    Instruction* createGetIterCount(Instruction *pInsertBefore = 0);

    /// @brief return all get_local_id call instructions in the module
    /// @returns container with all get_local_id call instructions
    TInstructionVector& getAllGetLocalId();

    /// @brief return all get_global_id call instructions in the module
    /// @returns container with all get_global_id call instructions
    TInstructionVector& getAllGetGlobalId();

    /// @brief Create new call instruction to get_global_id()
    /// @param dim dimensionality of kernel
    /// @param pInsertBefore instruction to insert new call instruction before
    /// @returns new created call instruction
    Instruction *createGetGlobalId(unsigned dim, IRBuilder<> &);

    /// @brief return an indicator regarding given function calls
    /// a function defined in the module (that was not inlined)
    /// @param pFunc the given function
    /// @returns true if and only if the function calls a module function
    bool doesCallModuleFunction(Function *pFunc);

  private:
    /// @brief Clean all collected values and assure
    ///  re-collecting them on the next access to them
    void clean();

    /// @brief Initialize the barrier and dummyBarrier
    ///  function pointers in given module
    void initializeSyncData();

    /// @brief Find all instructions in the module,
    ///  which use function with given name
    /// @param name the given function name
    /// @param usesSet container to insert all found usage instructions into
    void findAllUsesOfFunc(const llvm::StringRef& name, TInstructionSet &usesSet);

    /// @brief Create function declaration with given name and type specification
    /// @param name the given function name
    /// @param pResult type of return value of the function
    /// @param funcTyArgs types vector of all arguments values of the function
    /// @returns Function new declared function
    Function* createFunctionDeclaration(const llvm::Twine& name, Type *pResult, std::vector<Type*>& funcTyArgs);

    /// @brief Add ReadNone attribute to given function.
    /// @param pFunc the given function.
    void SetFunctionAttributeReadNone(Function* pFunc);

  private:
    /// Pointer to current processed module
    Module    *m_pModule;

    /// This holds size of size_t of processed module
    unsigned int m_uiSizeT;

    /// Pointer to value argument of barier function
    Value     *m_localMemFenceValue;
    /// Pointer to barrier function in module
    Function  *m_barrierFunc;
    /// Pointer to dummyBarrier function in module
    Function  *m_dummyBarrierFunc;

    /// This holds the get_special_buffer() function
    Function  *m_getSpecialBufferFunc;
    /// This holds the get_local_size() function
    Function  *m_getLocalSizeFunc;
    /// This holds the get_global_id() function
    Function  *m_getGIDFunc;
    /// This holds the get_base_global_id() function
    Function  *m_getBaseGIDFunc;

    /// This holds the all sync instructions of the module
    TInstructionVector  m_syncInstructions;
    /// This holds the all sync basic blocks of the module
    TBasicBlockVector   m_syncBasicBlocks;
    /// This holds the all functions of the module with sync instruction
    TFunctionSet        m_syncFunctions;
    /// This holds the all kernel functions of the module
    TFunctionVector     m_kernelFunctions;
    /// This holds all the WG function calls in the module
    TInstructionVector  m_WGcallInstructions;
    TFunctionToUnsigned m_kernelVectorizationWidths;
    

    /// This indecator for synchronize data initialization
    bool m_bSyncDataInitialized;
    /// This holds the all barrier instructions of the module
    TInstructionSet m_barriers;
    /// This holds the all dummyBarrier instructions of the module
    TInstructionSet m_dummyBarriers;
    /// This holds the all fiber instructions of the module
    TInstructionSet m_fibers;

    /// This indecator for get_local_id data initialization
    bool m_bLIDInitialized;
    /// This holds the all get_local_id instructions of the module
    TInstructionVector m_getLIDInstructions;

    /// This indecator for get_global_id data initialization
    bool m_bGIDInitialized;
    /// This holds the all get_global_id instructions of the module
    TInstructionVector m_getGIDInstructions;

    /// This indecator for functions with internal calls initialization
    bool m_bNonInlinedCallsInitialized;
    /// This holds the all function of the module with internal calls
    TFunctionSet m_functionsWithNonInlinedCalls;

    Type* m_SizetTy;
    Type* m_I32Ty;


  };

} // namespace intel

#endif // __BARRIER_UTILS_H__
