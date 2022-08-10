//===--- TraceByteParser.cpp - Parser For .trace Section --------*- C++ -*-===//
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
/// This file implements the methods of byte parser for .trace section.
///
//===----------------------------------------------------------------------===//

#include "llvm/DebugInfo/Intel_TraceBack/TraceByteParser.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdint>
#include <type_traits>

using namespace llvm;

TraceByteParser::TraceByteParser(
    uint8_t PointerSize, iterator_range<const_iterator> Range,
    const DenseMap<uint32_t, SmallString<32>> &RelMap, raw_ostream &OS)
    : PointerSize(PointerSize), Range(Range), RelMap(RelMap), OS(OS) {}

uint8_t TraceByteParser::takeLow6BitsAsDeltaPC(char Byte) {
  return static_cast<uint8_t>(Byte & ~TagMask);
}

bool TraceByteParser::consumeBytes(uint32_t Num) {
  assert(Num >= 1);
  if (Range.end() - It <= Num - 1)
    return false;
  auto InEnd = It + Num;
  Bytes.assign(It, InEnd);
  It = InEnd;
  return true;
}

void TraceByteParser::beginRoutine(const std::string &Routine) {
  // Update the last routine and clear address.
  LastRoutine = Routine;
  AddrPair = {0, 0};
}

void TraceByteParser::beginModule() { LinePair = {0, 0}; }

void TraceByteParser::updateTag(traceback::Tag Tag) {
  if (!LastTwoTags[0])
    LastTwoTags[0] = Tag;
  else if (!LastTwoTags[1])
    LastTwoTags[1] = Tag;
  else {
    LastTwoTags[0] = LastTwoTags[1];
    LastTwoTags[1] = Tag;
  }
}

char TraceByteParser::getByte() const { return Bytes[0]; }

const SmallVectorImpl<char> &TraceByteParser::getBytes() const { return Bytes; }

uint32_t TraceByteParser::getOffset() const {
  return It - Range.begin() - Bytes.size();
}

bool TraceByteParser::isNextZero() const {
  if (Range.end() == It)
    return false;
  return !(*It);
}

bool TraceByteParser::doesReachEnd() const { return Range.end() == It; }

traceback::Tag TraceByteParser::getLastTag() const {
  assert(LastTwoTags[0] && "Has not found any tag!");
  if (LastTwoTags[1])
    return LastTwoTags[1].value();
  return LastTwoTags[0].value();
}

static std::string takeByteAsHexString(char Byte) {
  std::string HexString;
  raw_string_ostream OS(HexString);

  OS << format_hex_no_prefix(static_cast<uint8_t>(Byte), 2);
  return HexString;
}

static std::string takeStringAsComment(StringRef Comment) {
  return std::string(" # ") + Comment.str() + "\n";
}

template <typename IntTy>
static IntTy takeBytesAsInteger(const SmallVectorImpl<char> &Bytes) {
  static_assert(std::is_integral<IntTy>::value, "Integral required.");
  assert(Bytes.size() == sizeof(IntTy) && "Size mismatch.");
  auto End = Bytes.rend();
  IntTy Num = 0;
  for (auto It = Bytes.rbegin(); It != End; ++It)
    Num = Num << 8 | static_cast<uint8_t>(*It);
  return Num;
}

static auto takeBytesAsUInt8 = takeBytesAsInteger<uint8_t>;
static auto takeBytesAsUInt16 = takeBytesAsInteger<uint16_t>;
static auto takeBytesAsUInt32 = takeBytesAsInteger<uint32_t>;
static auto takeBytesAsInt8 = takeBytesAsInteger<int8_t>;
static auto takeBytesAsInt16 = takeBytesAsInteger<int16_t>;
static auto takeBytesAsInt32 = takeBytesAsInteger<int32_t>;

static std::string takeBytesAsHexString(const SmallVectorImpl<char> &Bytes) {
  std::string HexString;
  for (auto Byte : Bytes) {
    HexString += takeByteAsHexString(Byte);
    HexString += " ";
  }
  return HexString;
}

void TraceByteParser::printStringWithComment(const std::string &String,
                                             StringRef Comment) {
  // At most time, the longest string to print is the entry point of
  // routine/section, and 3 characters are used to represent 1 byte,
  // e.g, "ff ".
  OS << left_justify(String, 3 * PointerSize) << takeStringAsComment(Comment);
}

void TraceByteParser::errorUnknownByte(char Byte) {
  printStringWithComment(takeByteAsHexString(Byte), "Unknown byte");
}

void TraceByteParser::dumpAttribute(traceback::Attribute Att) {
  printStringWithComment(takeBytesAsHexString(getBytes()),
                         traceback::getAttributeString(Att));
}

void TraceByteParser::TraceByteParser::dumpTag(traceback::Tag Tag) {
  printStringWithComment(takeByteAsHexString(traceback::getTagEncoding(Tag)),
                         traceback::getTagString(Tag));
}
void TraceByteParser::indentAfterAddr() { OS.indent(AddrWidth + 2); }

void TraceByteParser::dumpAndUpdateLine(int32_t DeltaLine) {
  LinePair.second = DeltaLine;
  uint32_t &Line = LinePair.first;
  if (DeltaLine > 0)
    Line += DeltaLine;
  else
    Line -= static_cast<uint32_t>(-DeltaLine);

  indentAfterAddr();
  OS << format("(line: %u, delta line: %d)\n", Line, DeltaLine);
}

void TraceByteParser::dumpAndUpdatePC(uint32_t DeltaPC) {
  AddrPair.second = DeltaPC;
  uint32_t &PC = AddrPair.first;
  indentAfterAddr();
  OS << format("(PC: %s+%#x, delta PC: %#x)\n", LastRoutine.c_str(), PC,
               DeltaPC + 1);
  PC += DeltaPC + 1;
}

void TraceByteParser::warnNonOptimalTag(traceback::Tag Tag) {
  if (Tag == getLastTag())
    return;
  indentAfterAddr();
  OS << "(warning: could use " << traceback::getTagString(Tag) << " here)\n";
}

void TraceByteParser::checkOptimalLineTag() {
#ifndef NDEBUG
  auto Tag = getLastTag();
  assert((Tag == traceback::TB_TAG_LN1 || Tag == traceback::TB_TAG_LN2 ||
          Tag == traceback::TB_TAG_LN4) &&
         "Unexpected control flow!");
#endif
  warnNonOptimalTag(traceback::getOptimalLineTag(LinePair.second));
}

void TraceByteParser::checkOptimalPCTag() {
#ifndef NDEBUG
  auto Tag = getLastTag();
  assert((Tag == traceback::TB_TAG_PC1 || Tag == traceback::TB_TAG_PC2 ||
          Tag == traceback::TB_TAG_PC4) &&
         "Unexpected control flow!");
#endif
  warnNonOptimalTag(traceback::getOptimalPCTag(AddrPair.second));
}

void TraceByteParser::checkOptimalCorrelationTag() {
  assert(getLastTag() == traceback::TB_TAG_CO2);
  auto TagOrNone =
      traceback::getOptimalCorrelationTag(LinePair.second, AddrPair.second);
  assert(TagOrNone && "Unexpected control flow!");
  warnNonOptimalTag(TagOrNone.value());
}

void TraceByteParser::checkReplacementForLastTwoTags() {
  assert(LastTwoTags[0] && "Unexpected control flow!");
  // Only find one tag here.
  if (!LastTwoTags[1])
    return;

  auto FirstTag = LastTwoTags[0].value();
  // The first tag is not a line tag here.
  if (FirstTag != traceback::TB_TAG_LN1 && FirstTag != traceback::TB_TAG_LN2 &&
      FirstTag != traceback::TB_TAG_LN4)
    return;

  auto SecondTag = LastTwoTags[1].value();
  // The second tag is not a PC tag here.
  if (SecondTag != traceback::TB_TAG_PC1 &&
      SecondTag != traceback::TB_TAG_PC2 && SecondTag != traceback::TB_TAG_PC4)
    return;

  auto TagOrNone =
      traceback::getOptimalCorrelationTag(LinePair.second, AddrPair.second);
  // Can not use a correlation tag here.
  if (!TagOrNone)
    return;

  indentAfterAddr();
  auto Tag = TagOrNone.value();
  OS << "(warning: could use " << traceback::getTagString(Tag)
     << " to replace the preivous two tags)\n";
}

static std::string takeBytesAsNameString(const SmallVectorImpl<char> &Bytes) {
  std::string NameString;
  for (auto Byte : Bytes) {
    if (isPrint(Byte))
      NameString.push_back(Byte);
    else
      NameString.push_back('.');
  }
  return NameString;
}

void TraceByteParser::dumpName() {
  const auto &Name = takeBytesAsNameString(getBytes());
  indentAfterAddr();
  OS << format("(name: \"%s\")\n", Name.c_str());
}

void TraceByteParser::dumpOffset() {
  auto Offset = getOffset();
  OS << format_hex_no_prefix(Offset, AddrWidth) << ": ";
}

void TraceByteParser::dumpRelocation() {
  auto Offset = getOffset();
  if (RelMap.find(Offset) == RelMap.end())
    return;
  indentAfterAddr();
  OS << "(relocation: " << RelMap[Offset] << ")\n";
}

traceback::Tag TraceByteParser::parseTag() {
  // The high 2 bits of tag:
  //  - CO1:   10
  //  - CO2:   11
  //  - Other: 00
  //
  //  CO1/CO2 only needs the high 2 bits are 10/11.
  consumeBytes();
  char Byte = getByte();
  char MaskedByte = Byte & TagMask;
  char TagByte = MaskedByte ? MaskedByte : Byte;
  auto Tag = traceback::getTagForEncoding(static_cast<uint8_t>(TagByte));
  updateTag(Tag);
  dumpOffset();
  return Tag;
}

bool TraceByteParser::parseAttribute(traceback::Attribute Att,
                                     uint32_t AttSize) {
  // Deduce the size of attribute if not provided.
  if (!AttSize) {
    switch (Att) {
    default:
      AttSize = getAttributeSize(Att);
      break;
    case traceback::TB_AT_CodeBegin:
    case traceback::TB_AT_RoutineBegin:
      AttSize = PointerSize;
      break;
    }
  }

  if (!consumeBytes(AttSize))
    return false;

  dumpOffset();
  dumpAttribute(Att);
  dumpRelocation();
  return true;
}

bool TraceByteParser::parseNamePossibleWithEntry(traceback::Attribute Att) {
  // Name length.
  if (!parseAttribute(traceback::TB_AT_NameLength))
    return false;
  auto NameLength = takeBytesAsUInt16(getBytes());
  // Allow empty module name.
  if (!NameLength) {
    if (Att == traceback::TB_AT_ModuleName)
      return true;
    else
      return false;
  }

  // Entry point of routine.
  if (Att == traceback::TB_AT_RoutineName &&
      !parseAttribute(traceback::TB_AT_RoutineBegin))
    return false;

  // Name string.
  if (!parseAttribute(Att, NameLength))
    return false;
  dumpName();

  switch (Att) {
  default:
    llvm_unreachable("Unexpected control flow!");
  case traceback::TB_AT_ModuleName:
    break;
  case traceback::TB_AT_FileName:
    break;
  case traceback::TB_AT_RoutineName:
    beginRoutine(takeBytesAsNameString(getBytes()));
    break;
  }

  return true;
}

void TraceByteParser::parseModule() {
  beginModule();

  // Major version.
  if (!parseAttribute(traceback::TB_AT_MajorV))
    return;
  // Minor version.
  if (!parseAttribute(traceback::TB_AT_MinorV))
    return;
  // Module size.
  if (!parseAttribute(traceback::TB_AT_ModuleSize))
    return;
  // Lowest PC covered by this module.
  if (!parseAttribute(traceback::TB_AT_CodeBegin))
    return;
  // Number of files
  if (!parseAttribute(traceback::TB_AT_NumOfFiles))
    return;
  unsigned NumOfFiles = takeBytesAsInt32(getBytes());
  // Code size.
  if (!parseAttribute(traceback::TB_AT_CodeSize))
    return;
  // Module name.
  if (!parseNamePossibleWithEntry(traceback::TB_AT_ModuleName))
    return;

  for (unsigned I = 0; I != NumOfFiles; ++I) {
    // File name.
    if (!parseNamePossibleWithEntry(traceback::TB_AT_FileName))
      return;
  }
  return;
}

void TraceByteParser::parseFile() {
  // File index.
  parseAttribute(traceback::TB_AT_FileIdx);
}

void TraceByteParser::parseRoutine() {
  // 1-byte padding.
  if (!(isNextZero()))
    return;
  if (!parseAttribute(traceback::TB_AT_Padding))
    return;
  // String of routine name.
  parseNamePossibleWithEntry(traceback::TB_AT_RoutineName);
}

void TraceByteParser::parseLine(traceback::Tag Tag) {
  if (!parseAttribute(traceback::getAttributeForTag(Tag)))
    return;

  const auto &Bytes = getBytes();
  int32_t DeltaLine;
  switch (Tag) {
  default:
    llvm_unreachable("Unexpected control flow!");
  case traceback::TB_TAG_LN1:
    DeltaLine = takeBytesAsInt8(Bytes);
    break;
  case traceback::TB_TAG_LN2:
    DeltaLine = takeBytesAsInt16(Bytes);
    break;
  case traceback::TB_TAG_LN4:
    DeltaLine = takeBytesAsInt32(Bytes);
    break;
  }
  dumpAndUpdateLine(DeltaLine);
  checkOptimalLineTag();
}

void TraceByteParser::parsePC(traceback::Tag Tag) {
  if (!parseAttribute(traceback::getAttributeForTag(Tag)))
    return;

  const auto &Bytes = getBytes();
  uint32_t DeltaPC;
  switch (Tag) {
  default:
    llvm_unreachable("Unexpected control flow!");
  case traceback::TB_TAG_PC1:
    DeltaPC = takeBytesAsUInt8(Bytes);
    break;
  case traceback::TB_TAG_PC2:
    DeltaPC = takeBytesAsUInt16(Bytes);
    break;
  case traceback::TB_TAG_PC4:
    DeltaPC = takeBytesAsUInt32(Bytes);
    break;
  }
  dumpAndUpdatePC(DeltaPC);
  checkOptimalPCTag();
  checkReplacementForLastTwoTags();
}

void TraceByteParser::parseOneByteCorrelation() {
  printStringWithComment(takeByteAsHexString(getByte()),
                         traceback::getTagString(traceback::TB_TAG_CO1));
  dumpAndUpdateLine(1);
  auto Byte = getByte();
  uint8_t DeltaPC = takeLow6BitsAsDeltaPC(Byte);
  dumpAndUpdatePC(DeltaPC);
}

void TraceByteParser::parseTwoByteCorrelation() {
  char FirstByte = getByte();
  if (doesReachEnd()) {
    errorUnknownByte(FirstByte);
    return;
  }
  consumeBytes();
  auto SecondByte = getByte();
  SmallVector<char, 2> Bytes = {FirstByte, SecondByte};
  printStringWithComment(takeBytesAsHexString(Bytes),
                         traceback::getTagString(traceback::TB_TAG_CO2));
  int8_t DeltaLine = SecondByte;
  dumpAndUpdateLine(DeltaLine);
  uint8_t DeltaPC = takeLow6BitsAsDeltaPC(FirstByte);
  dumpAndUpdatePC(DeltaPC);
  checkOptimalCorrelationTag();
}

bool TraceByteParser::parseAlignment() {
  SmallVector<char, 8> Bytes;

  // Byte zero can be used for padding.
  while (isNextZero()) {
    consumeBytes();
    Bytes.push_back(0);
  }

  if (Bytes.empty())
    return false;

  dumpOffset();
  printStringWithComment(takeBytesAsHexString(Bytes), "Align");
  return true;
}

void TraceByteParser::parse() {
  while (!doesReachEnd()) {
    // Alignment
    if (parseAlignment())
      continue;

    // How we interpret the bytes depends on the preceding tag, its following
    // bytes will be translated into different attributes. For tag CO1/CO2, we
    // need to do more things than simply dump it, because they contains the
    // information about PC and line.
    auto Tag = parseTag();
    switch (Tag) {
    default:
      dumpTag(Tag);
      break;
    case traceback::NUM_TAGS:
      errorUnknownByte(getByte());
      continue;
    case traceback::TB_TAG_CO1:
      parseOneByteCorrelation();
      continue;
    case traceback::TB_TAG_CO2:
      parseTwoByteCorrelation();
      continue;
    }

    // Interpret the bytes according to the tag.
    switch (Tag) {
    default:
      llvm_unreachable("Unexpected control flow!");
    case traceback::TB_TAG_Module:
      parseModule();
      break;
    case traceback::TB_TAG_File:
      parseFile();
      break;
    case traceback::TB_TAG_RTN64:
    case traceback::TB_TAG_RTN32:
      parseRoutine();
      break;
    case traceback::TB_TAG_LN1:
    case traceback::TB_TAG_LN2:
    case traceback::TB_TAG_LN4:
      parseLine(Tag);
      break;
    case traceback::TB_TAG_PC1:
    case traceback::TB_TAG_PC2:
    case traceback::TB_TAG_PC4:
      parsePC(Tag);
      break;
    }
  }
}
