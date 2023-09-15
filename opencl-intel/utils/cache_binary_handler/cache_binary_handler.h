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

#pragma once

#include "CLElfTypes.h"

#include <string>

namespace CLElfLib {
class CElfReader;
class CElfWriter;
} // namespace CLElfLib

namespace Intel {
namespace OpenCL {
namespace ELFUtils {
extern const char *g_metaSectionName;
extern const char *g_irSectionName;
extern const char *g_optSectionName;
extern const char *g_objSectionName;
extern const char *g_objVerSectionName;

class CacheBinaryReader {
public:
  CacheBinaryReader(const void *pBlob, size_t size);
  virtual ~CacheBinaryReader();

  CacheBinaryReader(const CacheBinaryReader &) = delete;
  CacheBinaryReader &operator=(const CacheBinaryReader &) = delete;

  static bool IsValidCacheObject(const void *pBlob, size_t size);

  bool IsCachedObject() const;

  int GetSectionSize(const char *sectionName) const;
  const void *GetSectionData(const char *sectionName) const;
  const CLElfLib::SElf64Header *GetElfHeader();

private:
  static int GetSectionIndexByName(CLElfLib::CElfReader *pReader,
                                   std::string Name);
  CLElfLib::CElfReader *m_pReader;
};

class CacheBinaryWriter {
public:
  CacheBinaryWriter(CLElfLib::E_EH_MACHINE machine, CLElfLib::E_EH_FLAGS flag);
  virtual ~CacheBinaryWriter();

  CacheBinaryWriter(const CacheBinaryWriter &) = delete;
  CacheBinaryWriter &operator=(const CacheBinaryWriter &) = delete;

  bool AddSection(const char *sectionName, const char *sectionData,
                  size_t sectionSize);
  size_t GetBinarySize() const;
  bool GetBinary(char *pBinary) const;

private:
  CLElfLib::CElfWriter *m_pWriter;
};
} // namespace ELFUtils
} // namespace OpenCL
} // namespace Intel
