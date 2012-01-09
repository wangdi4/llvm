// api_set_type.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"
#include "winbase.h"
#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "..\api_tester\DiscreteClass.h"

const char          TEMP_FILE[] = "Result_DeleteMe.txt";
const char          LIST_FILE[] = "List_DeleteMe.txt";
const int           WAIT_TIME = 10 * 1000;       // 10 seconds

class RunEXE
{
private:
    public:

    PROCESS_INFORMATION m_Info;
    HANDLE              m_hCommunication;

public:
    int    Run(const char* cmd, const char* fout);
    HANDLE GetProcessHandle()   {return m_Info.hProcess;}
    HANDLE GetCommunication()   {return m_hCommunication;}
           RunEXE();
           ~RunEXE();
};

RunEXE::RunEXE()
{
}

RunEXE::~RunEXE()
{
    CloseHandle(m_Info.hThread);
    CloseHandle(m_Info.hProcess);
    CloseHandle(m_hCommunication);
}

int RunEXE::Run(const char* cmd, const char* fout)
{
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = true;

    m_hCommunication = CreateFile(
        fout, 
        GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (m_hCommunication == NULL)
        return PROCESSED_NONE;

	memset(&m_Info, 0, sizeof(PROCESS_INFORMATION));
    STARTUPINFO         startup;
    memset(&startup, 0, sizeof(startup));
    startup.cb = sizeof(startup);
    startup.dwFlags  = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    startup.hStdOutput = m_hCommunication;
    startup.hStdInput = (HANDLE)0;
    startup.hStdError = (HANDLE)2;
    startup.wShowWindow  = SW_SHOWDEFAULT;

    bool                b = CreateProcess(
        NULL, (LPSTR)cmd,
        &sa, NULL, true, 0, NULL, NULL, &startup, &m_Info);
    if (!b)
    {
        char        str[4096];
        FormatMessage(
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                GetLastError(),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) str,
                0, NULL );
        int i=0;
    }

    // Display the     }
    return b ? PROCESSED_OK : PROCESSED_FAIL;
}

void help()
{
    printf("Using \"api_executer switch <parameter>\n");
    printf("    -e -- runs the existing test list. Parameter is <test_list>\n");
    printf("    -l -- builds list of all available tests, but doesn't run them. Parameter is <test_list>\n");
    printf("    -a -- builds list and runs all available tests. Parameter is absent.\n");
    printf("    -t -- builds and runs the test list. Parameter is <test_name>.\n");
}

int ExecuteBAT(const char* test, const char* fName)
{
    // open the batch file
    errno_t             rf;
    FILE*               bat;
    rf = fopen_s(&bat, fName, "r");
    if (NULL == bat)
        return PROCESSED_NONE;
    char                cmd_line[4096];
    int                 succeeded = 0;
    int                 failed = 0;
    while(NULL != fgets(cmd_line, sizeof(cmd_line), bat))
    {
        RunEXE*             pTest;
        pTest = new RunEXE();
        char*               new_line = strchr(cmd_line, 0x0D);
        if (NULL != new_line)
            *new_line = 0;
        new_line = strchr(cmd_line, 0x0A);
        if (NULL != new_line)
            *new_line = 0;
        int                 rc = pTest->Run(cmd_line, TEMP_FILE);
        if (PROCESSED_OK == rc)
        {
            switch(WaitForSingleObject(pTest->GetProcessHandle(), WAIT_TIME))
            {
            case WAIT_ABANDONED:
            case WAIT_TIMEOUT:
            case WAIT_FAILED:
                TerminateProcess(pTest->GetProcessHandle(), PROCESSED_FAIL);
                failed++;
                printf("Test '%s' has been aborted and considered as failed\n", cmd_line);
                break;
            case WAIT_OBJECT_0:
                char                result[4096];
                DWORD               rsize;
                SetFilePointer(pTest->GetCommunication(), 0, 0, FILE_BEGIN);
                ReadFile(pTest->GetCommunication(), result, sizeof(result), &rsize, NULL);
                if (0 != rsize && NULL != strstr(result, "succeeded"))
                {
                    succeeded++;
                    result[rsize] = 0;
                    printf("Test '%s' succeeded\n", cmd_line);
                }
                else
                {
                    failed++;
                    printf("Test '%s' failed\n", cmd_line);
                }
                result[rsize] = 0;
                break;
            }
        }
        delete pTest;           // invoke destructor to close communication file with "api_test_type.exe"
        remove(TEMP_FILE);
    }
    fclose(bat);
    if (failed)
    {
        printf("Run %s. Succeeded %d tests. Some of tests failed (%d of %d)\n", test, succeeded, failed, succeeded + failed);
        return PROCESSED_FAIL;
    }
    else
    {
        if (0 != succeeded + failed)
        {
            printf("Run %s. All tests succeeded (%d)\n", test, succeeded);
            return PROCESSED_OK;
        }
        else
        {
            printf("Run %s. The test doesn't exist\n", test);
            return PROCESSED_NONE;
        }
    }
}

int CreateBAT(const char* fName)
{
    char                cmd[4096];
    sprintf_s(cmd, sizeof(cmd), "%s -l", TESTER);
    RunEXE              test;
    int                 rc = test.Run(cmd, fName);
    if (PROCESSED_OK == rc)
    {
        switch(WaitForSingleObject(test.GetProcessHandle(), WAIT_TIME))
        {
        case WAIT_ABANDONED:
        case WAIT_TIMEOUT:
        case WAIT_FAILED:
            TerminateProcess(test.GetProcessHandle(), PROCESSED_FAIL);
            rc = PROCESSED_FAIL;
            break;
        case WAIT_OBJECT_0:
            rc = PROCESSED_OK;
            break;
        }
    }
    else
        rc = PROCESSED_FAIL;
    return rc;
}

int CreateSet(const char* test)
{
    RunEXE*             pTest;
    pTest = new RunEXE;
    char                cmd[4096];
    sprintf_s(cmd, sizeof(cmd), "%s -l %s", TESTER, test);
    int                 rc = pTest->Run(cmd, LIST_FILE);
    if (PROCESSED_OK == rc)
    {
        switch(WaitForSingleObject(pTest->GetProcessHandle(), WAIT_TIME))
        {
        case WAIT_ABANDONED:
        case WAIT_TIMEOUT:
        case WAIT_FAILED:
            rc = PROCESSED_FAIL;
            break;
        case WAIT_OBJECT_0:
            rc = PROCESSED_OK;
           break;
        }
    }
    else
        rc = PROCESSED_FAIL;
    delete pTest;           // invoke destructor to close communication file with "api_test_type.exe"
    return rc;
}

int _tmain(int argc, _TCHAR* argv[])
{
    switch(argc)
    {
    case 1:
        help();
        return PROCESSED_NONE;

    case 2:
        if (0 == strcmp(argv[1], "-a"))
        {
            int             rc;
            rc = CreateBAT(LIST_FILE);
            if (rc == PROCESSED_NONE)
                return rc;
            rc = ExecuteBAT("all tests", LIST_FILE);
            remove(LIST_FILE);
            return rc;
        }

    case 3:
        if (0 == strcmp(argv[1], "-e"))
        {
            int             rc;
            rc = ExecuteBAT("early build test set", argv[2]);
            return rc;
        }
        else if (0 == strcmp(argv[1], "-t"))
        {
            int             rc;
            rc = CreateSet(argv[2]);
            if (rc != PROCESSED_OK)
                return rc;
            rc = ExecuteBAT(argv[2], LIST_FILE);
            remove(LIST_FILE);
            return rc;
        }
        else if (0 == strcmp(argv[1], "-l"))
        {
            return CreateBAT(argv[2]);
        }
        // Fall through default
    default:
        {
            help();
            return PROCESSED_NONE;
        }
    }
    
    return 0;
}

