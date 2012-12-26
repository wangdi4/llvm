/*****************************************************************************\

Copyright (c) Intel Corporation (2012).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  OpenCLStamp.h

\*****************************************************************************/
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