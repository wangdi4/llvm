/*********************************************************************************************
 * TODO: add Copyright © 2011, Intel Corporation
 *********************************************************************************************/
#ifndef __BARRIER_UTILS_H__
#define __BARRIER_UTILS_H__

#include "CL/cl.h"

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

#include <set>
#include <vector>
using namespace llvm;

namespace intel {

  /// @brief OpenCL-specific barrier function names
  #define BARRIER_FUNC_NAME       "barrier"
  #define FIBER_FUNC_NAME         "fiber."
  #define DUMMY_BARRIER_FUNC_NAME "dummybarrier."

  #define GET_GID_NAME  "get_global_id"
  #define GET_LID_NAME  "get_local_id"
  #define GET_NEW_GID_NAME  "get_new_global_id."
  #define GET_NEW_LID_NAME  "get_new_local_id."

  #define GET_ITERATION_COUNT "get_iter_count."
  #define GET_SPECIAL_BUFFER "get_special_buffer."
  #define GET_CURR_WI "get_curr_wi."
  #define CURR_SB_INDEX_ALLOCA "CurrSBIndex."
  #define CURR_WI_ALLOCA "CurrWI."
  #define GET_ITER_COUNT "IterCount."

  #define VECTORIZED_KERNEL_PREFIX "__Vectorized_."

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

  typedef std::vector<Value*> TValueVector;
  typedef std::vector<Instruction*> TInstructionVector;
  typedef std::vector<BasicBlock*> TBasicBlockVector;
  typedef std::vector<Function*> TFunctionVector;

  typedef std::set<Instruction*> TInstructionSet;
  typedef std::set<Function*> TFunctionSet;

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
    /// @returns TFunctionVector container with found functions
    TFunctionVector& getAllFunctionsWithSynchronization();

    /// @brief Find all kernel functions in the module
    /// @returns TFunctionVector container with found functions
    TFunctionVector& getAllKernelFunctions();

    /// @brief Create new call instruction to barrier()
    /// @param pInsertBefore instruction to insert new call instruction before
    /// @returns new created call instruction
    Instruction* createBarrier(Instruction *pInsertBefore = 0);

    /// @brief Create new call instruction to dummyBarrier()
    /// @param pInsertBefore instruction to insert new call instruction before
    /// @returns new created call instruction
    Instruction* createDummyBarrier(Instruction *pInsertBefore = 0);

    /// @brief Create new call instruction to __mm_mfence()
    /// @param pAtEnd basic block to insert new call at its end
    /// @returns new created call instruction
    Instruction* createMemFence(BasicBlock *pAtEnd);

    /// @brief Create new call instruction to get_curr_wi()
    /// @param pInsertBefore instruction to insert new call instruction before
    /// @returns new created call instruction
    Instruction* createGetCurrWI(Instruction *pInsertBefore = 0);

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

    /// @brief Create new call instruction to get_new_local_id()
    /// @param pArg1 first argument to get_new_local_id()
    /// @param pArg2 first argument to get_new_local_id()
    /// @param pInsertBefore instruction to insert new call instruction before
    /// @returns new created call instruction
    Instruction* createNewGetLocalId(Value *pArg1, Value *pArg2, Instruction *pInsertBefore);

    /// @brief Create new call instruction to get_new_global_id()
    /// @param pArg1 first argument to get_new_global_id()
    /// @param pArg2 first argument to get_new_global_id()
    /// @param pInsertBefore instruction to insert new call instruction before
    /// @returns new created call instruction
    Instruction* createNewGetGlobalId(Value *pArg1, Value *pArg2, Instruction *pInsertBefore);

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
    Function* createFunctionDeclaration(const llvm::Twine& name, const Type *pResult, std::vector<const Type*>& funcTyArgs);

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

    /// This holds the get_curr_wi() function
    Function  *m_getCurrWIFunc;
    /// This holds the get_special_buffer() function
    Function  *m_getSpecialBufferFunc;
    /// This holds the get_iter_count() function
    Function  *m_getIterationCountFunc;
    /// This holds the get_new_local_id() function
    Function  *m_getNewLIDFunc;
    /// This holds the get_new_global_id() function
    Function  *m_getNewGIDFunc;

    /// This holds the all sync instructions of the module
    TInstructionVector  m_syncInstructions;
    /// This holds the all sync basic blocks of the module
    TBasicBlockVector   m_syncBasicBlocks;
    /// This holds the all functions of the module with sync instruction
    TFunctionVector     m_syncFunctions;
    /// This holds the all kernel functions of the module
    TFunctionVector     m_kernelFunctions;

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

  };

} // namespace intel

#endif // __BARRIER_UTILS_H__
