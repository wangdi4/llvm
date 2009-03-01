// Copyright (c) 2006-2008 Intel Corporation
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

/*
*
* File wg_executor.h
* Declares class WGExecutor, class that manages execution of single Work-Group
*
*/

#pragma once

#include "OCL_RT.h"

#include <list>
#include <map>

namespace Intel { namespace OpenCL { namespace CPUDevice {

class WGExecutor
{
public:
	WGExecutor();
	virtual ~WGExecutor();

	int	Initialize( unsigned int uiTaskId, const SKernelInfo* pKernelInfo, const SWGinfo* pWGInfo );
	void Destroy();

	unsigned int GetCurrentTaskid()	{ return m_uiTaskId;}

	int	UpdateGroup(const SWGinfo* pWGInfo);
	int Execute();

protected:
	// Private types definition
	// Holds information for singel work-item execution instance
	struct	SWIExecItem
	{
		void*					pFiber;				// Working fiber
		SWIinfo					sWIinfo;			// Work-Item information
		SWIExecutionParam		sWIExecParam;		// Work-Item execution param
	};

	typedef	std::list<SWIExecItem*>		TWIitemList;
	
	struct SWGSyncEvent // Structure defines cross WI syncronization event
	{
		TWIitemList	lWaiting;				// List of waiting WI to this event
	};

	typedef	std::map<void*, SWGSyncEvent*>	TWISyncEventMap;

	// Private class members
	unsigned int				m_uiTaskId;			// ID of task that currently is executed
	TWISyncEventMap				m_mapSyncEvents;	// Map of syncronization events 
	TWIitemList					m_lReady;			// List of WI ready to execute
	const SWGinfo*				m_psWGInfo;			// A pointer to current Work-Group run-time information
	const SKernelInfo*			m_psKernelInfo;		// A Pointer to Work-Group kernel execution paramters
	SWIExecItem*				m_psWIinfo;			// Work-Items inforamtion
	unsigned int				m_iWIcount;			// Total number of WI in WG
	void*						m_fbWGFiber;			// WG thread main fiber
	void*						m_pLocalMem;

//	Static members

	static void CALLBACK ExecuteWI(void* param);
};
}}}