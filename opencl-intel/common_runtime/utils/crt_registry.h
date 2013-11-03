// Copyright (c) 2006-2007 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly
//
//  Original author: rjiossy
///////////////////////////////////////////////////////////
#pragma once

#include "crt_types.h"
#include "stdlib.h"

namespace OCLCRT
{
namespace Utils
{

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

inline bool GetCpuPathFromRegistry( char *pCpuPath )
{
#if defined( _WIN32 )
    const char *regPath = "SOFTWARE\\Intel\\OpenCL";

    // pCpuPath is expected to be MAX_PATH in size
    if( NULL != pCpuPath )
    {
        return GetStringValueFromRegistry( HKEY_LOCAL_MACHINE, regPath, "cpu_path", pCpuPath, MAX_PATH );
    }
#endif
    return false;
}

// Will return true when OpenCL API Debugger is enabled
// Debugger is enabled when a PID-specific or global key exists, with value CL_CONFIG_API_DBG_ENABLE=1
bool isAPIDebuggingEnabled();

} // namespace Utils
} // namespace OCLCRT