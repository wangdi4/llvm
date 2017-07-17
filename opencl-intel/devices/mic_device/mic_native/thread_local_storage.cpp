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

#include "thread_local_storage.h"
#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "native_common_macros.h"

using namespace Intel::OpenCL::UtilsNative;

// tls index
static pthread_key_t g_tls_key;
static bool          g_init_done = false;

//
// TlsAccessor
//
void TlsAccessor::tls_initialize( void )
{
    assert(false == g_init_done && "SINK: TlsAccessor::tls_initialize called twice" );

    int err = pthread_key_create( &g_tls_key, nullptr );
    assert( 0 == err && "SINK: TlsAccessor::tls_initialize cannot pthread_key_create()" );

    g_init_done = (0 == err);

    if (err)
    {
        NATIVE_PRINTF("TlsAccessor::tls_initialize: pthread_key_create() returned error %d\n", err);
    }
}

void TlsAccessor::tls_finalize( void )
{
    assert( true == g_init_done && "SINK: TlsAccessor::tls_finalize: tls_initialize() was not called" );

    int err = pthread_key_delete( g_tls_key );
    assert( 0 == err && "SINK: TlsAccessor::tls_initialize cannot pthread_key_create()" );

    g_init_done = (0 != err);

    if (err)
    {
        NATIVE_PRINTF("TlsAccessor::tls_initialize: pthread_key_create() returned error %d\n", err);
    }
}

TlsAccessor::TlsAccessor()
{
    assert( true == g_init_done && "SINK: TlsAccessor::TlsAccessor: tls_initialize() was not called" );

	m_pTls_struct = (TlsStruct*)pthread_getspecific( g_tls_key );
	if (nullptr == m_pTls_struct)
	{
		m_pTls_struct = new TlsStruct;
		memset(m_pTls_struct->data, 0, sizeof(m_pTls_struct->data));
		memset(m_pTls_struct->alreadyHad, 0, sizeof(m_pTls_struct->alreadyHad));

		int err = pthread_setspecific( g_tls_key, m_pTls_struct );

		assert( 0 == err && "SINK: TlsAccessor::getTlsStructInt cannot pthread_setspecific()" );

		if (err)
		{
			NATIVE_PRINTF("TlsAccessor::getTlsStructInt: pthread_setspecific() returned error %d\n", err);
		}
	}
}


ProgramServiceTls::ProgramServiceTls(TlsAccessor* tlsAccessor) : TlsGeneralAccessor(tlsAccessor, TlsAccessor::PROGRAM_SERVICE) 
{
}

NDrangeTls::NDrangeTls(TlsAccessor* tlsAccessor) : TlsGeneralAccessor(tlsAccessor, TlsAccessor::NDRANGE) 
{
}

ExecutorTls::ExecutorTls(TlsAccessor* tlsAccessor) : TlsGeneralAccessor(tlsAccessor, TlsAccessor::EXECUTOR) 
{
}

QueueTls::QueueTls(TlsAccessor* tlsAccessor) : TlsGeneralAccessor(tlsAccessor, TlsAccessor::QUEUE) 
{
}
