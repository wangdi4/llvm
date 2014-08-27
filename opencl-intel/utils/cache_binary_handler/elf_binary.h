// Copyright (c) 2006-2009 Intel Corporation
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

///////////////////////////////////////////////////////////
//  elf_binary.h
///////////////////////////////////////////////////////////
#pragma once

#include "ElfReader.h"
#include "ElfWriter.h"
#include "cl_autoptr_ex.h"
#include "cl_device_api.h"
#include <string>
#include <assert.h>

namespace Intel{ namespace OpenCL{ namespace ELFUtils {
//
// ElfReaderDP- ElfReader delete policy for autoptr.
//
struct ElfReaderDP
{
    static void Delete(CLElfLib::CElfReader* pElfReader)
    {
        CLElfLib::CElfReader::Delete(pElfReader);
    }
};
typedef Utils::auto_ptr_ex<CLElfLib::CElfReader, ElfReaderDP> ElfReaderPtr;

//
// ElfWriterDP- ElfWriter delete policy for autoptr.
//
struct ElfWriterDP
{
    static void Delete(CLElfLib::CElfWriter* pElfWriter)
    {
        CLElfLib::CElfWriter::Delete(pElfWriter);
    }
};
typedef Utils::auto_ptr_ex<CLElfLib::CElfWriter, ElfWriterDP> ElfWriterPtr;

class OCLElfBinaryReader
{
public:
    static bool IsValidOpenCLBinary(const char* pBinary, size_t uiBinarySize);

    OCLElfBinaryReader(const char* pBinary, size_t uiBinarySize);

    void GetIR(char*& pData, size_t&uiSize) const;

    cl_prog_binary_type GetBinaryType() const;

private:
    mutable ElfReaderPtr m_pReader;
};
}}} //namespace Intel::OpenCL::ELFUtils
