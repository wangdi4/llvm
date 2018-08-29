// INTEL CONFIDENTIAL
//
// Copyright 2007-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "cl_config.h"
#include "cl_sys_defines.h"
#include "cl_sys_info.h"
#include "cl_cpu_detect.h"

#include <cassert>
#ifdef _WIN32
#include <Windows.h>
#else
#include "hw_utils.h"
#endif
using namespace Intel::OpenCL::Utils;
using std::string;

namespace Intel { namespace OpenCL { namespace Utils {

#ifdef _WIN32

static bool GetDisplayPrimaryDeviceIds(string& vendorId, string& devId)
{
    DISPLAY_DEVICE dd;
    dd.cb = sizeof(DISPLAY_DEVICE);
    DWORD devNum = 0;
    string id;

    while (EnumDisplayDevices(nullptr, devNum++, &dd, 0))
    {
        if (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
        {
            id = dd.DeviceID;
            break;
        }
    }
    if (id.empty())
    {
        return false;
    }
    vendorId = id.substr(8, 4);
    devId = id.substr(17, 4);
    return true;
}

OPENCL_VERSION GetOpenclVerByCpuModel()
{
    string vendorId, devId;
    // by using EnumDisplayDevices we don't need the GPU driver to be installed (this doesn't work in remote desktop)
    if (GetDisplayPrimaryDeviceIds(vendorId, devId))
    {
        if (vendorId == "8086")
        {
            const char* opencl_12_skus[] = {"190E", "1915", "1921", "190B", "192B", "190A", "191A", "192A", "1906", "1913", "1902", "1917", // SKL
                                            "1606", "160E"}; // BDW
            const char* opencl_21_skus[] = {"591E", "5916", "5912", "591B"}; // KBL

            for (size_t i = 0; i < sizeof(opencl_12_skus) / sizeof(opencl_12_skus[0]); ++i)
            {
                if (devId == opencl_12_skus[i])
                {
                    return OPENCL_VERSION_1_2;
                }
            }

            for (size_t i = 0; i < sizeof(opencl_21_skus) / sizeof(opencl_21_skus[0]); ++i)
            {
                if (devId == opencl_21_skus[i])
                {
                    return OPENCL_VERSION_2_1;
                }
            }
        }
    }
    // TODO: replace querying the registry with the above method for all SKUs

    const string sKmdDevId = GetRegistryKeyValue<string>("SOFTWARE\\Intel\\KMD", "DevId", std::string());
    if ("BDW GT1 MOBILE ULT" == sKmdDevId || "SKL GT1_5 ULT MOBILE F0" == sKmdDevId
        || "SKL GT1_5 ULX MOBILE F0" == sKmdDevId || "SKL GT1_5 DESKTOP F0" == sKmdDevId)
    {
        return OPENCL_VERSION_1_2;  // GPU SKUs Broadwell GT1 and Skylake GT1.5 support OpenCL 1.2, so we have to be aligned with it
    }

    if(CPUDetect::GetInstance()->isCannonlake() ||
       CPUDetect::GetInstance()->isKabylakeOrCoffeelake() ||
       CPUDetect::GetInstance()->isSkylake() ||
       CPUDetect::GetInstance()->isIcelake())
    {
        return OPENCL_VERSION_2_1;
    }

    if(CPUDetect::GetInstance()->isBroadwell()
       //Downgrade OpenCL version on GLK according to VPG targets
       //CPUDetect::GetInstance()->isGeminilake()
       //TODO. Uncomment next line as soon as VPG support OpenCL 2.0.
    //   CPUDetect::GetInstance()->isBroxton()   ||
       )
    {
        return OPENCL_VERSION_2_0;
    }

    // Workaround for Windows.
    // TODO: Replace with isBroxton()
    if ("0A84" == devId || // BXT A stepping
        "1A84" == devId || // BXT B stepping
        "5A84" == devId  ) // BXT A-C steppings
    {
    //TODO. Replace next line with OPENCL_VERSION_2_0 as soon as VPG support OpenCL 2.0.
        return OPENCL_VERSION_1_2;
    }

    return OPENCL_VERSION_1_2;
}

#else

OPENCL_VERSION GetOpenclVerByCpuModel()
{
    if(CPUDetect::GetInstance()->isSkylake()   ||
       //TODO. Uncomment next line as soon as VPG support OpenCL 2.0.
       //CPUDetect::GetInstance()->isBroxton()  ||
       CPUDetect::GetInstance()->isKabylakeOrCoffeelake() ||
       CPUDetect::GetInstance()->isCannonlake() ||
       CPUDetect::GetInstance()->isIcelake())
    {
        return OPENCL_VERSION_2_1;
    }

    if(CPUDetect::GetInstance()->isBroadwell())
    {
        return OPENCL_VERSION_2_0;
    }

    return OPENCL_VERSION_1_2;
}
#endif

}}}

// ParseStringToSize:
//  Parse a string that represents memory size of the format: <integer><units>
//  And convert it to unsigned long in bytes
//      e.g. 128MB --> 128 * 1024 * 1024 --> 134,217,728 bytes
cl_ulong BasicCLConfigWrapper::ParseStringToSize(const std::string& userStr) const
{
    cl_ulong integer = 0;
    std::string integerStr;
    std::string units;

    // parse the first part: the integer
    std::istringstream iss(userStr);
    iss >> integer;

    if (0 == integer)
    {
        return 0;
    }

    // all the rest of userStr are the units string
    std::stringstream ss;
    ss << integer;
    ss >> integerStr;
    units = userStr.substr(integerStr.size());

    // convert to bytes
    // accepted units are: "GB", "MB", "KB", "B"
    if (units == "GB")
    {
        integer = integer << 30;
    }
    else if (units == "MB")
    {
        integer = integer << 20;
    }
    else if (units == "KB")
    {
        integer = integer << 10;
    }
    else if (units != "B")
    {
        //invalid unit
        return 0;
    }

    return integer;
}


ConfigFile::ConfigFile(const string& filename, string delimiter, string comment, string sentry)
{
    // Construct a ConfigFile, getting keys and values from given file
    m_sDelimiter = delimiter;
    m_sComment = comment;
    m_sSentry = sentry;

    cl_err_code clErrRet = ReadFile(filename, (*this));
    if (CL_FAILED(clErrRet))
    {
        // set defaults
    }
}

ConfigFile::ConfigFile()
{
    // Construct a ConfigFile without a file; empty
    m_sDelimiter = string(1,'=');
    m_sComment = string(1,'#');
}


void ConfigFile::Remove(const string& key)
{
    // Remove key and its value
    m_mapContents.erase(m_mapContents.find(key));
    return;
}


bool ConfigFile::KeyExists(const string& key) const
{
    // Indicate whether key is found
    mapci p = m_mapContents.find(key);
    return (p != m_mapContents.end());
}

/* static */
int ConfigFile::tokenize(const string & sin, std::vector<string> & tokens)
{
    string s = sin;
    s += char(0);    // add a 0 char for getting end-of-string parsing
    string seps = ",;|";
    seps += char(0);
    string::size_type pos1 = 0;
    string::size_type pos2 = 0;
    while ((pos2 = s.find_first_of(seps, pos1)) != string::npos)
    {
        if (pos2 > pos1)
        {
            string sub = s.substr(pos1, pos2-pos1);
            trim(sub);
            tokens.push_back(sub);
        }
        pos1 = pos2+1;   // don't forget that or you'll get an infinite loop
    }
    // Rami linux-64bit port
    //assert(tokens.size() <= CL_MAX_INT32);
    return (int)tokens.size();
}


/* static */
void ConfigFile::trim( string& s )
{
    // Remove leading and trailing whitespace
    static const char whitespace[] = " \n\t\v\r\f";
    s.erase(0, s.find_first_not_of(whitespace));
    s.erase(s.find_last_not_of(whitespace) + 1U);
}


cl_err_code ConfigFile::ReadFile(const string& fileName, ConfigFile& cfg)
{
    std::ifstream fsInputStream;
    
    fsInputStream.open(fileName.c_str());
    
    if ( !fsInputStream.is_open() )
    {
        // File doesn't exists in current directory
        // Check installation path
        char szModulePath[MAX_PATH] = "";
    
        Intel::OpenCL::Utils::GetModuleDirectory(szModulePath, MAX_PATH);
    
        STRCAT_S(szModulePath, MAX_PATH, fileName.c_str());
        fsInputStream.open(szModulePath);
        if ( !fsInputStream.is_open() )
        {
            return CL_ERR_FILE_NOT_EXISTS;
        }
    }

    string strNextLine = "";
    string line;

    const string& strDelimeter  = cfg.m_sDelimiter;

    // get the length of the eparator
    const string::size_type szSepLength = strDelimeter.length();

    const string& strComment = cfg.m_sComment;
    const string& strEOF = cfg.m_sSentry;

    while ( fsInputStream.good() || (strNextLine.length() > 0) )
    {
        // if the next line is not empty, get its content and mark as empty line
        if (strNextLine.length() <= 0)
        {
            std::getline(fsInputStream, line);
        }
        else
        {
            line = strNextLine;
            strNextLine = "";
        }

        // find the first location of the commnet and get the string before it
        string::size_type szCommentPos = line.find(strComment);
        line = line.substr(0, szCommentPos);

        if ( (line.find(strEOF) != string::npos) && (strEOF != "") )
        {
            return CL_SUCCESS;
        }

        bool bFinish = false;
        // getting the first position of the delimeter
        string::size_type szDelimeterPos = line.find(strDelimeter);

        // continue only if the position is not the end of line
        if (string::npos > szDelimeterPos)
        {
            string strKey = line.substr(0, szDelimeterPos);
            ConfigFile::trim(strKey);

            string::size_type szPos = szDelimeterPos + szSepLength;
            line.replace(0, szPos, "");

            while ( (false == bFinish) && fsInputStream )
            {
                // get new line
                std::getline(fsInputStream, strNextLine);

                // save the next line
                string strCopy = strNextLine;
                ConfigFile::trim(strCopy);

                if(strCopy == "")
                {
                    continue;
                }

                bFinish = true;

                strNextLine = strNextLine.substr(0, strNextLine.find(strComment));
                if ( (string::npos != strNextLine.find(strDelimeter)) ||
                     ((strEOF != "") && (string::npos != strNextLine.find(strEOF))))
                {
                    continue;
                }

                strCopy = strNextLine;
                ConfigFile::trim(strCopy);

                if (strCopy != "")
                {
                    line += "\n";
                }
                line += strNextLine;
                bFinish = false;
            }

            ConfigFile::trim(line);
            // add the new config param to the container
            cfg.m_mapContents[strKey] = line;
        }
    }
    fsInputStream.close();
    return CL_SUCCESS;
}

cl_err_code ConfigFile::WriteFile(string fileName, ConfigFile& cf)
{
    std::fstream os(fileName.c_str(), std::ios::out);
    for (ConfigFile::mapci p = cf.m_mapContents.begin(); p != cf.m_mapContents.end(); ++p)
    {
        os << p->first << " " << cf.m_sDelimiter << " ";
        os << p->second << std::endl;
    }
    return CL_SUCCESS;

}

#ifdef _WIN32
template<>
string Intel::OpenCL::Utils::GetRegistryValue<string>(HKEY key, const string& valName, const string& defaultVal)
{
    DWORD regValSize;
    LONG res = RegQueryValueExA(key, valName.c_str(), nullptr, nullptr, nullptr, &regValSize);
    if (ERROR_SUCCESS != res)
    {
        return defaultVal;
    }
    std::vector<BYTE> str(regValSize);
    res = RegQueryValueExA(key, valName.c_str(), nullptr, nullptr, &str[0], &regValSize);
    assert(ERROR_SUCCESS == res);
    if (ERROR_SUCCESS != res)
    {
        return defaultVal;
    }
    return reinterpret_cast<const char*>(&str[0]);
}
#endif


OPENCL_VERSION BasicCLConfigWrapper::GetOpenCLVersion() const
{
    static OPENCL_VERSION s_ver = OPENCL_VERSION_UNKNOWN;
    if (OPENCL_VERSION_UNKNOWN != s_ver)
    {
        return s_ver;
    }
#ifndef INTEL_PRODUCT_RELEASE
    // first look in environment variable or configuration file
    string ver = m_pConfigFile->Read("ForceOCLCPUVersion", string(""));   // we are using this name to be aligned with GEN
    if ("1.2" == ver)
    {
        s_ver = OPENCL_VERSION_1_2;
        return OPENCL_VERSION_1_2;
    }
    else if ("2.0" == ver)
    {
        s_ver = OPENCL_VERSION_2_0;
        return OPENCL_VERSION_2_0;
    }
    else if ("2.1" == ver)
    {
        s_ver = OPENCL_VERSION_2_1;
        return OPENCL_VERSION_2_1;
    }
    else if ("2.2" == ver)
    {
        s_ver = OPENCL_VERSION_2_2;
        return OPENCL_VERSION_2_2;
    }
    // else look in registry/etc
#ifdef _WIN32
    DWORD iVer = m_pConfigFile->GetRegistryOrEtcValue<DWORD>("ForceOCLCPUVersion", 0);
#else
    int iVer = m_pConfigFile->GetRegistryOrEtcValue<int>("ForceOCLCPUVersion", 0);
#endif
    switch (iVer)
    {
    case 1:
        {
            s_ver = OPENCL_VERSION_1_2;
            return OPENCL_VERSION_1_2;
        }
    case 2:
        {
            s_ver = OPENCL_VERSION_2_0;
            return OPENCL_VERSION_2_0;
        }
    case 3:
        {
            s_ver = OPENCL_VERSION_2_1;
            return OPENCL_VERSION_2_1;
        }
    case 4:
        {
            s_ver = OPENCL_VERSION_2_2;
            return OPENCL_VERSION_2_2;
        }
    default:
        break;
    }
#endif // NDEBUG

#ifdef BUILD_OPENCL_21
    s_ver = OPENCL_VERSION_2_1;
    return OPENCL_VERSION_2_1;
#elif defined(BUILD_FPGA_EMULATOR)
    s_ver = OPENCL_VERSION_1_0;
    return OPENCL_VERSION_1_0;
#endif // BUILD_FPGA_EMULATOR

    s_ver = GetOpenclVerByCpuModel();
    return s_ver;
}
