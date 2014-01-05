/*****************************************************************************\

Copyright 2000 - 2008 Intel Corporation All Rights Reserved.

    The source code contained or described herein and all documents related to
    the source code ("Material") are owned by Intel Corporation or its suppliers
    or licensors. Title to the Material remains with Intel Corporation or its
    suppliers and licensors. The Material contains trade secrets and proprietary
    and confidential information of Intel or its suppliers and licensors. The
    Material is protected by worldwide copyright and trade secret laws and
    treaty provisions. No part of the Material may be used, copied, reproduced,
    modified, published, uploaded, posted, transmitted, distributed, or
    disclosed in any way without Intel's prior express written permission.

    No license under any patent, copyright, trade secret or other intellectual
    property right is granted to or conferred upon you by disclosure or delivery
    of the Materials, either expressly, by implication, inducement, estoppel or
    otherwise. Any license under such intellectual property rights must be
    express and approved by Intel in writing.

File Name: os_inc.h

Abstract: 

Notes:THIS IS A WINDOWS SPECIFIC FILE

\*****************************************************************************/
#pragma once

#include <windows.h>
#include <Mmsystem.h>
#include <cstdio>
#include <cstdlib>
#include <Logger.h>

#ifndef NTSTATUS
#define NTSTATUS LONG
#endif

#include <intrin.h>
#include <process.h>
#include <windef.h>
#include <CL\cl.h>
#include <list>

using namespace std;


// this is the only definition we need from ntstatus.h
#ifndef NULL
    #define NULL 0
#endif

#define OSAPI   WINAPI


//OS specific system calls...
#define OSInitializeCriticalSection             InitializeCriticalSection
#define OSEnterCriticalSection                  EnterCriticalSection
#define OSLeaveCriticalSection                  LeaveCriticalSection
#define OSDeleteCriticalSection                 DeleteCriticalSection
#define OSInterlockedIncrement                  InterlockedIncrement
#define OSInterlockedDecrement                  InterlockedDecrement

//Library Load functions and definitions
#ifdef _M_X64
static const char name_libfcl[] = "common_clang64.dll";
static const char name_libbcl[] = "igdbcl64.dll";
#else
static const char name_libfcl[] = "common_clang32.dll";
static const char name_libbcl[] = "igdbcl32.dll";
#endif

#define OSLoadLibrary                           LoadLibraryA
#define OSFreeLibrary                           FreeLibrary
#define OSGetProcAddress                        GetProcAddress

#ifndef QWORD
    #define QWORD UINT64
#endif

typedef HMODULE    OS_HMODULE;
typedef OS_HMODULE OS_HINSTANCE;

namespace OCLRT
{

    //OS structures and methods

    //Data Types:
    typedef CRITICAL_SECTION            OS_CRITICAL_SECTION;

    static const OS_HMODULE OS_HMNULL = NULL;

}; // namespace

