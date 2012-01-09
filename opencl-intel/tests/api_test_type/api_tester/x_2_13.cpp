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
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <share.h> 
#include "Chapter_2_13.h"
#include "Arrangement.h"

Chapter_2_13::Chapter_2_13(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_13::~Chapter_2_13()
{
}

int Chapter_2_13::Run(const char* test, int rc)
{
    rc |= queue_2_13_1(test);
    rc |= queue_2_13_2(test);
    rc |= queue_2_13_3(test);
    rc |= queue_2_13_4(test);
    rc |= empty_2_13_5(test);
    rc |= queue_2_13_6(test);
    rc |= queue_2_13_7(test);

    return rc;
}

int Chapter_2_13::queue_2_13_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.13.1.queue";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.Queue(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_13::queue_2_13_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.13.2.queue";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.Queue(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_13::queue_2_13_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.13.3.queue";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.Queue(CL_QUEUE_PROFILING_ENABLE);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_13::queue_2_13_4(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.13.4.queue";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.Queue(0);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_13::empty_2_13_5(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.13.5.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    // Execute action 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_13::queue_2_13_6(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.13.6.queue";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.Queue(-1);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_13::queue_2_13_7(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.13.7.queue";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.Queue();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    return Finish(&status, TEST_NAME, rc);
}
