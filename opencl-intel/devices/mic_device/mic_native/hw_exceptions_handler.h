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
#pragma once

#include "cl_dev_backend_api.h"
#include "thread_local_storage.h"
#include <setjmp.h> 
#include <signal.h>

namespace Intel { namespace OpenCL { namespace UtilsNative {

    class HWExceptionsWrapper 
    {
    public:
        // object is required only for JIT wrapping
        HWExceptionsWrapper(TlsAccessor* tlsAccessor);
        ~HWExceptionsWrapper();
        
        // wrap JIT code execution
        cl_dev_err_code Execute( ICLDevBackendExecutable_* code, 
                                 const size_t* IN pGroupId,
                    			 const size_t* IN pLocalOffset, 
                    			 const size_t* IN pItemsToProcess ) __attribute__((noinline));

        // setup global HW exception handling in a process
        // Convert all exceptions into std::runtime_error C++ exceptions
        static void Init( void );

    private:
		TlsAccessor* m_pTlsAccessor;
        sigjmp_buf setjump_buffer;
        bool       m_bInside_JIT;

        static void catch_signal(int signum, siginfo_t *siginfo, void *context);
        
    };

    typedef HWExceptionsWrapper HWExceptionsJitWrapper;
}}}

