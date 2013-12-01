#include "mic_native_logger.h"

#include <common/COIEngine_common.h>
#include <sink/COIProcess_sink.h>

#include <iostream>

using namespace Intel::OpenCL::MICDeviceNative;
using namespace Intel::OpenCL::Utils;
using namespace std;

IOCLDevLogDescriptor* MicNativeLogDescriptor::m_pLogDescriptor = NULL;
cl_int MicNativeLogDescriptor::m_clientId = 0;

bool MicNativeLogDescriptor::createLogDescriptor()
{
	if (NULL == m_pLogDescriptor)
	{
		m_pLogDescriptor = new MicNativeLogDescriptor;
		if (NULL == m_pLogDescriptor)
		{
			return false;
		}
		if (!((MicNativeLogDescriptor*)m_pLogDescriptor)->init())
		{
			delete m_pLogDescriptor;
			m_pLogDescriptor = NULL;
			return false;
		}
	}
	return true;
}

void MicNativeLogDescriptor::releaseLogDescriptor()
{
	if (m_pLogDescriptor)
	{
		delete m_pLogDescriptor;
		m_pLogDescriptor = NULL;
	}
	m_clientId = 0;
}

cl_int MicNativeLogDescriptor::clLogAddLine( cl_int IN client_id, cl_int IN log_level, const char* IN source_file, const char* IN function_name, cl_int IN line_num, const char* IN message, ...)
{
    va_list va;
    va_start(va, message);
	m_pLoggerClient->LogArgList((ELogLevel)log_level, source_file, function_name, line_num, message, va);
    va_end(va);
	COIRESULT coiErr = COI_SUCCESS;
	coiErr = COIProcessProxyFlush();
	assert(COI_SUCCESS == coiErr && "calling to COIProcessProxyFlush() failed");
	return (COI_SUCCESS == coiErr) ? CL_DEV_SUCCESS : CL_DEV_ERROR_FAIL;
}

MicNativeLogDescriptor::MicNativeLogDescriptor() : m_pLogHandler(NULL), m_pLoggerClient(NULL)
{
}

MicNativeLogDescriptor::~MicNativeLogDescriptor()
{
	if (m_pLogHandler)
	{
		delete m_pLogHandler;
		m_pLogHandler = NULL;
	}
	if (m_pLoggerClient)
	{
		delete m_pLoggerClient;
		m_pLoggerClient = NULL;
	}
}

bool MicNativeLogDescriptor::init()
{
    m_clientId = 1;
	bool ret = true;
	do
	{
		COI_ISA_TYPE out_pType;
		uint32_t out_pIndex = 0;
		COIEngineGetIndex(&out_pType, &out_pIndex);

		stringstream devTitleStream;
		devTitleStream << "\n" << "----- MIC-" << out_pIndex << " -----\n";

	    m_pLogHandler = new FileDescriptorLogHandler(devTitleStream.str().c_str());
		if (NULL == m_pLogHandler)
		{
			ret = false;
			break;
		}
	    m_pLoggerClient = new LoggerClient(devTitleStream.str().c_str(),LL_DEBUG);
		if (NULL == m_pLoggerClient)
		{
			ret = false;
			break;
		}
		cl_err_code clErrRet = m_pLogHandler->Init(LL_DEBUG, NULL, devTitleStream.str().c_str(), stderr);
        if (CL_FAILED(clErrRet))
        {
            ret = false;
			break;
        }
		clErrRet = Logger::GetInstance().AddLogHandler(m_pLogHandler);
		if (CL_FAILED(clErrRet))
        {
            ret = false;
			break;
        }
		Logger::GetInstance().SetActive(true);
		INIT_LOGGER_CLIENT(devTitleStream.str().c_str(), LL_DEBUG);
	} while (0);
	if (!ret)
	{
		if (m_pLogHandler)
		{
			delete m_pLogHandler;
			m_pLogHandler = NULL;
		}
		if (m_pLoggerClient)
		{
			delete m_pLoggerClient;
			m_pLoggerClient = NULL;
		}
	}
	return ret;
}
