// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
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

#include "ComparisonResults.h"
#include <algorithm>

using namespace Validation;

void ComparisonResults::AddMismatch(const MismatchedVal &in_Val) {
  /// Check if mismatchedVal with this index has been added
  /// to mismatch values previously
  uint32_t bufN = in_Val.GetBufIdx();
  CompStatMap::iterator mit = m_statMap.find(bufN);
  // if not exist add with zero initialized
  if (mit == m_statMap.end()) {
    CompStatistics cs(in_Val.GetDesc());
    cs.numMismatches = 0;
    mit = m_statMap.insert(std::pair<uint32_t, CompStatistics>(bufN, cs)).first;
  }

  // increase counter of mismatches
  (mit->second).numMismatches++;
  (mit->second).maxDiff =
      std::max(in_Val.ComputeDeviation(), mit->second.maxDiff);
  if ((mit->second).numMismatches < MAX_MISMATCHES)
    mismatches.push_back(in_Val);
}

ComparisonResults::MismatchedVal ComparisonResults::GetMismatch(size_t index) {
  return mismatches[index];
}

size_t ComparisonResults::GetMismatchCount() { return mismatches.size(); }

bool ComparisonResults::isFailed() { return (0 != GetMismatchCount()); }

void ComparisonResults::ReportDetail() {
  if (!mismatches.empty()) {
    std::cout << "Detailed statistics:" << std::endl;
    std::cout << mismatches.size() << " mismatches:" << std::endl;
    for (uint32_t i = 0; i < mismatches.size(); i++) {
      std::cout << mismatches[i].ToString() << std::endl;
    }
    std::cout << "------" << std::endl;
    std::cout << GetStatistics()->ToString() << std::endl;
  }
}

void ComparisonResults::Clear() { mismatches.clear(); }

void ComparisonResults::Report() {
  if (!m_statMap.empty()) {
    std::cout << "Kernel " << m_kernelName << "\n";
    std::cout << "BufferContainer[0] mismatch statistics\n";
    for (CompStatMap::iterator e = m_statMap.end(), it = m_statMap.begin();
         it != e; ++it) {
      const IMemoryObjectDesc *pDesc = (it->second.pDesc).get();

      if (BufferDesc::GetBufferDescName() == pDesc->GetName()) {
        const BufferDesc *pBufDesc = static_cast<const BufferDesc *>(pDesc);
        std::string typeStr = pBufDesc->GetElementDescription().TypeToString();
        std::cout << "Mismatches in Buffer[" << it->first
                  << "] : " << (it->second).numMismatches << std::endl;
        std::cout << "     Type: " << typeStr << std::endl;
        std::cout << "     Length: " << pBufDesc->NumOfElements() << std::endl;
        std::cout << "     Maximal mismatch deviation: "
                  << (it->second).maxDiff;
        bool isFloat;
        isFloat = pBufDesc->GetElementDescription().IsFloatingPoint();
        if (isFloat) {
          std::cout << " ulps";
        }
        std::cout << std::endl;
      } else if (ImageDesc::GetImageDescName() == pDesc->GetName()) {
        const ImageDesc *pImgDesc = static_cast<const ImageDesc *>(pDesc);
        const std::string typeStr = pImgDesc->DataTypeToString();
        const std::string orderStr = pImgDesc->OrderToString();
        std::cout << "Mismatches in Image[" << it->first
                  << "] : " << (it->second).numMismatches << std::endl;
        std::cout << "     Datatype  " << pImgDesc->DataTypeToString() << " "
                  << "Channel order " << pImgDesc->OrderToString() << std::endl
                  << "     Sizes[WxHxD]: " << pImgDesc->GetSizesDesc().width
                  << " " << pImgDesc->GetSizesDesc().height << " "
                  << pImgDesc->GetSizesDesc().depth << std::endl;
        std::cout << "     Maximal mismatch deviation: " << (it->second).maxDiff
                  << std::endl;
      } else {
        throw Exception::InvalidArgument("Not supported IMemObjectDesc");
      }
    }
  }
}

ComparisonResults::~ComparisonResults() {}
