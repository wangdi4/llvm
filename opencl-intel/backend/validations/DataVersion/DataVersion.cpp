// INTEL CONFIDENTIAL
//
// Copyright 2013 Intel Corporation.
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

#include "DataVersion.h"
#include "Buffer.h"
#include "Image.h"
#include "cl_types.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "DataVersion"

enum convertEnum {
  ADDRESS_BASE = 0,
  CL_DEV_SAMPLER_ADDRESS_NONE = 0,
  CL_DEV_SAMPLER_ADDRESS_CLAMP =
      1 << ADDRESS_BASE, //!< Sampler is defined with CLAMP attribute
  CL_DEV_SAMPLER_ADDRESS_CLAMP_TO_EDGE =
      2 << ADDRESS_BASE, //!< Sampler is defined with CLAMP_TO_EDGE attribute
  CL_DEV_SAMPLER_ADDRESS_REPEAT =
      3 << ADDRESS_BASE, //!< Sampler is defined with REPEAT attribute
  CL_DEV_SAMPLER_ADDRESS_MIRRORED_REPEAT =
      4 << ADDRESS_BASE, //!< Sampler is defined with MIRRORED_REPEAT attribute
  ADDRESS_BITS = 3,      //!< number of bits required to represent address info
  ADDRESS_MASK = ((1 << ADDRESS_BITS) - 1),

  NORMALIZED_BASE = ADDRESS_BITS,
  CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE =
      0, //!< Sampler is defined with normalized coordinates set to FALSE
  CL_DEV_SAMPLER_NORMALIZED_COORDS_TRUE =
      1 << NORMALIZED_BASE, //!< Sampler is defined with normalized coordinates
                            //!< set to TRUE
  NORMALIZED_BITS = 1,      //!< number of bits required to represent normalize
                            //!< coordinates selection
  NORMALIZED_MASK = (((1 << NORMALIZED_BITS) - 1) << NORMALIZED_BASE),

  FILTER_BASE = NORMALIZED_BASE + NORMALIZED_BITS,
  CL_DEV_SAMPLER_FILTER_NEAREST =
      0 << FILTER_BASE, //!< Sampler is defined with filtering set to NEAREST
  CL_DEV_SAMPLER_FILTER_LINEAR =
      1 << FILTER_BASE, //!< Sampler is defined with filtering set to LINEAR
  FILTER_BITS = 2,      //!< number of bits required to represent filter info
  FILTER_MASK = (((1 << FILTER_BITS) - 1) << FILTER_BASE)
};

using namespace Validation;

static void convertSampler(uint32_t *inOutSampler) {
  uint32_t sampler = 0;

  switch ((*inOutSampler) & ADDRESS_MASK) {
  case CL_DEV_SAMPLER_ADDRESS_NONE:
    sampler |= CLK_ADDRESS_NONE;
    break;
  case CL_DEV_SAMPLER_ADDRESS_CLAMP:
    sampler |= CLK_ADDRESS_CLAMP;
    break;
  case CL_DEV_SAMPLER_ADDRESS_CLAMP_TO_EDGE:
    sampler |= CLK_ADDRESS_CLAMP_TO_EDGE;
    break;
  case CL_DEV_SAMPLER_ADDRESS_REPEAT:
    sampler |= CLK_ADDRESS_REPEAT;
    break;
  case CL_DEV_SAMPLER_ADDRESS_MIRRORED_REPEAT:
    sampler |= CLK_ADDRESS_MIRRORED_REPEAT;
    break;
  default:
    throw Exception::InvalidArgument(
        "DataVersion::convertSampler can't convert the old sampler format : "
        "addressing mode");
    break;
  }

  switch ((*inOutSampler) & FILTER_MASK) {
  case CL_DEV_SAMPLER_FILTER_NEAREST:
    sampler |= CLK_FILTER_NEAREST;
    break;
  case CL_DEV_SAMPLER_FILTER_LINEAR:
    sampler |= CLK_FILTER_LINEAR;
    break;
  default:
    throw Exception::InvalidArgument(
        "DataVersion::convertSampler can't convert the old sampler format : "
        "filter type");
  }

  *inOutSampler = sampler;
}

// This function returns vector of order numbers of
// "sampler_t" arguments of kernel
static std::vector<unsigned int> FindSamplers(llvm::Function *pKernel) {
  std::vector<unsigned int> res;

  assert(pKernel && "Invalid kernel!");

  auto *pArgTypeMD = pKernel->getMetadata("kernel_arg_type");

  if (!pArgTypeMD)
    return res;

  for (unsigned int i = 0; i < pArgTypeMD->getNumOperands(); ++i) {
    auto *pTypeName = llvm::cast<llvm::MDString>(pArgTypeMD->getOperand(i));
    std::string szTypeName = pTypeName->getString().str();
    LLVM_DEBUG(llvm::dbgs()
               << "kernel argument type " << (i) << " " << szTypeName << "\n");
    if (szTypeName == "sampler_t")
      res.push_back(i);
  }

  return res;
}

void ConvertData_v0_to_v1(IBufferContainerList *pContainerList,
                          llvm::Function *pKernel) {

  const IBufferContainer *pBufferContainer =
      pContainerList->GetBufferContainer(0);

  std::vector<unsigned int> samplerIndxs = FindSamplers(pKernel);

  for (unsigned int i = 0; i < samplerIndxs.size(); ++i) {
    uint32_t *pSampler =
        (uint32_t *)pBufferContainer->GetMemoryObject(samplerIndxs[i])
            ->GetDataPtr();
    // converting sampler to the new format
    convertSampler(pSampler);
  }
}

typedef void (*ConvertData)(IBufferContainerList *, llvm::Function *);

// the table of converters
ConvertData arrConvertData[] = {&ConvertData_v0_to_v1};

void DataVersion::ConvertData(IBufferContainerList *pContainerList,
                              llvm::Function *pKernel) {
  uint32_t finalVer = GetCurrentDataVersion();
  uint32_t numDataVersions = sizeof(arrConvertData) / sizeof(arrConvertData[0]);

  if (finalVer > numDataVersions)
    throw Exception::InvalidArgument(
        "DataVersion::ConvertData can't convert data : version number is "
        "bigger than the highest version of available convertor");
  else {
    // each converter changes the data in the container list
    for (uint32_t i = pContainerList->GetDataVersion(); i < finalVer; i++) {
      arrConvertData[i](pContainerList, pKernel);
    }
  }
}
