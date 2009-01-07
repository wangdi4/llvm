#pragma once

/*****************************************************************************************************************************
*		Syncronization and memory fence functions
*****************************************************************************************************************************/

__forceinline void __barrier()
{
	int*	pInfoSeparator = (int*)_AddressOfReturnAddress();
	while (*pInfoSeparator != STACK_SEPARATOR)
		++pInfoSeparator;

	if ( MAX_DIMINSION <= dim )
	{
		return 0;
	}

	SWGinfo*	pWIinfo = (SWGinfo*)(*(pInfoSeparator+1));
	return pWIinfo->iWorkDim;
}
