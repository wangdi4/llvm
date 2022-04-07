// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
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

#pragma once

#include <string>

#include "crt_types.h"


namespace OCLCRT
{
namespace Utils
{

inline std::string FormatLibNameForOS( std::string &libName )
{
    std::string retName;
#if defined( _WIN64 )
    retName = libName + "64.dll";
#elif defined( _WIN32 )
    retName = libName + "32.dll";
#elif defined( __linux__ )
    retName = "lib" + libName + ".so";
#endif
    return retName;
}

#if defined( _WIN32 )
inline bool GetStringValueFromRegistry( HKEY       top_hkey,
                                        const char *keyPath,
                                        const char *valueName,
                                        char       *retValue,
                                        DWORD      size )
{
    HKEY hkey;

    // Open the registry path. hkey will hold the entry
    LONG retCode = RegOpenKeyExA(
        top_hkey,                   // hkey
        keyPath,                    // lpSubKey
        0,                          // ulOptions
        KEY_READ,                   // samDesired
        &hkey                       // phkResult
        );

    if( ERROR_SUCCESS == retCode )
    {
        // Get the value by name from the key
        retCode = RegQueryValueExA(
            hkey,                   // hkey
            valueName,              // lpValueName
            0,                      // lpReserved
            NULL,                   // lpType
            ( LPBYTE )retValue,     // lpData
            &size                   // lpcbData
            );

        // Close the key - we don't need it any more
        RegCloseKey( hkey );

        if( ERROR_SUCCESS == retCode )
        {
            return true;
        }
    }

    return false;
}

#endif

inline bool GetCpuPathFromRegistry( const std::string valueName, std::string &cpuPath )
{
#if defined( _WIN32 )
    const char *regPath = "SOFTWARE\\Intel\\OpenCL";
    char pCpuPath[MAX_PATH];
	bool retVal = GetStringValueFromRegistry( HKEY_LOCAL_MACHINE, regPath, valueName.c_str(), pCpuPath, MAX_PATH );

    if( retVal )
    {
        cpuPath = pCpuPath;
#if defined( _WIN64 )
        cpuPath = cpuPath + "\\bin\\x64";
#else // _WIN32
        cpuPath = cpuPath + "\\bin\\x86";
#endif
    }
    return retVal;

#endif
    return false;
}

} // namespace Utils
} // namespace OCLCRT
