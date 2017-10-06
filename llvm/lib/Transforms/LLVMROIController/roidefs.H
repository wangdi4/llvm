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
#ifndef ROIDEFS_H
#define ROIDEFS_H
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <iostream>
#include <fstream>
#include <sstream>

// Exceptions are disabled, but boost requires the ability to
// throw an exception. Continuing from this call is undefined
// behavior. It should never happen, so die.
//
// Note that the function needs to be inlined so it won't get
// multiple definitions.
namespace boost {
inline void throw_exception(std::exception const & e) {
  // Die, die, die!
  exit(1);
}
}  // namespace boost

// GCC is whining about an extra ";" in
// /usr/intel/pkgs/boost/1.59.0/include/boost/archive/detail/interface_oarchive.hpp:78:6
// I could fix it, but that would be wrong. Instead, I'll tell GCC to stop
// being pedantic.
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#pragma GCC diagnostic pop
#else
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#endif  // __GNUC__

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
#define MAX_THREADS 1
#define MAX_ROICOUNT 30
//#define INT_MAX     0x7fffffff  /* max value for an int */
#define LOCALTYPE
#ifndef TRUE
#define TRUE 1
#endif  
#ifndef FALSE       
#define FALSE 0
#endif

#define ASSERT(condition,message)   \
    do{ if(!(condition)) \
        cerr <<  string("") + message;} while(0)

#define ASSERTX(condition) ASSERT(condition,"assertion failed: " #condition  "\n")

struct ROIinfo{
    UINT32 startBbid;
    UINT32 endBbid;
    UINT64 startBbCount;
    UINT64 endBbCount;
    double weight;
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & startBbid;
        ar & endBbid;
        ar & startBbCount;
        ar & endBbCount;
        ar & weight;
    }
};

struct ROIbbinfo{
    UINT32 bbid; // for sanity check since we will index using 
                 // a separate index not bbid
    UINT64 execcount;
    UINT32 nextStartingROI;
    UINT32 nextEndingROI;
    string bbDesc;
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & bbid;
        ar & execcount;
        ar & nextStartingROI;
        ar & nextEndingROI;
        ar & bbDesc;
    }
};
// Support for manually marking ROIs in source code
struct markerInfo{
  UINT64 currentCount;
  string funcName;
};

struct markerROIInfo{
  UINT64 triggerCount;
  BOOL isStart;
  BOOL isTriggered;
};
#endif
