// CpuDevice.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "device_api.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif


#ifdef _MANAGED
#pragma managed(pop)
#endif

cl_dev_err_code clDevCreateDevice(	cl_uint	dev_id,
									cl_dev_entry_points	*pDevEntry,
									cl_dev_call_backs	*pDevCallBacks
									)
{
	return CL_DEV_INVALID_OPERATION;
}
