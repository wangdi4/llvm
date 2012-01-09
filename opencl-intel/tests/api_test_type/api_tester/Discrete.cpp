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

#include "stdafx.h"
#include "windows.h"
#include "winbase.h"
#include <stdlib.h>
#include "Chapter_2_1.h"
#include "Chapter_2_2.h"
#include "Chapter_2_3.h"
#include "Chapter_2_4.h"
#include "Chapter_2_5.h"
#include "Chapter_2_6.h"
#include "Chapter_2_7.h"
#include "Chapter_2_8.h"
#include "Chapter_2_9.h"
#include "Chapter_2_10.h"
#include "Chapter_2_11.h"
#include "Chapter_2_12.h"
#include "Chapter_2_13.h"
#include "Chapter_2_14.h"
#include "Chapter_2_15.h"
#include "Chapter_2_16.h"
#include "Chapter_2_17.h"
#include "Chapter_2_18.h"
#include "Chapter_2_19.h"
#include "Chapter_2_20.h"
#include "Chapter_2_21.h"
#include "Chapter_2_22.h"
#include "Chapter_2_23.h"
#include "Chapter_2_24.h"
#include "Chapter_2_25.h"
#include "Chapter_2_26.h"
#include "Chapter_2_27.h"
#include "Chapter_2_28.h"
#include "Chapter_2_29.h"
#include "Chapter_2_30.h"
#include "Chapter_2_31.h"
#include "Chapter_2_32.h"
#include "Chapter_2_33.h"
#include "Chapter_2_34.h"
#include "Chapter_2_35.h"
#include "Chapter_2_36.h"
#include "Chapter_2_37.h"
#include "Chapter_2_38.h"
#include "Chapter_2_39.h"
#include "Chapter_2_40.h"
#include "Chapter_2_41.h"
#include "Chapter_2_43.h"
#include "Chapter_2_44.h"
#include "Chapter_2_45.h"
#include "Chapter_2_46.h"
#include "Chapter_2_47.h"
#include "Chapter_2_48.h"
#include "Chapter_2_49.h"
#include "Chapter_2_50.h"
#include "Chapter_2_51.h"
#include "Chapter_2_52.h"
#include "Chapter_2_53.h"
#include "Chapter_2_54.h"
#include "Chapter_2_55.h"
#include "Chapter_2_56.h"
#include "Chapter_2_57.h"
#include "Chapter_2_58.h"
#include "Chapter_2_59.h"
#include "Chapter_2_60.h"
#include "Chapter_2_61.h"
#include "Chapter_2_62.h"
#include "Chapter_2_63.h"
#include "Chapter_2_64.h"

tDiscrete       sDiscr;             // Global data for all Chapters and Helper
Helper          cDiscr(&sDiscr);
Chapter_2_1     cpt_2_1(&sDiscr);
Chapter_2_2     cpt_2_2(&sDiscr);
Chapter_2_3     cpt_2_3(&sDiscr);
Chapter_2_4     cpt_2_4(&sDiscr);
Chapter_2_5     cpt_2_5(&sDiscr);
Chapter_2_6     cpt_2_6(&sDiscr);
Chapter_2_7     cpt_2_7(&sDiscr);
Chapter_2_8     cpt_2_8(&sDiscr);
Chapter_2_9     cpt_2_9(&sDiscr);
Chapter_2_10    cpt_2_10(&sDiscr);
Chapter_2_11    cpt_2_11(&sDiscr);
Chapter_2_12    cpt_2_12(&sDiscr);
Chapter_2_13    cpt_2_13(&sDiscr);
Chapter_2_14    cpt_2_14(&sDiscr);
Chapter_2_15    cpt_2_15(&sDiscr);
Chapter_2_16    cpt_2_16(&sDiscr);
Chapter_2_17    cpt_2_17(&sDiscr);
Chapter_2_18    cpt_2_18(&sDiscr);
Chapter_2_19    cpt_2_19(&sDiscr);
Chapter_2_20    cpt_2_20(&sDiscr);
Chapter_2_21    cpt_2_21(&sDiscr);
Chapter_2_22    cpt_2_22(&sDiscr);
Chapter_2_23    cpt_2_23(&sDiscr);
Chapter_2_24    cpt_2_24(&sDiscr);
Chapter_2_25    cpt_2_25(&sDiscr);
Chapter_2_26    cpt_2_26(&sDiscr);
Chapter_2_27    cpt_2_27(&sDiscr);
Chapter_2_28    cpt_2_28(&sDiscr);
Chapter_2_29    cpt_2_29(&sDiscr);
Chapter_2_30    cpt_2_30(&sDiscr);
Chapter_2_31    cpt_2_31(&sDiscr);
Chapter_2_32    cpt_2_32(&sDiscr);
Chapter_2_33    cpt_2_33(&sDiscr);
Chapter_2_34    cpt_2_34(&sDiscr);
Chapter_2_35    cpt_2_35(&sDiscr);
Chapter_2_36    cpt_2_36(&sDiscr);
Chapter_2_37    cpt_2_37(&sDiscr);
Chapter_2_38    cpt_2_38(&sDiscr);
Chapter_2_39    cpt_2_39(&sDiscr);
Chapter_2_40    cpt_2_40(&sDiscr);
Chapter_2_41    cpt_2_41(&sDiscr);
Chapter_2_43    cpt_2_43(&sDiscr);
Chapter_2_44    cpt_2_44(&sDiscr);
Chapter_2_45    cpt_2_45(&sDiscr);
Chapter_2_46    cpt_2_46(&sDiscr);
Chapter_2_47    cpt_2_47(&sDiscr);
Chapter_2_48    cpt_2_48(&sDiscr);
Chapter_2_49    cpt_2_49(&sDiscr);
Chapter_2_50    cpt_2_50(&sDiscr);
Chapter_2_51    cpt_2_51(&sDiscr);
Chapter_2_52    cpt_2_52(&sDiscr);
Chapter_2_53    cpt_2_53(&sDiscr);
Chapter_2_54    cpt_2_54(&sDiscr);
Chapter_2_55    cpt_2_55(&sDiscr);
Chapter_2_56    cpt_2_56(&sDiscr);
Chapter_2_57    cpt_2_57(&sDiscr);
Chapter_2_58    cpt_2_58(&sDiscr);
Chapter_2_59    cpt_2_59(&sDiscr);
Chapter_2_60    cpt_2_60(&sDiscr);
Chapter_2_61    cpt_2_61(&sDiscr);
Chapter_2_62    cpt_2_62(&sDiscr);
Chapter_2_63    cpt_2_63(&sDiscr);
Chapter_2_64    cpt_2_64(&sDiscr);

typedef int tRun(const char* s, int rc);

tRun*           pRun[] = {
    &cpt_2_1.Run,
    &cpt_2_2.Run, 
    &cpt_2_3.Run,
    &cpt_2_4.Run,
    &cpt_2_5.Run,
    &cpt_2_6.Run,
    &cpt_2_7.Run,
    &cpt_2_8.Run,
    &cpt_2_9.Run, 
    &cpt_2_10.Run,
    &cpt_2_11.Run,
    &cpt_2_12.Run,
    &cpt_2_13.Run,
    &cpt_2_14.Run,
    &cpt_2_15.Run,
    &cpt_2_16.Run,
    &cpt_2_17.Run,
    &cpt_2_18.Run,
    &cpt_2_19.Run,
    &cpt_2_20.Run,
    &cpt_2_21.Run,
    &cpt_2_22.Run,
    &cpt_2_23.Run,
	&cpt_2_24.Run,
	&cpt_2_25.Run,
    &cpt_2_26.Run,
    &cpt_2_27.Run,
	&cpt_2_28.Run,
	&cpt_2_29.Run,
	&cpt_2_30.Run,
	&cpt_2_31.Run,
	&cpt_2_32.Run,
    &cpt_2_33.Run,
    &cpt_2_34.Run,
    &cpt_2_35.Run,
    &cpt_2_36.Run,
    &cpt_2_37.Run,
    &cpt_2_38.Run,
    &cpt_2_39.Run,
    &cpt_2_40.Run,
    &cpt_2_41.Run,
	&cpt_2_43.Run,
	&cpt_2_44.Run,
    &cpt_2_45.Run,
    &cpt_2_46.Run,
    &cpt_2_47.Run,
    &cpt_2_48.Run,
    &cpt_2_49.Run,
    &cpt_2_50.Run,
	&cpt_2_51.Run,
	&cpt_2_52.Run,
    &cpt_2_53.Run,
	&cpt_2_54.Run,
	&cpt_2_55.Run,
	&cpt_2_56.Run,
	&cpt_2_57.Run,
	&cpt_2_58.Run,
	&cpt_2_59.Run,
	&cpt_2_60.Run,
	&cpt_2_61.Run,
	&cpt_2_62.Run,
	&cpt_2_63.Run,
	&cpt_2_64.Run,
    NULL
};

LONG WINAPI Filter(__in  struct _EXCEPTION_POINTERS *ExceptionInfo)
{
    printf("Fatal exception. Succeded %d tests. Some of tests failed (%d of %d)\n", 
            cDiscr.GetSucceeded(), cDiscr.GetFailed(), cDiscr.GetSucceeded() + cDiscr.GetFailed());
    TerminateProcess(GetCurrentProcess(), PROCESSED_FAIL);
    return -1;
}

int exception(int rc)
{
    int         i;
    SetUnhandledExceptionFilter(&Filter);
    __try
    {
        for(i = 0; ; i++)
        {
            if (pRun[i] == NULL)
                break;
            rc |= (*pRun[i])("", rc);
        }
    }
    __except(1)
    {
        cDiscr.SetException();
        printf("__try exception.\n");
        TerminateProcess(GetCurrentProcess(), PROCESSED_FAIL);
    }
    return rc;
}

void help()
{
    printf("Using \"%s switch [test_name]\". Where:\n", TESTER);
    printf("    -a           -- run all tests\n");
    printf("    -t test_name -- run the specific test(s)\n");
    printf("    -l test_name -- dump name(s)\n");
}

int _tmain(int argc, _TCHAR* argv[])
{
    switch(argc)
    {
    case 2:
        if (0 == strcmp("-a", argv[1]))
        {
            cDiscr.SetChapter("*");
            cDiscr.SetFunction("*");
            cDiscr.SetTail("*");
            break;
        }
        else if (0 == strcmp("-l", argv[1]))
        {
            cDiscr.EnableDump();
            
            cDiscr.SetChapter("*");
            cDiscr.SetFunction("*");
            cDiscr.SetTail("*");
        }
        break;

    case 1:
        help();
        return PROCESSED_NONE;

    default:
        if (0 == strcmp(argv[1], "-t") && 3 == argc)
        {
            char*           first_dot = strchr(argv[2], '.');
            char*           second_dot = strchr(first_dot + 1, '.');
            char*           third_dot = strchr(second_dot + 1, '.');
            if (3 != argc || NULL == first_dot || NULL == second_dot || NULL == third_dot)
            {
                help();
                return PROCESSED_NONE;
            }
            *first_dot = 0;
            *second_dot = 0;
            *third_dot = 0;
            cDiscr.SetChapter(first_dot + 1);
            cDiscr.SetFunction(second_dot + 1);
            cDiscr.SetTail(third_dot + 1);
        }
        else if (0 == strcmp("-l", argv[1]))
        {
            cDiscr.EnableDump();
            
            char*           first_dot = strchr(argv[2], '.');
            char*           second_dot = strchr(first_dot + 1, '.');
            char*           third_dot = strchr(second_dot + 1, '.');
            if (NULL == first_dot || NULL == second_dot || NULL == third_dot)
            {
                help();
                return PROCESSED_NONE;
            }
            *first_dot = 0;
            *second_dot = 0;
            *third_dot = 0;
            cDiscr.SetChapter(first_dot + 1);
            cDiscr.SetFunction(second_dot + 1);
            cDiscr.SetTail(third_dot + 1);

            break;
        }
        else
        {
            help();
            return PROCESSED_NONE;
        }
    }

    // Execution tests
    int             rc = PROCESSED_OK; 
    while(cDiscr.IsDone() == false)
    {
        cDiscr.ResetStatistic();
        cDiscr.SetDone();
        exception(rc);
    }

    char            str[128];
    if (PROCESSED_OK == rc && 0 == cDiscr.GetFailed() && 0 != cDiscr.GetSucceeded())
        sprintf_s(str, 127, "Run 2.%s.%s.%s. All tests succeeded (%d)\n", 
			cDiscr.GetChapter(), cDiscr.GetFunction(), cDiscr.GetTail(), cDiscr.GetSucceeded());
    else
        sprintf_s(str, 127, "Run 2.%s.%s.%s. Succeded %d tests. Some of tests failed (%d of %d)\n", 
            cDiscr.GetChapter(), cDiscr.GetFunction(), cDiscr.GetTail(), cDiscr.GetSucceeded(), 
			cDiscr.GetFailed(), cDiscr.GetSucceeded() + cDiscr.GetFailed());

    if ((cDiscr.GetSucceeded() + cDiscr.GetFailed() >= 1) && true == cDiscr.IsProcessed())
        printf("%s", str);

    if (false == cDiscr.IsProcessed())
    {
        if (0 == strcmp(argv[1], "-t"))
            printf("Run 2.%s.%s.%s. The test doesn't exist\n", cDiscr.GetChapter(), cDiscr.GetFunction(), cDiscr.GetTail());
        rc = PROCESSED_NONE;
    }
    _flushall();

    return rc;
}

