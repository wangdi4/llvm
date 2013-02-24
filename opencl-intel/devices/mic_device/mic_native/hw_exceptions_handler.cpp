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
#include <pthread.h>
#include <stdexcept> 
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <execinfo.h>
#include "native_globals.h"

using namespace Intel::OpenCL::MICDevice;
using namespace Intel::OpenCL::MICDeviceNative;
using namespace Intel::OpenCL::UtilsNative;

int HWExceptionsWrapper::g_sigs[] = { 
    /*SIGFPE,*/  // for some reason (?) STL containers throw this HW exception that is handled internally
    /*SIGBUS,*/  // triggered by unalined data access (+some other cases, relevant to drivers and OS)
    SIGILL,      // invalid operation only
    SIGSEGV      // segmentation fault only
};

volatile bool HWExceptionsWrapper::g_finished = false;

void HWExceptionsWrapper::catch_signal(int signum, siginfo_t *siginfo, void *context)
{
    if (g_finished)
    {
        return;
    }
    
    psiginfo(siginfo, "*** OPENCL MIC DEVICE HW EXCEPTION ***");fflush(stderr);
    //psignal( signum, " ");

	TlsAccessor tlsAccessor;
	NDrangeTls ndRangeTls(&tlsAccessor);
    HWExceptionsWrapper* exec_wrapper = (HWExceptionsWrapper*)ndRangeTls.getTls( NDrangeTls::HW_EXCEPTION );

    if ((NULL == exec_wrapper) || (false == exec_wrapper->m_bInside_JIT))
    {

		fprintf(stderr,"\nBACKTRACE:\n");
		void *frames[16];
		int n = backtrace(&frames[0],(int)(sizeof(frames)/sizeof(frames[0])));
		if (n > 0) {
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
        longjmp(exec_wrapper->setjump_buffer, 1);        
    }

    return;
}


void HWExceptionsWrapper::setup_signals( bool install )
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

void HWExceptionsWrapper::thread_init( TlsAccessor* tlsAccessor )
{
	if (!gMicExecEnvOptions.kernel_safe_mode)
	{
		return;
	}

    if (NULL != tlsAccessor)
    {
        NDrangeTls ndRangeTls(tlsAccessor);
        ndRangeTls.setTls( NDrangeTls::HW_EXCEPTION, this );   
        m_is_attached = true;
    }
}

void HWExceptionsWrapper::thread_fini( TlsAccessor* tlsAccessor )
{
	if (!gMicExecEnvOptions.kernel_safe_mode)
	{
		return;
	}

    if (m_is_attached)
    {
        if (NULL != tlsAccessor)
        {
            NDrangeTls ndRangeTls(tlsAccessor);
            ndRangeTls.setTls( NDrangeTls::HW_EXCEPTION, NULL );
        }
        else
        {
            TlsAccessor tls;
            NDrangeTls ndRangeTls(&tls);
            ndRangeTls.setTls( NDrangeTls::HW_EXCEPTION, NULL );
        }
        m_is_attached = false;
    }
}

cl_dev_err_code HWExceptionsWrapper::Execute(   ICLDevBackendExecutable_* code, 
                                                 const size_t* IN pGroupId,
                                                 const size_t* IN pLocalOffset, 
                                                 const size_t* IN pItemsToProcess )
{
	if (!gMicExecEnvOptions.kernel_safe_mode)
	{
		return code->Execute( pGroupId, pLocalOffset, pItemsToProcess );
	}

	cl_dev_err_code return_code;
    
	// save current state including signal handlers state
	if (0 == setjmp(setjump_buffer))
	{
		// normal JIT execution
		m_bInside_JIT = true;
		return_code = code->Execute( pGroupId, pLocalOffset, pItemsToProcess );
		m_bInside_JIT = false;
	}
	else
	{
		// exception occurred
		m_bInside_JIT = false;
		return_code = CL_DEV_ERROR_FAIL;
		NATIVE_PRINTF("***FATAL***: Most likely exception occured inside JIT code\n");
	}

	return return_code;
}

