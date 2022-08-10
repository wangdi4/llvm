//===--- TraceContext.cpp - TraceBack Debug Info Interpreter ----*- C++ -*-===//
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
/// This file implements the methods of a top level entity to deal with
/// traceback debug information parsing.
///
//===----------------------------------------------------------------------===//

#include "llvm/DebugInfo/Intel_TraceBack/TraceContext.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/DebugInfo/Intel_TraceBack/TraceByteParser.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/Format.h"
#include <memory>

using namespace llvm;
using object::ObjectFile;
using object::SectionRef;

const StringRef TraceContext::TraceName = ".trace";

std::unique_ptr<TraceContext>
TraceContext::create(const ObjectFile &Obj,
                     const DenseMap<uint32_t, SmallString<32>> &RelMap) {
  SectionRef TraceSection;

  for (const SectionRef Section : Obj.sections()) {
    auto NameOrErr = Section.getName();
    if (!NameOrErr) {
      consumeError(NameOrErr.takeError());
      continue;
    }
    // Find the .trace section here.
    if (TraceName == *NameOrErr) {
      TraceSection = Section;
      break;
    }
  }

  // Find no .trace section.
  if (!TraceSection.getObject())
    return std::make_unique<TraceContext>();

  uint8_t PointerSize = Obj.getBytesInAddress();
  return std::make_unique<TraceContext>(PointerSize, TraceSection, RelMap);
}

void TraceContext::dump(raw_ostream &OS) {
  if (!TraceSectionOrNone) {
    OS << "Can not find section " << TraceName << "!\n";
    return;
  }

  auto TraceSection = TraceSectionOrNone.value();

  // Check the alignment.
  auto *Object = TraceSection.getObject();
  assert(Object && "Unexpected control flow!");
  bool NotAlign = [=]() {
    if (!Object->isRelocatableObject() && Object->isCOFF()) {
      // For COFF, the section alignment information is removed after link.
      // So we check if the start address can mod pointer size.
      uint64_t Addr = TraceSection.getAddress();
      return Addr % PointerSize != 0;
    }

    return TraceSection.getAlignment() != PointerSize;
  }();

  if (NotAlign)
    OS << format("Expect %u-byte align for section ", PointerSize) << TraceName
       << "!\n";

  Expected<StringRef> E = TraceSection.getContents();
  if (!E) {
    consumeError(E.takeError());
  }
  StringRef Data = *E;

  // Start dumping the contents of .trace section.
  OS << TraceName << " contents:\n";
  TraceByteParser Parser(PointerSize, make_range(Data.begin(), Data.end()),
                         RelMap, OS);
  Parser.parse();
}
