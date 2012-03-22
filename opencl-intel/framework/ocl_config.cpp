
///////////////////////////////////////////////////////////
//  ocl_config.cpp
//  Implementation of the configuration class
//  Created on:      10-Dec-2008 8:45:02 AM
//  Original author: ulevy
///////////////////////////////////////////////////////////

#include "ocl_config.h"
using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

// First device is a default device 
static const char* DEFAULT_DEVICES_LIST = "cpu_device" 
#ifdef INCLUDE_MIC_DEVICE
                                          ";mic_device"
#endif
                                          ;

OCLConfig::OCLConfig()
{
	m_pConfigFile = NULL;
}
OCLConfig::~OCLConfig()
{
	Release();
}
cl_err_code OCLConfig::Initialize(std::string filename)
{
	m_pConfigFile = new ConfigFile(filename);
	return CL_SUCCESS;
}
void OCLConfig::Release()
{
	if (NULL != m_pConfigFile)
	{
		delete m_pConfigFile;
		m_pConfigFile = NULL;
	}
}

string OCLConfig::GetDefaultDevice() const
{
    vector<string> vectDevices = GetDevices();
    return (vectDevices.size() > 0) ? vectDevices[0] : "";
}

vector<string> OCLConfig::GetDevices() const
{
	vector<string> vectDevices;
	string s = m_pConfigFile->Read<string>(CL_CONFIG_DEVICES, DEFAULT_DEVICES_LIST);
	ConfigFile::tokenize(s, vectDevices);
	return vectDevices;
}
