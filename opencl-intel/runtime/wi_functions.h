#pragma once

#ifdef __USING_FIBERS__
#include <windows.h>
#endif

/*****************************************************************************************************************************
*		Work-Item functions
*****************************************************************************************************************************/

__forceinline int get_work_dim()
{
	const SWGinfo*	pWGinfo;
#ifdef __USING_FIBERS__
	pWGinfo = ((SWIExecutionParam*)GetFiberData())->psWGInfo;
#else
	int*	pInfoSeparator = (int*)_AddressOfReturnAddress();
	while (*pInfoSeparator != STACK_SEPARATOR)
		++pInfoSeparator;

	if ( MAX_DIMINSION <= dim )
	{
		return 0;
	}
	pWGinfo = (SWGinfo*)(*(pInfoSeparator+1));
#endif
	return pWGinfo->pWorkingDim->iWorkDim;
}

__forceinline int get_global_id(int dim)
{
	if ( MAX_DIMENSION <= dim )
	{
		return 0;
	}

	const SWIinfo*	pWIinfo;
#ifdef __USING_FIBERS__
	pWIinfo = ((SWIExecutionParam*)GetFiberData())->psWIInfo;
#else
	int*	pInfoSeparator = (int*)_AddressOfReturnAddress();
	while (*pInfoSeparator != STACK_SEPARATOR)
		++pInfoSeparator;

	pWIinfo = (SWIinfo*)(*(pInfoSeparator+2));
#endif
	return pWIinfo->viGlobalId[dim];
}

__forceinline int get_local_id(int dim)
{
	if ( MAX_DIMENSION <= dim )
	{
		return 0;
	}

	const SWIinfo*	pWIinfo;
#ifdef __USING_FIBERS__
	pWIinfo = ((SWIExecutionParam*)GetFiberData())->psWIInfo;
#else
	int*	pInfoSeparator = (int*)_AddressOfReturnAddress();
	while (*pInfoSeparator != STACK_SEPARATOR)
		++pInfoSeparator;


	pWIinfo = (SWIinfo*)(*(pInfoSeparator+2));
#endif
	return pWIinfo->viLocalId[dim];
}
