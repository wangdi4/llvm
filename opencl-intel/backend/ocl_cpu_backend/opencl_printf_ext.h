// INTEL CONFIDENTIAL
//
// Copyright 2007-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

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


