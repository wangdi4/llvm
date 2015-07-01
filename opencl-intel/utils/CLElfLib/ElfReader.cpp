/******************************************************************************\

Copyright 2012 Intel Corporation All Rights Reserved.

    The source code contained or described herein and all documents related to
    the source code ("Material") are owned by Intel Corporation or its suppliers
    or licensors. Title to the Material remains with Intel Corporation or its
    suppliers and licensors. The Material contains trade secrets and proprietary
    and confidential information of Intel or its suppliers and licensors. The
    Material is protected by worldwide copyright and trade secret laws and
    treaty provisions. No part of the Material may be used, copied, reproduced,
    modified, published, uploaded, posted, transmitted, distributed, or
    disclosed in any way without Intel's prior express written permission.

    No license under any patent, copyright, trade secret or other intellectual
    property right is granted to or conferred upon you by disclosure or delivery
    of the Materials, either expressly, by implication, inducement, estoppel or
    otherwise. Any license under such intellectual property rights must be
    express and approved by Intel in writing.

File Name: ElfReader.cpp

Abstract: 

Notes: 

\******************************************************************************/
#include "ElfReader.h"
#include <string.h>
#include <stdio.h>

#if defined _M_X64 || defined __x86_64__
#define MACHINE EM_X86_64
#else
#define MACHINE EM_860
#endif

namespace CLElfLib
{
/******************************************************************************\
 Constructor: CElfReader::CElfReader
\******************************************************************************/
CElfReader::CElfReader( 
    const char* pElfBinary,
    const size_t elfBinarySize )
{
    m_pNameTable = NULL;
    m_nameTableSize = 0;
    m_pElfHeader = (SElf64Header*)pElfBinary;
    m_pBinary = pElfBinary;

    // get a pointer to the string table
    if( m_pElfHeader )
    {
        GetSectionData( 
            m_pElfHeader->SectionNameTableIndex, 
            m_pNameTable, m_nameTableSize );
    }
}

/******************************************************************************\
 Destructor: CElfReader::~CElfReader
\******************************************************************************/
CElfReader::~CElfReader()
{
}

/******************************************************************************\
 Member Function: CElfReader::Create
\******************************************************************************/
CElfReader* CElfReader::Create( 
    const char* pElfBinary,
    const size_t elfBinarySize )
{
    CElfReader* pNewReader = NULL;

    if( IsValidElf64( pElfBinary, elfBinarySize ) )
    {
        pNewReader = new CElfReader( pElfBinary, elfBinarySize );
    }

    return pNewReader;
}

/******************************************************************************\
 Member Function: CElfReader::Delete
\******************************************************************************/
void CElfReader::Delete( 
    CElfReader* pElfReader )
{
    if( pElfReader )
    {
        delete pElfReader;
        pElfReader = NULL;
    }
}

/******************************************************************************\
 Member Function: IsValidElf64
 Description:     Determines if a binary is in the ELF64 format checks for
                  invalid offsets.
\******************************************************************************/
bool CElfReader::IsValidElf64( 
    const void* pBinary,
    const size_t binarySize )
{
    bool retVal = false;
    SElf64Header* pElf64Header = NULL;
    SElf64SectionHeader* pSectionHeader = NULL;
    char* pNameTable = NULL;
    char* pEnd = NULL;
    size_t ourSize = 0;
    size_t entrySize = 0;
    size_t indexedSectionHeaderOffset = 0;

    // validate header
    if( pBinary && ( binarySize >= sizeof( SElf64Header ) ) )
    {
        // calculate a pointer to the end
        pEnd = (char*)pBinary + binarySize;
        pElf64Header = (SElf64Header*)pBinary;

        if( ( pElf64Header->Identity[ID_IDX_MAGIC0] == ELF_MAG0 ) &&
            ( pElf64Header->Identity[ID_IDX_MAGIC1] == ELF_MAG1 ) &&
            ( pElf64Header->Identity[ID_IDX_MAGIC2] == ELF_MAG2 ) &&
            ( pElf64Header->Identity[ID_IDX_MAGIC3] == ELF_MAG3 ) &&
            ( pElf64Header->Identity[ID_IDX_CLASS]  == EH_CLASS_64 ) )
        {
            ourSize += pElf64Header->ElfHeaderSize;
            retVal = true;
        }
    }

    // validate machine
    if (retVal == true)
    {
        // get the machine header section
        if (pElf64Header->Machine == MACHINE ||
            pElf64Header->Machine == 0)
            retVal = true;
        else
            retVal = false;
    }

    // validate sections
    if( retVal == true )
    {
        // get the section entry size
        entrySize = pElf64Header->SectionHeaderEntrySize;

        // get an offset to the name table
        if( pElf64Header->SectionNameTableIndex < 
            pElf64Header->NumSectionHeaderEntries )
        {
            indexedSectionHeaderOffset = 
                (size_t)pElf64Header->SectionHeadersOffset + 
                ( pElf64Header->SectionNameTableIndex * entrySize );

            if( ( (char*)pBinary + indexedSectionHeaderOffset ) <= pEnd )
            {
                pNameTable = (char*)pBinary + indexedSectionHeaderOffset;
            }
        }

        for( unsigned char i = 0; i < pElf64Header->NumSectionHeaderEntries; i++ )
        {
            indexedSectionHeaderOffset = (size_t)pElf64Header->SectionHeadersOffset + 
                ( i * entrySize );

            // check section header offset
            if( ( (char*)pBinary + indexedSectionHeaderOffset ) > pEnd )
            {
                retVal = false;
                break;
            }

            pSectionHeader = (SElf64SectionHeader*)( 
                (char*)pBinary + indexedSectionHeaderOffset );

            // check section data
            if( ( (char*)pBinary + pSectionHeader->DataOffset + pSectionHeader->DataSize ) > pEnd )
            {
                retVal = false;
                break;
            }

            // check section name index
            if( ( pNameTable + pSectionHeader->Name ) > pEnd )
            {
                retVal = false;
                break;
            }

            // tally up the sizes
            ourSize += (size_t)pSectionHeader->DataSize;
            ourSize += (size_t)entrySize;
        }

        if( ourSize != binarySize )
        {
            retVal = false;
        }
    }

    return retVal;
}

/******************************************************************************\
 Member Function: GetElfHeader
 Description:     Returns a pointer to the requested section header
\******************************************************************************/
const SElf64Header* CElfReader::GetElfHeader()
{
    return m_pElfHeader;
}

/******************************************************************************\
 Member Function: GetSectionHeader
 Description:     Returns a pointer to the requested section header
\******************************************************************************/
const SElf64SectionHeader* CElfReader::GetSectionHeader( 
    unsigned int sectionIndex )
{
    SElf64SectionHeader* pSectionHeader = NULL;
    size_t indexedSectionHeaderOffset = 0;
    size_t entrySize = m_pElfHeader->SectionHeaderEntrySize;

    if( sectionIndex < m_pElfHeader->NumSectionHeaderEntries )
    {
        indexedSectionHeaderOffset = (size_t)m_pElfHeader->SectionHeadersOffset + 
            ( sectionIndex * entrySize );

        pSectionHeader = (SElf64SectionHeader*)( 
                (char*)m_pElfHeader + indexedSectionHeaderOffset );
    }

    return pSectionHeader;
}

/******************************************************************************\
 Member Function: GetSectionData
 Description:     Returns a pointer to and size of the requested section's 
                  data
\******************************************************************************/
E_RETVAL CElfReader::GetSectionData( 
    const unsigned int sectionIndex, 
    char* &pData, 
    size_t &dataSize )
{
    E_RETVAL retVal = FAILURE;
    const SElf64SectionHeader* pSectionHeader = GetSectionHeader( sectionIndex );

    if( pSectionHeader )
    {
        pData = (char*)m_pBinary + pSectionHeader->DataOffset;
        dataSize = ( size_t )pSectionHeader->DataSize;
        retVal = SUCCESS;
    }

    return retVal;
}

/******************************************************************************\
 Member Function: GetSectionData
 Description:     Returns a pointer to and size of the requested section's 
                  data
\******************************************************************************/
E_RETVAL CElfReader::GetSectionData( 
    const char* pName, 
    char* &pData, 
    size_t &dataSize )
{
    E_RETVAL retVal = FAILURE;
    const char* pSectionName = NULL;

    for( unsigned int i = 1; i < m_pElfHeader->NumSectionHeaderEntries; i++ )
    {
        pSectionName = GetSectionName( i );

        if( pSectionName && ( strcmp( pName, pSectionName ) == 0 ) )
        {
            GetSectionData( i, pData, dataSize );
            retVal = SUCCESS;
            break;
        }
    }

    return retVal;
}

/******************************************************************************\
 Member Function: GetSectionName
 Description:     Returns a pointer to a NULL terminated string
\******************************************************************************/
const char* CElfReader::GetSectionName( 
    unsigned int sectionIndex )
{
    char* pName = NULL;
    const SElf64SectionHeader* pSectionHeader = GetSectionHeader( sectionIndex );

    if( pSectionHeader )
    {
        pName = m_pNameTable + pSectionHeader->Name;
    }

    return pName;
}

} // namespace OclElfLib
