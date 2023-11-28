// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#include "ElfWriter.h"

#include "cl_sys_defines.h"

#include <cstring>
#include <string.h>

namespace CLElfLib {

/******************************************************************************\
 Constructor: CElfWriter::CElfWriter
\******************************************************************************/
CElfWriter::CElfWriter(E_EH_TYPE type, E_EH_MACHINE machine, E_EH_FLAGS flags) {
  m_type = type;
  m_machine = machine;
  m_flags = flags;
  m_dataSize = 0;
  m_numSections = 0;
  m_stringTableSize = 0;
}

/******************************************************************************\
 Destructor: CElfWriter::~CElfWriter
\******************************************************************************/
CElfWriter::~CElfWriter() {
  SSectionNode *pNode = nullptr;

  // Walk through the section nodes
  while (m_nodeQueue.empty() == false) {
    pNode = m_nodeQueue.front();
    m_nodeQueue.pop();

    // delete the node and it's data
    if (pNode) {
      if (pNode->pData) {
        delete[] pNode->pData;
        pNode->pData = nullptr;
      }

      delete pNode;
    }
  }
}

/******************************************************************************\
 Member Function: CElfWriter::Create
\******************************************************************************/
CElfWriter *CElfWriter::Create(E_EH_TYPE type, E_EH_MACHINE machine,
                               E_EH_FLAGS flags) {
  CElfWriter *pWriter = new CElfWriter(type, machine, flags);

  if ((pWriter) && (pWriter->Initialize() != SUCCESS)) {
    Delete(pWriter);
  }

  return pWriter;
}

/******************************************************************************\
 Member Function: CElfWriter::Delete
\******************************************************************************/
void CElfWriter::Delete(CElfWriter *&pWriter) {
  if (pWriter) {
    delete pWriter;
    pWriter = nullptr;
  }
}

/******************************************************************************\
 Member Function: CElfWriter::AddSection
\******************************************************************************/
E_RETVAL CElfWriter::AddSection(SSectionNode *pSectionNode) {
  SSectionNode *pNode = nullptr;
  unsigned int nameSize = 0;
  unsigned int dataSize = 0;

  // The section header must be non-NULL
  if (pSectionNode == nullptr)
    return FAILURE;

  pNode = new SSectionNode();

  pNode->Flags = pSectionNode->Flags;
  pNode->Type = pSectionNode->Type;

  nameSize = pSectionNode->Name.size() + 1;
  dataSize = pSectionNode->DataSize;

  pNode->Name = pSectionNode->Name;

  // ok to have NULL data
  if (dataSize > 0) {
    pNode->pData = new char[dataSize];
    MEMCPY_S(pNode->pData, dataSize, pSectionNode->pData, dataSize);
    pNode->DataSize = dataSize;
  }

  // push the node onto the queue
  m_nodeQueue.push(pNode);

  // increment the sizes for each section
  m_dataSize += dataSize;
  m_stringTableSize += nameSize;
  m_numSections++;

  return SUCCESS;
}

/******************************************************************************\
 Member Function: CElfWriter::ResolveBinary
\******************************************************************************/
E_RETVAL CElfWriter::ResolveBinary(char *&pBinary, unsigned int &binarySize) {
  E_RETVAL retVal = SUCCESS;
  SSectionNode *pNode = nullptr;
  SElf64SectionHeader *pCurSectionHeader = nullptr;
  char *pData = nullptr;
  char *pStringTable = nullptr;
  char *pCurString = nullptr;

  m_totalBinarySize =
      sizeof(SElf64Header) +
      ((m_numSections + 1) *
       sizeof(SElf64SectionHeader)) + // +1 to account for string table entry
      m_dataSize +
      m_stringTableSize;

  if (pBinary) {
    // get a pointer to the first section header
    pCurSectionHeader = (SElf64SectionHeader *)(pBinary + sizeof(SElf64Header));

    // get a pointer to the data
    pData =
        pBinary + sizeof(SElf64Header) +
        ((m_numSections + 1) *
         sizeof(SElf64SectionHeader)); // +1 to account for string table entry

    // get a pointer to the string table
    pStringTable =
        pBinary + sizeof(SElf64Header) +
        ((m_numSections + 1) *
         sizeof(SElf64SectionHeader)) + // +1 to account for string table entry
        m_dataSize;

    pCurString = pStringTable;

    // Walk through the section nodes
    while (m_nodeQueue.empty() == false) {
      pNode = m_nodeQueue.front();

      if (pNode) {
        m_nodeQueue.pop();

        // Copy data into the section header
        memset(pCurSectionHeader, 0, sizeof(SElf64SectionHeader));
        pCurSectionHeader->Type = pNode->Type;
        pCurSectionHeader->Flags = pNode->Flags;
        pCurSectionHeader->DataSize = pNode->DataSize;
        pCurSectionHeader->DataOffset = pData - pBinary;
        pCurSectionHeader->Name = (Elf64_Word)(pCurString - pStringTable);
        pCurSectionHeader =
            (SElf64SectionHeader *)((unsigned char *)pCurSectionHeader +
                                    sizeof(SElf64SectionHeader));

        // copy the data, move the data pointer
        MEMCPY_S(pData, pNode->DataSize, pNode->pData, pNode->DataSize);
        pData += pNode->DataSize;

        // copy the name into the string table, move the string pointer
        if (pNode->Name.size() > 0) {
          MEMCPY_S(pCurString, pNode->Name.size(), pNode->Name.c_str(),
                   pNode->Name.size());
          pCurString += pNode->Name.size();
        }
        *(pCurString++) = '\0';

        // delete the node and it's data
        if (pNode) {
          if (pNode->pData) {
            delete[] pNode->pData;
            pNode->pData = nullptr;
          }

          delete pNode;
        }
      }
    }

    // add the string table section header
    SElf64SectionHeader stringSectionHeader;
    std::memset(&stringSectionHeader, 0, sizeof(SElf64SectionHeader));
    stringSectionHeader.Type = SH_TYPE_STR_TBL;
    stringSectionHeader.Flags = 0;
    stringSectionHeader.DataOffset = pStringTable - pBinary;
    stringSectionHeader.DataSize = m_stringTableSize;
    stringSectionHeader.Name = 0;

    // Copy into the last section header
    MEMCPY_S(pCurSectionHeader, sizeof(SElf64SectionHeader),
             &stringSectionHeader, sizeof(SElf64SectionHeader));

    // Add to our section number
    m_numSections++;

    // patch up the ELF header
    retVal = PatchElfHeader(pBinary);
  }

  if (retVal == SUCCESS) {
    binarySize = m_totalBinarySize;
  }

  return retVal;
}

/******************************************************************************\
 Member Function: CElfWriter::Initialize
\******************************************************************************/
E_RETVAL CElfWriter::Initialize() {
  E_RETVAL retVal = SUCCESS;
  SSectionNode emptySection;

  // Add an empty section 0 (points to "no-bits")
  AddSection(&emptySection);

  return retVal;
}

/******************************************************************************\
 Member Function: CElfWriter::PatchElfHeader
\******************************************************************************/
E_RETVAL CElfWriter::PatchElfHeader(char *&pBinary) {
  E_RETVAL retVal = SUCCESS;
  SElf64Header *pElfHeader = (SElf64Header *)pBinary;

  if (pElfHeader) {
    // Setup the identity
    memset(pElfHeader, 0x00, sizeof(SElf64Header));
    pElfHeader->Identity[ID_IDX_MAGIC0] = ELF_MAG0;
    pElfHeader->Identity[ID_IDX_MAGIC1] = ELF_MAG1;
    pElfHeader->Identity[ID_IDX_MAGIC2] = ELF_MAG2;
    pElfHeader->Identity[ID_IDX_MAGIC3] = ELF_MAG3;
    pElfHeader->Identity[ID_IDX_CLASS] = EH_CLASS_64;
    pElfHeader->Identity[ID_IDX_VERSION] = EH_VERSION_CURRENT;

    // Add other non-zero info
    pElfHeader->Type = m_type;
    pElfHeader->Machine = m_machine;
    pElfHeader->Flags = (unsigned int)m_flags;
    pElfHeader->ElfHeaderSize = sizeof(SElf64Header);
    pElfHeader->SectionHeaderEntrySize = sizeof(SElf64SectionHeader);
    pElfHeader->NumSectionHeaderEntries = (Elf64_Half)m_numSections;
    pElfHeader->SectionHeadersOffset = (unsigned int)(sizeof(SElf64Header));
    pElfHeader->SectionNameTableIndex = m_numSections - 1; // last index
  }

  return retVal;
}

} // namespace CLElfLib
