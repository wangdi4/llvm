/////////////////////////////////////////////////////////////////////////
// cl_monitor.h:
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
// or disclosed in any way without Intel’s prior express written permission. 
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery 
// of the Materials, either expressly, by implication, inducement, estoppel or 
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice 
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors 
// in any way.
/////////////////////////////////////////////////////////////////////////

#include <cl_types.h>
#include <cl_stopwatch.h>

#ifndef	_CL_MONITOR_H
#define _CL_MONITOR_H

//#define __PERF_MONITOR__
#define MAX_SAMPLE_NAME		64
#define MAX_SAMPLES_COUNT	100

namespace Intel { namespace OpenCL { namespace Utils {

	/**********************************************************************************************
	* Class name:	Sample
	*
	* Inherit:		
	* Description:	represents a sampling point object
	* Author:		Uri Levy
	* Date:			July 2009
	**********************************************************************************************/
	class Sample
	{
	public:
		Sample(){}		// Constructor
		~Sample(){}		// Destructor

		// Start the sampling
		void Start(wchar_t * pwsName)
		{
			m_StopWatch.Start();
			wcscpy_s(m_wsName, MAX_SAMPLE_NAME, pwsName);
		}

		// Stop the sampling
		unsigned long long Stop()
		{
			m_ullTime = m_StopWatch.Stop();
			return m_ullTime;
		}

		// get the name of the sampling object
		const wchar_t * GetName() const { return m_wsName; }

		// get the total time from the start of the sampling to the end
		const unsigned long long GetTime() const { return m_ullTime; }


	protected:

		wchar_t				m_wsName[MAX_SAMPLE_NAME];
		unsigned long long	m_ullTime;

		StopWatch			m_StopWatch;

	};

	/**********************************************************************************************
	* Class name:	PerformanceMeter
	*
	* Inherit:		
	* Description:	represents a sampling PerformanceMeter object
	* Author:		Uri Levy
	* Date:			July 2009
	**********************************************************************************************/
	class PerformanceMeter
	{
	public: 

		static int Start(wchar_t * pwsSampleName)
		{
			if (MAX_SAMPLES_COUNT == g_uiSamplesCount)
			{
				return MAX_SAMPLES_COUNT;
			}
			g_pSamples[g_uiSamplesCount].Start(pwsSampleName);
			g_uiSamplesCount++;
			return g_uiSamplesCount-1;
		}

		static void Stop(int iSampleId)
		{
			if (MAX_SAMPLES_COUNT == iSampleId)
			{
				return;
			}
			g_pSamples[iSampleId].Stop();
		}


		static Sample * GetSamples(unsigned int * puiSamplesCount)
		{
			*puiSamplesCount = g_uiSamplesCount;
			return g_pSamples;
		}
	private:

		static unsigned int	g_uiSamplesCount;
		static Sample		g_pSamples[MAX_SAMPLES_COUNT];

	};

}}};

#ifdef __PERF_MONITOR__

#define __PERF_INIT			int iPerf = 0;
#define __PERF_START(NAME)	iPerf = Intel::OpenCL::Utils::PerformanceMeter::Start(NAME);
#define __PERF_STOP			Intel::OpenCL::Utils::PerformanceMeter::Stop(iPerf);
#define __PERF_MONITOR_INIT									\
	unsigned int PerformanceMeter::g_uiSamplesCount = 0;	\
	Sample PerformanceMeter::g_pSamples[100];
#define __PERF_PRINT_SUMMARY												\
	unsigned int uiSamplesCount = 0;										\
	Sample * pSamples = PerformanceMeter::GetSamples(&uiSamplesCount);		\
	for(unsigned int ui=0; ui<uiSamplesCount; ++ui)							\
{																		\
	printf("%ws,%llu\n", pSamples[ui].GetName(), pSamples[ui].GetTime());	\
}

#else

#define __PERF_INIT
#define __PERF_START(NAME)
#define __PERF_STOP
#define __PERF_MONITOR_INIT
#define	__PERF_PRINT_SUMMARY

#endif

#define cl_return	\
	__PERF_STOP;	\
	return			

#define cl_start	\
	__PERF_INIT;	\
	__PERF_START(WIDEN(__FUNCTION__));

#define cl_monitor_init		\
	__PERF_MONITOR_INIT

#define cl_monitor_summary	\
	__PERF_PRINT_SUMMARY

#endif