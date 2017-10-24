
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
#if defined (_WIN32)
    #if defined (_M_X64)
    static const char* DEFAULT_DEVICES_LIST = "cpu_device64" OPENCL_BINARIES_POSTFIX;
    #else //_M_X64
    static const char* DEFAULT_DEVICES_LIST = "cpu_device32" OPENCL_BINARIES_POSTFIX;
    #endif //_M_X64
#else // _WIN32
    static const char* DEFAULT_DEVICES_LIST = "cpu_device" OPENCL_BINARIES_POSTFIX;
#endif // _WIN32

OCLConfig::OCLConfig()
{
    // BasicCLConfigWrapper
}

OCLConfig::~OCLConfig()
{
    // ~BasicCLConfigWrapper
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

// I declare this function here and not in utils to make sure that the instance returned is truely a singleton in the whole the program

FrameworkUserLogger& FrameworkUserLogger::Instance()
{
    static FrameworkUserLogger instance;
    return instance;
}

namespace Intel { namespace OpenCL { namespace Utils {

FrameworkUserLogger* g_pUserLogger = &FrameworkUserLogger::Instance();

}}}
