// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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
#include "CLElfTypes.h"
#include <queue>
#include <string>

#if defined(_WIN32) && (__KLOCWORK__ == 0)
  #define ELF_CALL __stdcall
#else
  #define ELF_CALL
#endif

using namespace std;

namespace CLElfLib
{
static const unsigned int g_scElfHeaderAlignment    = 16;   // allocation alignment restriction
static const unsigned int g_scInitialElfSize        = 2048; // initial elf size (in bytes)
static const unsigned int g_scInitNumSectionHeaders = 8;

struct SSectionNode
{
    E_SH_TYPE    Type;
    unsigned int Flags;
    string Name;
    char* pData;
    unsigned int DataSize;

    SSectionNode()
    {
        Type     = SH_TYPE_NULL;
        Flags    = 0;
        pData    = nullptr;
        DataSize = 0;
    }

    ~SSectionNode()
    {
    }
};

/******************************************************************************\

 Class:         CElfWriter

 Description:   Class to provide simpler interaction with the ELF standard
                binary object.  SElf64Header defines the ELF header type and 
                SElf64SectionHeader defines the section header type.

\******************************************************************************/
class CElfWriter
{
public:
    static CElfWriter* ELF_CALL Create(
        E_EH_TYPE type,
        E_EH_MACHINE machine,
        E_EH_FLAGS flags );

    static void ELF_CALL Delete( CElfWriter* &pElfWriter );

    E_RETVAL ELF_CALL AddSection(
        SSectionNode* pSectionNode );

    E_RETVAL ELF_CALL ResolveBinary( 
        char* &pBinary,
        unsigned int& dataSize );

    E_RETVAL ELF_CALL Initialize();
    E_RETVAL ELF_CALL PatchElfHeader( char* &pBinary );

protected:
    ELF_CALL CElfWriter(
        E_EH_TYPE type,
        E_EH_MACHINE machine,
        E_EH_FLAGS flags );

    ELF_CALL ~CElfWriter();

    E_EH_TYPE m_type;
    E_EH_MACHINE m_machine;
    E_EH_FLAGS m_flags;

    std::queue<SSectionNode*> m_nodeQueue;

    unsigned int m_dataSize;
    unsigned int m_numSections;
    unsigned int m_stringTableSize;
    unsigned int m_totalBinarySize;
};

} // namespace ELFLib
