// Copyright (c) 2006-2013 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#pragma once

// This file contains local definitions required for 
// the GPA instrumentation in the OpenCL framework 
#if defined(USE_GPA)
#ifndef USE_ITT
#define USE_ITT
#endif
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
#include <stdio.h>
#ifdef USE_ITT
#include <ittnotify.h>
#endif
#endif

#define ITT_SHOW_QUEUED_MARKER		0x1
#define ITT_SHOW_SUBMITTED_MARKER	0x2
#define ITT_SHOW_RUNNING_MARKER		0x4
#define ITT_SHOW_COMPLETED_MARKER	0x8

#define ITT_TASK_NAME_LEN     64

struct ocl_gpa_data
{
  bool					    bUseGPA;
  bool					    bEnableAPITracing;
  bool					    bEnableContextTracing;
  unsigned char				cStatusMarkerFlags;
  unsigned int				iIDCounter;
#ifdef USE_ITT
	__itt_domain*			pDeviceDomain;
	__itt_domain*			pContextDomain;
	__itt_domain*			pAPIDomain;
	__itt_string_handle*	pNDRangeHandle;
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
#endif
#if defined(USE_GPA)
	__itt_track_group*		pContextTrackGroup;
	__ittx_task_state*		pWaitingTaskState;
	__ittx_task_state*		pRunningTaskState;
#endif
};

struct ocl_gpa_queue
{
#if defined(USE_GPA)
    __itt_track*            m_pTrack;
#endif
#ifdef USE_ITT
    __itt_string_handle*    m_pStrHndl;
#endif
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
#ifdef USE_ITT
  __itt_id                m_CmdId;
  __itt_string_handle*    m_strCmdName;
#endif
};

