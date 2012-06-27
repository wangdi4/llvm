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
// Defines 2 classes:
//
//   TlsContainer - contains local storage for current thread.
//                  installs new values at constructor and
//                  restores old values at destructor
//
//   TlsAccessor  - reads values from TlsContainer installed in the
//                  current thread.
//
///////////////////////////////////////////////////////////
#pragma once

#include <stdlib.h>

namespace Intel { namespace OpenCL { namespace MICDeviceNative {
    class ProgramMemoryManager;
}}}

using namespace Intel::OpenCL::MICDeviceNative;

namespace Intel { namespace OpenCL { namespace UtilsNative {

    //
    //   TlsAccessor  - reads values from TlsContainer installed in the
    //                  current thread.
    //
    class TlsAccessor
    {
        public:
            // must be call at startup and shutdown
            static void tls_initialize( void );
            static void tls_finalize( void );

            // working mode
            enum ThreadAttachMode {
                AUTO = 0,   // attach to thread at constructor, detach at destructor
                MANUAL      // use thread_attach/thread_detach functions
            };

            TlsAccessor( ThreadAttachMode mode = AUTO );
            virtual ~TlsAccessor() {};

            // should be called ONLY in AUTO mode
            virtual void thread_attach( void );
            virtual void thread_detach( void );

            ProgramMemoryManager* get_program_memory_manager( void ) const
                { return m_pTls_struct ? m_pTls_struct->prog_manager : NULL; };

        protected:
            struct TlsStruct {
                ProgramMemoryManager* prog_manager;
            };

            TlsStruct*          m_pTls_struct;
            ThreadAttachMode    m_mode;

            // constructor to be used by Container
            TlsAccessor( int, ThreadAttachMode mode = AUTO ) :
                    m_pTls_struct(NULL), m_mode(mode) {};

        private:
            void thread_attach_int( void );
    };

    //
    //   TlsContainer - contains local storage for current thread.
    //                  installs new values at constructor and
    //                  restores old values at destructor
    //
    class TlsContainer : public TlsAccessor
    {
        public:
            TlsContainer( ThreadAttachMode mode = AUTO );
            virtual ~TlsContainer();

            // should be called ONLY in AUTO mode
            virtual void thread_attach( void );
            virtual void thread_detach( void );

            void set_program_memory_manager( ProgramMemoryManager* p )
                { m_Tls_struct.prog_manager = p; };

        private:
            TlsStruct       m_Tls_struct;
            void*           m_previous_tls_value;

        private:
            void thread_attach_int( void );
            void thread_detach_int( void );
    };

}}}

