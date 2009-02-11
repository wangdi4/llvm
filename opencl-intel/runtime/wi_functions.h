#pragma once

#ifdef __USING_FIBERS__
#include <windows.h>
#endif

/*****************************************************************************************************************************
*		Work-Item functions
*****************************************************************************************************************************/

__forceinline unsigned int get_work_dim()
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

__forceinline size_t get_global_id(unsigned int dim)
{
	if ( MAX_WORK_DIM <= dim )
	{
		return 0;
	}

	const SWIinfo*	pWIinfo;
	const SWGinfo*	pWGinfo;
#ifdef __USING_FIBERS__
	pWIinfo = ((SWIExecutionParam*)GetFiberData())->psWIInfo;
	pWGinfo = ((SWIExecutionParam*)GetFiberData())->psWGInfo;
#else
	int*	pInfoSeparator = (int*)_AddressOfReturnAddress();
	while (*pInfoSeparator != STACK_SEPARATOR)
		++pInfoSeparator;

	pWIinfo = (SWIinfo*)(*(pInfoSeparator+2));
	pWGinfo = (SWGinfo*)(*(pInfoSeparator+1));
#endif
	if ( pWGinfo->pWorkingDim->iWorkDim <= dim )
	{
		return 0;
	}
	return pWIinfo->viGlobalId[dim];
}

__forceinline size_t get_local_id(unsigned int dim)
{
	if ( MAX_WORK_DIM <= dim )
	{
		return 0;
	}

	const SWIinfo*	pWIinfo;
	const SWGinfo*	pWGinfo;
#ifdef __USING_FIBERS__
	pWIinfo = ((SWIExecutionParam*)GetFiberData())->psWIInfo;
	pWGinfo = ((SWIExecutionParam*)GetFiberData())->psWGInfo;
#else
	int*	pInfoSeparator = (int*)_AddressOfReturnAddress();
	while (*pInfoSeparator != STACK_SEPARATOR)
		++pInfoSeparator;


	pWGinfo = (SWGinfo*)(*(pInfoSeparator+1));
	pWIinfo = (SWIinfo*)(*(pInfoSeparator+2));
#endif
	if ( pWGinfo->pWorkingDim->iWorkDim <= dim )
	{
		return 0;
	}
	return pWIinfo->viLocalId[dim];
}

__forceinline size_t get_local_size(unsigned int dim)
{
	if ( MAX_WORK_DIM <= dim )
	{
		return 0;
	}

	const SWGinfo*	pWGinfo;
#ifdef __USING_FIBERS__
	pWGinfo = ((SWIExecutionParam*)GetFiberData())->psWGInfo;
	
#else
	int*	pInfoSeparator = (int*)_AddressOfReturnAddress();
	while (*pInfoSeparator != STACK_SEPARATOR)
		++pInfoSeparator;

	pWIinfo = (SWIinfo*)(*(pInfoSeparator+1));
#endif
	if ( pWGinfo->pWorkingDim->iWorkDim <= dim )
	{
		return 0;
	}
	return pWGinfo->pWorkingDim->viLocalSize[dim];
}

__forceinline size_t get_global_size(unsigned int dim)
{
	if ( MAX_WORK_DIM <= dim )
	{
		return 0;
	}

	const SWGinfo*	pWGinfo;
#ifdef __USING_FIBERS__
	pWGinfo = ((SWIExecutionParam*)GetFiberData())->psWGInfo;
	
#else
	int*	pInfoSeparator = (int*)_AddressOfReturnAddress();
	while (*pInfoSeparator != STACK_SEPARATOR)
		++pInfoSeparator;

	pWIinfo = (SWIinfo*)(*(pInfoSeparator+1));
#endif
	if ( pWGinfo->pWorkingDim->iWorkDim <= dim )
	{
		return 0;
	}
	return pWGinfo->pWorkingDim->viGlobalSize[dim];
}

__forceinline size_t get_num_groups(unsigned int dim)
{
	if ( MAX_WORK_DIM <= dim )
	{
		return 0;
	}

	const SWGinfo*	pWGinfo;
#ifdef __USING_FIBERS__
	pWGinfo = ((SWIExecutionParam*)GetFiberData())->psWGInfo;
	
#else
	int*	pInfoSeparator = (int*)_AddressOfReturnAddress();
	while (*pInfoSeparator != STACK_SEPARATOR)
		++pInfoSeparator;

	pWIinfo = (SWIinfo*)(*(pInfoSeparator+1));
#endif
	if ( pWGinfo->pWorkingDim->iWorkDim <= dim )
	{
		return 0;
	}
	return pWGinfo->viNumGroups[dim];
}

__forceinline size_t get_group_id(unsigned int dim)
{
	if ( MAX_WORK_DIM <= dim )
	{
		return 0;
	}

	const SWGinfo*	pWGinfo;
#ifdef __USING_FIBERS__
	pWGinfo = ((SWIExecutionParam*)GetFiberData())->psWGInfo;
	
#else
	int*	pInfoSeparator = (int*)_AddressOfReturnAddress();
	while (*pInfoSeparator != STACK_SEPARATOR)
		++pInfoSeparator;

	pWIinfo = (SWIinfo*)(*(pInfoSeparator+1));
#endif
	if ( pWGinfo->pWorkingDim->iWorkDim <= dim )
	{
		return 0;
	}
	return pWGinfo->viGroupId[dim];
}
