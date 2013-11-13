// Copyright (c) 2006-2008 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////
//
// Defines 1 class:
//
//   HWExceptionsWrapper - contains support for setjmp/longjmp wrapping
//
///////////////////////////////////////////////////////////
#include "pragmas.h"

#include "native_common_macros.h"
#include "hw_exceptions_handler.h"
#include "native_globals.h"

#include <pthread.h>
#include <stdexcept> 
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <execinfo.h>

using namespace Intel::OpenCL::MICDevice;
using namespace Intel::OpenCL::MICDeviceNative;
using namespace Intel::OpenCL::UtilsNative;

// Must triger all exception, minimal during execution
int HWExceptionWrapper::g_sigs[] = {
    SIGFPE,  // for some reason (?) STL containers throw this HW exception that is handled internally
    SIGBUS,  // triggered by unaligned data access (+some other cases, relevant to drivers and OS)
    SIGILL,  // invalid operation only
    SIGSEGV  // segmentation fault only
};

// Static variable definition
volatile bool HWExceptionWrapper::g_finished = false;
__thread HWExceptionWrapper::execution_context* volatile HWExceptionWrapper::t_pExecContext = NULL;

void HWExceptionWrapper::catch_signal(int signum, siginfo_t *siginfo, void *context)
{
    if (g_finished)
    {
        return;
    }
    
    psiginfo(siginfo, "*** OPENCL MIC DEVICE HW EXCEPTION ***");fflush(stderr);

    execution_context* pExecContext = t_pExecContext;
    void *frames[64];
    int n = backtrace(&frames[0],(int)(sizeof(frames)/sizeof(frames[0])));

    if ( NULL ==  pExecContext)
    {
        fprintf(stderr,"\nBACKTRACE:\n");
        if (n > 0)
        {
              // Flush needed since we must write symbols to a raw fd
              fflush(stderr);
              backtrace_symbols_fd(frames, n, 2);
        }

        fprintf(stderr,"\n******************\n\n");
        fflush(stderr);

        // exception occured outside of JIT
        throw std::runtime_error( strsignal(signum) ); //sys_siglist[signum] );
    }
    else
    {
        // exception inside JIT
        const ICLDevBackendKernel_* pKernel = pExecContext->pKernel;
        if ( NULL != pKernel)
        {
            int lineNum = -1;
            for (unsigned int i = 0; i < n; i++)
            {
                lineNum = pKernel->GetLineNumber(frames[i]);
                if (-1 != lineNum)
                {
                    // If the kernel didn't build with "-profiling" flag;
                    if (0 == lineNum)
                    {
                        fprintf(stderr, "EXECEPTION OCCURED IN KERNEL \"%s\", To print to faulty line number, please compile the kernel with \"-profiling\"\n", pKernel->GetKernelName());
                    }
                    else
                    {
                        fprintf(stderr, "EXECEPTION OCCURED IN KERNEL \"%s\" AT LINE %d\n", pKernel->GetKernelName(), lineNum);
                    }
                    fflush(stderr);
                    break;
                }
            }
        }

        longjmp(pExecContext->setjump_buffer, 1);
    }

    return;
}


void HWExceptionWrapper::setup_signals( bool install )
{   
    //
    // Setup Linux signal handlers
    //
    
    struct sigaction sig_setup;    

    if (install)
    {
        sig_setup.sa_sigaction = catch_signal;
        sig_setup.sa_flags = SA_NODEFER|SA_SIGINFO;
    }
    else
    {
        sig_setup.sa_handler = SIG_DFL;
        sig_setup.sa_flags   = SA_RESETHAND;
    }
    sigemptyset(&sig_setup.sa_mask); 

    for (unsigned int i = 0; i < sizeof(g_sigs)/sizeof(g_sigs[0]); ++i)
    {
        if (0 != sigaction( g_sigs[i], &sig_setup, NULL ))
        {
            NATIVE_PRINTF("Cannot establish HW exception handler for %s\n", sys_siglist[g_sigs[i]]);
        }
    }

    if (!install)
    {
        g_finished = true;
    }
}

cl_dev_err_code HWExceptionWrapper::Execute( const ICLDevBackendKernel_* kernel,
                                             ICLDevBackendExecutable_* exec,
                                             const cl_uniform_kernel_args* args,
                                             const size_t* IN pGroupId
                                           )
{
    execution_context ctx;

    cl_dev_err_code return_code = CL_DEV_SUCCESS;

    // save current state including signal handlers state
    if (0 == setjmp(ctx.setjump_buffer))
    {
      // normal JIT execution
        ctx.pKernel = kernel;
        t_pExecContext = &ctx;
        exec->Execute( pGroupId, NULL, NULL );
        t_pExecContext = NULL;
    }
    else
    {
        // exception occurred
        t_pExecContext = NULL;
        return_code = CL_DEV_ERROR_FAIL;
        NATIVE_PRINTF("***FATAL***: Most likely exception occurred inside JIT code\n");
    }

    return return_code;
}

