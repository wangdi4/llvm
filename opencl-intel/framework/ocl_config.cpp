
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
	#ifdef BUILD_EXPERIMENTAL_21
		static const char* DEFAULT_DEVICES_LIST = "cpu_device64_2_1"
        #else// BUILD_EXPERIMENTAL_21
		static const char* DEFAULT_DEVICES_LIST = "cpu_device64"
	#endif// BUILD_EXPERIMENTAL_21
    #else //_M_X64
	#ifdef BUILD_EXPERIMENTAL_21
		static const char* DEFAULT_DEVICES_LIST = "cpu_device32_2_1"
        #else// BUILD_EXPERIMENTAL_21
		static const char* DEFAULT_DEVICES_LIST = "cpu_device32"
	#endif// BUILD_EXPERIMENTAL_21
    #endif //_M_X64
#else // _WIN32
    #ifdef BUILD_EXPERIMENTAL_21
        static const char* DEFAULT_DEVICES_LIST = "cpu_device_2_1"
    #else// BUILD_EXPERIMENTAL_21
        static const char* DEFAULT_DEVICES_LIST = "cpu_device"
    #endif// BUILD_EXPERIMENTAL_21
#endif // _WIN32

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

FrameworkUserLogger& FrameworkUserLogger::Instance()
{
    static FrameworkUserLogger instance;
    return instance;
}

namespace Intel { namespace OpenCL { namespace Utils {

#ifdef __ANROID__
    
    FrameworkUserLogger* g_pUserLogger = nullptr;
#else        
    FrameworkUserLogger* g_pUserLogger = &FrameworkUserLogger::Instance();
#endif

}}}
