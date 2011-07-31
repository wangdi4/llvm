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

#include "pragmas.h"

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

    int err = pthread_key_create( &g_tls_key, NULL );
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

inline
void TlsAccessor::thread_attach_int( void )
{
     m_pTls_struct = (TlsStruct*)pthread_getspecific( g_tls_key );
}

TlsAccessor::TlsAccessor( ThreadAttachMode mode ) : m_pTls_struct(NULL), m_mode(mode)
{
    assert( true == g_init_done && "SINK: TlsAccessor::TlsAccessor: tls_initialize() was not called" );

    if (AUTO == mode)
    {
        thread_attach_int();
    }
}

void TlsAccessor::thread_attach( void )
{
    assert( MANUAL == m_mode && "SINK: TlsAccessor::thread_attach should be called only in MANUAL mode" );
    thread_attach_int();
}

void TlsAccessor::thread_detach( void )
{
    assert( MANUAL == m_mode && "SINK: TlsAccessor::thread_detach should be called only in MANUAL mode" );
    m_pTls_struct = NULL;
}

//
// TlsContainer
//
inline
void TlsContainer::thread_attach_int( void )
{
    assert( NULL == m_pTls_struct && "SINK: TlsContainer::thread_attach_int attaching already attached thread");

    // member of accessor
    m_pTls_struct = &m_Tls_struct;

    m_previous_tls_value = pthread_getspecific( g_tls_key );
    int err = pthread_setspecific( g_tls_key, &m_Tls_struct );

    assert( 0 == err && "SINK: TlsContainer::thread_attach_int cannot pthread_setspecific()" );

    if (err)
    {
        NATIVE_PRINTF("TlsContainer::thread_attach_int: pthread_setspecific() returned error %d\n", err);
    }
}

inline
void TlsContainer::thread_detach_int( void )
{
    assert( NULL != m_pTls_struct && "SINK: TlsContainer::thread_detach_int detaching already detached thread");

    // member of accessor
    m_pTls_struct = NULL;

    int err = pthread_setspecific( g_tls_key, m_previous_tls_value );

    assert( 0 == err && "SINK: TlsContainer::thread_detach_int cannot pthread_setspecific()" );

    if (err)
    {
        NATIVE_PRINTF("TlsContainer::thread_detach_int: pthread_setspecific() returned error %d\n", err);
    }
}

TlsContainer::TlsContainer( ThreadAttachMode mode ) : TlsAccessor(0, mode)
{
    memset( &m_Tls_struct, 0, sizeof(m_Tls_struct) );

    assert( true == g_init_done && "SINK: TlsContainer::TlsContainer: tls_initialize() was not called" );

    if (MANUAL == mode)
    {
        thread_attach_int();
    }
}

TlsContainer::~TlsContainer()
{
    if (NULL != m_pTls_struct) // still attached
    {
        thread_detach_int();
    }
}

void TlsContainer::thread_attach( void )
{
    assert( MANUAL == m_mode && "SINK: TlsContainer::thread_attach should be called only in MANUAL mode" );
    thread_attach_int();
}

void TlsContainer::thread_detach( void )
{
    assert( MANUAL == m_mode && "SINK: TlsContainer::thread_detach should be called only in MANUAL mode" );
    thread_detach_int();
}

