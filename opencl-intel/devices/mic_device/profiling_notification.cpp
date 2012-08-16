#include "profiling_notification.h"
#include "command.h"
#include <cstddef>
#include <pthread.h>
#include <assert.h>

using namespace Intel::OpenCL::MICDevice;
using namespace Intel::OpenCL::Utils;

ProfilingNotification ProfilingNotification::m_singleProfilingNotification;

bool ProfilingNotification::registerProfilingNotification(COIPROCESS coiProcess)
{
	COIRESULT coi_err = COIRegisterNotificationCallback(coiProcess, &ProfilingNotification::callbackNotifier, NULL);
	assert(COI_SUCCESS == coi_err);
	if (COI_SUCCESS != coi_err)
	{
		return false;
	}
	COINotificationCallbackSetContext(NULL);
	return true;
}
	
bool ProfilingNotification::unregisterProfilingNotification(COIPROCESS coiProcess)
{
	COIRESULT coi_err = COIUnregisterNotificationCallback(coiProcess, &ProfilingNotification::callbackNotifier);
	assert(COI_SUCCESS == coi_err);
	if (COI_SUCCESS != coi_err)
	{
		return false;
	}
	return true;
}

void ProfilingNotification::callbackNotifier(COI_NOTIFICATIONS in_Type, COIPROCESS in_Process, COIEVENT in_Event, const void* in_UserData)
{
	if (NULL == in_UserData)
	{
		return;
	}
	((Command*)in_UserData)->eventProfilingCall(in_Type);
}
