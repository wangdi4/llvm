#include "commands.h"
#include "in_order_command_list.h"

#include <source/COIBarrier_source.h>

using namespace Intel::OpenCL::MICDevice;

Command::Command() : NotificationPort::CallBack()
{
}

NDRange1::NDRange1() : Command()
{
}

cl_dev_err_code NDRange1::Create(bool isInOrder, Command** pOutCommand)
{
    NDRange1* pCommand = NULL;
	isInOrder ? pCommand = new InOrderNDRange() : pCommand = new OutOfOrderNDRange();
	if (NULL == pCommand)
	{
	    return CL_DEV_OUT_OF_MEMORY;
	}
	*pOutCommand = pCommand;
	return CL_DEV_SUCCESS;
}

InOrderNDRange::InOrderNDRange() : NDRange1()
{
}

cl_dev_err_code InOrderNDRange::execute(CommandList* pCommandList)
{
    COIBARRIER* barrier = NULL;
	unsigned int numDependecies = 0;
    pCommandList->getLastDependentBarrier(&barrier, &numDependecies, true);

	COIPIPELINE pipe = pCommandList->getPipelineHandle();

	//TODO Get COIFUNCTION handle according to func name (ask from DeviceServiceCommunication dictionary (NOT implemented yet)
	COIFUNCTION func = pCommandList->getDeviceFunction( DeviceServiceCommunication::EXECUTE_IN_ORDER );

	// TODO set arg
	void* arg = NULL;

	//TODO complete the missing arguments (such as buffers...)
	COIRESULT result = COIPipelineRunFunction(pipe, func, 0, NULL, NULL, numDependecies, barrier, NULL, 0, NULL, 0, &m_completionBarrier);
    if (result != COI_SUCCESS)
	{
	    return CL_DEV_ERROR_FAIL;
	}
	pCommandList->setLastDependentBarrier(m_completionBarrier, true);
	pCommandList->getNotificationPort()->addBarrier(m_completionBarrier, this, arg);

    return CL_DEV_SUCCESS;

}

void InOrderNDRange::fireCallBack(void* arg)
{
    //TODO something with arg if needed
	delete(this);
}


OutOfOrderNDRange::OutOfOrderNDRange() : NDRange1()
{
}

cl_dev_err_code OutOfOrderNDRange::execute(CommandList* pCommandList)
{
	COIPIPELINE pipe = pCommandList->getPipelineHandle();
	//TODO Get COIFUNCTION handle according to func name (ask from DeviceServiceCommunication dictionary (NOT implemented yet)
	COIFUNCTION func = NULL;
	// TODO set arg
	void* arg = NULL;
	COIBarrierRegisterUserBarrier(&m_completionBarrier);
	pCommandList->getNotificationPort()->addBarrier(m_completionBarrier, this, arg);
	//TODO complete the missing arguments (such as buffers... DO not forget to send m_completionBarrier to device side)
	COIRESULT result = COIPipelineRunFunction(pipe, func, 0, NULL, NULL, 0, NULL, NULL, 0, NULL, 0, NULL);
	return (COI_SUCCESS == result) ? CL_DEV_SUCCESS : CL_DEV_ERROR_FAIL;
}

void OutOfOrderNDRange::fireCallBack(void* arg)
{
    //TODO something with arg if needed
	delete(this);
}


FailureNotification::FailureNotification() : Command()
{
}

cl_dev_err_code FailureNotification::execute(CommandList* pCommandList)
{
    //TODO
    return CL_DEV_SUCCESS;
}

void FailureNotification::fireCallBack(void* arg)
{
    //TODO something with arg if needed
	delete(this);
}

