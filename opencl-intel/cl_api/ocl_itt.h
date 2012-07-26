#pragma once

// This file contains local definitions required for 
// the GPA instrumentation in the OpenCL framework 
#if defined(USE_GPA)
	#define USE_ITT
	#define INTEL_ITTNOTIFY_API_PRIVATE
	#include <stdio.h>
    #include <tal.h>
	#include <ittxnotify.h>
#else
// A bug in 4.0 < GCC < 4.6 treats cdecl attribute ignore (on 64 bit) as error.
//#if __x86_64__ && __GNUC__ == 4 &&  __GNUC_MINOR__ < 6
	#undef CDECL
//#else
//	#define CDECL   __attribute__((cdecl))
//#endif
	#if defined(USE_ITT)
		#include <stdio.h>
		#include <ittnotify.h>
	#endif
#endif

#define GPA_SHOW_QUEUED_MARKER		0x1
#define GPA_SHOW_SUBMITTED_MARKER	0x2
#define GPA_SHOW_RUNNING_MARKER		0x4
#define GPA_SHOW_COMPLETED_MARKER	0x8

#define ITT_TASK_NAME_LEN     64

struct ocl_gpa_data
{
	bool					bUseGPA;
	bool					bEnableAPITracing;
    bool					bEnableContextTracing;
	unsigned char			cStatusMarkerFlags;
	unsigned int			iIDCounter;
	__itt_domain*			pDeviceDomain;
	__itt_domain*			pContextDomain;
	__itt_domain*			pAPIDomain;
	__itt_string_handle*	pReadHandle;
	__itt_string_handle*	pWriteHandle;
	__itt_string_handle*	pCopyHandle;
	__itt_string_handle*	pMapHandle;
	__itt_string_handle*	pUnmapHandle;
	__itt_string_handle*	pSyncDataHandle;
	__itt_string_handle*	pSizeHandle;
	__itt_string_handle*	pWidthHandle;
	__itt_string_handle*	pHeightHandle;
	__itt_string_handle*	pDepthHandle;
	__itt_string_handle*	pWorkGroupSizeHandle;
	__itt_string_handle*	pNumberOfWorkGroupsHandle;
	__itt_string_handle*	pWorkGroupRangeHandle;
	__itt_string_handle*	pMarkerHandle;
	__itt_string_handle*	pWorkDimensionHandle;
	__itt_string_handle*	pGlobalWorkSizeHandle;
	__itt_string_handle*	pLocalWorkSizeHandle;
	__itt_string_handle*	pGlobalWorkOffsetHandle;
	__itt_string_handle*	pStartPos;
	__itt_string_handle*	pEndPos;
	__itt_string_handle*	pIsBlocking;
	__itt_string_handle*	pNumEventsInWaitList;

#if defined(USE_GPA)
	__itt_track_group*      pContextTrackGroup;
	__ittx_task_state*		pWaitingTaskState;
	__ittx_task_state*		pRunningTaskState;
#endif
};

struct ocl_gpa_queue
{
#if defined(USE_GPA)
    __itt_track*            m_pTrack;
#endif
    __itt_string_handle*    m_pStrHndl;
};

struct ocl_gpa_command
{
    // each GPA command contains 2 different taks: wait taks and execution task
    // the wait task represent the time command waited in the queue
    // the execution taks represent the acutal execution time of the command
    //
    // ********************************************************
    // *       WAIT TIME       *        EXECUTION TIME        *
    // ********************************************************
    //
	__itt_id                m_CmdId;
    __itt_string_handle*    m_strCmdName;
};
