/////////////////////////////////////////////////////////////////////////
// cl_utils.h:
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2008 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related
// to the source code ("Material") are owned by Intel Corporation or its
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and
// proprietary and confidential information of Intel Corporation and its
// suppliers and licensors, and is protected by worldwide copyright and trade
// secret laws and treaty provisions. No part of the Material may be used, copied,
// reproduced, modified, published, uploaded, posted, transmitted, distributed,
// or disclosed in any way without Intel�s prior express written permission.
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery
// of the Materials, either expressly, by implication, inducement, estoppel or
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice
// or any other notice embedded in Materials by Intel or Intel�s suppliers or licensors
// in any way.
/////////////////////////////////////////////////////////////////////////

#pragma once

#include <cl_types.h>
#include <cl_device_api.h>
#include <cl_monitor.h>
#include <string>
#include <iostream>
#include <sstream>
#ifdef WIN32
typedef int threadid_t;
#else
#include <sched.h>
typedef pid_t  threadid_t;
#endif


/**************************************************************************************************
* Function: 	ClErrTxt
* Description:	returns a wide-character string of the error code
* Arguments:	error_code [in] -	error code
* Return value:	wide-character string - representing the error code
* Author:		Uri Levy
* Date:			December 2008
**************************************************************************************************/
const wchar_t* ClErrTxt(cl_err_code error_code);

/**************************************************************************************************
* Function: 	stringify
* Description:	Turn any type supporting the ostream&<< operator into a string
* Arguments:	a variable to convert into string
* Return value:	string
* Author:		Eli Bendersky
* Date:			?
**************************************************************************************************/
template<typename T> inline std::string stringify(const T& x)
{
	std::ostringstream out;
	out << x;
	return out.str();
}

// Specialization for booleans
//
template<> inline std::string stringify(const bool& b)
{
	return b ? "true" : "false";
}

// Specialization for signed and unsigned chars: we want them to be just
// displayed as numbers
//
template<> inline std::string stringify(const unsigned char& c)
{
	return stringify(static_cast<unsigned int>(c));
}


template<> inline std::string stringify(const signed char& c)
{
	return stringify(static_cast<signed int>(c));
}

/**************************************************************************************************
* Function: 	XXXtoString
* Description:	Turn some kind of OpenCL objects defines into strings
				Note: can not overload stringfy because most of the object are merely a typedefs
* Arguments:	a variable to convert into string
* Return value:	string
* Author:		Nitzan Dori
* Date:			May 2012
**************************************************************************************************/
const string channelOrderToString(const cl_channel_order& co);
const string channelTypeToString(const cl_channel_type& ct);
const string imageFormatToString (const cl_image_format& format);
const string memTypeToString(const cl_mem_object_type& mo);
const string memFlagsToString(const cl_mem_flags& flags);
const string addressingModeToString(const cl_addressing_mode& am);
const string filteringModeToString(const cl_filter_mode& fm);
const string commandQueuePropertiesToString(const cl_command_queue_properties& prop);
const string fpConfigToString(const cl_device_fp_config& fp_config);
const string memCacheTypeToString(const cl_device_mem_cache_type& memType);
const string localMemTypeToString(const cl_device_local_mem_type& memType);
const string execCapabilitiesToString(const cl_device_exec_capabilities& execCap);
const string commandTypeToString(const cl_command_type& type);


/**************************************************************************************************
* Function: 	clIsNumaAvailable
* Description:	Checks if machine supports NUMA
* Return value:	bool
* Author:		Evgeny Fiksman
* Date:			November 2011
**************************************************************************************************/
bool clIsNumaAvailable();

/**************************************************************************************************
* Function: 	clNUMASetLocalNodeAlloc
* Description:	Set prefered node for memory allocation for the calling thread
* Return value:	void
* Author:		Evgeny Fiksman
* Date:			November 2011
**************************************************************************************************/
void clNUMASetLocalNodeAlloc();

/**************************************************************************************************
* Function: 	clSleep
* Description:	put the calling thread on sleep for time 'milliseconds'
* Arguments:	milliseconds [ int ] - duration of required thread sleep
* Return value:	void
* Author:		Rami Jioussy
* Date:			June 2010
**************************************************************************************************/
void clSleep(int milliseconds);


/**************************************************************************************************
* Function: 	clSetThreadAffinityMask
* Description:	sets the given affinity mask 
* Arguments:	mask [ affinityMask_t* ] - a pointer to the mask to use
* Return value:	void
* Author:		Doron Singer
* Date:			March 2011
**************************************************************************************************/
void clSetThreadAffinityMask(affinityMask_t* mask, threadid_t tid = 0);

/**************************************************************************************************
* Function: 	clGetThreadAffinityMask
* Description:	retrieves the given thread affinity mask 
* Arguments:	mask [ affinityMask_t* ] - a pointer to the mask to be set
* Return value:	void
* Author:		Evgeny Fiksman
* Date:			March 2012
**************************************************************************************************/
void clGetThreadAffinityMask(affinityMask_t* mask, threadid_t tid = 0);

/**************************************************************************************************
* Function: 	clTranslateAffinityMask
* Description:	fills the given array of unsigned ints with the values corresponding to set bits in the mask
* Arguments:	mask [ affinityMask_t* ] - a pointer to the mask to use
*               IDs  [ unsigned int*   ] - the array to fill
*               len  [ size_t          ] - the size of the array
* Return value:	bool
* Author:		Doron Singer
* Date:			March 2011
**************************************************************************************************/
bool clTranslateAffinityMask(affinityMask_t* mask, unsigned int* arr, size_t len);

/**************************************************************************************************
* Function: 	clSetThreadAffinityToCore
* Description:	affinitizes the calling thread to the requested core
* Arguments:	core [ unsigned int ] - the index of the core
* Return value:	void
* Author:		Doron Singer
* Date:			August 2011
**************************************************************************************************/
void clSetThreadAffinityToCore(unsigned int core, threadid_t tid = 0);

/**************************************************************************************************
* Function: 	clResetThreadAffinityMask
* Description:	allows the thread to run on any CPU in the process
* Return value:	void
* Author:		Doron Singer
* Date:			March 2011
**************************************************************************************************/
void clResetThreadAffinityMask(threadid_t tid = 0);

/**************************************************************************************************
* Function: 	clMyThreadId
* Description:	returns the caller's OS-specific tid
* Return value:	threadid_t
* Author:		Doron Singer
* Date:			October 2011
**************************************************************************************************/
threadid_t clMyThreadId();

/**************************************************************************************************
* Function: 	clCopyMemoryRegion
* Description:	Copies memory region defined in SMemCpyParams
* Return value:	void
* Author:		Evgeny Fiksman
* Date:			March 2011
**************************************************************************************************/
struct SMemCpyParams
{
	cl_uint			uiDimCount;
	const cl_char*	pSrc;
	size_t			vSrcPitch[MAX_WORK_DIM-1];
	cl_char*		pDst;
	size_t			vDstPitch[MAX_WORK_DIM-1];
	size_t			vRegion[MAX_WORK_DIM];
};

void clCopyMemoryRegion(SMemCpyParams* pCopyCmd);

/************************************************************************
 * Function:        clGetPixelBytesCount
 * Parameters:
 *  pclImageFormat  a cl_image_format
 * Return value:    the size in bytes of each pixel in this image format
 * Author:          Aharon Abramson
 * Date:            January 2012
 ************************************************************************/
size_t clGetPixelBytesCount(const cl_image_format* pclImageFormat);
