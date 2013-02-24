/////////////////////////////////////////////////////////////////////////
// cl_utils.cpp:
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

#include "cl_stopwatch.h"
#include "hw_utils.h"

using namespace Intel::OpenCL::Utils;

#ifdef WIN32
#include<windows.h>
#endif
#include <assert.h>

StopWatch::StopWatch()
{
	assert( 0 && "Deprecated code");
	m_ullTime = 0;
	m_uiCounter = 0;
}

StopWatch::~StopWatch()
{
}

void StopWatch::Start()
{
	m_ullTime = RDTSC();
}

unsigned long long StopWatch::Stop()
{
	unsigned long long ullPrevTime = m_ullTime;
	m_ullTime = RDTSC();
	m_uiCounter++;
	return (m_ullTime - ullPrevTime);
}

unsigned long long StopWatch::Reset()
{
	unsigned long long ullPrevTime = m_ullTime;
	m_uiCounter = 0;
	return ullPrevTime;
}

unsigned long long StopWatch::GetTime() const
{
	unsigned long long currTime;
	currTime = RDTSC();
	return currTime - m_ullTime;
}
