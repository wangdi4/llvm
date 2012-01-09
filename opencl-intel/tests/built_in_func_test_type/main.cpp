// build-in-func_test_type.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CL/cl.h"

#include "windows.h"
#include "winbase.h"
#include <stdlib.h>
#include "Test_6_11_1.h"

tDiscrete       sDiscr;             // Global data for all Chapters and Helper
CBuildInFunc    cDiscr(&sDiscr);
Test_6_11_1     tst_6_11_1(&sDiscr);


typedef int tRun(int rc);

tRun*           pRun[] = {
	&tst_6_11_1.Run,
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
			rc |= (*pRun[i])(rc);
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
	printf("Using \"%s switch [test_name]\". Where:\n");
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
			CParseString	strings(argv[2]);
			if (3 != argc || NULL == strings.GetChapter() || NULL == strings.GetFunction())
			{
				help();
				return PROCESSED_NONE;
			}
			cDiscr.SetChapter(strings.GetChapter());
			cDiscr.SetFunction(strings.GetChapter());
			cDiscr.SetTail(strings.GetTail());
		}
		else if (0 == strcmp("-l", argv[1]))
		{
			cDiscr.EnableDump();

			CParseString	strings(argv[2]);
			if (NULL == strings.GetChapter() || NULL == strings.GetFunction())
			{
				help();
				return PROCESSED_NONE;
			}
			cDiscr.SetChapter(strings.GetChapter());
			cDiscr.SetFunction(strings.GetChapter());
			cDiscr.SetTail(strings.GetTail());
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

