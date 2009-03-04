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
	pWGinfo = ((SWIExecutionParam*)GetFiberData())->psWGInfo;
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
	SWIExecutionParam* pWIExecParam = (SWIExecutionParam*)GetFiberData();
	pWIinfo = pWIExecParam->psWIInfo;
	pWGinfo = pWIExecParam->psWGInfo;

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
	SWIExecutionParam* pWIExecParam = (SWIExecutionParam*)GetFiberData();
	pWIinfo = pWIExecParam->psWIInfo;
	pWGinfo = pWIExecParam->psWGInfo;
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
	pWGinfo = ((SWIExecutionParam*)GetFiberData())->psWGInfo;
	
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
	pWGinfo = ((SWIExecutionParam*)GetFiberData())->psWGInfo;
	
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
	pWGinfo = ((SWIExecutionParam*)GetFiberData())->psWGInfo;
	
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
	pWGinfo = ((SWIExecutionParam*)GetFiberData())->psWGInfo;
	
	if ( pWGinfo->pWorkingDim->iWorkDim <= dim )
	{
		return 0;
	}

	return pWGinfo->viGroupId[dim];
}
