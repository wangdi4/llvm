// INTEL CONFIDENTIAL
//
// Copyright 2012-2023 Intel Corporation.
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

#include "OpenCLStamp.h"
#include "auto_ptr_ex.h"
#include <algorithm>
#include <string.h>
#include <vector>

using namespace llvm;

namespace Validation {

template <typename T> uint8_t *begin_binary(T &obj) {
  return reinterpret_cast<uint8_t *>(&obj);
}
template <typename T> uint8_t *end_binary(T &obj) {
  return begin_binary(obj) + sizeof(obj);
}

OCLStamp::OCLStamp(const IRunComponentConfiguration *pRunConfiguration,
                   const IProgramConfiguration *pProgramConfiguration,
                   const IProgram *pProgram) {

  std::string compflags; // compilation flags

  m_pProgramConfig =
      static_cast<const OpenCLProgramConfiguration *>(pProgramConfiguration);

  std::vector<uint8_t> buffer;

  // add program text to buffer as binary
  readBinaryInputFile(m_pProgramConfig->GetProgramFilePath(), buffer);

  // get compilation flags
  compflags = m_pProgramConfig->GetCompilationFlags();
  buffer.insert(buffer.end(), compflags.begin(), compflags.end());

  // get execution options
  uint32_t defLocalWGSize =
      static_cast<const ReferenceRunOptions *>(pRunConfiguration)
          ->GetValue<uint32_t>(RC_COMMON_DEFAULT_LOCAL_WG_SIZE, 0);
  uint32_t runSingleWG =
      uint32_t(static_cast<const ReferenceRunOptions *>(pRunConfiguration)
                   ->GetValue<bool>(RC_COMMON_RUN_SINGLE_WG, false));

  uint64_t refVersion = RefALUVersion::GetVersion();

  // TODO: add randomDGSeed for radom data files
  // uint64_t randomDGSeed =  static_cast<const
  // ReferenceRunOptions*>(pRunConfiguration)->GetValue<uint64_t>(RC_COMMON_RANDOM_DG_SEED,
  // 0); buffer.insert(buffer.end(), begin_binary(randomDGSeed),
  // end_binary(randomDGSeed) );
  buffer.insert(buffer.end(), begin_binary(refVersion), end_binary(refVersion));
  buffer.insert(buffer.end(), begin_binary(defLocalWGSize),
                end_binary(defLocalWGSize));
  buffer.insert(buffer.end(), begin_binary(runSingleWG),
                end_binary(runSingleWG));

  // calculate MD5 of buffer
  m_RefStampCommon = generateMD5(buffer);

  // get NEAT parameters
  m_useNEAT = static_cast<const ReferenceRunOptions *>(pRunConfiguration)
                  ->GetValue<bool>(RC_REF_USE_NEAT, false);

  // data to calculate stamp for neat, it includes data for reference
  if (m_useNEAT) {
    uint64_t neatVersion = NEATVersion::GetVersion();

    uint32_t useFmaNEAT = uint32_t(true);

    buffer.insert(buffer.end(), begin_binary(neatVersion),
                  end_binary(neatVersion));
    buffer.insert(buffer.end(), begin_binary(useFmaNEAT),
                  end_binary(useFmaNEAT));
    // calculate MD5 of buffer
    m_NeatStampCommon = generateMD5(buffer);
  }
}

MD5::MD5Result OCLStamp::generateMD5(const std::vector<uint8_t> &buffer) {
  MD5 Hash;
  return Hash.hash(ArrayRef<uint8_t>(buffer));
}

void OCLStamp::readBinaryInputFile(const std::string inputFileName,
                                   std::vector<uint8_t> &buffer) {
  std::ifstream is;
  is.open(inputFileName.c_str(), std::ios::binary);

  if (is.fail()) {
    throw Exception::IOError("readBinaryInputFile: cannot open input file " +
                             inputFileName);
  }

  is.seekg(0, std::ios::end);
  int fileDataSize = is.tellg(); // get input data file length
  is.seekg(0, std::ios::beg);

  if (fileDataSize <= 0) {
    throw Exception::IOError("readBinaryInputFile: empty input file " +
                             inputFileName);
  }

  buffer.resize(fileDataSize);

  is.read((char *)&buffer[0], fileDataSize); // read data file to buffer
  if (is.fail()) {
    throw Exception::IOError("readBinaryInputFile: cannot read input file " +
                             inputFileName);
  }
  is.close();
}

MD5::MD5Result
OCLStamp::calcStampKernelRef(const OpenCLKernelConfiguration *const config) {

  size_t *pGlobalWorkOffset =
      const_cast<size_t *>(config->GetGlobalWorkOffset());
  size_t *pGlobalWorkSize = const_cast<size_t *>(config->GetGlobalWorkSize());
  size_t *pLocalWorkSize = const_cast<size_t *>(config->GetLocalWorkSize());

  std::vector<uint8_t> buffer;

  if (config->GetInputFileType() == Binary) {
    readBinaryInputFile(config->GetInputFilePath(), buffer);
  }

  // add integer data to buffer
  uint32_t workDimension = uint32_t(config->GetWorkDimension());
  buffer.insert(buffer.end(), begin_binary(workDimension),
                end_binary(workDimension));

  for (uint32_t i = 0; i < workDimension; i++) {
    uint64_t globalWorkOffset = uint64_t(*pGlobalWorkOffset);
    uint64_t globalWorkSize = uint64_t(*pGlobalWorkSize);
    uint64_t localWorkSize = uint64_t(*pLocalWorkSize);
    buffer.insert(buffer.end(), begin_binary(globalWorkOffset),
                  end_binary(globalWorkOffset));
    buffer.insert(buffer.end(), begin_binary(globalWorkSize),
                  end_binary(globalWorkSize));
    buffer.insert(buffer.end(), begin_binary(localWorkSize),
                  end_binary(localWorkSize));
  }

  buffer.insert(buffer.end(), m_RefStampCommon[0],
                m_RefStampCommon[0] + m_RefStampCommon.size());

  // calculate MD5 of buffer
  return generateMD5(buffer);
}

// neat stamps depends on ref stamp
MD5::MD5Result
OCLStamp::calcStampKernelNEAT(const OpenCLKernelConfiguration *const config) {

  std::vector<uint8_t> buffer;

  if (config->GetInputFileType() == Binary) {
    readBinaryInputFile(config->GetInputFilePath(), buffer);
  }

  buffer.insert(buffer.end(), m_NeatStampCommon[0],
                m_NeatStampCommon[0] + m_NeatStampCommon.size());

  // calculate MD5 of buffer
  return generateMD5(buffer);
}

void OCLStamp::generateStamps() {
  for (OpenCLProgramConfiguration::KernelConfigList::const_iterator it =
           m_pProgramConfig->beginKernels();
       it != m_pProgramConfig->endKernels(); ++it) {
    // so far we support stamps for binary input only
    if ((*it)->GetInputFileType() == Binary) {
      (*it)->SetReferenceStamp(calcStampKernelRef(*it));

      if (m_useNEAT)
        (*it)->SetNeatStamp(calcStampKernelNEAT(*it));
    } else {
      (*it)->SetReferenceStamp(ArrayRef<uint8_t>{});

      if (m_useNEAT)
        (*it)->SetNeatStamp(ArrayRef<uint8_t>{});
    }
  }
}

} // namespace Validation
