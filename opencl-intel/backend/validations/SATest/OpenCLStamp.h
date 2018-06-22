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

#ifndef OpenCLStamp_H
#define OpenCLStamp_H

#include <iostream>
#include <fstream>
#include "md5.h"

#include "RefALUVER.h"
#include "NEATVER.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/DataTypes.h"

#include "IRunConfiguration.h"
#include "OpenCLRunConfiguration.h"
#include "OpenCLProgramConfiguration.h"
#include "OpenCLProgram.h"

#include "Exception.h"

namespace Validation
{

class OCLStamp {

public:

   OCLStamp(const IRunComponentConfiguration* pRunConfiguration, 
             const IProgramConfiguration* pProgramConfiguration, 
             const IProgram* pProgram);


  // returns stamp length
  static int GetStampLen()
  {
      return m_stampLen;
  }


   void generateStamps();

private:
    std::vector<uint8_t>  generateMD5 (const std::vector<uint8_t>& buffer);
    void readBinaryInputFile (const std::string inputFileName, std::vector<uint8_t>& buffer);
    std::vector<uint8_t> calcStampKernelRef (const OpenCLKernelConfiguration * const config);
    std::vector<uint8_t> calcStampKernelNEAT (const OpenCLKernelConfiguration * const config);

    static const int m_stampLen = 16;

    std::vector<uint8_t> m_RefStampCommon;
    std::vector<uint8_t> m_NeatStampCommon;

    bool m_useNEAT;

    const OpenCLProgramConfiguration * m_pProgramConfig;
};

}
#endif // OpenCLStamp_H
