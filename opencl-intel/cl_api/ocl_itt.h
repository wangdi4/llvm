#pragma once

// This file contains local definitions required for 
// the GPA instrumentation in the OpenCL framework 
#if defined(USE_GPA)
	#include "tal.h"
	#include <ittnotify.h>
#endif

#define GPA_SHOW_QUEUED_MARKER		0x1
#define GPA_SHOW_SUBMITTED_MARKER	0x2
#define GPA_SHOW_RUNNING_MARKER		0x4
#define GPA_SHOW_COMPLETED_MARKER	0x8

struct ocl_gpa_data
{
	bool					bUseGPA;
	unsigned char			cStatusMarkerFlags;
#if defined(USE_GPA)
	__itt_domain*			pDomain;
	__itt_string_handle*	pReadHandle;
	__itt_string_handle*	pWriteHandle;
	__itt_string_handle*	pCopyHandle;
	__itt_string_handle*	pMapHandle;
	__itt_string_handle*	pUnmapHandle;
	__itt_string_handle*	pSizeHandle;
	__itt_string_handle*	pWidthHandle;
	__itt_string_handle*	pHeightHandle;
	__itt_string_handle*	pDepthHandle;
	__itt_string_handle*	pWorkGroupSizeHandle;
	__itt_string_handle*	pNumberOfWorkGroupsHandle;
	__itt_string_handle*	pWorkGroupRangeHandle;
	__itt_string_handle*	pMarkerHandle;
#endif
};