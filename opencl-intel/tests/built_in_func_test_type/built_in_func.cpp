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
#include <assert.h>
#include <malloc.h>
#include "built_in_func.h"

#pragma warning(disable:4996)

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
	if (m_Chapter)
		free(m_Chapter);
	if (m_Function)
		free(m_Function);
	if (m_Tail)
		free(m_Tail);
}

tDiscrete*      CBuildInFunc::m_pDiscrete;

CParseString::CParseString(const char *str)
{
	assert(str);
	int		lenght = strlen(str);
	m_str = new char[lenght + 1];
	assert(m_str);
	m_str[lenght] = 0;
	strcpy(m_str, str);
	m_first_semicolon = strchr(m_str, ':');
	if (m_first_semicolon == NULL)
		m_last_semicolon = NULL;
	else
	{
		*m_first_semicolon = 0;
		m_last_semicolon = strrchr(m_first_semicolon + sizeof(str[0]), ':');
		if (m_last_semicolon)
			*m_last_semicolon = 0;
	}
}

CParseString::~CParseString()
{
	if (m_str)
		delete m_str;
}

char* CParseString::GetChapter()
{
	return m_str;
}

char* CParseString::GetFunction()
{
	return m_first_semicolon ? m_first_semicolon + sizeof(m_str[0]) : NULL;
}

char* CParseString::GetTail()
{
	return m_last_semicolon ? m_last_semicolon + sizeof(m_str[0]) : NULL;
}

CBuildInFunc::CBuildInFunc(tDiscrete* pDiscr)
{
    m_pDiscrete = pDiscr;
}

CBuildInFunc::~CBuildInFunc(void)
{
}

void CBuildInFunc::EnableDump()
{
    m_pDiscrete->m_bList = true;
}

void CBuildInFunc::SetProcessed()
{
    m_pDiscrete->m_bProcessed = true;
}

void CBuildInFunc::ResetStatistic()
{
    m_pDiscrete->m_Succeeded = 0;
    m_pDiscrete->m_Failed = 0;
}

void CBuildInFunc::SetDone()
{
    m_pDiscrete->m_bDone = true;
}

void CBuildInFunc::SetException()
{
    m_pDiscrete->m_bDone = false;
}

void CBuildInFunc::SetChapter(char* cpt)
{
    m_pDiscrete->m_Chapter = strdup(cpt);
}

void CBuildInFunc::SetFunction(char* func)
{
    m_pDiscrete->m_Function = strdup(func);
}

void CBuildInFunc::SetTail(char* tail)
{
	if (tail)
		m_pDiscrete->m_Tail = strdup(tail);
	else
		m_pDiscrete->m_Tail = NULL;
}

char* CBuildInFunc::GetChapter(void)
{
    return m_pDiscrete->m_Chapter;
}

char* CBuildInFunc::GetFunction(void)
{
    return m_pDiscrete->m_Function;
}

char* CBuildInFunc::GetTail(void)
{
    return m_pDiscrete->m_Tail;
}

void CBuildInFunc::SetTestChapter(const char* cpt)
{
    m_pDiscrete->m_TestChapter = cpt;
}

void CBuildInFunc::SetTestFunction(const char* func)
{
    m_pDiscrete->m_TestFunction = func;
}

void CBuildInFunc::SetTestTail(const char* tail)
{
    m_pDiscrete->m_TestTail = tail;
}

const char* CBuildInFunc::GetTestChapter(void)
{
    return m_pDiscrete->m_TestChapter;
}

const char* CBuildInFunc::GetTestFunction(void)
{
    return m_pDiscrete->m_TestFunction;
}

const char* CBuildInFunc::GetTestTail(void)
{
    return m_pDiscrete->m_TestTail;
}

bool CBuildInFunc::IsProcessed()
{
    return m_pDiscrete->m_bProcessed;
}

bool CBuildInFunc::IsDump()
{
    return m_pDiscrete->m_bList;
}

bool CBuildInFunc::IsDone()
{
    return m_pDiscrete->m_bDone;
}

bool CBuildInFunc::Start(tTestStatus* status, const char* test, const char* func, const char* tail)
{
	// chapter, function, tail are assigned according the command line
    if (0 == strcmp(GetChapter(), "*"))
        ;   // All chapters
    else
    {
        if (0 == strcmp(GetChapter(), test))
            ;   // This specific chapter
        else
            return true; // another specific chapter
    }
    if (0 == strcmp(GetFunction(), "*"))
        ;   // All functions of this chapter
    else
    {
        if (0 == strcmp(GetFunction(), func))
            ;   // This specific function
        else
            return true;    // Another specific function
    }
    if (0 == strcmp(GetTail(), "*"))
        ;   // All tests of this chapter and function
    else
    {
        if (0 == strcmp(GetTail(), tail))
            ;   // This test
        else
            return true; // another specific tail
    }
	// assign chapter, function, tail for error report. it will not influence values returned by GetChapter(), GetFunction(), GetTail()
	SetTestChapter(test);
	SetTestFunction(func);
	SetTestTail(tail);
    // mark-up a test is going to run
	SetProcessed();
    if (IsDump())
    {
		printf("%s -t %s:%s%s%s\n", TESTER, test, func, NULL == tail ? "" : ":", NULL == tail ? "" : tail);
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

int CBuildInFunc::GetSucceeded()
{
    return m_pDiscrete->m_Succeeded;
}

int CBuildInFunc::GetFailed()
{
    return m_pDiscrete->m_Failed;
}

int CBuildInFunc::Finish(tTestStatus* status, const char* test, const char* func, const char* tail, const int rc)
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
	printf("Test %-18s %s:%s%s%s\n", test, func, NULL == tail ? "" : ":", NULL == tail ? "" : tail, exec);
    _flushall();
    return rc;
}
