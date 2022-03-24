//===--- TraceDINodeTest.cpp - TraceBack Debug Info Node Tests --*- C++ -*-===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/DebugInfo/Intel_TraceBack/TraceDINode.h"
#include "llvm/MC/Intel_MCTrace.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "gtest/gtest.h"
#include <cstdint>
#include <vector>

using namespace llvm;
using namespace traceback;

struct Context {
  const char *Triple = "x86_64-pc-linux";
  std::unique_ptr<MCRegisterInfo> MRI;
  std::unique_ptr<MCAsmInfo> MAI;
  std::unique_ptr<MCSubtargetInfo> MSI;
  std::unique_ptr<MCContext> Ctx;

  Context() {
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargetMCs();

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

namespace {
TEST(TraceDINodeTest, TraceLine) {
  TraceRoutine Routine(8U, "foo", 1U, nullptr);
  constexpr unsigned LineNo1 = 1U;
  constexpr unsigned LineNo2 = LineNo1 + static_cast<unsigned>(INT8_MAX);
  constexpr unsigned LineNo3 = LineNo2 + static_cast<unsigned>(INT8_MAX + 1);
  constexpr unsigned LineNo4 = LineNo3 + static_cast<unsigned>(INT16_MAX);
  constexpr unsigned LineNo5 = LineNo4 + static_cast<unsigned>(INT16_MAX + 1);
  auto *Line1 = new TraceLine(0U, LineNo1, nullptr, &Routine);
  auto *Line2 = new TraceLine(LineNo1, LineNo2, nullptr, &Routine);
  auto *Line3 = new TraceLine(LineNo2, LineNo3, nullptr, &Routine);
  auto *Line4 = new TraceLine(LineNo3, LineNo4, nullptr, &Routine);
  auto *Line5 = new TraceLine(LineNo4, LineNo5, nullptr, &Routine);
  auto *Line6 = new TraceLine(LineNo5, LineNo4, nullptr, &Routine);
  auto *Line7 = new TraceLine(LineNo4, LineNo3, nullptr, &Routine);
  auto *Line8 = new TraceLine(LineNo3, LineNo2, nullptr, &Routine);
  auto *Line9 = new TraceLine(LineNo2, LineNo1, nullptr, &Routine);
  Routine.push_back(Line1);
  Routine.push_back(Line2);
  Routine.push_back(Line3);
  Routine.push_back(Line4);
  Routine.push_back(Line5);
  Routine.push_back(Line6);
  Routine.push_back(Line7);
  Routine.push_back(Line8);
  Routine.push_back(Line9);

  // Check delta line
  EXPECT_EQ(Line1->getDeltaLine(), 1);
  EXPECT_EQ(Line2->getDeltaLine(), INT8_MAX);
  EXPECT_EQ(Line3->getDeltaLine(), INT8_MAX + 1);
  EXPECT_EQ(Line4->getDeltaLine(), INT16_MAX);
  EXPECT_EQ(Line5->getDeltaLine(), INT16_MAX + 1);
  EXPECT_EQ(Line6->getDeltaLine(), INT16_MIN);
  EXPECT_EQ(Line7->getDeltaLine(), INT16_MIN + 1);
  EXPECT_EQ(Line8->getDeltaLine(), INT8_MIN);
  EXPECT_EQ(Line9->getDeltaLine(), INT8_MIN + 1);

  // Check line tag
  EXPECT_EQ(Line1->getTag(), traceback::TB_TAG_LN1);
  EXPECT_EQ(Line2->getTag(), traceback::TB_TAG_LN1);
  EXPECT_EQ(Line3->getTag(), traceback::TB_TAG_LN2);
  EXPECT_EQ(Line4->getTag(), traceback::TB_TAG_LN2);
  EXPECT_EQ(Line5->getTag(), traceback::TB_TAG_LN4);
  EXPECT_EQ(Line6->getTag(), traceback::TB_TAG_LN2);
  EXPECT_EQ(Line7->getTag(), traceback::TB_TAG_LN2);
  EXPECT_EQ(Line8->getTag(), traceback::TB_TAG_LN1);
  EXPECT_EQ(Line9->getTag(), traceback::TB_TAG_LN1);
}

TEST(TraceDINodeTest, TraceRoutine) {
  TraceRoutine Routine1(4U, "foo", 1U, nullptr);
  TraceRoutine Routine2(8U, "bar", 4U, nullptr);
  // Check routine name
  EXPECT_STREQ(Routine1.getName().c_str(), "foo");
  EXPECT_STREQ(Routine2.getName().c_str(), "bar");
  // Check routine tag
  EXPECT_EQ(Routine1.getTag(), traceback::TB_TAG_RTN32);
  EXPECT_EQ(Routine2.getTag(), traceback::TB_TAG_RTN64);
  // Check line number
  EXPECT_EQ(Routine1.getLine(), 1U);
  EXPECT_EQ(Routine2.getLine(), 4U);
}

TEST(TraceDINodeTest, TraceFile) {
  TraceFile File("main.c", 1U);
  // Check file name
  EXPECT_STREQ(File.getName().c_str(), "main.c");
  // Check file tag
  EXPECT_EQ(File.getTag(), traceback::TB_TAG_File);
  // Check file index
  EXPECT_EQ(File.getIndex(), 1U);
  File.setIndex(2U);
  EXPECT_EQ(File.getIndex(), 2U);
}

TEST(TraceDINodeTest, TraceModule) {
  if (!getContext())
    return;

  MCContext &Ctx = getContext();

  // Check last line number in last file.
  TraceModule Module(8U, 200U, "org");
  EXPECT_FALSE(Module.getLastLineNo());
  Module.addFile("file0", 1);
  EXPECT_FALSE(Module.getLastLineNo());
  Module.addRoutine("routine0", 1U,
                    Ctx.createTempSymbol("routine0_beg", false));
  EXPECT_FALSE(Module.getLastLineNo());
  Module.addLine(2U, nullptr);
  EXPECT_EQ(Module.getLastLineNo(), 2U);
  Module.addLine(3U, nullptr);
  EXPECT_EQ(Module.getLastLineNo(), 3U);
  Module.endRoutine(Ctx.createTempSymbol("routine0_end", false));

  // Check the empty routine is removed.
  Module.addRoutine("routine1", 4U,
                    Ctx.createTempSymbol("routine1_beg", false));
  Module.endRoutine(Ctx.createTempSymbol("routine1_end", false));
  EXPECT_EQ(Module.back().back().getName(), "routine0");

  // Check the empty file is removed.
  Module.addFile("file1", 2);
  Module.addFile("file2", 3);
  EXPECT_EQ(Module.back().getName(), "file2");
  Module.endModule();
  EXPECT_EQ(Module.back().getName(), "file0");

  // Check the indices of files are updated correctly
  // 1, 4, 3, 1 --> 0, 1, 2, 0
  TraceModule Module2(8U, 200U, "com");
  Module2.addFile("file0", 1);
  Module2.addRoutine("routine0", 1U,
                     Ctx.createTempSymbol("routine0_beg", false));
  Module2.addLine(3U, nullptr);
  Module2.addFile("file3", 4);
  Module2.addRoutine("routine1", 1U,
                     Ctx.createTempSymbol("routine1_beg", false));
  Module2.addLine(4U, nullptr);
  Module2.endRoutine(Ctx.createTempSymbol("routine1_end", false));
  Module2.addFile("file2", 3);
  Module2.addRoutine("routine2", 1U,
                     Ctx.createTempSymbol("routine2_beg", false));
  Module2.addLine(5U, nullptr);
  Module2.endRoutine(Ctx.createTempSymbol("routine2_end", false));
  Module2.addFile("file0", 1);
  Module2.addRoutine("routine3", 1U,
                     Ctx.createTempSymbol("routine3_beg", false));
  Module2.addLine(6U, nullptr);

  std::vector<unsigned> OldIndices;
  std::vector<unsigned> OldRef = {1, 4, 3, 1};
  for (const auto &File : Module2) {
    OldIndices.push_back(File.getIndex());
  }
  EXPECT_EQ(OldIndices, OldRef);
  Module2.endModule();

  std::vector<unsigned> NewIndices;
  std::vector<unsigned> NewRef = {0, 1, 2, 0};
  for (const auto &File : Module2) {
    NewIndices.push_back(File.getIndex());
  }
  EXPECT_EQ(NewIndices, NewRef);
}
} // end anonymous namespace
