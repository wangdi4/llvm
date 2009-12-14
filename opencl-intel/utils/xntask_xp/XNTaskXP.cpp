/* ************************************************************************* *\
                  INTEL CORPORATION PROPRIETARY INFORMATION
      This software is supplied under the terms of a license agreement or 
      nondisclosure agreement with Intel Corporation and may not be copied 
      or disclosed except in accordance with the terms of that agreement. 
          Copyright (C) 2007 Intel Corporation. All Rights Reserved.
\* ************************************************************************* */

// Sceleton of XNTask library to be run on Windows* XP

#include <XN0Task_common.h>
#include <XN0Sys_common.h>
#include <stdlib.h>
#include <windows.h>
#include <process.h>

struct XNTaskXPNotify
{
	XNSYNCOBJECT			SyncObject;
	XN_SYNC_OBJECT_CALLBACK Callback;
	void*					Arg;
};

unsigned int __stdcall NotifyEntryPoint(void *data)
{
	XNTaskXPNotify*	pNotify = (XNTaskXPNotify*)(data);

	pNotify->Callback(pNotify->SyncObject, pNotify->Arg);

	return 0;
}

unsigned int XN0SysGetHardwareThreadCount()
{
	return 1;
}

XNERROR XN0TaskInit( const   UINT32  in_NumThreadsInThreadPool, void*   in_pTaskControlBlockPool, const   UINT32  in_SizeOfTaskControlBlockPool)
{
	return XN_SUCCESS;
}

XNERROR XN0TaskShutdown(void)
{
	return XN_SUCCESS;
}

XNERROR XN0TaskCreateSet(
    const   UINT32              in_NumberOfTasksToCreateInSet, 
    const   XN_TASK_PRIORITY    in_Priority,
            XNTaskSetFunction   in_TaskSetFunction, 
            void*               in_pTaskSetFunctionArg, 
    const   XNSYNCOBJECT*       in_pDependsOnSyncObjectsArray, 
    const   UINT32              in_SizeOfDependsOnSyncObjectsArray, 
            XNTASK*             out_pTaskSetCompletedSyncObject)
{
	for(unsigned int i=0; i<in_NumberOfTasksToCreateInSet; ++i)
	{
		in_TaskSetFunction(in_pTaskSetFunctionArg, i, in_NumberOfTasksToCreateInSet);
	}

	*out_pTaskSetCompletedSyncObject = (XNTASK)0;

	return XN_SUCCESS;
}

XNERROR XN0TaskCreate(
    const   XN_TASK_PRIORITY    in_Priority,
            XNTaskFunction      in_TaskFunction, 
            void*               in_pTaskFunctionArg, 
    const   XNSYNCOBJECT*       in_pDependsOnSyncObjectsArray, 
    const   UINT32              in_SizeOfDependsOnSyncObjectsArray, 
            XNTASK*             out_pTaskCompletedSyncObject)
{
	in_TaskFunction(in_pTaskFunctionArg);

	*out_pTaskCompletedSyncObject = (XNTASK)0;

	return XN_SUCCESS;
}

void XN0SyncObjectAddWaiter(
            XNSYNCOBJECT            in_SyncObject, 
            void*                   in_ScratchMemory,
            XN_SYNC_OBJECT_CALLBACK in_Callback, 
            void*                   in_Arg)
{
	XNTaskXPNotify	*pNotify = new XNTaskXPNotify;

	pNotify->Callback = in_Callback;
	pNotify->SyncObject = in_SyncObject;
	pNotify->Arg = in_Arg;

	free(in_ScratchMemory);

    void* threadHandle = (void*)_beginthreadex(NULL, 0, NotifyEntryPoint, pNotify, 0, NULL);
	CloseHandle(threadHandle);
}

XNERROR
XN0SyncObjectRemoveWaiter(
            XNSYNCOBJECT    in_SyncObject, 
            void*           in_ScratchMemory)
{
	free(in_ScratchMemory);

	return XN_SUCCESS;
}

XNERROR
XN0TaskIncRef(
			  const   XNTASK  in_Task,
			  UINT32* out_pCount)
{
	return XN_SUCCESS;
}

XNERROR
XN0TaskDecRef(
    const   XNTASK  in_Task,
            UINT32* out_pCount)
{
	return XN_SUCCESS;
}

XNERROR
XN0SyncObjectWait(
    const   XNSYNCOBJECT*   in_pSyncObjectArray, 
            uint32_t        in_NumberOfSyncObjectsInArray )
{
	return XN_SUCCESS;
}

XNERROR
XN0TaskExtendCompletion(
    const   XNTASK  in_Task)
{
	return XN_SUCCESS;
}
