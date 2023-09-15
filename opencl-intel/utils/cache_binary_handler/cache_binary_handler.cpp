// INTEL CONFIDENTIAL
//
// Copyright 2006 Intel Corporation.
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

#include "cache_binary_handler.h"

#include "CLElfTypes.h"
#include "ElfReader.h"
#include "ElfWriter.h"
#include "elf_binary.h"
#include <assert.h>

namespace Intel {
namespace OpenCL {
namespace ELFUtils {
const char *g_metaSectionName = ".ocl.meta";
const char *g_irSectionName = ".ocl.ir";
const char *g_optSectionName = ".ocl.opt";
const char *g_objSectionName = ".ocl.obj";
const char *g_objVerSectionName = ".ocl.ver";

CacheBinaryReader::CacheBinaryReader(const void *pBlob, size_t size) {
  assert(pBlob && "pBlob is null");
  m_pReader = CLElfLib::CElfReader::Create((const char *)pBlob, size);
}

CacheBinaryReader::~CacheBinaryReader() {
  if (m_pReader)
    CLElfLib::CElfReader::Delete(m_pReader);
  m_pReader = nullptr;
}

bool CacheBinaryReader::IsValidCacheObject(const void *pBlob, size_t size) {
  if (CLElfLib::CElfReader::IsValidElf64((const char *)pBlob, size)) {
    ElfReaderPtr pReader(
        CLElfLib::CElfReader::Create((const char *)pBlob, size));
    if (nullptr != pReader.get()) {
      return (CLElfLib::EH_TYPE_OPENCL_EXECUTABLE ==
              pReader->GetElfHeader()->Type);
    }
  }
  return false;
}

bool CacheBinaryReader::IsCachedObject() const {
  if (m_pReader) {
    const CLElfLib::SElf64Header *pElfReaderHeader = m_pReader->GetElfHeader();
    if (pElfReaderHeader) {
      return (CLElfLib::EH_TYPE_OPENCL_EXECUTABLE == pElfReaderHeader->Type);
    }
    return false;
  }
  return false;
}

int CacheBinaryReader::GetSectionIndexByName(CLElfLib::CElfReader *pReader,
                                             std::string Name) {
  assert(pReader && "reader is null");
  const CLElfLib::SElf64Header *pElfReaderHeader = pReader->GetElfHeader();
  if (pElfReaderHeader) {
    unsigned int numSections = pElfReaderHeader->NumSectionHeaderEntries;
    for (unsigned int i = 0; i < numSections; ++i) {
      const char *sectionName = pReader->GetSectionName(i);
      assert(sectionName && "There is no name for that section");
      if (0 == Name.compare(sectionName)) {
        return i;
      }
    }
  }
  return -1;
}

int CacheBinaryReader::GetSectionSize(const char *sectionName) const {
  assert(CacheBinaryReader::IsCachedObject() && "not a valid cache object");
  if (m_pReader) {
    int index = GetSectionIndexByName(m_pReader, sectionName);
    if (index < 0)
      return 0;

    const char *pData = nullptr;
    size_t sectionSize = 0;
    m_pReader->GetSectionData(index, pData, sectionSize);
    return sectionSize;
  }
  assert(false && "file cannot be opened by elf reader!");
  return -1;
}

const void *CacheBinaryReader::GetSectionData(const char *sectionName) const {
  assert(CacheBinaryReader::IsCachedObject() && "not a valid cache object");
  if (m_pReader) {
    int index = GetSectionIndexByName(m_pReader, sectionName);
    if (index < 0)
      return nullptr;

    const char *pData = nullptr;
    size_t sectionSize = 0;
    m_pReader->GetSectionData(index, pData, sectionSize);
    return pData;
  }
  assert(false && "file cannot be opened by elf reader!");
  return nullptr;
}

const CLElfLib::SElf64Header *CacheBinaryReader::GetElfHeader() {
  assert(m_pReader && "reader is null");
  if (m_pReader) {
    return m_pReader->GetElfHeader();
  }
  return nullptr;
}

CacheBinaryWriter::CacheBinaryWriter(CLElfLib::E_EH_MACHINE machine,
                                     CLElfLib::E_EH_FLAGS flag) {
  m_pWriter = CLElfLib::CElfWriter::Create(CLElfLib::EH_TYPE_OPENCL_EXECUTABLE,
                                           machine, flag);
}

CacheBinaryWriter::~CacheBinaryWriter() {
  assert(m_pWriter && "pWriter is null");
  CLElfLib::CElfWriter::Delete(m_pWriter);
  m_pWriter = nullptr;
}

bool CacheBinaryWriter::AddSection(const char *sectionName,
                                   const char *sectionData,
                                   size_t sectionSize) {
  CLElfLib::SSectionNode sectionNode;
  sectionNode.Name = sectionName;
  sectionNode.pData = const_cast<char *>(sectionData);
  sectionNode.DataSize = sectionSize;
  sectionNode.Flags = 0;
  sectionNode.Type = CLElfLib::SH_TYPE_NULL;

  return (m_pWriter->AddSection(&sectionNode) == CLElfLib::SUCCESS);
}

size_t CacheBinaryWriter::GetBinarySize() const {
  char *pData = nullptr;
  unsigned int DataSize = 0;

  if (m_pWriter->ResolveBinary(pData, DataSize) == CLElfLib::SUCCESS)
    return DataSize;
  return 0;
}

bool CacheBinaryWriter::GetBinary(char *pBinary) const {
  unsigned int DataSize = 0;
  return (m_pWriter->ResolveBinary(pBinary, DataSize) == CLElfLib::SUCCESS);
}
} // namespace ELFUtils
} // namespace OpenCL
} // namespace Intel
