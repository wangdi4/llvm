#include "native_printf.h"
#include "execution_task.h"
#include "wg_context.h"
#include "thread_local_storage.h"
#include <sink/COIProcess_sink.h>

#include <stdio.h>
#include <assert.h>

using namespace Intel::OpenCL::MICDeviceNative;

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
	TlsAccessor tlsAccessor;
	NDrangeTls ndRangeTls(&tlsAccessor);
	WGContext* pCtx = (WGContext*)ndRangeTls.getTls(NDrangeTls::WG_CONTEXT);
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
