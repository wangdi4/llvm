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

File Name:  CompilationUtils.h

\*****************************************************************************/

#ifndef __COMPILATION_UTILS_H__
#define __COMPILATION_UTILS_H__

#include "cpu_dev_limits.h"
#include "cl_device_api.h"
#include "exceptions.h"

#include "llvm/Module.h"
#include "llvm/Function.h"

#include <set>
#include <vector>
#include <map>

using namespace llvm;

const unsigned int BYTE_SIZE = 8;

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  DEFINE_EXCEPTION(CompilerException)

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
                    
    /// @brief collect all scalar kernel functions
    /// @param functionSet container to insert all scalar kernel function into
    /// @param pModule the module to search kernel function inside
    static void getAllScalarKernels(std::set<Function*> &functionSet, Module *pModule);

    /// @brief  fills a vector of cl_kernel_argument with arguments representing pFunc's
    ///         OpenCL level arguments
    /// @param pModule    The module
    /// @param pFunc      The kernel for which to create argument vector
    /// @param args       The args string ectracted from pFunc metadata
    /// @param arguments  OUT param, the cl_kernel_argument which represent pFunc's
    ///                   OpenCL level argument
    static void parseKernelArguments( Module* pModule, 
                                      Function* pFunc, 
                                      const std::string& args,
                                      std::vector<cl_kernel_argument>& /* OUT */ arguments);
    
    /// @brief  maps between kernels (both scalar and vectorized) and their metdata
    /// @param pModule          The module
    /// @param pVectFunctions   The vectorized kernels, these kernel should be mapped
    ///                         to their scalar version metadata
    /// @param pVectFunctions   OUT param, maps between kernels (both scalar and
    ///                         vectorized) and their metdata
    static void getKernelsMetadata( Module* pModule, 
                                    const SmallVectorImpl<Function*>& pVectFunctions, 
                                    std::map<Function*, MDNode*>& /* OUT */ kernelMetadata);

  public:
    /// This holds the number of implicite arguments addeded to function
    static const unsigned int NUMBER_IMPLICIT_ARGS;

    /// '3' is a magic number for global variables
    /// that were in origin kernel local variable!
    static const unsigned int LOCL_VALUE_ADDRESS_SPACE;

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