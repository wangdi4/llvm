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

#include "CLWGBoundDecoder.h"

namespace intel {

const std::string CLWGBoundDecoder::WGBoundPrefix  = "WG.boundaries.";

std::string CLWGBoundDecoder::encodeWGBound(std::string &funcName) {
  return WGBoundPrefix + funcName;
}


bool CLWGBoundDecoder::isWGBoundFunction(std::string& name) {
    return name.find(WGBoundPrefix) != std::string::npos;
}

unsigned CLWGBoundDecoder::getUniformIndex() {
  return 0;
}

unsigned CLWGBoundDecoder::getNumWGBoundArrayEntries(unsigned numDim) {
  return 2*numDim + 1;
}

unsigned CLWGBoundDecoder::getIndexOfInitGIDAtDim(unsigned dim) {
  return dim*2 + 1;
}

unsigned CLWGBoundDecoder::getIndexOfSizeAtDim(unsigned dim) {
  return dim*2 + 2;
}

} // namespace intel
