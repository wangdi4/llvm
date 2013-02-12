/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
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
