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

	int	UpdateGroupId(const int pGroupId[]);
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

//	Static members
	static __declspec(thread)	void*				ms_pWGFiber;

	static void CALLBACK ExecuteWI(void* param);
};
}}}