
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
static const char* DEFAULT_DEVICES_LIST = "cpu_device64" 
#else
static const char* DEFAULT_DEVICES_LIST = "cpu_device32" 
#endif
#else
static const char* DEFAULT_DEVICES_LIST = "cpu_device" 
#endif

#ifdef INCLUDE_MIC_DEVICE
                                          ";mic_device"
#endif

#ifdef INCLUDE_ISP_DEVICE
#if defined (_WIN32)
#if defined (_M_X64)
                                          ";isp_device64" 
#else
                                          ";isp_device32" 
#endif
#else
                                          ";isp_device" 
#endif
#endif
                                          ;

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

Intel::OpenCL::Utils::UserLogger& GetUserLoggerInstance()
{
    static Intel::OpenCL::Utils::UserLogger instance;
    return instance;
}
