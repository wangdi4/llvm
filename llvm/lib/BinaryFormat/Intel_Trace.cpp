//===--- Intel_Trace.cpp - TraceBack Tag && Attribute -----------*- C++ -*-===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the utilities for TraceBack tag && attribute.
///
//===----------------------------------------------------------------------===//

#include "llvm/BinaryFormat/Intel_Trace.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"

using namespace llvm;

uint8_t traceback::getTagEncoding(traceback::Tag T) {
  switch (T) {
  default:
    llvm_unreachable("Unknown tag");
#define HANDLE_TB_TAG(NAME, ENCODING)                                          \
  case TB_TAG_##NAME:                                                          \
    return static_cast<uint8_t>(ENCODING);
#include "llvm/BinaryFormat/Intel_Trace.def"
  }
}

traceback::Tag traceback::getTagForEncoding(uint8_t Byte) {
  switch (Byte) {
  default:
    return NUM_TAGS;
#define HANDLE_TB_TAG(NAME, ENCODING)                                          \
  case ENCODING:                                                               \
    return TB_TAG_##NAME;
#include "llvm/BinaryFormat/Intel_Trace.def"
  }
}

StringRef traceback::getTagString(traceback::Tag T) {
  switch (T) {
  default:
    llvm_unreachable("Unknown tag");
#define HANDLE_TB_TAG(NAME, ENCODING)                                          \
  case TB_TAG_##NAME:                                                          \
    return "TB_TAG_" #NAME;
#include "llvm/BinaryFormat/Intel_Trace.def"
  }
}

uint32_t traceback::getAttributeSize(traceback::Attribute Att) {
  switch (Att) {
  default:
    llvm_unreachable("Unknown attribute");
#define HANDLE_TB_AT(NAME, SIZE)                                               \
  case TB_AT_##NAME:                                                           \
    return SIZE;
#include "llvm/BinaryFormat/Intel_Trace.def"
  }
}

StringRef traceback::getAttributeString(traceback::Attribute Att) {
  switch (Att) {
  default:
    llvm_unreachable("Unknown attribute");
#define HANDLE_TB_AT(NAME, SIZE)                                               \
  case TB_AT_##NAME:                                                           \
    return "TB_AT_" #NAME;
#include "llvm/BinaryFormat/Intel_Trace.def"
  }
}

traceback::Attribute traceback::getAttributeForTag(traceback::Tag T) {
  switch (T) {
  default:
    return NUM_ATS;
#define TB_TAG_TO_TB_AT(TAG, AT)                                               \
  case TB_TAG_##TAG:                                                           \
    return TB_AT_##AT;
#include "llvm/BinaryFormat/Intel_Trace.def"
  }
}

traceback::Tag traceback::getOptimalLineTag(int32_t DeltaLine) {
  if (isInt<8>(DeltaLine))
    return traceback::TB_TAG_LN1;
  else if (isInt<16>(DeltaLine))
    return traceback::TB_TAG_LN2;
  return traceback::TB_TAG_LN4;
}

traceback::Tag traceback::getOptimalPCTag(uint32_t DeltaPC) {
  if (isUInt<8>(DeltaPC))
    return traceback::TB_TAG_PC1;
  else if (isUInt<16>(DeltaPC))
    return traceback::TB_TAG_PC2;
  return traceback::TB_TAG_PC4;
}

std::optional<traceback::Tag>
traceback::getOptimalCorrelationTag(int32_t DeltaLine, uint32_t DeltaPC) {
  if (DeltaPC > 0b11'1111)
    return std::nullopt;

  // CO1 (1 byte short form correlation)
  //   - Tag (high 2 bits): always 10 (binary)
  //   - Delta PC (low 6 bits): unsigned delta PC value minus 1
  //   - Delta line is always 1 (not encoded)
  if (DeltaLine == 1)
    return traceback::TB_TAG_CO1;
  // CO2 (2 byte short form correlation)
  //   - Tag (high 2 bits): always 11 (binary)
  //   - PC delta (low 6 bits): unsigned PC delta value minus 1
  //   - Line delta (1 byte): signed value
  else if (isInt<8>(DeltaLine))
    return traceback::TB_TAG_CO2;
  else
    return std::nullopt;
}
