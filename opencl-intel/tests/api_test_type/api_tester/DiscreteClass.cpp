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

#include <stdio.h>
#include <string.h>
#include "DiscreteClass.h"

Discrete::Discrete(void)
{
    m_bList = false;
    m_bProcessed = false;
    m_bDone = false;
    m_Succeeded = 0;
    m_Failed = 0;
    m_Chapter = NULL;
    m_Function = NULL;
    m_Tail = NULL;
}

Discrete::~Discrete(void)
{
}

tDiscrete*      Helper::m_pDiscrete;

Helper::Helper(tDiscrete* pDiscr)
{
    m_pDiscrete = pDiscr;
}

Helper::~Helper(void)
{
}

void Helper::EnableDump()
{
    m_pDiscrete->m_bList = true;
}

void Helper::SetProcessed()
{
    m_pDiscrete->m_bProcessed = true;
}

void Helper::ResetStatistic()
{
    m_pDiscrete->m_Succeeded = 0;
    m_pDiscrete->m_Failed = 0;
}

void Helper::SetDone()
{
    m_pDiscrete->m_bDone = true;
}

void Helper::SetException()
{
    m_pDiscrete->m_bDone = false;
}

void Helper::SetChapter(char* cpt)
{
    m_pDiscrete->m_Chapter = cpt;
}

void Helper::SetFunction(char* func)
{
    m_pDiscrete->m_Function = func;
}

void Helper::SetTail(char* tail)
{
    m_pDiscrete->m_Tail = tail;
}

char* Helper::GetChapter(void)
{
    return m_pDiscrete->m_Chapter;
}

char* Helper::GetFunction(void)
{
    return m_pDiscrete->m_Function;
}

char* Helper::GetTail(void)
{
    return m_pDiscrete->m_Tail;
}

void Helper::SetTestChapter(char* cpt)
{
    m_pDiscrete->m_TestChapter = cpt;
}

void Helper::SetTestFunction(char* func)
{
    m_pDiscrete->m_TestFunction = func;
}

void Helper::SetTestTail(char* tail)
{
    m_pDiscrete->m_TestTail = tail;
}

char* Helper::GetTestChapter(void)
{
    return m_pDiscrete->m_TestChapter;
}

char* Helper::GetTestFunction(void)
{
    return m_pDiscrete->m_TestFunction;
}

char* Helper::GetTestTail(void)
{
    return m_pDiscrete->m_TestTail;
}

bool Helper::IsProcessed()
{
    return m_pDiscrete->m_bProcessed;
}

bool Helper::IsDump()
{
    return m_pDiscrete->m_bList;
}

bool Helper::IsDone()
{
    return m_pDiscrete->m_bDone;
}

bool Helper::Start(tTestStatus* status, const char* test)
{
    char        _test[128];
    memset(_test, 0, sizeof(_test));
    strcpy_s(_test, strlen(_test) - 1, test);
    char*           first_dot = strchr(_test, '.');
    char*           second_dot = strchr(first_dot + 1, '.');
    char*           third_dot = strchr(second_dot + 1, '.');
    *first_dot = 0;
    *second_dot = 0;
    *third_dot = 0;
    SetTestChapter(first_dot + 1);
    SetTestFunction(second_dot + 1);
    SetTestTail(third_dot + 1);
    if (0 == strcmp(GetChapter(), "*"))
        ;   // All chapters
    else
    {
        if (0 == strcmp(GetChapter(), first_dot + 1))
            ;   // This specific chapter
        else
            return true; // another specific chapter
    }
    if (0 == strcmp(GetFunction(), "*"))
        ;   // All functions of this chapter
    else
    {
        if (0 == strcmp(GetFunction(), second_dot + 1))
            ;   // This specific function
        else
            return true;    // Another specific function
    }
    if (0 == strcmp(GetTail(), "*"))
        ;   // All tests of this chapter and function
    else
    {
        if (0 == strcmp(GetTail(), third_dot + 1))
            ;   // This test
        else
            return true; // another specific tail
    }
    SetProcessed();      // mark-up a test is going to run
    if (IsDump())
    {
        printf("%s -t %s\n", TESTER, test);
        _flushall();
        return true;
    }

    // If the test was executed, avoid to run now
    if (TEST_SUCCESSED == *status)
    {
        m_pDiscrete->m_Succeeded++;
        return true;
    }
    else if (TEST_FAILED == *status)
    {
        m_pDiscrete->m_Failed++;
        return true;
    }
    else if (TEST_STARTED == *status)
    {
        // If the test was started, but wasn't finished - mark it failed and avoid to execute it second time.
        m_pDiscrete->m_Failed++;
        *status = TEST_FAILED;
        printf("Test %-18s failed. Caused GPF\n", test);
        return true;
    }
    *status = TEST_STARTED;

    return false;
}

int Helper::GetSucceeded()
{
    return m_pDiscrete->m_Succeeded;
}

int Helper::GetFailed()
{
    return m_pDiscrete->m_Failed;
}

int Helper::Finish(tTestStatus* status, const char* TEST_NAME, const int rc)
{
    char*       exec;
    if (PROCESSED_OK == rc)
    {
        m_pDiscrete->m_Succeeded++;
        exec = "succeeded";
        *status = TEST_SUCCESSED;
    }
    else
    {
        m_pDiscrete->m_Failed++;
        exec = "failed";
        *status = TEST_FAILED;
    }

    _flushall();
    printf("Test %-18s %s\n", TEST_NAME, exec);
    _flushall();
    return rc;
}
