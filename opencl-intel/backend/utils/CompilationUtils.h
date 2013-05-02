/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __COMPILATION_UTILS_H__
#define __COMPILATION_UTILS_H__

#include "cl_kernel_arg_type.h"
#ifndef __APPLE__
#include "exceptions.h"
#endif

#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Constants.h"

#include <set>
#include <vector>
#include <map>

using namespace llvm;

namespace Intel { namespace OpenCL { namespace DeviceBackend {

#ifndef __APPLE__
  DEFINE_EXCEPTION(CompilerException)
#endif

  /// @brief  CompilationUtils class used to provide helper utilies that are
  ///         used by several other classes.
  /// @Author Marina Yatsina
  class CompilationUtils {

  public:
    /// @brief Removes the from the given basic block the instruction pointed 
    ///        by the given iterator
    /// @param pBB         A Basic block from which the instruction needs to be removed
    /// @param it          An iterator pointing to the instruction that needs to be removed
    /// @returns An iterator to the next instruction after the instruction that was removed
    static BasicBlock::iterator removeInstruction(BasicBlock* pBB, BasicBlock::iterator it);
    
    /// @brief  Retrieves the pointer to the implicit arguments added to the given function
    /// @param  pFunc        The function for which implicit arguments need to be retrieved
    /// @param  ppLocalMem   The pLocalMem argument, NULL if this argument shouldn't be retrieved
    /// @param  ppWorkDim    The pWorkDim argument, NULL if this argument shouldn't be retrieved
    /// @param  ppWGId       The pWGId argument, NULL if this argument shouldn't be retrieved
    /// @param  ppBaseGlbId  The pBaseGlbId argument, NULL if this argument shouldn't be retrieved
    /// @param  ppLocalId    The LocalIds argument, NULL if this argument shouldn't be retrieved
    /// @param  ppSpecialBuf The SpecialBuf argument, NULL if this argument shouldn't be retrieved
    /// @param  ppIterCount  The IterCount argument, NULL if this argument shouldn't be retrieved
    /// @param  ppCurrWI     The CurrWI argument, NULL if this argument shouldn't be retrieved
    /// @param  ppCtx        The pCtx argument, NULL if this argument shouldn't be retrieved
    static void getImplicitArgs(Function *pFunc,
      Argument **ppLocalMem, Argument **ppWorkDim, Argument **ppWGId,
      Argument **ppBaseGlbId, Argument **ppLocalId, Argument **ppIterCount,
      Argument **ppSpecialBuf, Argument **ppCurrWI, Argument **ppCtx);
                    
    /// @brief collect all kernel functions
    /// @param functionSet container to insert all kernel function into
    /// @param pModule the module to search kernel function inside
    static void getAllKernels(std::set<Function*> &functionSet, Module *pModule);

    /// @brief collect all kernel wrapper functions
    /// @param functionSet container to insert all kernel wrapper function into
    /// @param pModule the module to search kernel wrapper function inside
    static void getAllKernelWrappers(std::set<Function*> &functionSet, Module *pModule);

    /// @brief  fills a vector of cl_kernel_argument with arguments representing pFunc's
    ///         OpenCL level arguments
    /// @param pModule    The module
    /// @param pFunc      The kernel for which to create argument vector
    /// @param arguments  OUT param, the cl_kernel_argument which represent pFunc's
    ///                   OpenCL level argument
    static void parseKernelArguments(  Module* pModule, 
                                              Function* pFunc, 
                                              std::vector<cl_kernel_argument>& /* OUT */ arguments);
    
  public:
    /// This holds the number of implicite arguments addeded to function
    static const unsigned int NUMBER_IMPLICIT_ARGS;

    /// '3' is a magic number for global variables
    /// that were in origin kernel local variable!
    static const unsigned int LOCL_VALUE_ADDRESS_SPACE;

    static const std::string NAME_GET_BASE_GID;
    static const std::string NAME_GET_GID;
    static const std::string NAME_GET_LID;
    static const std::string NAME_GET_ITERATION_COUNT;
    static const std::string NAME_GET_SPECIAL_BUFFER;
    static const std::string NAME_GET_CURR_WI;

    static const std::string NAME_GET_WORK_DIM;
    static const std::string NAME_GET_GLOBAL_SIZE;
    static const std::string NAME_GET_LOCAL_SIZE;
    static const std::string NAME_GET_NUM_GROUPS;
    static const std::string NAME_GET_GROUP_ID;
    static const std::string NAME_GET_GLOBAL_OFFSET;
    static const std::string NAME_PRINTF;

    static const std::string NAME_ASYNC_WORK_GROUP_COPY;
    static const std::string NAME_WAIT_GROUP_EVENTS;
    static const std::string NAME_PREFETCH;
    static const std::string NAME_ASYNC_WORK_GROUP_STRIDED_COPY;

  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __COMPILATION_UTILS_H__