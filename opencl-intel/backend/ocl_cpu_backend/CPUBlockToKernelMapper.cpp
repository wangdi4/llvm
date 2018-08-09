// INTEL CONFIDENTIAL
//
// Copyright 2013-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#define DEBUG_TYPE "cpublocktokernelmapper"

#include "Program.h"
#include "CPUProgram.h"
#include "Kernel.h"
#include "CPUBlockToKernelMapper.h"
#include "exceptions.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include <assert.h>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  CPUBlockToKernelMapper::CPUBlockToKernelMapper(Program *pProgram, const llvm::Module* pModule)
  {
    assert(pModule && "Module is NULL");
    assert(pProgram && "Program is NULL");
    LLVM_DEBUG(llvm::dbgs() << "Entry point CPUBlockToKernelMapper ctor \n");
    LLVM_DEBUG(llvm::dbgs() << pProgram->GetKernelsCount() << " kernels in program \n");


    // loop over kernels in program
    for (int cnt = 0; cnt < pProgram->GetKernelsCount(); ++cnt){
      // obtain kernel
      const ICLDevBackendKernel_ * pIKernel;
      pProgram->GetKernel(cnt, &pIKernel);
      const Kernel * pKernel = static_cast<const Kernel *>(pIKernel);

      // detect block
      if(!pKernel->GetKernelProporties()->IsBlock())
        continue;
      // get llvm function for block_invoke
      llvm::Function *pBlockInvokeFunc =
        pModule->getFunction(pKernel->GetKernelName());
      assert(pBlockInvokeFunc &&
             "Cannot find block invoke kernel in the module");

      // obtain CPUProgram
      CPUProgram * pCpuProgram = static_cast<CPUProgram*>(pProgram);
      // obtain entry point of block_invoke function
      const void * entry = pCpuProgram->GetPointerToFunction(pBlockInvokeFunc);
      assert(entry && "pointer to JIT of block_invoke is NULL");
      // insert pair <key, Kernel object)
      m_map[entry] = pKernel;

    } // for (int cnt = 0; cnt < pProgram->GetKernelsCount(); ++cnt)
    
    LLVM_DEBUG(llvm::dbgs() << "map <key, Kernel object> has " << m_map.size() << " elements \n");
  }
  
  const ICLDevBackendKernel_ * CPUBlockToKernelMapper::Map(const void * key) const
  {
    std::map<const void *, const ICLDevBackendKernel_ *>::const_iterator it = m_map.find(key);
    assert(it != m_map.end() && 
      "CPUBlockToKernelMapper not found key in map. Key must be in map");
    if(it == m_map.end())
      throw Exceptions::DeviceBackendExceptionBase(
        std::string("CPUBlockToKernelMapper not found key in map. Key must be in map"));
    return it->second;
  }
}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {
