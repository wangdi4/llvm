//===--- Intel_MCTraceTest.cpp - Machine Code Trace Tests -------*- C++ -*-===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/MC/Intel_MCTrace.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "gtest/gtest.h"
#include <cstdint>

using namespace llvm;
using namespace traceback;

namespace {
struct Context {
  const char *Triple = "x86_64-pc-linux";
  std::unique_ptr<MCRegisterInfo> MRI;
  std::unique_ptr<MCAsmInfo> MAI;
  std::unique_ptr<MCSubtargetInfo> MSI;
  std::unique_ptr<MCContext> Ctx;

  Context() {
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllDisassemblers();

    // If we didn't build x86, do not run the test.
    std::string Error;
    const Target *TheTarget = TargetRegistry::lookupTarget(Triple, Error);
    if (!TheTarget)
      return;

    MRI.reset(TheTarget->createMCRegInfo(Triple));
    MCTargetOptions MCOptions;
    MAI.reset(TheTarget->createMCAsmInfo(*MRI, Triple, MCOptions));
    MSI.reset(
        TheTarget->createMCSubtargetInfo(Triple, /*CPU=*/"", /*Features=*/""));
    Ctx = std::make_unique<MCContext>(llvm::Triple(Triple), MAI.get(),
                                      MRI.get(), MSI.get());
  }

  operator bool() { return Ctx.get(); }
  operator MCContext &() { return *Ctx; };
};

Context &getContext() {
  static Context Ctxt;
  return Ctxt;
}

TEST(MCTraceLineTest, Getter) {
  if (!getContext())
    return;

  const MCExpr *DeltaPC = MCConstantExpr::create(6, getContext());
  MCTraceLine Line(traceback::TB_TAG_LN1, 1, DeltaPC);
  EXPECT_EQ(Line.getDeltaLine(), 1);
  EXPECT_EQ(Line.getDeltaPC(), DeltaPC);
}

void verifyEncoding(int32_t DeltaLine, uint32_t DeltaPC,
                    ArrayRef<uint8_t> ExpectedEncoding) {
  SmallString<8> Buffer;
  raw_svector_ostream EncodingOS(Buffer);
  MCTraceLine::encode(EncodingOS, DeltaLine, DeltaPC);
  EXPECT_EQ(ExpectedEncoding, arrayRefFromStringRef(Buffer));
}

TEST(MCTraceLineTest, Encoding) {
  if (!getContext())
    return;

  constexpr uint8_t Bits6Max = 0b11'1111;
  constexpr uint8_t Bits7Min = 0b100'0000;
  const uint8_t LN1 = traceback::getTagEncoding(traceback::TB_TAG_LN1);
  const uint8_t LN2 = traceback::getTagEncoding(traceback::TB_TAG_LN2);
  const uint8_t LN4 = traceback::getTagEncoding(traceback::TB_TAG_LN4);
  const uint8_t PC1 = traceback::getTagEncoding(traceback::TB_TAG_PC1);
  const uint8_t PC2 = traceback::getTagEncoding(traceback::TB_TAG_PC2);
  const uint8_t PC4 = traceback::getTagEncoding(traceback::TB_TAG_PC4);

  // Line += 1, PC += 6; Format: CO1
  const uint8_t Encoding0[] = {(uint8_t)0b10'000'110};
  verifyEncoding(1, 6, Encoding0);
  // Line += 1, PC += Bits6Max; Format: CO1
  const uint8_t Encoding1[] = {(uint8_t)0b10'111'111};
  verifyEncoding(1, Bits6Max, Encoding1);

  // Line += 2, PC += Bits6Max; Format: CO2
  const uint8_t Encoding2[] = {(uint8_t)0b11'111'111, (uint8_t)2};
  verifyEncoding(2, Bits6Max, Encoding2);

  // Line += 1, PC += Bits7Min; Format: LN1 + PC1
  const uint8_t Encoding3[] = {LN1, 1, PC1, Bits7Min};
  verifyEncoding(1, Bits7Min, Encoding3);

  // Line += 2, PC += UINT8_MAX; Format: LN1 + PC1
  const uint8_t Encoding4[] = {LN1, (uint8_t)2, PC1, UINT8_MAX};
  verifyEncoding(2, UINT8_MAX, Encoding4);

  // Line += 2, PC += UINT8_MAX + 1; Format: LN1 + PC2
  const uint8_t Encoding5[] = {LN1, (uint8_t)2, PC2, 0, (uint8_t)1};
  verifyEncoding(2, UINT8_MAX + 1, Encoding5);

  // Line += 2, PC += UINT16_MAX; Format: LN1 + PC2
  const uint8_t Encoding6[] = {LN1, (uint8_t)2, PC2, UINT8_MAX, UINT8_MAX};
  verifyEncoding(2, UINT16_MAX, Encoding6);

  // Line += 2, PC += UINT16_MAX + 1; Format: LN1 + PC4
  const uint8_t Encoding7[] = {LN1, (uint8_t)2, PC4, 0, 0, (uint8_t)1, 0};
  verifyEncoding(2, UINT16_MAX + 1, Encoding7);

  // Line += 2, PC += UINT32_MAX; Format: LN1 + PC4
  const uint8_t Encoding8[] = {LN1,       (uint8_t)2, PC4,      UINT8_MAX,
                               UINT8_MAX, UINT8_MAX,  UINT8_MAX};
  verifyEncoding(2, UINT32_MAX, Encoding8);

  // Line += INT8_MAX, PC += Bits7Min; Format: LN1 + PC1
  const uint8_t Encoding9[] = {LN1, (uint8_t)INT8_MAX, PC1, Bits7Min};
  verifyEncoding(INT8_MAX, Bits7Min, Encoding9);

  // Line += INT8_MIN, PC += Bits7Min; Format: LN1 + PC1
  const uint8_t Encoding10[] = {LN1, (uint8_t)INT8_MIN, PC1, Bits7Min};
  verifyEncoding(INT8_MIN, Bits7Min, Encoding10);

  // Line += INT8_MAX + 1, PC += Bits7Min; Format: LN2 + PC1
  const uint8_t Encoding11[] = {LN2, (uint8_t)INT8_MIN, 0, PC1, Bits7Min};
  verifyEncoding(INT8_MAX + 1, Bits7Min, Encoding11);

  // Line += INT8_MIN - 1, PC += Bits7Min; Format: LN2 + PC1
  const uint8_t Encoding12[] = {LN2, (uint8_t)INT8_MAX, UINT8_MAX, PC1,
                                Bits7Min};
  verifyEncoding(INT8_MIN - 1, Bits7Min, Encoding12);

  // Line += INT16_MAX, PC += Bits7Min; Format: LN2 + PC1
  const uint8_t Encoding13[] = {LN2, UINT8_MAX, (uint8_t)INT8_MAX, PC1,
                                Bits7Min};
  verifyEncoding(INT16_MAX, Bits7Min, Encoding13);

  // Line += INT16_MIN, PC += Bits7Min; Format: LN2 + PC1
  const uint8_t Encoding14[] = {LN2, 0, (uint8_t)INT8_MIN, PC1, Bits7Min};
  verifyEncoding(INT16_MIN, Bits7Min, Encoding14);

  // Line += INT16_MAX + 1, PC += Bits7Min; Format: LN4 + PC1
  const uint8_t Encoding15[] = {LN4, 0, (uint8_t)INT8_MIN, 0, 0, PC1, Bits7Min};
  verifyEncoding(INT16_MAX + 1, Bits7Min, Encoding15);

  // Line += INT16_MIN, PC += Bits7Min; Format: LN4 + PC1
  const uint8_t Encoding16[] = {
      LN4, UINT8_MAX, (uint8_t)INT8_MAX, UINT8_MAX, UINT8_MAX, PC1, Bits7Min};
  verifyEncoding(INT16_MIN - 1, Bits7Min, Encoding16);
}
} // end anonymous namespace
