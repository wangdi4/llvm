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

#pragma once
#include "CLElfTypes.h"

#include <stdlib.h>

#if defined(_WIN32) && (__KLOCWORK__ == 0)
#define ELF_CALL __stdcall
#else
#define ELF_CALL
#endif

namespace CLElfLib {
/******************************************************************************\

 Class:         CElfReader

 Description:   Class to provide simpler interaction with the ELF standard
                binary object.  SElf64Header defines the ELF header type and
                SElf64SectionHeader defines the section header type.

\******************************************************************************/

class CElfReader {
public:
  static CElfReader *ELF_CALL Create(const char *pElfBinary,
                                     const size_t elfBinarySize);

  static void ELF_CALL Delete(CElfReader *pElfObject);

  static bool ELF_CALL IsValidElf64(const char *pBinary,
                                    const size_t binarySize);

  const SElf64Header *ELF_CALL GetElfHeader();

  const SElf64SectionHeader *ELF_CALL
  GetSectionHeader(unsigned int sectionIndex);

  const char *ELF_CALL GetSectionName(unsigned int sectionIndex);

  E_RETVAL ELF_CALL GetSectionData(const unsigned int sectionIndex,
                                   const char *&pData, size_t &dataSize);

  E_RETVAL ELF_CALL GetSectionData(const char *sectionName, const char *&pData,
                                   size_t &dataSize);

protected:
  ELF_CALL CElfReader(const char *pElfBinary);

  ELF_CALL ~CElfReader();

  const SElf64Header *m_pElfHeader; // pointer to the ELF header
  const char *m_pBinary;            // portable ELF binary
  const char *m_pNameTable;         // pointer to the string table
  size_t m_nameTableSize;           // size of string table in bytes
};

} // namespace CLElfLib
