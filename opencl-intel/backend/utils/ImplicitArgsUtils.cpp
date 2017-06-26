/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "ImplicitArgsUtils.h"
#include "CompilationUtils.h"

#include "ExecutionContext.h"

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
  {"pSpecialBuf",     true },
  {"RuntimeHandle",  true}}; //IA_RUNTIME_HANDLE

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

size_t ImplicitArgsUtils::getAdjustedAlignment(size_t offset, size_t ST) {
  // Implicit args will be aligned to size_t to allow KNC VBROADCAST's on size_t values
  return ((offset + ST - 1) / ST) * ST;
}

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {
