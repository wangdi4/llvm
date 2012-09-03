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
///////////////////////////////////////////////////////////
#pragma once

#include <stdlib.h>
#include <assert.h>

namespace Intel { namespace OpenCL { namespace UtilsNative {

	class TlsAccessor;
	
	struct TlsGeneralAccessor
	{
	public:
		void* getTls(unsigned int index);
		void* getTls(unsigned int index, bool& alreadyHad);
		void  setTls(unsigned int index, void* value);
	protected:
		TlsGeneralAccessor(TlsAccessor* tlsAccessor, unsigned int startIndex) : m_pTlsAccessor(tlsAccessor), m_startIndex(startIndex) {};
	private:
		TlsAccessor* m_pTlsAccessor;
		unsigned int m_startIndex;
	};


	struct ProgramServiceTls : TlsGeneralAccessor
	{
	public:
		ProgramServiceTls(TlsAccessor* tlsAccessor);

		enum
		{
			PROGRAM_MEMORY_MANAGER = 0,

			NUM_TLS_TYPES
		};
	};

	struct NDrangeTls : TlsGeneralAccessor
	{
	public:
		NDrangeTls(TlsAccessor* tlsAccessor);

		enum
		{
			WG_CONTEXT = 0,
			HW_EXCEPTION,

			NUM_TLS_TYPES
		};
	};

	struct TbbTls : TlsGeneralAccessor
	{
	public:
		TbbTls(TlsAccessor* tlsAccessor);

		enum
		{
			WORKER_ID = 0,
			SCHEDULER,

			NUM_TLS_TYPES
		};
	};

	struct QueueTls : TlsGeneralAccessor
	{
	public:
		QueueTls(TlsAccessor* tlsAccessor);

		enum
		{
			QUEUE_TLS_ENTRY = 0,

			NUM_TLS_TYPES
		};
	};

    //
    //   TlsAccessor  - reads values from TlsContainer installed in the
    //                  current thread.
    //
    class TlsAccessor
    {
		friend class TlsGeneralAccessor;
		friend class ProgramServiceTls;
		friend class NDrangeTls;
		friend class TbbTls;
		friend class QueueTls;

        public:
            // must be call at startup and shutdown
            static void tls_initialize( void );
            static void tls_finalize( void );

            TlsAccessor();

		private:

		enum 
		{
			PROGRAM_SERVICE = 0,
			NDRANGE = PROGRAM_SERVICE + ProgramServiceTls::NUM_TLS_TYPES,
			TBB = NDRANGE + NDrangeTls::NUM_TLS_TYPES,
			QUEUE = TBB + TbbTls::NUM_TLS_TYPES,

			NUM_OF_TLS_OBJECTS = QUEUE + QueueTls::NUM_TLS_TYPES
		};

		struct TlsStruct
		{
			void* data[NUM_OF_TLS_OBJECTS];
			bool  alreadyHad[NUM_OF_TLS_OBJECTS];
		};

		TlsStruct*			m_pTls_struct;

		void* getTlsValue(unsigned int index);

		void* getTlsValue(unsigned int index, bool& alreadyHad);

		void setTlsValue(unsigned int index, void* value);
    };


inline void* TlsAccessor::getTlsValue(unsigned int index) 
{
	assert(index >= 0 && index < TlsAccessor::NUM_OF_TLS_OBJECTS);
	assert(m_pTls_struct);
	return m_pTls_struct->data[index];
};

inline void* TlsAccessor::getTlsValue(unsigned int index, bool& alreadyHad)
{
	assert(index >= 0 && index < TlsAccessor::NUM_OF_TLS_OBJECTS);
	assert(m_pTls_struct);
	alreadyHad = m_pTls_struct->alreadyHad[index];
	return m_pTls_struct->data[index];
}

inline void TlsAccessor::setTlsValue(unsigned int index, void* value)
{
	assert(index >= 0 && index < TlsAccessor::NUM_OF_TLS_OBJECTS);
	assert(m_pTls_struct);
	m_pTls_struct->data[index] = value;
	m_pTls_struct->alreadyHad[index] = true;
}


inline void* TlsGeneralAccessor::getTls(unsigned int index)
{
	return m_pTlsAccessor->getTlsValue(m_startIndex + index);
}

inline void* TlsGeneralAccessor::getTls(unsigned int index, bool& alreadyHad)
{
	return m_pTlsAccessor->getTlsValue(m_startIndex + index, alreadyHad);
}

inline void TlsGeneralAccessor::setTls(unsigned int index, void* value)
{
	m_pTlsAccessor->setTlsValue(m_startIndex + index, value);
}

}}}
