//===--- TraceContext.h - TraceBack Debug Info Context ----------*- C++ -*-===//
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
/// This file defines a top level entity to interpret the traceback debug
/// information, which has enough context to create real workers to do the
/// parsing work.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_DEBUGINFO_TRACEBACK_TRACECONTEXT_H
#define LLVM_DEBUGINFO_TRACEBACK_TRACECONTEXT_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/BinaryFormat/Intel_Trace.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm {
/// This data structure is the top level entity that deals with traceback debug
/// information parsing. The actual data is supplied through object file. It
/// creates a new TraceByteParser to do the real dump work when finding .trace
/// section.
class TraceContext {
private:
  /// The byte size of pointers.
  uint8_t PointerSize;
  /// The reference to the .trace section if it exists.
  std::optional<object::SectionRef> TraceSectionOrNone;
  /// Map from the offset in .trace section to its value string of relocations.
  DenseMap<uint32_t, SmallString<32>> RelMap;

public:
  /// Constants for string ".trace".
  static const StringRef TraceName;

  TraceContext(
      uint8_t PointerSize = 0,
      std::optional<object::SectionRef> TraceSectionOrNone = std::nullopt,
      const DenseMap<uint32_t, SmallString<32>> &RelMap =
          DenseMap<uint32_t, SmallString<32>>())
      : PointerSize(PointerSize), TraceSectionOrNone(TraceSectionOrNone),
        RelMap(RelMap) {}
  static std::unique_ptr<TraceContext>
  create(const object::ObjectFile &Obj,
         const DenseMap<uint32_t, SmallString<32>> &RelMap);

  /// Dump the .trace section.
  void dump(raw_ostream &OS);
};
} // namespace llvm

#endif // LLVM_DEBUGINFO_TRACEBACK_TRACECONTEXT_H
