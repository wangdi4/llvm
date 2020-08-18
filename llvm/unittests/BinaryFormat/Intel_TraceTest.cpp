//===--- Intel_TraceTest.cpp - TraceBack Tag && Attribute Tests -*- C++ -*-===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/BinaryFormat/Intel_Trace.h"
#include "llvm/ADT/StringRef.h"
#include "gtest/gtest.h"
#include <cstdint>

using namespace llvm;
using namespace llvm::traceback;

namespace {

TEST(TraceTest, getTagString) {
  // A couple of valid tags.
  EXPECT_EQ(StringRef("TB_TAG_Module"), getTagString(TB_TAG_Module));
  EXPECT_EQ(StringRef("TB_TAG_RTN32"), getTagString(TB_TAG_RTN32));
  EXPECT_EQ(StringRef("TB_TAG_RTN64"), getTagString(TB_TAG_RTN64));
  EXPECT_EQ(StringRef("TB_TAG_File"), getTagString(TB_TAG_File));
  EXPECT_EQ(StringRef("TB_TAG_LN1"), getTagString(TB_TAG_LN1));
  EXPECT_EQ(StringRef("TB_TAG_LN2"), getTagString(TB_TAG_LN2));
  EXPECT_EQ(StringRef("TB_TAG_LN4"), getTagString(TB_TAG_LN4));
  EXPECT_EQ(StringRef("TB_TAG_PC1"), getTagString(TB_TAG_PC1));
  EXPECT_EQ(StringRef("TB_TAG_PC2"), getTagString(TB_TAG_PC2));
  EXPECT_EQ(StringRef("TB_TAG_PC4"), getTagString(TB_TAG_PC4));
  EXPECT_EQ(StringRef("TB_TAG_CO1"), getTagString(TB_TAG_CO1));
  EXPECT_EQ(StringRef("TB_TAG_CO2"), getTagString(TB_TAG_CO2));
}

TEST(TraceTest, getAttributeString) {
  // A couple of valid attributes.
  EXPECT_EQ(StringRef("TB_AT_MajorV"), getAttributeString(TB_AT_MajorV));
  EXPECT_EQ(StringRef("TB_AT_MinorV"), getAttributeString(TB_AT_MinorV));
  EXPECT_EQ(StringRef("TB_AT_TraceSize"), getAttributeString(TB_AT_TraceSize));
  EXPECT_EQ(StringRef("TB_AT_TextBegin"), getAttributeString(TB_AT_TextBegin));
  EXPECT_EQ(StringRef("TB_AT_NumOfFiles"),
            getAttributeString(TB_AT_NumOfFiles));
  EXPECT_EQ(StringRef("TB_AT_FileIdx"), getAttributeString(TB_AT_FileIdx));
  EXPECT_EQ(StringRef("TB_AT_TextSize"), getAttributeString(TB_AT_TextSize));
  EXPECT_EQ(StringRef("TB_AT_NameLength"),
            getAttributeString(TB_AT_NameLength));
  EXPECT_EQ(StringRef("TB_AT_ModuleName"),
            getAttributeString(TB_AT_ModuleName));
  EXPECT_EQ(StringRef("TB_AT_FileName"), getAttributeString(TB_AT_FileName));
  EXPECT_EQ(StringRef("TB_AT_Padding"), getAttributeString(TB_AT_Padding));
  EXPECT_EQ(StringRef("TB_AT_RoutineBegin"),
            getAttributeString(TB_AT_RoutineBegin));
  EXPECT_EQ(StringRef("TB_AT_RoutineName"),
            getAttributeString(TB_AT_RoutineName));
  EXPECT_EQ(StringRef("TB_AT_LN1"), getAttributeString(TB_AT_LN1));
  EXPECT_EQ(StringRef("TB_AT_LN2"), getAttributeString(TB_AT_LN2));
  EXPECT_EQ(StringRef("TB_AT_LN4"), getAttributeString(TB_AT_LN4));
  EXPECT_EQ(StringRef("TB_AT_PC1"), getAttributeString(TB_AT_PC1));
  EXPECT_EQ(StringRef("TB_AT_PC2"), getAttributeString(TB_AT_PC2));
  EXPECT_EQ(StringRef("TB_AT_PC4"), getAttributeString(TB_AT_PC4));
}

TEST(TraceTest, getAttributeForTag) {
  // Invalid tags.
  EXPECT_EQ(NUM_ATS, getAttributeForTag(NUM_TAGS));
  // A couple of valid tags.
  EXPECT_EQ(TB_AT_LN1, getAttributeForTag(TB_TAG_LN1));
  EXPECT_EQ(TB_AT_LN2, getAttributeForTag(TB_TAG_LN2));
  EXPECT_EQ(TB_AT_LN4, getAttributeForTag(TB_TAG_LN4));
  EXPECT_EQ(TB_AT_PC1, getAttributeForTag(TB_TAG_PC1));
  EXPECT_EQ(TB_AT_PC2, getAttributeForTag(TB_TAG_PC2));
  EXPECT_EQ(TB_AT_PC4, getAttributeForTag(TB_TAG_PC4));
}

TEST(TraceTest, getTagEncoding) {
  // A couple of valid tags.
  EXPECT_EQ(0x0A, getTagEncoding(TB_TAG_Module));
  EXPECT_EQ(0x02, getTagEncoding(TB_TAG_RTN32));
  EXPECT_EQ(0x0C, getTagEncoding(TB_TAG_RTN64));
  EXPECT_EQ(0x03, getTagEncoding(TB_TAG_File));
  EXPECT_EQ(0x04, getTagEncoding(TB_TAG_LN1));
  EXPECT_EQ(0x05, getTagEncoding(TB_TAG_LN2));
  EXPECT_EQ(0x06, getTagEncoding(TB_TAG_LN4));
  EXPECT_EQ(0x07, getTagEncoding(TB_TAG_PC1));
  EXPECT_EQ(0x08, getTagEncoding(TB_TAG_PC2));
  EXPECT_EQ(0x09, getTagEncoding(TB_TAG_PC4));
  EXPECT_EQ(0x80, getTagEncoding(TB_TAG_CO1));
  EXPECT_EQ(0xC0, getTagEncoding(TB_TAG_CO2));
}

TEST(TraceTest, getTagForEncoding) {
  // Invalid encoding.
  EXPECT_EQ(NUM_TAGS, getTagForEncoding(0x00));
  EXPECT_EQ(NUM_TAGS, getTagForEncoding(0xFF));
  // A couple of valid encodings for tags.
  EXPECT_EQ(TB_TAG_Module, getTagForEncoding(0x0A));
  EXPECT_EQ(TB_TAG_RTN32, getTagForEncoding(0x02));
  EXPECT_EQ(TB_TAG_RTN64, getTagForEncoding(0x0C));
  EXPECT_EQ(TB_TAG_File, getTagForEncoding(0x03));
  EXPECT_EQ(TB_TAG_LN1, getTagForEncoding(0x04));
  EXPECT_EQ(TB_TAG_LN2, getTagForEncoding(0x05));
  EXPECT_EQ(TB_TAG_LN4, getTagForEncoding(0x06));
  EXPECT_EQ(TB_TAG_PC1, getTagForEncoding(0x07));
  EXPECT_EQ(TB_TAG_PC2, getTagForEncoding(0x08));
  EXPECT_EQ(TB_TAG_PC4, getTagForEncoding(0x09));
  EXPECT_EQ(TB_TAG_CO1, getTagForEncoding(0x80));
  EXPECT_EQ(TB_TAG_CO2, getTagForEncoding(0xC0));
}

TEST(TraceTest, getAttributeSize) {
  // A couple of valid attributes.
  EXPECT_EQ(2U, getAttributeSize(TB_AT_MajorV));
  EXPECT_EQ(1U, getAttributeSize(TB_AT_MinorV));
  EXPECT_EQ(4U, getAttributeSize(TB_AT_TraceSize));
  EXPECT_EQ(0U, getAttributeSize(TB_AT_TextBegin));
  EXPECT_EQ(4U, getAttributeSize(TB_AT_NumOfFiles));
  EXPECT_EQ(4U, getAttributeSize(TB_AT_FileIdx));
  EXPECT_EQ(4U, getAttributeSize(TB_AT_TextSize));
  EXPECT_EQ(2U, getAttributeSize(TB_AT_NameLength));
  EXPECT_EQ(0U, getAttributeSize(TB_AT_ModuleName));
  EXPECT_EQ(0U, getAttributeSize(TB_AT_FileName));
  EXPECT_EQ(1U, getAttributeSize(TB_AT_Padding));
  EXPECT_EQ(0U, getAttributeSize(TB_AT_RoutineBegin));
  EXPECT_EQ(0U, getAttributeSize(TB_AT_RoutineName));
  EXPECT_EQ(1U, getAttributeSize(TB_AT_LN1));
  EXPECT_EQ(2U, getAttributeSize(TB_AT_LN2));
  EXPECT_EQ(4U, getAttributeSize(TB_AT_LN4));
  EXPECT_EQ(1U, getAttributeSize(TB_AT_PC1));
  EXPECT_EQ(2U, getAttributeSize(TB_AT_PC2));
  EXPECT_EQ(4U, getAttributeSize(TB_AT_PC4));
}

TEST(TraceTest, getOptimalLineTag) {
  EXPECT_EQ(getOptimalLineTag(INT8_MAX), TB_TAG_LN1);
  EXPECT_EQ(getOptimalLineTag(INT8_MIN), TB_TAG_LN1);
  EXPECT_EQ(getOptimalLineTag(INT16_MAX), TB_TAG_LN2);
  EXPECT_EQ(getOptimalLineTag(INT16_MIN), TB_TAG_LN2);
  EXPECT_EQ(getOptimalLineTag(INT32_MAX), TB_TAG_LN4);
  EXPECT_EQ(getOptimalLineTag(INT32_MIN), TB_TAG_LN4);
}

TEST(TraceTest, getOptimalPCTag) {
  EXPECT_EQ(getOptimalPCTag(UINT8_MAX), TB_TAG_PC1);
  EXPECT_EQ(getOptimalPCTag(UINT16_MAX), TB_TAG_PC2);
  EXPECT_EQ(getOptimalPCTag(UINT32_MAX), TB_TAG_PC4);
}

TEST(TraceTest, getOptimalCorrelationTag) {
  EXPECT_FALSE(getOptimalCorrelationTag(1, 0b11'1111 + 1));
  EXPECT_FALSE(getOptimalCorrelationTag(INT8_MAX + 1, 1));
  EXPECT_FALSE(getOptimalCorrelationTag(INT8_MIN - 1, 1));
  EXPECT_EQ(TB_TAG_CO1, getOptimalCorrelationTag(1, 0b11'1111));
  EXPECT_EQ(TB_TAG_CO2, getOptimalCorrelationTag(INT8_MAX, 0b11'1111));
}
} // end namespace
