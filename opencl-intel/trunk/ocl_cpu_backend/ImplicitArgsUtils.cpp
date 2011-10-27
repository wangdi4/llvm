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

File Name:  ImplicitArgsUtils.cpp

\*****************************************************************************/

#include "ImplicitArgsUtils.h"
#include "cpu_dev_limits.h"
#include "cl_device_api.h"

#include <cstring>


namespace Intel { namespace OpenCL { namespace DeviceBackend {

// Initialize the implicit arguments properties
const std::vector<ImplicitArgProperties> ImplicitArgsUtils::m_implicitArgProps = ImplicitArgsUtils::initArgPropsVector();

std::vector<ImplicitArgProperties> ImplicitArgsUtils::initArgPropsVector() {
  
  std::vector<ImplicitArgProperties> implicitArgProps;
  
  // Add argument name, size, alignment (if different from size)
  implicitArgProps.push_back(ImplicitArgProperties("pLocalMem",       sizeof(void*)));
  implicitArgProps.push_back(ImplicitArgProperties("pWorkDim",        sizeof(sWorkInfo*),       0));
  implicitArgProps.push_back(ImplicitArgProperties("pWGId",           sizeof(size_t*)));
  implicitArgProps.push_back(ImplicitArgProperties("BaseGlbId",       sizeof(void*)));
  implicitArgProps.push_back(ImplicitArgProperties("pLocalIds",       sizeof(void*)));
  implicitArgProps.push_back(ImplicitArgProperties("contextpointer",  sizeof(void*)));
  implicitArgProps.push_back(ImplicitArgProperties("iterCount",       sizeof(size_t)));
  implicitArgProps.push_back(ImplicitArgProperties("pSpecialBuf",     sizeof(void*)));
  implicitArgProps.push_back(ImplicitArgProperties("pCurrWI",         sizeof(unsigned int*)));
  
  return implicitArgProps;
}

void ImplicitArgsUtils::createImplicitArgs(char* pDest, std::vector<ImplicitArgument>& /* OUT */ implicitArgs) {
  
  // Start from the beginning of the given dest buffer
  char* pArgValueDest = pDest;
  
  // go over all implicit arguments' properties
  for(std::vector<ImplicitArgProperties>::const_iterator implicitArgIterator = m_implicitArgProps.begin(), 
        e = m_implicitArgProps.end(); 
          implicitArgIterator != e; ++implicitArgIterator) {
        
    ImplicitArgProperties implicitArgProps = *implicitArgIterator;
    
    // Create implicit argument pointing at the right place in the dest buffer
    ImplicitArgument arg(pArgValueDest, implicitArgProps);
    
    implicitArgs.push_back(arg);
    
    // Advance the dest buffer according to argument's size and alignment
    pArgValueDest += arg.getAlignedSize();
  }
}

void ImplicitArgsUtils::setImplicitArgsPerBinary(
                         std::vector<ImplicitArgument>& implicitArgument, 
                         const Binary* pBinary) {
}

void ImplicitArgsUtils::setImplicitArgsPerExecutable(
                         std::vector<ImplicitArgument>& implicitArgument, 
                         const Executable* pExecutable, 
                         void* *pLocalMemoryBuffers, 
                         void* pWGStackFrame, 
                         unsigned int uiWICount) {
  
   // TODO : remove hard coded numbers, map implicit arg and index
   // TODO : refactor this code - try to get rid of pWGStackFrame
   
  void* pLocalMemArg;
  // Set implicit local buffer pointer
  if ( pExecutable->m_pBinary->GetImplicitLocalMemoryBufferSize() ) {
    // Is the next buffer after explicit locals
    pLocalMemArg = pLocalMemoryBuffers[pExecutable->m_pBinary->m_kernelLocalMem.size()];
  } else {
    //Initialize an easily identifiable junk address to catch uninitialized memory accesses
    pLocalMemArg = (void *)0x000DEAD0;
  }
  implicitArgument[0].setValue(reinterpret_cast<char *>(&pLocalMemArg));

  // Set Work Dimension Info pointer
  const sWorkInfo* pWorkInfo = &(pExecutable->m_pBinary->m_WorkInfo);
  implicitArgument[1].setValue(const_cast<char *>(reinterpret_cast<const char *>(&pWorkInfo)));

  // Leave space for WorkGroup id
  // WorkGroup id should be initialized per WorkGroup and not per Executable
  // implicitArgument[2]

  // Set Global id to (0,0,0,0)
  memset(const_cast<char *>(reinterpret_cast<const char*>(&(pExecutable->m_GlobalId[0]))), 0, CPU_MAX_WI_DIM_POW_OF_2 * sizeof(size_t));
  const size_t* pGlobalId = &(pExecutable->m_GlobalId[0]);
  implicitArgument[3].setValue(const_cast<char *>(reinterpret_cast<const char *>(&pGlobalId)));

  // Initialize and Set Local ids
  size_t* pWIids = (size_t*)(((char*)pWGStackFrame) + pExecutable->m_pBinary->GetAlignedKernelParametersSize());
  initWILocalIds(pExecutable, pWIids);
  implicitArgument[4].setValue(reinterpret_cast<char *>(&pWIids));

  // Setup Context pointer
  implicitArgument[5].setValue(const_cast<char *>(reinterpret_cast<const char *>(&pExecutable)));

  // Setup iterCount
  assert( uiWICount > 0 && "uiWICount is zero!" );
  size_t iterCounter = uiWICount - 1;
  implicitArgument[6].setValue(reinterpret_cast<char *>(&iterCounter)); /*set iter count*/;

  char* pPrivateBuffer  = ((char*)pWGStackFrame) + 
    pExecutable->m_pBinary->GetAlignedKernelParametersSize() + pExecutable->m_pBinary->GetLocalWIidsSize();
  // Setup pPrivateBuffer 
  implicitArgument[7].setValue(reinterpret_cast<char *>(&pPrivateBuffer)) ; /*set pSB*/;

  // Setup pCurrWI
  const unsigned int * pCurrWI = &(pExecutable->m_CurrWI);
  implicitArgument[8].setValue(const_cast<char *>(reinterpret_cast<const char *>(&pCurrWI))) /*set pCurrWI*/;

}

void ImplicitArgsUtils::setImplicitArgsPerWG(std::vector<ImplicitArgument>& implicitArgument, void* pParams) {
  
  // TODO : remove hard coded numbers, map implicitarg and index
  implicitArgument[2].setValue(reinterpret_cast<char *>(&pParams));
}

void ImplicitArgsUtils::initWILocalIds(const Executable* pExecutable, size_t* pWIids) {
  // Initialize local id buffer
  const Binary* m_pBinary = pExecutable->m_pBinary;
  size_t uiVectWidth = m_pBinary->m_bVectorized ? m_pBinary->m_uiVectorWidth : 1;
  switch (m_pBinary->m_WorkInfo.uiWorkDim) {
  case 1:
    for ( size_t i=0, j=0;(i + uiVectWidth - 1)<m_pBinary->m_WorkInfo.LocalSize[0];i+=uiVectWidth, j++ ) {
      pWIids[CPU_MAX_WI_DIM_POW_OF_2*j+0] = i;
      //Must initialize dimensions y and z to zero for OOB handling
      pWIids[CPU_MAX_WI_DIM_POW_OF_2*j+1] = 0;
      pWIids[CPU_MAX_WI_DIM_POW_OF_2*j+2] = 0;
    }
    break;
  case 2:
    {
      size_t strideVec = m_pBinary->m_WorkInfo.LocalSize[0]/uiVectWidth;
      for ( size_t y=0; y<m_pBinary->m_WorkInfo.LocalSize[1]; ++y ) {
        for ( size_t x=0, j=0; (x + uiVectWidth - 1)<m_pBinary->m_WorkInfo.LocalSize[0]; x+=uiVectWidth, j++ ) {
          pWIids[CPU_MAX_WI_DIM_POW_OF_2*(j+y*strideVec)+0] = x;
          pWIids[CPU_MAX_WI_DIM_POW_OF_2*(j+y*strideVec)+1] = y;
          //Must initialize dimension z to zero for OOB handling
          pWIids[CPU_MAX_WI_DIM_POW_OF_2*(j+y*strideVec)+2] = 0;
        }
      }
    }
    break;
  case 3:
    {
      size_t strideVec1 = m_pBinary->m_WorkInfo.LocalSize[0]/uiVectWidth;
      size_t strideVec2 = strideVec1*m_pBinary->m_WorkInfo.LocalSize[1];
      for ( size_t z=0;z<m_pBinary->m_WorkInfo.LocalSize[2];++z )
        for ( size_t y=0;y<m_pBinary->m_WorkInfo.LocalSize[1];++y )
          for ( size_t x=0, j=0; (x + uiVectWidth - 1)<m_pBinary->m_WorkInfo.LocalSize[0]; x+=uiVectWidth, j++ ) {
            pWIids[CPU_MAX_WI_DIM_POW_OF_2*(j+y*strideVec1+z*strideVec2)+0] = x;
            pWIids[CPU_MAX_WI_DIM_POW_OF_2*(j+y*strideVec1+z*strideVec2)+1] = y;
            pWIids[CPU_MAX_WI_DIM_POW_OF_2*(j+y*strideVec1+z*strideVec2)+2] = z;
          }
    }
    break;
  default:
    assert(false);
  }
}


}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {