
///////////////////////////////////////////////////////////
//  ocl_config.cpp
//  Implementation of the configuration class
//  Created on:      10-Dec-2008 8:45:02 AM
//  Original author: ulevy
///////////////////////////////////////////////////////////

#include "ocl_config.h"
using namespace Intel::OpenCL::Framework;

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

vector<string> OCLConfig::GetDevices(string & default_device)
{
	vector<string> vectDevices;
	string s = m_pConfigFile->Read<string>(CL_CONFIG_DEVICES, "");
	ConfigFile::tokenize(s, vectDevices);
	default_device = m_pConfigFile->Read<string>(CL_CONFIG_DEFAULT_DEVICE, "");
	return vectDevices;
}

vector<string> OCLConfig::GetFeCompilers(string & default_compiler)
{
	vector<string> vectFeCompilers;
	string s = m_pConfigFile->Read<string>(CL_CONFIG_FE_COMPILERS, "");
	ConfigFile::tokenize(s, vectFeCompilers);
	default_compiler = m_pConfigFile->Read<string>(CL_CONFIG_DEFAULT_FE_COMPILER, "");
	return vectFeCompilers;
}

