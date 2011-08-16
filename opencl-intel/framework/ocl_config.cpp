
///////////////////////////////////////////////////////////
//  ocl_config.cpp
//  Implementation of the configuration class
//  Created on:      10-Dec-2008 8:45:02 AM
//  Original author: ulevy
///////////////////////////////////////////////////////////

#include "ocl_config.h"
using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

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
	string default_device = m_pConfigFile->Read<string>(CL_CONFIG_DEFAULT_DEVICE, "cpu_device");
	return default_device;
}

vector<string> OCLConfig::GetDevices(string const& default_device) const
{
	vector<string> vectDevices;
	string s = m_pConfigFile->Read<string>(CL_CONFIG_DEVICES, default_device);
	ConfigFile::tokenize(s, vectDevices);
	return vectDevices;
}
