// Copyright (c) 2013 Intel Corporation
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
///////////////////////////////////////////////////////////

#include "crt_registry.h"
#include <string>
#include <sstream>
#include <algorithm>

#if defined(_WIN32)
#include <windows.h>
#endif

// Will return true when OpenCL API Debugger is enabled
// Debugger is enabled when a PID-specific or global key exists, with value CL_CONFIG_API_DBG_ENABLE=1
bool OCLCRT::Utils::isAPIDebuggingEnabled()
{
#if defined(_WIN32)
    const DWORD size = 2048;
    char retVal[size];

    const std::string regKeyPath    = "Software\\Intel\\OpenCL\\API_Debugger\\";
    const char * regValName         = "CL_CONFIG_API_DBG_ENABLE";

    // check value of CL_CONFIG_API_DBG_ENABLE
    std::stringstream pidSString;
    pidSString << GetCurrentProcessId();
    std::string pidPath = regKeyPath + pidSString.str();

    if( GetStringValueFromRegistry( HKEY_CURRENT_USER, pidPath.c_str(), regValName, retVal, size ) &&
        !strcmp( retVal, "1" ) )
    {
        return true;
    }
#endif
    return false;
}
