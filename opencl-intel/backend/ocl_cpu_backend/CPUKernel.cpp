// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
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


