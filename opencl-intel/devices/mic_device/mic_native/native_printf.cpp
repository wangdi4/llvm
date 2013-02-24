#include "native_printf.h"
#include "wg_context.h"
#include "native_thread_pool.h"
#include <sink/COIProcess_sink.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>

using namespace Intel::OpenCL::MICDeviceNative;
using namespace Intel::OpenCL::Utils;

PrintfHandle::PrintfHandle() : m_flushAtExit(false)
{
}

PrintfHandle::~PrintfHandle()
{
	if (m_flushAtExit)
	{
		COIRESULT coiErr = COI_SUCCESS;
		coiErr = COIProcessProxyFlush();
		assert(COI_SUCCESS == coiErr);
	}
}

int PrintfHandle::print(const char* buffer) 
{ 
	printf("%s", buffer);
	m_flushAtExit = true;
	return strlen(buffer); 
}

int MICNativeBackendPrintfFiller::Print(void* id, const char* buffer)
{
    if (NULL == m_thread_pool)
    {
        OclAutoMutex lock(&m_lock);
        if (NULL == m_thread_pool)
        {
            m_thread_pool = ThreadPool::getInstance();
        }
    }

    assert( NULL != m_thread_pool );
    if (NULL == m_thread_pool)
    {
        return 0;
    }

    // remove volatile qualifier
	WGContext* pCtx = const_cast<ThreadPool*>(m_thread_pool)->findActiveWGContext();
	assert(pCtx);
	if (NULL == pCtx)
	{
		return 0;
	}

	PrintfHandle* pPrintfHandle = pCtx->getPrintHandle();
	assert(pPrintfHandle);
	if (NULL == pPrintfHandle)
	{
		return 0;
	}

	return pPrintfHandle->print(buffer);
}
