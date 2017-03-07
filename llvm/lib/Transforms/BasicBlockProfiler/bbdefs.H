/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2016-2017 Intel Corporation. All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */
#ifndef BBDEFS_H
#define BBDEFS_H
#include <stdint.h>
#include <limits.h>
#include <iostream>
#include <fstream>
using namespace std;
typedef uint8_t  UINT8;   //LINUX HOSTS
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef uint64_t UINT64;
typedef int8_t  INT8;
typedef int16_t INT16;
typedef int32_t INT32;
typedef int64_t INT64;
#ifdef TARGET_IA32
typedef UINT32 ADDRINT;
#else
typedef UINT64 ADDRINT;
#endif
typedef void VOID;
typedef char CHAR;
typedef int BOOL;
typedef ADDRINT    USIZE;
typedef UINT32 THREADID;
//#define INT_MAX     0x7fffffff  /* max value for an int */
#define LOCALTYPE
#ifndef TRUE
#define TRUE 1
#endif  
#ifndef FALSE       
#define FALSE 0
#endif

#define MAX_BBLCOUNT    2*100000 // FIXME
#define PIN_MAX_THREADS    1 // FIXME
#define GLOBAL_SLICE_SIZE    100 // FIXME

#define ASSERT(condition,message)   \
    do{ if(!(condition)) \
        cerr <<  string("") + message;} while(0)

#define ASSERTX(condition) ASSERT(condition,"assertion failed: " #condition  "\n")

#define BBPROFILE_MAX_IMAGES 64
#define ADDRESS64_MASK (~63)
#endif
