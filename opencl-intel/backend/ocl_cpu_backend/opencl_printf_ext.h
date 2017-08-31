/////////////////////////////////////////////////////////////////////////
// opencl_printf_ext.h:
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2012 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related 
// to the source code ("Material") are owned by Intel Corporation or its 
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and 
// proprietary and confidential information of Intel Corporation and its 
// suppliers and licensors, and is protected by worldwide copyright and trade 
// secret laws and treaty provisions. No part of the Material may be used, copied, 
// reproduced, modified, published, uploaded, posted, transmitted, distributed, 
// or disclosed in any way without Intel?s prior express written permission. 
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery 
// of the Materials, either expressly, by implication, inducement, estoppel or 
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice 
// or any other notice embedded in Materials by Intel or Intel?s suppliers or licensors 
// in any way.
/////////////////////////////////////////////////////////////////////////

#pragma once 

#if defined(_WIN32)
    #include <windows.h>
    #include <process.h>
#endif

#pragma warning(disable:654)

class OutputAccumulator
{
public:
    // Append character c to output
    //
    virtual void append(char c) = 0;

    // Append a whole NUL-terminated C-string to output.
    //
    virtual void append(const char* cstr)
    {
        while (*cstr) {
            append(*cstr++);
        }
    }

    // Finalize output
    //
    virtual void finalize() = 0;
    
    // The amount of characters requested to output so far.
    //
    virtual int output_count() = 0;
};

// An accumulator that writes into a FILE* stream.
//
class StreamOutputAccumulator : public OutputAccumulator
{
public:
    StreamOutputAccumulator(FILE* stream);

    virtual void append(char c);

    virtual void finalize();

    virtual int output_count();
private:
    FILE* stream;
    int count;
#if (defined(_WIN32) || defined(_WIN64)) 
    HANDLE hStdout;
#endif
};

// An accumulator that writes into a size-limited buffer. No more than size
// chars will ever be written into the buffer. finalize() inserts the final
// '\0'.
//
class StringOutputAccumulator : public OutputAccumulator
{
public:
    StringOutputAccumulator(char* outstr_, size_t outstr_size_);

    virtual void append(char c);

    virtual void finalize();

    virtual int output_count();
private:
    char* outstr;
    size_t outstr_size;
    size_t outstr_ptr;
    int count;
};

extern "C" int printFormatCommon(OutputAccumulator& output, const char* format, 
    const char* args);


