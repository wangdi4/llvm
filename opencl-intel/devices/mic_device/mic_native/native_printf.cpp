#include "native_printf.h"
#include "execution_task.h"
#include "wg_context.h"
#include "thread_local_storage.h"

#include <stdio.h>
#include <assert.h>

using namespace Intel::OpenCL::MICDeviceNative;

int PrintfHandle::print(const char* buffer) 
{ 
	printf("%s\n", buffer); 
	fflush(stdout);  
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
