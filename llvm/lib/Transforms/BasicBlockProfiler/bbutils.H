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
#ifndef BBUTILS_H
#define BBUTILS_H

#include <map>
#include <queue>
#include <string.h>
#include <sstream>
#include <math.h>
#include <unistd.h>

#include "bbdefs.H"

// GCC is whining about an extra ";" in
// /usr/intel/pkgs/boost/1.59.0/include/boost/archive/detail/interface_oarchive.hpp:78:6
// I could fix it, but that would be wrong. Instead, I'll tell GCC to stop
// being pedantic.
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <boost/archive/text_oarchive.hpp>
#pragma GCC diagnostic pop
#else
#include <boost/archive/text_oarchive.hpp>
#endif  // __GNUC__

#include <boost/archive/text_iarchive.hpp>

using namespace std;

class PROFILE;
class BBPROFILE;

class BLOCK
{
  public:
    BLOCK();
    INT32 StaticInstructionCount(THREADID tid) const 
        { return  _staticInstructionCount; }
    INT32 WeightedStaticInstructionCount(THREADID tid) const 
        { return  _sliceBlockCount[tid] * _staticInstructionCount; }
    VOID Execute(THREADID tid) { _sliceBlockCount[tid]++; }
    VOID Execute(THREADID tid, const BLOCK* prev_block, BBPROFILE *bbprofile);
    VOID EmitSliceEnd(THREADID tid, PROFILE *profile);
    VOID EmitProgramEnd(THREADID tid, PROFILE * profile) const;
    INT64 GlobalBlockCount(THREADID tid) const 
        { return _globalBlockCount[tid] + _sliceBlockCount[tid]; }
    INT32 Id() const { return _id; }
    string FnName() const { return _fnName; }
    string BbName() const { return _bbName; }
    string SourceInfo() const { return _sourceInfo; }
    INT32 _staticInstructionCount; // number of instrs in this block.
    INT32 _id;
    string _fnName;
    string _bbName;
    string _sourceInfo;
    
  private:
    INT32 SliceInstructionCount(THREADID tid) const 
        { return _sliceBlockCount[tid] * _staticInstructionCount; }


    INT32 _sliceBlockCount[PIN_MAX_THREADS]; 
    // times this block was executed in the current slice.
    INT64 _globalBlockCount[PIN_MAX_THREADS]; 
    // times this block was executed prior to the current slice.
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & _id;
        ar & _fnName;
        ar & _bbName;
        ar & _sourceInfo;
        ar & _staticInstructionCount;
        ar & _sliceBlockCount;
        ar & _globalBlockCount;
    }
};

    
class PROFILE
{
    private:
    static const UINT32 BUFSIZE=100;
    public: 
    PROFILE(INT64 slice_size);
    PROFILE();
    VOID OpenFile(THREADID tid, UINT32 pid, string output_file);

    ofstream BbFile;
    INT64 GlobalInstructionCount;
    // The first time, we want a marker, but no T vector
    ADDRINT first_eip;
    BOOL first;
    BOOL active;
    // Emit the first marker immediately
    INT64 SliceTimer;
    INT64 CurrentSliceSize;
    BLOCK *last_block;
    private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & GlobalInstructionCount;
        ar & first_eip;
        ar & first;
        ar & active;
        ar & SliceTimer;
        ar & CurrentSliceSize;
        ar & last_block;
    }
};

class BBPROFILE
{
  public:
    BBPROFILE();
    INT32 Usage();
    PROFILE * profiles[PIN_MAX_THREADS];
    BLOCK block_array[MAX_BBLCOUNT];
    UINT32 maxBblCount;
    INT64 sliceSize;
    string outputFileName;
    UINT32 Pid;

    VOID EmitSliceStartInfo(UINT32 endMarker, INT64 markerCount, THREADID tid);
    VOID EmitSliceEnd(UINT32 endMarker, THREADID tid);
    
    // Lookup a block by its BBL key.
    // Create a new one and return it if it doesn't already exist.
    //BLOCK * LookupBlock(llvm::BasicBlock bbl);
    BLOCK * LookupBlock(void *vbbl, UINT32 bblId);

    BLOCK * FindBlock(void *vbbl);
    
    BOOL DoInsertGetFirstIpInstrumentation();
    VOID GetCommand(int argc, char *argv[]);

    //VOID activate(int argc, char** argv); TODO
    VOID Activate(INT64 sliceSize, string outputFile);

    BOOL ParseFilenameTid(const string& str, string *fn, UINT32 *tidp);
    //VOID AddInstrumentation(int argc, char *argv[]); TODO
    VOID InitProfile(INT64 sliceSize);
    
    private:
    string commandLine;    
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & outputFileName;
        ar & block_array;
        ar & maxBblCount;
        ar & sliceSize;
        // ar & commandLine;    //FIXME
        ar & Pid;
        ar & profiles; //FIXME
    }
};
#endif
