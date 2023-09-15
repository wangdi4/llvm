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

#include "elf_binary.h"

#include "CLElfTypes.h"

namespace Intel {
namespace OpenCL {
namespace ELFUtils {
bool OCLElfBinaryReader::IsValidOpenCLBinary(const char *pBinary,
                                             size_t uiBinarySize) {
  if (CLElfLib::CElfReader::IsValidElf64((const char *)pBinary, uiBinarySize)) {
    ElfReaderPtr pReader(
        CLElfLib::CElfReader::Create((const char *)pBinary, uiBinarySize));
    switch (pReader->GetElfHeader()->Type) {
    case CLElfLib::EH_TYPE_OPENCL_OBJECTS:
    case CLElfLib::EH_TYPE_OPENCL_LIBRARY:
    case CLElfLib::EH_TYPE_OPENCL_LINKED_OBJECTS:
      return true;
    }
  }
  return false;
}

OCLElfBinaryReader::OCLElfBinaryReader(const char *pBinary, size_t uiBinarySize)
    : m_pReader(CLElfLib::CElfReader::Create(pBinary, uiBinarySize)) {
  assert(IsValidOpenCLBinary(pBinary, uiBinarySize) && "invalid opencl binary");
  if (nullptr == m_pReader.get()) {
    throw std::bad_alloc();
  }
}

void OCLElfBinaryReader::GetIR(const char *&pData, size_t &uiSize) const {
  if (CLElfLib::SUCCESS !=
      m_pReader->GetSectionData(".ocl.ir", pData, uiSize)) {
    throw "no .ocl.ir section";
  }
}

cl_prog_binary_type OCLElfBinaryReader::GetBinaryType() const {
  switch (m_pReader->GetElfHeader()->Type) {
  case CLElfLib::EH_TYPE_OPENCL_OBJECTS:
    return CL_PROG_BIN_COMPILED_LLVM;
  case CLElfLib::EH_TYPE_OPENCL_LIBRARY:
    return CL_PROG_BIN_LINKED_LLVM;
  case CLElfLib::EH_TYPE_OPENCL_LINKED_OBJECTS:
    return CL_PROG_BIN_EXECUTABLE_LLVM;
  }
  throw "unsupported binary type";
}
} // namespace ELFUtils
} // namespace OpenCL
} // namespace Intel
