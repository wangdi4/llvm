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
//  cache_binary_handler.cpp
///////////////////////////////////////////////////////////

#include "cache_binary_handler.h"
#include "ElfReader.h"
#include "ElfWriter.h"
#include "CLElfTypes.h"
#include "elf_binary.h"
#include "assert.h"

namespace Intel{ namespace OpenCL{ namespace ELFUtils {
const char* g_metaSectionName = ".ocl.meta";
const char* g_irSectionName   = ".ocl.ir";
const char* g_optSectionName  = ".ocl.opt";
const char* g_objSectionName  = ".ocl.obj";

CacheBinaryReader::CacheBinaryReader(const void* pBlob, size_t size)
{
    assert(pBlob && "pBlob is null");
    m_pReader = CLElfLib::CElfReader::Create( (const char*)pBlob, size );
}

CacheBinaryReader::~CacheBinaryReader()
{
    if(m_pReader) CLElfLib::CElfReader::Delete(m_pReader);
    m_pReader = NULL;
}

bool CacheBinaryReader::IsValidCacheObject(const void*pBlob, size_t size)
{
    if( CLElfLib::CElfReader::IsValidElf64((const char*)pBlob, size))
    {
        ElfReaderPtr pReader(CLElfLib::CElfReader::Create((const char*)pBlob, size));
        if( NULL != pReader.get() )
        {
            return (CLElfLib::EH_TYPE_OPENCL_EXECUTABLE == pReader->GetElfHeader()->Type);
        }
    }
    return false;
}

bool CacheBinaryReader::IsCachedObject() const
{
    if(m_pReader)
    {
        const CLElfLib::SElf64Header* pElfReaderHeader = m_pReader->GetElfHeader();
        if(pElfReaderHeader)
        {
            return (CLElfLib::EH_TYPE_OPENCL_EXECUTABLE == pElfReaderHeader->Type);
        }
        return false;
    }
    return false;
}

int CacheBinaryReader::GetSectionIndexByName(CLElfLib::CElfReader* pReader, std::string Name)
{
    assert(pReader && "reader is null");
    const CLElfLib::SElf64Header* pElfReaderHeader = pReader->GetElfHeader();
    if(pElfReaderHeader)
    {
        unsigned int numSections = pElfReaderHeader->NumSectionHeaderEntries;
        for(unsigned int i =0; i <numSections; ++i)
        {
            std::string sectionName = pReader->GetSectionName(i);
            if(0 == sectionName.compare(Name))
            {
                 return i;
            }
        }
    }
    return -1;
}

int CacheBinaryReader::GetSectionSize(const char* sectionName) const
{
    assert(CacheBinaryReader::IsCachedObject() && "not a valid cache object");
    if(m_pReader)
    {
        int index = GetSectionIndexByName(m_pReader, sectionName);
        if(index < 0) return 0;

        char* pData = NULL;
        size_t sectionSize = 0;
        m_pReader->GetSectionData(index, pData, sectionSize);
        return sectionSize;
    }
    assert(false && "file cannot be opened by elf reader!");
    return -1;
}

const void* CacheBinaryReader::GetSectionData(const char* sectionName) const
{
    assert(CacheBinaryReader::IsCachedObject() && "not a valid cache object");
    if(m_pReader)
    {
        int index = GetSectionIndexByName(m_pReader, sectionName);
        if(index < 0) return NULL;

        char* pData = NULL;
        size_t sectionSize = 0;
        m_pReader->GetSectionData(index, pData, sectionSize);
        return pData;
    }
    assert(false && "file cannot be opened by elf reader!");
    return NULL;
}

const CLElfLib::SElf64Header* CacheBinaryReader::GetElfHeader()
{
    assert(m_pReader && "reader is null");
    if (m_pReader)
    {
        return m_pReader->GetElfHeader();
    }
    return NULL;
}

CacheBinaryWriter::CacheBinaryWriter(CLElfLib::E_EH_MACHINE  machine, CLElfLib::E_EH_FLAGS flag)
{
    m_pWriter = CLElfLib::CElfWriter::Create( CLElfLib::EH_TYPE_OPENCL_EXECUTABLE,
                                              machine,
                                              flag );
}

CacheBinaryWriter::~CacheBinaryWriter()
{
    assert(m_pWriter && "pWriter is null");
    CLElfLib::CElfWriter::Delete(m_pWriter);
    m_pWriter = NULL;
}

bool CacheBinaryWriter::AddSection(const char* sectionName, const char* sectionData, size_t sectionSize)
{
    CLElfLib::SSectionNode sectionNode;
    sectionNode.Name = sectionName;
    sectionNode.pData = const_cast<char*>(sectionData);
    sectionNode.DataSize = sectionSize;
    sectionNode.Flags = 0;
    sectionNode.Type = CLElfLib::SH_TYPE_NULL;

    return (m_pWriter->AddSection( &sectionNode ) == CLElfLib::SUCCESS );
}

size_t CacheBinaryWriter::GetBinarySize() const
{
    char *pData = NULL;
    unsigned int DataSize = 0;

    if( m_pWriter->ResolveBinary( pData, DataSize ) == CLElfLib::SUCCESS )
        return DataSize;
    return 0;
}

bool CacheBinaryWriter::GetBinary(char* pBinary) const
{
    unsigned int DataSize = 0;
    return ( m_pWriter->ResolveBinary( pBinary, DataSize ) == CLElfLib::SUCCESS );
}
}}}