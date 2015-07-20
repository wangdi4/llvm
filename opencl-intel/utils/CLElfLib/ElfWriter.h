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

File Name: ElfWriter.h

Abstract: 

Notes: 

\******************************************************************************/
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
        pData    = NULL;
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
