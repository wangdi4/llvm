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
#include "ExecutionContext.h"
#include "cpu_dev_limits.h"

#include <assert.h>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

// Initialize the implicit arguments properties
ImplicitArgProperties ImplicitArgsUtils::m_implicitArgProps[m_numberOfImplicitArgs] = {
  {"pLocalMem",       sizeof(void*),  sizeof(void*)},
  {"pWorkDim",        sizeof(void*),  sizeof(void*)},
  {"pWGId",           sizeof(void*),  sizeof(void*)},
  {"BaseGlbId",       sizeof(void*),  sizeof(void*)},
  {"contextpointer",  sizeof(void*),  sizeof(void*)},
  {"pLocalIds",       sizeof(void*),  sizeof(void*)},
  {"iterCount",       sizeof(size_t), sizeof(size_t)},
  {"pSpecialBuf",     sizeof(void*),  sizeof(void*)},
  {"pCurrWI",         sizeof(void*),  sizeof(void*)}
};

const ImplicitArgProperties& ImplicitArgsUtils::getImplicitArgProps(unsigned int arg) {
  assert(arg < m_numberOfImplicitArgs && "arg is bigger than implicit args number");
  return m_implicitArgProps[arg]; 
}

void ImplicitArgsUtils::createImplicitArgs(char* pDest) {
  
  // Start from the beginning of the given dest buffer
  char* pArgValueDest = pDest;
  
  // go over all implicit arguments' properties
  for(unsigned int i=0; i<m_numberOfImplicitArgs; ++i) {
    // Create implicit argument pointing at the right place in the dest buffer
    ImplicitArgument arg(pArgValueDest, m_implicitArgProps[i]);
    m_implicitArgs[i] = arg;
    // Advance the dest buffer according to argument's size and alignment
    pArgValueDest += arg.getAlignedSize();
  }
}

void ImplicitArgsUtils::setImplicitArgsPerExecutable(
                         void* pLocalMemoryBuffer,
                         const sWorkInfo* pWorkInfo,
                         const size_t* pGlobalBaseId,
                         const CallbackContext* pCallBackContext, 
                         bool bJitCreateWIids,
                         unsigned int packetWidth,
                         size_t* pWIids,
                         const size_t iterCounter,
                         char* pBarrierBuffer,
                         size_t* pCurrWI) {
  
  // Set implicit local buffer pointer
  m_implicitArgs[IA_SLM_BUFFER].setValue(reinterpret_cast<const char *>(&pLocalMemoryBuffer));

  // Set Work Dimension Info pointer
  m_implicitArgs[IA_WORK_GROUP_INFO].setValue(reinterpret_cast<const char *>(&pWorkInfo));

  // Leave space for WorkGroup id
  // WorkGroup id should be initialized per WorkGroup and not per Executable
  // m_implicitArgs[IA_WORK_GROUP_ID]

  // Set Global id to (0,0,0,0)
  m_implicitArgs[IA_GLOBAL_BASE_ID].setValue(reinterpret_cast<const char *>(&pGlobalBaseId));

  // Setup Context pointer
  m_implicitArgs[IA_CALLBACK_CONTEXT].setValue(reinterpret_cast<const char *>(&pCallBackContext));

  // Initialize Barrier WI ids variables only if jit is not creating the ids.
  if (!bJitCreateWIids) {
    // Initialize and Set Local ids
    initWILocalIds(pWorkInfo, packetWidth, pWIids);
    m_implicitArgs[IA_LOCAL_ID_BUFFER].setValue(reinterpret_cast<const char *>(&pWIids));

    // Setup iterCount
    m_implicitArgs[IA_LOOP_ITER_COUNT].setValue(reinterpret_cast<const char *>(&iterCounter)); /*set iter count*/;

    // Setup pPrivateBuffer 
    m_implicitArgs[IA_BARRIER_BUFFER].setValue(reinterpret_cast<const char *>(&pBarrierBuffer)) ; /*set pSB*/;

    // Setup pCurrWI
    m_implicitArgs[IA_CURRENT_WORK_ITEM].setValue(reinterpret_cast<const char *>(&pCurrWI)) /*set pCurrWI*/;
  }
}

void ImplicitArgsUtils::setImplicitArgsPerWG(const void* pParams) {
  m_implicitArgs[IA_WORK_GROUP_ID].setValue(reinterpret_cast<const char *>(&pParams));

  const sWorkInfo* pWorkInfo = (const sWorkInfo*)m_implicitArgs[IA_WORK_GROUP_INFO].getValue();
  size_t* pGlobalBaseId = (size_t*)m_implicitArgs[IA_GLOBAL_BASE_ID].getValue();
  size_t* pGroupId = (size_t*)pParams;
  pGlobalBaseId[0] = pGroupId[0]*pWorkInfo->LocalSize[0] + pWorkInfo->GlobalOffset[0];

  if (pWorkInfo->uiWorkDim > 1 ) {
    pGlobalBaseId[1] = pGroupId[1]*pWorkInfo->LocalSize[1] + pWorkInfo->GlobalOffset[1];
  }

  if (pWorkInfo->uiWorkDim > 2 ) {
    pGlobalBaseId[2] = pGroupId[2]*pWorkInfo->LocalSize[2] + pWorkInfo->GlobalOffset[2];
  }
}

void ImplicitArgsUtils::initWILocalIds(const sWorkInfo* pWorkInfo, const unsigned int packetWidth, size_t* pWIids) {
  // Initialize local id buffer
  switch (pWorkInfo->uiWorkDim) {
  case 1:
    for ( size_t i=0, j=0;(i + packetWidth - 1)<pWorkInfo->LocalSize[0];i+=packetWidth, j++ ) {
      pWIids[CPU_MAX_WI_DIM_POW_OF_2*j+0] = i;
      //Must initialize dimensions y and z to zero for OOB handling
      pWIids[CPU_MAX_WI_DIM_POW_OF_2*j+1] = 0;
      pWIids[CPU_MAX_WI_DIM_POW_OF_2*j+2] = 0;
    }
    break;
  case 2:
    {
      size_t strideVec = pWorkInfo->LocalSize[0]/packetWidth;
      for ( size_t y=0; y<pWorkInfo->LocalSize[1]; ++y ) {
        for ( size_t x=0, j=0; (x + packetWidth - 1)<pWorkInfo->LocalSize[0]; x+=packetWidth, j++ ) {
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
      size_t strideVec1 = pWorkInfo->LocalSize[0]/packetWidth;
      size_t strideVec2 = strideVec1*pWorkInfo->LocalSize[1];
      for ( size_t z=0;z<pWorkInfo->LocalSize[2];++z )
        for ( size_t y=0;y<pWorkInfo->LocalSize[1];++y )
          for ( size_t x=0, j=0; (x + packetWidth - 1)<pWorkInfo->LocalSize[0]; x+=packetWidth, j++ ) {
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
