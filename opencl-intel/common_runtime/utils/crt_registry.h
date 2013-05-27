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
    inline bool GetCpuPathFromRegistry( char* pCpuPath )
    {
#if defined(_WIN32)
        const char* regPath = "SOFTWARE\\Intel\\OpenCL";
        HKEY  key           = NULL;
        void* pData         = NULL;
        DWORD dataSize      = 0;
        DWORD regType       = REG_SZ;
        DWORD success       = ERROR_SUCCESS;
        bool  retVal        = true;

        // pCpuPath is expected to be MAX_PATH in size
        if( pCpuPath == NULL )
        {
            retVal = false;
        }

        if( retVal == true )
        {
            success = RegOpenKeyExA( 
                HKEY_LOCAL_MACHINE, 
                regPath, 
                0,
                KEY_READ | KEY_WOW64_64KEY,
                &key );

            if( ERROR_SUCCESS == success )
            {
                // set data and size
                dataSize = MAX_PATH;

                success = RegQueryValueExA( 
                    key, 
                    "cpu_path",
                    NULL,
                    NULL,
                    (BYTE*)pCpuPath,
                    &dataSize );

                // close key
                RegCloseKey( key );
            }
            else
            {
                retVal = false;
            }
        }

        return retVal;
#else
        return false;
#endif
    }
} // namespace Utils
} // namespace OCLCRT