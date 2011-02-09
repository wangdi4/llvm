
///////////////////////////////////////////////////////////
//  ocl_config.cpp
//  Implementation of the configuration class
//  Created on:      10-Dec-2008 8:45:02 AM
//  Original author: ulevy
///////////////////////////////////////////////////////////

#include "ocl_config.h"
using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

#if defined (_WIN32)
#define OS_DLL_POST(fileName) ((fileName) + ".dll")
#else
#define OS_DLL_POST(fileName) ("lib" + (fileName) + ".so")
#endif

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
	string s = m_pConfigFile->Read<string>(CL_CONFIG_DEVICES, "cpu_device");
	ConfigFile::tokenize(s, vectDevices);
    for (std::size_t i = 0; i < vectDevices.size(); ++i)
    {
        vectDevices[i] = OS_DLL_POST(vectDevices[i]);
    }
	default_device = m_pConfigFile->Read<string>(CL_CONFIG_DEFAULT_DEVICE, "cpu_device");
	default_device = OS_DLL_POST(default_device);
	return vectDevices;
}

vector<string> OCLConfig::GetFeCompilers(string & default_compiler)
{
	vector<string> vectFeCompilers;
	string s = m_pConfigFile->Read<string>(CL_CONFIG_FE_COMPILERS, "clang_compiler");
	ConfigFile::tokenize(s, vectFeCompilers);
    for (std::size_t i = 0; i < vectFeCompilers.size(); ++i)
    {
        vectFeCompilers[i] = OS_DLL_POST(vectFeCompilers[i]);
    }
	default_compiler = m_pConfigFile->Read<string>(CL_CONFIG_DEFAULT_FE_COMPILER, "clang_compiler");
	default_compiler = OS_DLL_POST(default_compiler);
	return vectFeCompilers;
}

