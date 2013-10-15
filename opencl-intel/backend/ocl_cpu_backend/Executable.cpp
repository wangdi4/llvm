/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  Executable.cpp

\*****************************************************************************/

#include "Executable.h"
#include "Binary.h"
#include "TypeAlignment.h"
#include "cl_device_api.h"
#include "cl_types.h"

#include <string.h>
#include <algorithm>
#include <assert.h>

using namespace Intel::OpenCL::DeviceBackend;

#ifdef _WIN32
#define FORCE_INLINE_PRE __forceinline
#define FORCE_INLINE_POST 
#else
#define FORCE_INLINE_PRE inline
#define FORCE_INLINE_POST  __attribute__((always_inline))
#endif

#if defined(ENABLE_SDE)
// These functions are used as marks for the debug trace of the JIT execution
extern "C" 
{
void BeforeExecution() { }
void AfterExecution() { }
}
#endif // ENABLE_SDE

Executable::Executable(const Binary* pBin) :
  m_pBinary(pBin), m_pParameters(NULL), m_stParamSize(0),
  m_uiCSRMask(0), m_uiCSRFlags(0)
{    
}

Executable::~Executable() {
}

void Executable::Release()
{
    delete this;
}

// Initialize context to use specific binary with specific number of WorkItems 
cl_dev_err_code Executable::Init(void* *pLocalMemoryBuffers, void* pWGStackFrame, const ICLDevBackendBinary_* pBin) {

  m_pBinary = static_cast<const Binary*>(pBin);

  return Init(pLocalMemoryBuffers, pWGStackFrame, m_pBinary->m_uiWGSize);
}

// Initialize context to with specific number of WorkItems 
cl_dev_err_code Executable::Init(void* *pLocalMemoryBuffers, void* pWGStackFrame, unsigned int uiWICount) {

  // Clear FP configuration flags
  m_uiCSRMask = 0;
  m_uiCSRFlags = 0;
  
  m_DAZ = m_pBinary->GetDAZ();
  // Initialize callback context with device buffer printer from binary
  m_callbackContext.SetDevicePrinter(m_pBinary->GetDevicePrinter());

  //Initialize Kernel parameters
  m_stParamSize = m_pBinary->GetKernelParametersSize();
  // pBuffers that is passed to the wrapper kernel is assumed to be aligned to TypeAlignment::MAX_ALIGNMENT
  m_pParameters = TypeAlignment::align(TypeAlignment::MAX_ALIGNMENT, (char*)pWGStackFrame);

  // Copy parameters to context
  std::copy((char*)m_pBinary->GetFormalParameters(), (char*)m_pBinary->GetFormalParameters() + m_pBinary->GetFormalParametersSize(), m_pParameters);
  //memcpy( m_pParameters, m_pBinary->GetFormalParameters(), m_pBinary->GetFormalParametersSize());

  char* pWIParams = m_pParameters + m_pBinary->GetFormalParametersSize();
  m_implicitArgsUtils.initImplicitArgProps(m_pBinary->GetPointerSize());
  m_implicitArgsUtils.createImplicitArgs(pWIParams);
  // Set implicit local buffer pointer
  memset(&m_GlobalId[0], 0, sizeof(m_GlobalId));
  size_t* pWIids = (size_t*)(((char*)pWGStackFrame) + m_pBinary->GetAlignedKernelParametersSize());
  assert( (m_pBinary->m_bJitCreateWIids || uiWICount > 0) && "uiWICount is zero!" );
  // Set implicit arguments per executable
  m_implicitArgsUtils.setImplicitArgsPerExecutable(
            &m_pBinary->m_WorkInfo,
            &m_GlobalId[0],
            &m_callbackContext,
            m_pBinary->m_bJitCreateWIids,
            m_pBinary->m_uiVectorWidth,
            pWIids,
            uiWICount-1,
            m_pBinary->GetExtendedExecutionContext()
            );


  // Set CSR flags
  m_uiCSRMask |= _MM_FLUSH_ZERO_MASK;
  m_uiCSRMask |= _MM_DENORMALS_ZERO_MASK;

  if ( m_DAZ ) {
    m_uiCSRFlags |= _MM_FLUSH_ZERO_ON;     // OFF is default
    m_uiCSRFlags |= _MM_DENORMALS_ZERO_ON; // OFF is default
  }
  m_uiCSRMask |= _MM_ROUND_MASK;
  m_uiCSRFlags |= _MM_ROUND_NEAREST;  // Default

  return CL_DEV_SUCCESS;
}

// usage of the function forward declaration prior to the function definition is because "always_inline" attribute cannot appear with definition 
FORCE_INLINE_PRE static void InvokeKernel(size_t params_size, void* pParameters, void* pEntryPoint) FORCE_INLINE_POST;
static void InvokeKernel(size_t params_size, void* pParameters, void* pEntryPoint)
{
  assert(params_size == sizeof(void*) && "invalid number of arguments");
  // This union is used to convert between the parameter and the 
  // function pointer because the c++ standard forbids the casting of the two 
  // and as a result, some versions of g++ fail with a warning.
  union uk {
    void *(*kernel)(void *);
    const void* ptr; 
  };
  uk kernel;
  // Set the entry point
  kernel.ptr = pEntryPoint;
  // Call the kernel
  kernel.kernel(pParameters);
}

cl_dev_err_code Executable::Execute( const size_t* IN pGroupId,
                                   const size_t* IN pLocalOffset, 
                                   const size_t* IN pItemsToProcess) {

  // Set implicit arguments per work-group
  m_implicitArgsUtils.setImplicitArgsPerWG(reinterpret_cast<const void *>(pGroupId));
  
  // clear set checking if async_wg_copy built-ins were executed 
  m_callbackContext.Reset();

  assert( (m_stParamSize < (1 << 12)) && "Parameters on stack size is more than 4K!" );
#if defined(ENABLE_SDE)
  BeforeExecution();
#endif
  // The kernel has a single parameter &m_pParameters - a pointer to the buffers containg all the arguments
  // This parameter is of size sizeof(void*)
  InvokeKernel(sizeof(void*), m_pParameters, const_cast<void*>(m_pBinary->m_pUsedEntryPoint));
#if defined(ENABLE_SDE)
  AfterExecution();
#endif
  
  return CL_DEV_SUCCESS;
}
