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

#ifndef OpenCLStamp_H
#define OpenCLStamp_H

#include "Exception.h"
#include "IRunConfiguration.h"
#include "NEATVER.h"
#include "OpenCLProgram.h"
#include "OpenCLProgramConfiguration.h"
#include "OpenCLRunConfiguration.h"
#include "RefALUVER.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/MD5.h"
#include "llvm/Support/raw_ostream.h"
#include <fstream>
#include <iostream>

namespace Validation {

class OCLStamp {

public:
  OCLStamp(const IRunComponentConfiguration *pRunConfiguration,
           const IProgramConfiguration *pProgramConfiguration,
           const IProgram *pProgram);

  void generateStamps();

private:
  llvm::MD5::MD5Result generateMD5(const std::vector<uint8_t> &buffer);
  void readBinaryInputFile(const std::string inputFileName,
                           std::vector<uint8_t> &buffer);
  llvm::MD5::MD5Result
  calcStampKernelRef(const OpenCLKernelConfiguration *const config);
  llvm::MD5::MD5Result
  calcStampKernelNEAT(const OpenCLKernelConfiguration *const config);

  llvm::MD5::MD5Result m_RefStampCommon;
  llvm::MD5::MD5Result m_NeatStampCommon;

  bool m_useNEAT;

  const OpenCLProgramConfiguration *m_pProgramConfig;
};

} // namespace Validation
#endif // OpenCLStamp_H
