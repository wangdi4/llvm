// Copyright (c) 1997-2004 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTO_S_/_"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
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

#pragma once
#include "CL\cl.h"

#define PROCESSED_OK    0
#define PROCESSED_FAIL  1
#define PROCESSED_NONE  -1

#define INVALID_VALUE   -1

const char          TESTER[] = "api_tester.exe";

enum tTestStatus {
    TEST_STARTED,
    TEST_FAILED,
    TEST_SUCCESSED,
    TEST_UNTOUCHED
};

class Discrete
{
public:
    bool            m_bProcessed;
    bool            m_bList;
    bool            m_bDone;
    int             m_Succeeded;
    int             m_Failed;
    char*           m_Chapter;
    char*           m_Function;
    char*           m_Tail;
    // each test will change the following data to make info available accross exceptions
    char*           m_TestChapter;
    char*           m_TestFunction;
    char*           m_TestTail;
                    Discrete(void);
                    ~Discrete(void);
} ;
typedef class Discrete tDiscrete;

class Helper
{
private:
    static tDiscrete*      m_pDiscrete;
public:
                    Helper(tDiscrete* pDiscr);
                    ~Helper(void);

    static void     EnableDump(void);
    static void     SetProcessed(void);
    static void     ResetStatistic(void);
    
    static void     SetChapter(char* cpt);
    static void     SetFunction(char* func);
    static void     SetTail(char* tail);
    static char*    GetChapter(void);
    static char*    GetFunction(void);
    static char*    GetTail(void);

    static void     SetTestChapter(char* cpt);
    static void     SetTestFunction(char* func);
    static void     SetTestTail(char* tail);
    static char*    GetTestChapter(void);
    static char*    GetTestFunction(void);
    static char*    GetTestTail(void);

    static bool     IsProcessed(void);
    static bool     IsDump(void);
    static bool     IsAllFunc(void);
    static bool     IsDone(void);

    static void     SetDone(void);
    static void     SetException(void);

    static bool     Start(tTestStatus* status, const char* test);        // Returns false for continue, true for exit
    static int      Finish(tTestStatus* status, const char* test, const int rc);

    static int      GetSucceeded();
    static int      GetFailed();
};
