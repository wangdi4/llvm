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

File Name:  CPUKernel.cpp

\*****************************************************************************/

#include "CPUKernel.h"
#include "KernelProperties.h"

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
#elif defined(__ANDROID__)
static void Emit_VZeroUpper(void) {}
#else
#error Unsupported target
#endif
#endif

CPUKernel::CPUKernel():
    Kernel(),
    m_hasAVX1(false),
    m_hasAVX2(false)
    { }

CPUKernel::CPUKernel(const std::string& name,
    const std::vector<cl_kernel_argument>& args,
    const std::vector<unsigned int>& memArgs,
    KernelProperties* pProps) :
    Kernel(name, args, memArgs, pProps)
{
    m_hasAVX1 = m_pProps->GetCpuId().HasAVX1();
    m_hasAVX2 = m_pProps->GetCpuId().HasAVX2();     
}

cl_dev_err_code CPUKernel::PrepareThreadState(ICLDevExecutionState& state) const
{
    // Use vzeroupper to avoid the AVX state transition penalty.
    if(m_hasAVX1 && !m_hasAVX2) 
    {
        Emit_VZeroUpper();
    }

    state.MXCSRstate = _mm_getcsr();
    unsigned int uiNewFlags = (state.MXCSRstate & ~m_CSRMask) | m_CSRFlags;
    _mm_setcsr( uiNewFlags);
    return CL_DEV_SUCCESS;    
}

cl_dev_err_code CPUKernel::RestoreThreadState(ICLDevExecutionState& state) const
{
    // Use vzeroupper to avoid the AVX state transition penalty.
    if(m_hasAVX1 && !m_hasAVX2) 
    {
        Emit_VZeroUpper();
    }

    _mm_setcsr(state.MXCSRstate);
    return CL_DEV_SUCCESS; 
}


