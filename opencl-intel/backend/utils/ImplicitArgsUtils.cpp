/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "ImplicitArgsUtils.h"
#include "CompilationUtils.h"

#ifndef __APPLE__
#include "ExecutionContext.h"
#endif

#include <assert.h>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

struct ArgData {
  const char *name;
  bool        initByWrapper;
};

const ArgData impArgs[ImplicitArgsUtils::IA_NUMBER] = {  
  {"pLocalMemBase",   true }, //IA_SLM_BUFFER,
  {"pWorkDim",        false}, //IA_WORK_GROUP_INFO
  {"pWGId",           true},  //IA_WORK_GROUP_ID
  {"BaseGlbId",       true},  //IA_GLOBAL_BASE_ID
  //TODO: Remove this #ifndef when apple no longer pass barrier memory buffer
#ifndef __APPLE__
  {"pSpecialBuf",     true },
#else
  {"pSpecialBuf",     false },
#endif
  {"pCurrWI",         true},  //IA_CURRENT_WORK_ITEM
  {"RuntimeContext",  true}}; //IA_RUNTIME_CONTEXT

const char* ImplicitArgsUtils::getArgName(unsigned Idx) {
    //TODO: maybe we don't need impargs?
    assert(Idx < NUMBER_IMPLICIT_ARGS);
    return impArgs[Idx].name;
}
// Initialize the implicit arguments properties
ImplicitArgProperties ImplicitArgsUtils::m_implicitArgProps[NUMBER_IMPLICIT_ARGS];
bool ImplicitArgsUtils::m_initialized = false;

const ImplicitArgProperties& ImplicitArgsUtils::getImplicitArgProps(unsigned int arg) {
  assert(arg < NUMBER_IMPLICIT_ARGS && "arg is bigger than implicit args number");
  assert(!m_implicitArgProps[arg].m_bInitializedByWrapper &&
    "arg is initialized by wrapper no need for Props!");
  assert(m_initialized);
  return m_implicitArgProps[arg]; 
}

void ImplicitArgsUtils::initImplicitArgProps(unsigned int SizeT) {
  for(unsigned int i=0; i<NUMBER_IMPLICIT_ARGS; ++i) {
    switch (i) {
    case IA_WORK_GROUP_INFO:
      m_implicitArgProps[i].m_size = sizeof(cl_uniform_kernel_args);
      break;
    default:
      m_implicitArgProps[i].m_size = SizeT;
      break;
    }
    m_implicitArgProps[i].m_name = impArgs[i].name;
    m_implicitArgProps[i].m_alignment = SizeT;
    m_implicitArgProps[i].m_bInitializedByWrapper = impArgs[i].initByWrapper;
  }
  m_initialized = true;
}

#ifndef __APPLE__
void ImplicitArgsUtils::createImplicitArgs(char* pDest) {
  
  // Start from the beginning of the given dest buffer
  char* pArgValueDest = pDest;

   // go over all implicit arguments' properties
  for(unsigned int i=0; i<NUMBER_IMPLICIT_ARGS; ++i) {
    // Only implicit arguments that are not initialized by the wrapper
    // Should be loaded from the parameter structutre.
    if(!m_implicitArgProps[i].m_bInitializedByWrapper) {
      // Create implicit argument pointing at the right place in the dest buffer
      ImplicitArgument arg(pArgValueDest, m_implicitArgProps[i]);
      m_implicitArgs[i] = arg;
      // Advance the dest buffer according to argument's size and alignment
      pArgValueDest += arg.getAlignedSize();
    }
  }
}

void ImplicitArgsUtils::setImplicitArgsPerExecutable(
                         const sWorkInfo* pWorkInfo,
                         const size_t* pGlobalBaseId,
                         const CallbackContext* pCallBackContext, 
                         bool bJitCreateWIids,
                         unsigned int packetWidth,
                         size_t* pWIids,
                         const size_t iterCounter,
                         const ExtendedExecutionContext* 
                                pCallbackExtendedExecutionContext) {
  
  // Set Work Dimension Info pointer
  m_implicitArgs[IA_WORK_GROUP_INFO].setValue(reinterpret_cast<const char *>(&pWorkInfo));

  // Leave space for WorkGroup id
  // WorkGroup id should be initialized per WorkGroup and not per Executable
  // m_implicitArgs[IA_WORK_GROUP_ID]

  // Set Global id to (0,0,0,0)
  m_implicitArgs[IA_GLOBAL_BASE_ID].setValue(reinterpret_cast<const char *>(&pGlobalBaseId));

  // Initialize Barrier WI ids variables only if jit is not creating the ids.
  if (!bJitCreateWIids) {
    // Initialize and Set Local ids
    initWILocalIds(pWorkInfo->uiWorkDim,pWorkInfo->LocalSize, packetWidth, pWIids);
    //m_implicitArgs[IA_LOCAL_ID_BUFFER].setValue(reinterpret_cast<const char *>(&pWIids));
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

void ImplicitArgsUtils::initWILocalIds(size_t dim, const size_t *pLocalSize, const unsigned int packetWidth, size_t* pWIids) {
  // Initialize local id buffer
  assert(pWIids);
  assert(pLocalSize);
  switch (dim) {
  case 1:
    for ( size_t i=0, j=0;(i + packetWidth - 1)<pLocalSize[0];i+=packetWidth, j++ ) {
      pWIids[MAX_WI_DIM_POW_OF_2*j+0] = i;
      //Must initialize dimensions y and z to zero for OOB handling
      pWIids[MAX_WI_DIM_POW_OF_2*j+1] = 0;
      pWIids[MAX_WI_DIM_POW_OF_2*j+2] = 0;
    }
    break;
  case 2:
    {
      size_t strideVec = pLocalSize[0]/packetWidth;
      for ( size_t y=0; y<pLocalSize[1]; ++y ) {
        for ( size_t x=0, j=0; (x + packetWidth - 1)<pLocalSize[0]; x+=packetWidth, j++ ) {
          pWIids[MAX_WI_DIM_POW_OF_2*(j+y*strideVec)+0] = x;
          pWIids[MAX_WI_DIM_POW_OF_2*(j+y*strideVec)+1] = y;
          //Must initialize dimension z to zero for OOB handling
          pWIids[MAX_WI_DIM_POW_OF_2*(j+y*strideVec)+2] = 0;
        }
      }
    }
    break;
  case 3:
    {
      size_t strideVec1 = pLocalSize[0]/packetWidth;
      size_t strideVec2 = strideVec1*pLocalSize[1];
      for ( size_t z=0;z<pLocalSize[2];++z )
        for ( size_t y=0;y<pLocalSize[1];++y )
          for ( size_t x=0, j=0; (x + packetWidth - 1)<pLocalSize[0]; x+=packetWidth, j++ ) {
            pWIids[MAX_WI_DIM_POW_OF_2*(j+y*strideVec1+z*strideVec2)+0] = x;
            pWIids[MAX_WI_DIM_POW_OF_2*(j+y*strideVec1+z*strideVec2)+1] = y;
            pWIids[MAX_WI_DIM_POW_OF_2*(j+y*strideVec1+z*strideVec2)+2] = z;
          }
    }
    break;
  default:
    assert(false);
  }
}
#endif

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {
