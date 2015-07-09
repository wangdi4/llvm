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
//  cache_binary_handler.h
///////////////////////////////////////////////////////////
#pragma once

#include <string>
#include "CLElfTypes.h"

namespace CLElfLib
{
    class CElfReader;
    class CElfWriter;
}

namespace Intel{ namespace OpenCL{ namespace ELFUtils {
extern const char* g_metaSectionName;
extern const char* g_irSectionName;
extern const char* g_optSectionName;
extern const char* g_objSectionName;

class CacheBinaryReader
{
public:
    CacheBinaryReader(const void* pBlob, size_t size);
    virtual ~CacheBinaryReader();

    static bool IsValidCacheObject(const void*pBlob, size_t size);

    bool IsCachedObject() const;

    int GetSectionSize(const char* sectionName) const;
    const void* GetSectionData(const char* sectionName) const;
    const CLElfLib::SElf64Header* GetElfHeader();

private:
    // Disable copy ctor and assignment operator
    CacheBinaryReader( const CacheBinaryReader& );
    bool operator = (const CacheBinaryReader& );

    static int GetSectionIndexByName(CLElfLib::CElfReader* pReader, std::string Name);
    CLElfLib::CElfReader* m_pReader;
};

class CacheBinaryWriter
{
public:
    CacheBinaryWriter(CLElfLib::E_EH_MACHINE machine, CLElfLib::E_EH_FLAGS flag);
    virtual ~CacheBinaryWriter();
    bool AddSection(const char* sectionName, const char* sectionData, size_t sectionSize);
    size_t GetBinarySize() const;
    bool GetBinary(char* pBinary) const;

private:
    // Disable copy ctor and assignment operator
    CacheBinaryWriter( const CacheBinaryWriter& );
    bool operator = (const CacheBinaryWriter& );

    CLElfLib::CElfWriter* m_pWriter;
};
}}} // namespace
