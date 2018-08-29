// Copyright (c) 2018 Intel Corporation
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
// Regression test to validate
//

#include <iostream>
#include <string.h>
#include "test_utils.h"
#include <stdio.h>

extern cl_device_type gDeviceType;

bool cl_CheckBuildNumber() {
    cl_int iRet = CL_SUCCESS;
    cl_platform_id platform = 0;
    bool bResult = true;
    cl_device_id device = NULL;

    try
    {
        iRet = clGetPlatformIDs(1, &platform, NULL);
        CheckException("clGetPlatformIDs", CL_SUCCESS, iRet);
        iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
        CheckException("clGetDeviceIDs", CL_SUCCESS, iRet);

        char Buffer[1024];
        iRet = clGetDeviceInfo(device, CL_DRIVER_VERSION, 1024, Buffer, nullptr);
        CheckException("clGetDeviceIDs", CL_SUCCESS, iRet);

        Buffer[1024 - 1] = 0;
        cout << "CL_DRIVER_VERSION: " << Buffer << '\n';

        // The expected string in Buffer should be 'XX.Y.Z.MMDD'.
        // Let's validate build date 'MMDD' only
        // For Aug-03 MMDD should be 0803

        // 2. Check for 'MMDD' string:
        const char *S = strrchr(Buffer, '.');
        if (!S) {
            CheckException("'.' symbol is missed", true, false);
        }
        S++;
        int i = 0;
        while (i < 4) {
            if (!isdigit(S[i])) {
                // Only digits are allowed
                CheckException("MMDD", true, false);
            }
            i++;
        }
        if (S[i]) {
            CheckException("Zero symbol was expected here", true, false);
        }

        // First two symbols can not be more than 12 (Month):
        char Date[3];
        Date[0] = S[0];
        Date[1] = S[1];
        Date[2] = 0;
        int Val = atoi(Date);
        CheckException("Month", Val <= 12, true);

        // Next two symbols can not be more than 31 (Day):
        Date[0] = S[2];
        Date[1] = S[3];
        Val = atoi(Date);
        CheckException("Day", Val <= 31, true);
    }
    catch (const std::exception&)
    {
        bResult = false;
    }

    return bResult;
}
