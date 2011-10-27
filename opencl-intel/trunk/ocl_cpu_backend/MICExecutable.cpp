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

File Name:  MICExecutable.cpp

\*****************************************************************************/

#include "MICExecutable.h"
#include "Binary.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

MICExecutable::MICExecutable(const Binary* pBin):
    Executable(pBin)
{}

// Prepares current thread for the executable execution
cl_dev_err_code MICExecutable::PrepareThread() 
{
    // TODO: check if KNF Support something ?!
    //m_uiMXCSRstate = _mm_getcsr(); 
    //unsigned int uiNewFlags = (m_uiMXCSRstate & ~m_uiCSRMask) | m_uiCSRFlags;
    //_mm_setcsr( uiNewFlags);
    return CL_DEV_SUCCESS;
}

// Restores Thread state as it was before the execution
cl_dev_err_code MICExecutable::RestoreThreadState() 
{  
    // TODO: check if KNF Support something ?!
    //_mm_setcsr( m_uiMXCSRstate);
    return CL_DEV_SUCCESS;
}

}}} // namespace
