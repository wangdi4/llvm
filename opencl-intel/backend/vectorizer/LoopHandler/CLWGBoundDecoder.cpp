/*********************************************************************************************
 * Copyright ? 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
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

unsigned CLWGBoundDecoder::getInitGIDIndex(unsigned dim) {
  return dim*2 + 1;
}

unsigned CLWGBoundDecoder::getSizeIndex(unsigned dim) {
  return dim*2 + 2;
}

} // namespace intel
