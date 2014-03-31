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

File Name: ElfReader.h

Abstract: 

Notes: 

\******************************************************************************/
#pragma once
#include "CLElfTypes.h"
#include <stdlib.h>

#if defined(_WIN32) && (__KLOCWORK__ == 0)
  #define ELF_CALL __stdcall
#else
  #define ELF_CALL
#endif

namespace CLElfLib
{
/******************************************************************************\

 Class:         CElfReader

 Description:   Class to provide simpler interaction with the ELF standard
                binary object.  SElf64Header defines the ELF header type and 
                SElf64SectionHeader defines the section header type.

\******************************************************************************/

class CElfReader
{
public:
    static CElfReader* ELF_CALL Create( 
        const char* pElfBinary,
        const size_t elfBinarySize );

    static void ELF_CALL Delete( 
        CElfReader* pElfObject );
    
    static bool ELF_CALL IsValidElf64( 
        const void* pBinary, 
        const size_t binarySize );

    const SElf64Header* ELF_CALL GetElfHeader();
    
    const SElf64SectionHeader* ELF_CALL GetSectionHeader( 
        unsigned int sectionIndex );
    
    const char* ELF_CALL GetSectionName( 
        unsigned int sectionIndex );

    E_RETVAL ELF_CALL GetSectionData( 
        const unsigned int sectionIndex, 
        char* &pData, 
        size_t &dataSize );

    E_RETVAL ELF_CALL GetSectionData( 
        const char* sectionName, 
        char* &pData, 
        size_t &dataSize );

protected:
    ELF_CALL CElfReader( 
        const char* pElfBinary, 
        const size_t elfBinarySize );
    
    ELF_CALL ~CElfReader();

    SElf64Header*  m_pElfHeader;    // pointer to the ELF header
    const char*    m_pBinary;       // portable ELF binary
    char*          m_pNameTable;    // pointer to the string table
    size_t         m_nameTableSize; // size of string table in bytes
};

} // namespace CLElfLib
