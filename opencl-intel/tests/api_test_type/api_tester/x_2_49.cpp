// Copyright (c) 1997-2004 Intel Corporation
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

#include <stdlib.h>
#include "stdafx.h"
#include "Chapter_2_49.h"
#include "Arrangement.h"

Chapter_2_49::Chapter_2_49(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_49::~Chapter_2_49()
{
}

int Chapter_2_49::Run(const char* test, int rc)
{
    rc |= kernel_2_49_1(test);

    return rc;
}

int Chapter_2_49::kernel_2_49_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.49.1.kernel";


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Kernel())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_kernel       kernel = ar.GetKernel();
    int             arg;
    cl_int          ra;
    ra = clSetKernelArg(kernel, 0, sizeof(arg), &arg);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}
