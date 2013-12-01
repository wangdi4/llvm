/*****************************************************************************\

Copyright (c) Intel Corporation (2013).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  CPUBlockToKernelMapper.cpp

\*****************************************************************************/
#define DEBUG_TYPE "cpublocktokernelmapper"

#include "Program.h"
#include "CPUProgram.h"
#include "Kernel.h"
#include "CPUBlockToKernelMapper.h"
#include "BlockUtils.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include <assert.h>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  CPUBlockToKernelMapper::CPUBlockToKernelMapper(Program *pProgram, const llvm::Module* pModule)
  {
    DEBUG(llvm::dbgs() << "Entry point CPUBlockToKernelMapper ctor \n");
    DEBUG(llvm::dbgs() << pProgram->GetKernelsCount() << " kernels in program \n");
    assert(pProgram && "Program is NULL");
    assert(pModule && "Module is NULL");


    // loop over kernels in program
    for (int cnt = 0; cnt < pProgram->GetKernelsCount(); ++cnt){
      // obtain kernel
      const ICLDevBackendKernel_ * pIKernel;
      pProgram->GetKernel(cnt, &pIKernel);
      const Kernel * pKernel = static_cast<const Kernel *>(pIKernel);

      // detect block
      if(!pKernel->GetKernelProporties()->IsBlock())
        continue;
      // get block_invoke function name
      std::string BlockInvokeName = 
        BlockUtils::ObtainBlockInvokeFuncNameFromKernel(pKernel->GetKernelName());
      DEBUG(llvm::dbgs() << "Found block_invoke kernel " 
        << pKernel->GetKernelName() << " \n");
      // get llvm function for block_invoke 
      llvm::Function *pBlockInvokeFunc = pModule->getFunction(BlockInvokeName);
      assert(pBlockInvokeFunc && "Cannot find original block_invoke func in module");

      DEBUG(llvm::dbgs() << "Found original block_invoke function " << BlockInvokeName << " \n");

      // obtain CPUProgram
      CPUProgram * pCpuProgram = static_cast<CPUProgram*>(pProgram);
      // obtain entry point of block_invoke function
      const void * entry = pCpuProgram->GetPointerToFunction(pBlockInvokeFunc);
      assert(entry && "pointer to JIT of block_invoke is NULL");
      // insert pair <key, Kernel object)
      m_map[entry] = pKernel;

    } // for (int cnt = 0; cnt < pProgram->GetKernelsCount(); ++cnt)
    
    DEBUG(llvm::dbgs() << "map <key, Kernel object> has " << m_map.size() << " elements \n");
  }
  
  const ICLDevBackendKernel_ * CPUBlockToKernelMapper::Map(const void * key) const
  {
    assert(m_map.find(key) != m_map.end() && 
      "CPUBlockToKernelMapper not found key in map. Key must be in map");
    return m_map.find(key)->second;
  }


}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {
