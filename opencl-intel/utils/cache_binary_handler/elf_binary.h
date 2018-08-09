// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
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
