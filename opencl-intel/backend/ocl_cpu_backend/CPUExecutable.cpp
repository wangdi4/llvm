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

File Name:  CPUExecutable.cpp

\*****************************************************************************/

#include "CPUExecutable.h"
#include "TargetArch.h"
#include "Binary.h"
#include "cl_types.h"

#if defined(_M_X64) || defined(__LP64__)
// defined in kernel_execute[att].asm
extern "C" void Emit_VZeroUpper(void);
#else

#ifdef _WIN32
static void Emit_VZeroUpper(void) {
  __asm
  {
    //vzeroupper
#if _MSC_VER < 1600
    _emit 197
    _emit 248
    _emit 119
#else
    vzeroupper
#endif
  }
}
#else //_WIN32
#error Unsupported target
#endif
#endif

namespace Intel { namespace OpenCL { namespace DeviceBackend {

CPUExecutable::CPUExecutable(const Binary* pBin):
    Executable(pBin)
{
    m_hasAVX1 = (Intel::CFS_AVX1 & pBin->GetCpuFeatures()) != 0;
    m_hasAVX2 = (Intel::CFS_AVX2 & pBin->GetCpuFeatures()) != 0;
}

// Prepares current thread for the executable execution
cl_dev_err_code CPUExecutable::PrepareThread() 
{
    // Use vzeroupper to avoid the AVX state transition penalty.
    if (  m_hasAVX1 && !m_hasAVX2 ) 
	  {
        Emit_VZeroUpper();
    }
  
    m_uiMXCSRstate = _mm_getcsr();
    unsigned int uiNewFlags = (m_uiMXCSRstate & ~m_uiCSRMask) | m_uiCSRFlags;
    _mm_setcsr( uiNewFlags);
    return CL_DEV_SUCCESS;
}

// Restores Thread state as it was before the execution
cl_dev_err_code CPUExecutable::RestoreThreadState() {
    // Use vzeroupper to avoid the AVX state transition penalty.
    if ( m_hasAVX1 && !m_hasAVX2 ) 
	  {
        Emit_VZeroUpper();
    }
  
    _mm_setcsr( m_uiMXCSRstate);
    return CL_DEV_SUCCESS;
}

}}} // namespace
