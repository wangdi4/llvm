#include "exe_cmd_mem_handler.h"

#include <cstring>
#include <assert.h>

using namespace Intel::OpenCL::MICDevice;

RunFuncData<DispatcherDataHandler::data>* DispatcherDataHandler::m_dispatcherDataOptionsArr[2] = { NULL, NULL };

class DispatcherDataHandler::StaticInitializer
{
public:
    StaticInitializer() 
    {
		DispatcherDataHandler::m_dispatcherDataOptionsArr[0] = new RunFuncDataUsingBuffer<DispatcherDataHandler::data>;
		DispatcherDataHandler::m_dispatcherDataOptionsArr[1] = new RunFuncDataUsingMisc<DispatcherDataHandler::data>;
    };
};

DispatcherDataHandler::StaticInitializer DispatcherDataHandler::init_statics;

RunFuncData<MiscDataHandler::data>* MiscDataHandler::m_miscDataOptionsArr[2] = { NULL, NULL };

class MiscDataHandler::StaticInitializer
{
public:
    StaticInitializer() 
    {
        MiscDataHandler::m_miscDataOptionsArr[0] = new RunFuncDataUsingMisc<MiscDataHandler::data>;
        MiscDataHandler::m_miscDataOptionsArr[1] = new RunFuncDataUsingBuffer<MiscDataHandler::data>;
    };
};

MiscDataHandler::StaticInitializer MiscDataHandler::init_statics;


DispatcherDataHandler::DispatcherDataHandler() : m_size(0), m_pDispatcherData(NULL)
{
	// Nullify m_data
	memset(&m_data, 0, sizeof(data));
}

DispatcherDataHandler::~DispatcherDataHandler()
{
	if (m_pDispatcherData)
	{
		m_pDispatcherData->release(m_data);
		m_pDispatcherData = NULL;
		if (m_data.pDataBuffer)
		{
			delete [] m_data.pDataBuffer;
			m_data.pDataBuffer = NULL;
		}
	}
}


cl_dev_err_code DispatcherDataHandler::init(dispatcher_data& dispatcherData, const directive_pack* preExeDirectives, const directive_pack* postExeDirectives, const char* pKernelParams, COIPROCESS* pCoiProcess)
{
	m_size = dispatcherData.getDispatcherDataSize();
	assert(m_size >= sizeof(dispatcher_data));
	// We going to create block of bytes which define the extended dispatcher_data, it will contain:
	//    * dispatcherData
	//    * preExeDirectives
	//    * postExeDirectives
	//    * other dispatcher specific dispatcher data such as pKernelParams (kernel args blob) in case of ndrange_dispatcher_data
	// Going to collect all the data together
	m_data.pDataBuffer = new char[m_size];
	if (NULL == m_data.pDataBuffer)
	{
		return CL_DEV_OUT_OF_MEMORY;
	}
	// Copy the dispatcher data with the tails to m_data.pDataBuffer
	if (false == dispatcherData.copyDispatcherDataWithTail(m_data.pDataBuffer, m_size, preExeDirectives, postExeDirectives, pKernelParams))
	{
		delete [] m_data.pDataBuffer;
		m_data.pDataBuffer = NULL;
		return CL_DEV_ERROR_FAIL;
	}

	m_pDispatcherData = m_dispatcherDataOptionsArr[m_size <= COI_PIPELINE_MAX_IN_MISC_DATA_LEN];
	cl_dev_err_code err = m_pDispatcherData->init(m_data, m_size, pCoiProcess);
	if (CL_DEV_FAILED(err))
	{
		delete [] m_data.pDataBuffer;
		m_data.pDataBuffer = NULL;
	}
	return err;
}

void DispatcherDataHandler::registerDispatcherData(vector<COIBUFFER>& coiBuffsArr, vector<COI_ACCESS_FLAGS>& coiBuffsAccessFlag)
{
	assert(m_pDispatcherData);
	m_pDispatcherData->registerData(m_data, COI_SINK_READ, coiBuffsArr, coiBuffsAccessFlag); 
}

void* DispatcherDataHandler::getDispatcherDataPtrForCoiRunFunc()
{
	assert(m_pDispatcherData);
	return m_pDispatcherData->getDataPtrForCoiRunFunc(m_data);
}

uint16_t DispatcherDataHandler::getDispatcherDataSizeForCoiRunFunc()
{
	assert(m_pDispatcherData);
	return m_pDispatcherData->getDataSizeForCoiRunFunc(m_size);
}




MiscDataHandler::MiscDataHandler() : m_pMiscData(NULL)
{
	// Nullify m_data.
	memset(&m_data, 0, sizeof(data));
}

MiscDataHandler::~MiscDataHandler()
{
	if (m_pMiscData)
	{
		m_pMiscData->release(m_data);
		m_pMiscData = NULL;
	}
}

cl_dev_err_code MiscDataHandler::init(bool useCoiBuffer, COIPROCESS* pCoiProcess) 
{ 
	m_data.pDataBuffer = &m_miscData;
	// Choose the appripriate MiscData object according to "useCoiBuffer" flag.
	m_pMiscData = m_miscDataOptionsArr[useCoiBuffer];
	return m_pMiscData->init(m_data, sizeof(misc_data), pCoiProcess); 
}

void MiscDataHandler::registerMiscData(vector<COIBUFFER>& coiBuffsArr, vector<COI_ACCESS_FLAGS>& coiBuffsAccessFlag) 
{ 
	assert(m_pMiscData);
	m_pMiscData->registerData(m_data, COI_SINK_WRITE_ENTIRE, coiBuffsArr, coiBuffsAccessFlag); 
}

void* MiscDataHandler::getMiscDataPtrForCoiRunFunc()
{
	assert(m_pMiscData);
	return m_pMiscData->getDataPtrForCoiRunFunc(m_data);
}
	
uint16_t MiscDataHandler::getMiscDataSizeForCoiRunFunc()
{
	assert(m_pMiscData);
	return m_pMiscData->getDataSizeForCoiRunFunc(sizeof(misc_data));
}

cl_dev_err_code MiscDataHandler::readMiscData(misc_data* pOutMiscData) 
{ 
	assert(m_pMiscData);
	return m_pMiscData->readData(m_data, pOutMiscData, sizeof(misc_data)); 
}
