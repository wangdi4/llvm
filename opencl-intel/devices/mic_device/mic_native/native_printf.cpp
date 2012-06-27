#include "native_printf.h"
#include "execution_task.h"
#include "wg_context.h"

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
	WGContext* pCtx = (WGContext*)(ThreadPool::getInstance()->getGeneralTls(GENERIC_TLS_STRUCT::NDRANGE_TLS_ENTRY));
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
