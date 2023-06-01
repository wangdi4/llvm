//===--- TraceByteParser.h - Parser For .trace Section ----------*- C++ -*-===//
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
/// This file defines a byte parser to recognize the bytes in .trace section.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_DEBUGINFO_TRACEBACK_TRACEBYTEPARSER_H
#define LLVM_DEBUGINFO_TRACEBACK_TRACEBYTEPARSER_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/BinaryFormat/Intel_Trace.h"
#include <array>
#include <string>

namespace llvm {
class raw_ostream;

/// A parser that extracts the traceback debug from a range of bytes supplied
/// by TraceContext. It doesn't check the completeness of .trace section, on
/// the contrary, the goal is to interpret as much information as possible.
class TraceByteParser {
private:
  using const_iterator = StringRef::const_iterator;

private:
  /// Pointer Size of the target.
  uint8_t PointerSize;
  /// Range of the bytes.
  iterator_range<const_iterator> Range;
  /// Map the offset in .trace section to reloaction string.
  DenseMap<uint32_t, SmallString<32>> RelMap;
  /// Output streamer.
  raw_ostream &OS;
  /// Point to the byte to eat.
  const_iterator It = Range.begin();
  /// Store the latest consumed bytes.
  SmallVector<char, 8> Bytes;
  /// Relative offset from the begin of the routine and the PC length of the
  /// line.
  std::pair<uint32_t, uint32_t> AddrPair = {0, 0};
  /// Accumulated lines in the module and the delta line number between latest
  /// two lines.
  std::pair<uint32_t, int32_t> LinePair = {0, 0};
  /// Last seen routine.
  std::string LastRoutine;
  /// Last seen tags.
  std::array<std::optional<traceback::Tag>, 2> LastTwoTags;

private:
  /// Mask used to help interpret the tag byte.
  static constexpr uint8_t TagMask = 0b11'000000;
  /// Text width of the address in .trace section.
  static constexpr uint8_t AddrWidth = 8;
  /// Utility to extract delta PC from the tag of CO1/CO2.
  static uint8_t takeLow6BitsAsDeltaPC(char Byte);

  /// \returns true if the parser consume \p bytes successfully.
  bool consumeBytes(uint32_t Num = 1);
  /// Hint the following the line records are in scope of routine \p Routine.
  void beginRoutine(const std::string &Routine);
  /// Hint that we start a new module.
  void beginModule();
  /// Update the last seen tags.
  void updateTag(traceback::Tag Tag);

  /// Indent the streamer to the end of text of the address.
  void indentAfterAddr();
  /// Print the string \p String and add a comment \p Comment on the same line.
  void printStringWithComment(const std::string &String, StringRef Comment);
  /// Output the error message for unknown byte.
  void errorUnknownByte(char Byte);
  /// Dump the attribute \p Att.
  void dumpAttribute(traceback::Attribute Att);
  /// Dump the tag \p Tag.
  void dumpTag(traceback::Tag Tag);
  /// Dump and update the line information.
  void dumpAndUpdateLine(int32_t DeltaLine);
  /// Dump and update the PC information.
  void dumpAndUpdatePC(uint32_t DeltaPC);
  /// Warn we didn't use \p Tag as optimal tag.
  void warnNonOptimalTag(traceback::Tag Tag);
  /// Check if we could use the optimal tag for the last line.
  void checkOptimalLineTag();
  /// Check if we could use the optimal tag for the last PC.
  void checkOptimalPCTag();
  /// Check if we could use the optimal tag for the last correlation.
  void checkOptimalCorrelationTag();
  /// Check if we could use one tag to replace the last two tags.
  void checkReplacementForLastTwoTags();
  /// Dump the last consumed bytes as a ascii name.
  void dumpName();
  /// Dump the offset in .trace section for the last consumed bytes.
  void dumpOffset();
  /// Dump the relocation if the last consumed bytes correspond to a relocation.
  void dumpRelocation();

  /// Parse the next byte as a tag.
  traceback::Tag parseTag();
  /// Parse the following \p AttSize bytes as attribute \p Att, the attribute's
  /// size is deduced if AttSize is 0.
  /// \returns true on success.
  bool parseAttribute(traceback::Attribute Att, uint32_t AttSize = 0);
  /// Parse the following bytes as name length, entry(optional) and name string.
  /// \returns true on success.
  bool parseNamePossibleWithEntry(traceback::Attribute Att);
  /// Parse the following bytes as alignment greedily.
  /// \returns true on success.
  bool parseAlignment();
  /// Parse the following bytes as attributes of module.
  void parseModule();
  /// Parse the following bytes as attributes of file.
  void parseFile();
  /// Parse the following bytes as attributes of routine.
  void parseRoutine();
  /// Parse the following bytes as attributes of line \p Tag.
  void parseLine(traceback::Tag Tag);
  /// Parse the following bytes as attributes of PC \p Tag.
  void parsePC(traceback::Tag Tag);
  /// Parse the following bytes as attributes of CO1.
  void parseOneByteCorrelation();
  /// Parse the following bytes as attributes of CO2.
  void parseTwoByteCorrelation();

  /// \returns the last consumed byte.
  char getByte() const;
  /// \returns all the bytes consumed last time.
  const SmallVectorImpl<char> &getBytes() const;
  /// \returns the offset in .trace section for the last consumed bytes.
  uint32_t getOffset() const;
  /// \returns true if the next byte is zero.
  bool isNextZero() const;
  /// \returns true if we reach the end of .trace section.
  bool doesReachEnd() const;
  /// \returns the last seen tag.
  traceback::Tag getLastTag() const;

public:
  TraceByteParser(uint8_t PointerSize, iterator_range<const_iterator> Range,
                  const DenseMap<uint32_t, SmallString<32>> &RelMap,
                  raw_ostream &OS);

  void parse();
};

} // namespace llvm

#endif // LLVM_DEBUGINFO_TRACEBACK_TRACEBYTEPARSER_H
