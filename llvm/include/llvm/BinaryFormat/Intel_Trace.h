//===--- Intel_Trace.h - TraceBack Tag && Attribute -------------*- C++ -*-===//
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
/// This file defines the enumerators and declare the utilities for TraceBack
/// tag && attribute.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_BINARYFORMAT_TRACE_H
#define LLVM_BINARYFORMAT_TRACE_H

#include <cstdint>
#include <optional>

namespace llvm {
class StringRef;

/// A uniform namespace for the entities in .trace section.
namespace traceback {
/// The contents of .trace section consists of various records, each record
/// has one tag and one or several attributes. The tag tells what the following
/// bytes represents while the attribute tells what's the value of specific
/// field.
enum Tag : uint8_t {
#define HANDLE_TB_TAG(NAME, ENCODING) TB_TAG_##NAME,
#include "llvm/BinaryFormat/Intel_Trace.def"
  NUM_TAGS
};

enum Attribute : uint8_t {
#define HANDLE_TB_AT(NAME, SIZE) TB_AT_##NAME,
#include "llvm/BinaryFormat/Intel_Trace.def"
  NUM_ATS
};

/// \returns the encoding value of tag \p T.
uint8_t getTagEncoding(Tag T);

/// \returns the tag if \p Byte is any tag's encoding, otherwise returns
/// the sentinel tag.
Tag getTagForEncoding(uint8_t Byte);

/// \returns the string representation of tag \p T.
StringRef getTagString(Tag T);

/// \returns the byte size of attribute \p Att.
uint32_t getAttributeSize(Attribute Att);

/// \returns the string representation of attribute \p Att.
StringRef getAttributeString(Attribute Att);

/// \returns the corresponding attribute for tag \p T if there is a
/// 1-to-1 mapping.
Attribute getAttributeForTag(Tag T);

/// \returns the optimal line tag with given delta line \p DeltaLine.
Tag getOptimalLineTag(int32_t DeltaLine);

/// \returns the optimal PC tag with given delta PC \p DeltaPC.
Tag getOptimalPCTag(uint32_t DeltaPC);

/// \returns the optimal correlation tag with given delta line \p DeltaLine
/// and delta pc \p DeltaPC.
std::optional<Tag> getOptimalCorrelationTag(int32_t DeltaLine,
                                            uint32_t DeltaPC);

} // namespace traceback
} // namespace llvm

#endif // LLVM_BINARYFORMAT_TRACE_H
