//===--- TraceDINode.h - TraceBack Debug Info Node --------------*- C++ -*-===//
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
/// This file implements the methods of the data structures used to store the
/// debug information for traceback in a compact and easy-to-debug format.
///
//===----------------------------------------------------------------------===//

#include "llvm/DebugInfo/Intel_TraceBack/TraceDINode.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/MC/Intel_MCTrace.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCStreamer.h"
#include <cstdint>

using namespace llvm;

static int computeSignedDeltaLine(unsigned PrevLine, unsigned CurrLine) {
  // Take care of the signed overflow here.
  bool PositiveDelta = CurrLine > PrevLine;
  unsigned DeltaLineAbs =
      PositiveDelta ? CurrLine - PrevLine : PrevLine - CurrLine;
  assert(DeltaLineAbs <= INT32_MAX && "Unsupported delta line");
  return PositiveDelta ? static_cast<int>(DeltaLineAbs)
                       : -static_cast<int>(DeltaLineAbs);
}

traceback::Tag TraceLine::getOptimalTag(unsigned PrevLine, unsigned CurrLine) {
  int DeltaLine = computeSignedDeltaLine(PrevLine, CurrLine);

  return traceback::getOptimalLineTag(DeltaLine);
}

int TraceLine::getDeltaLine() const {
  return computeSignedDeltaLine(PrevLine, Line);
}

const MCExpr *TraceLine::getDeltaPCMinusOne(MCContext &Context) const {
  const TraceLine *NextLine = getNextNode();
  MCSymbol *End = NextLine ? NextLine->getBegin() : getParent()->getEnd();

  const MCExpr *HiMinusLo =
      MCBinaryExpr::createSub(MCSymbolRefExpr::create(End, Context),
                              MCSymbolRefExpr::create(Begin, Context), Context);
  return MCBinaryExpr::createAdd(HiMinusLo, MCConstantExpr::create(-1, Context),
                                 Context);
}

TraceModule::TraceModule(unsigned PointerSize, unsigned Version,
                         const std::string &Name)
    : TraceDINodeWithChildren<TraceFile>(traceback::TB_TAG_Module, Name),
      PointerSize(PointerSize), Version(Version) {}

void TraceDINode::emitTag(MCStreamer &OS, traceback::Tag Tag) {
  OS.AddComment(traceback::getTagString(Tag));
  OS.emitIntValue(traceback::getTagEncoding(Tag), 1);
}

void TraceDINode::emitIntAttribute(MCStreamer &OS, traceback::Attribute Att,
                                   int Val) {
  OS.AddComment(traceback::getAttributeString(Att));
  OS.emitIntValue(Val, traceback::getAttributeSize(Att));
}

void TraceDINode::emitNameAttribute(MCStreamer &OS, traceback::Attribute Att,
                                    const std::string &Name) {
  OS.AddComment(traceback::getAttributeString(Att));
  OS.emitBytes(Name);
}

void TraceDINode::emitReferenceAttribute(MCStreamer &OS,
                                         traceback::Attribute Att,
                                         MCSymbol *Ref, unsigned PointerSize) {
  OS.AddComment(traceback::getAttributeString(Att));
  const MCExpr *Expr = MCSymbolRefExpr::create(Ref, OS.getContext());
  OS.emitValue(Expr, PointerSize);
}

void TraceDINode::emitRangeAttribute(MCStreamer &OS, traceback::Attribute Att,
                                     MCSymbol *Begin, MCSymbol *End) {
  OS.AddComment(traceback::getAttributeString(Att));
  OS.emitAbsoluteSymbolDiff(End, Begin, traceback::getAttributeSize(Att));
}

void TraceLine::emit(MCStreamer &OS) const {
  OS.emitTraceLine(MCTraceLine(getTag(), getDeltaLine(),
                               getDeltaPCMinusOne(OS.getContext())));
}

void TraceRoutine::emit(MCStreamer &OS) const {
  unsigned PointerSize = (getTag() == traceback::TB_TAG_RTN32) ? 4 : 8;
  // Alignment
  OS.AddComment("Align to boundary " + Twine(PointerSize));
  OS.emitValueToAlignment(Align(PointerSize));
  // Tag
  emitTag(OS, getTag());
  // 1-byte zero padding
  emitIntAttribute(OS, traceback::TB_AT_Padding, 0);
  // Name length
  const std::string &Name = getName();
  emitIntAttribute(OS, traceback::TB_AT_NameLength, Name.size());
  // Routine entry
  emitReferenceAttribute(OS, traceback::TB_AT_RoutineBegin, getBegin(),
                         PointerSize);
  // Name string
  emitNameAttribute(OS, traceback::TB_AT_RoutineName, Name);

  emitChildren(OS);
}

void TraceFile::emit(MCStreamer &OS) const {
  if (Parent != nullptr && getPrevNode() != nullptr) {
    // Don't emit the index for the first file.
    emitTag(OS, getTag());
    emitIntAttribute(OS, traceback::TB_AT_FileIdx, getIndex());
  }
  emitChildren(OS);
}

// Get the last child from the parent \p Node.
template <typename NodeTy>
static auto getLastChildFrom(NodeTy *Node) -> decltype(&Node->back()) {
  if (!Node || Node->empty())
    return nullptr;
  return &(Node->back());
}

TraceFile *TraceModule::getLastFile() { return getLastChildFrom(this); }

const TraceFile *TraceModule::getLastFile() const {
  return const_cast<TraceModule *>(this)->getLastFile();
}

TraceRoutine *TraceModule::getLastRoutine() {
  return getLastChildFrom(getLastFile());
}

const TraceRoutine *TraceModule::getLastRoutine() const {
  return const_cast<TraceModule *>(this)->getLastRoutine();
}

TraceLine *TraceModule::getLastLine() {
  return getLastChildFrom(getLastRoutine());
}

const TraceLine *TraceModule::getLastLine() const {
  return const_cast<TraceModule *>(this)->getLastLine();
}

Optional<unsigned> TraceModule::getLastLineNo() const {
  if (getLastLine())
    return getLastLine()->getLine();
  return None;
}

bool TraceModule::isLastRoutineEmpty() const {
  assert(getLastRoutine() && "No routine is added!");
  return getLastRoutine()->empty();
}

// While libirc resets the accumulated line number to zero when starting a new
// module, it doesn't do it when starting a new file, e.g, if a.c and b.c are
// in the same module, the last line record for a.c represents line 16, the
// first line record for b.c represents line 1 and b.c is placed right after
// a.c in .trace section, then the delta line for first line record of b.c
// should be -17 rather than 1.
unsigned TraceModule::getLastLineNoInModuleOrZero() const {
  // No file added.
  const TraceFile *File = getLastFile();
  if (!File)
    return 0;
  assert(!File->empty() && "Unexpected control flow!");

  const TraceRoutine &Routine = File->back();
  // Return the previous line record in current routine.
  if (!Routine.empty())
    return Routine.back().getLine();

  const TraceRoutine *PrevRoutine = Routine.getPrevNode();
  // Return the last line record in previous routine.
  if (PrevRoutine) {
    assert(!PrevRoutine->empty() && "Empty routine should be removed!");
    return PrevRoutine->back().getLine();
  }

  const TraceFile *PrevFile = File->getPrevNode();
  // There is no line record in current module.
  if (!PrevFile)
    return 0;

  // Return the last line record in previous file.
  assert(!PrevFile->empty() && "Empty file should be removed!");
  const TraceRoutine &LastRoutineInPrevFile = PrevFile->back();
  assert(!LastRoutineInPrevFile.empty() && "Empty routine should be removed!");
  return LastRoutineInPrevFile.back().getLine();
}

void TraceModule::emit(MCStreamer &OS) const {
  assert(IsEnded && "This module has not been ended!");
  // No files, return directly.
  if (Children.empty())
    return;

  assert(!Children.front().empty() && "Find an empty file!");

  // .trace section range
  MCContext &Context = OS.getContext();
  MCSection *TraceSection = Context.getObjectFileInfo()->getTraceSection();
  assert(TraceSection && "Unexpected control flow!");
  if (!TraceSection->getBeginSymbol())
    TraceSection->setBeginSymbol(Context.createTempSymbol("sec_begin", true));
  MCSymbol *ModuleBegin = Context.createTempSymbol("module_begin", true);
  MCSymbol *ModuleEnd = Context.createTempSymbol("module_end", true);

  // Set alignment for .trace section, 8-byte align for x86_64 and 4-byte align
  // for ia32
  TraceSection->setAlignment(Align(PointerSize));

  // Switch to .trace for writing
  OS.switchSection(TraceSection);
  OS.emitLabel(ModuleBegin);
  emitTag(OS, getTag());
  // Format version
  emitIntAttribute(OS, traceback::TB_AT_MajorV, getMajorID());
  emitIntAttribute(OS, traceback::TB_AT_MinorV, getMinorID());
  // Module size
  emitRangeAttribute(OS, traceback::TB_AT_ModuleSize, ModuleBegin, ModuleEnd);
  // Code begin
  MCSymbol *CodeBegin = Children.front().front().getBegin();
  emitReferenceAttribute(OS, traceback::TB_AT_CodeBegin, CodeBegin,
                         PointerSize);
  // File count
  emitIntAttribute(OS, traceback::TB_AT_NumOfFiles, IndexToFile.size());
  // Code size
  emitRangeAttribute(OS, traceback::TB_AT_CodeSize, CodeBegin,
                     Children.back().back().getEnd());
  // Module name
  emitIntAttribute(OS, traceback::TB_AT_NameLength, getName().size());
  if (getName().size())
    emitNameAttribute(OS, traceback::TB_AT_ModuleName, getName());
  // File name
  for (unsigned I = 0; I != IndexToFile.size(); ++I) {
    auto It = IndexToFile.find(I);
    assert(It != IndexToFile.end());
    const std::string &Filename = (*It).second->getName();
    emitIntAttribute(OS, traceback::TB_AT_NameLength, Filename.size());
    emitNameAttribute(OS, traceback::TB_AT_FileName, Filename);
  }

  emitChildren(OS);

  // Emit a label at the end of this module.
  assert(ModuleEnd && "End label should not be null");
  OS.emitLabel(ModuleEnd);
}

void TraceModule::removeEmptyFile() {
  if (!Children.empty() && Children.back().empty())
    Children.pop_back();
}

void TraceModule::addFile(const std::string &Name, unsigned Index) {
  assert(!IsEnded && "The module is ended!");
  removeEmptyFile();

  push_back(new TraceFile(Name, Index));
}

void TraceModule::addRoutine(const std::string &Name, unsigned Line,
                             MCSymbol *Begin) {
  assert(!IsEnded && "The module is ended!");
  assert(getLastFile() && "No file is added!");

  // Add a new routine
  getLastFile()->push_back(new TraceRoutine(PointerSize, Name, Line, Begin));
}

void TraceModule::addLine(unsigned Line, MCSymbol *Begin) {
  assert(!IsEnded && "The module is ended!");
  assert(getLastRoutine() && "No routine is added!");
  assert(!getLastRoutine()->getEnd() && "The routine is ended!");

  getLastRoutine()->push_back(
      new TraceLine(getLastLineNoInModuleOrZero(), Line, Begin));
}

void TraceModule::endRoutine(MCSymbol *End) {
  assert(getLastRoutine() && "No routine is added!");
  assert(!getLastRoutine()->getEnd() && "Routine already has end label!");

  if (getLastRoutine()->empty())
    getLastFile()->pop_back();
  else
    getLastRoutine()->setEnd(End);
}

void TraceModule::endModule() {
  // This module is already ended, return directly.
  if (IsEnded)
    return;

  removeEmptyFile();

  // Update the indices of files.
  // Some corner cases need to be handled here.
  // 1. A TraceFile is inserted into this module by the order in which its child
  //    routine appears in IR and the routines from different files can be
  //    overlapped in a IR file, so the indices of the TraceFiles may be
  //    duplicated, such as 0, 1, 0.
  // 2. A TraceFile may be empty due to the source file is empty or all its
  //    child routines miss debug information, and the empty TraceFile would be
  //    removed when contructing this module. So the indices of TraceFiles may
  //    be discontinuous even if we use continuous indices when call addFile,
  //    such as 1, 2, 4.
  // 3. 1 and 2 mix together. The indices of TraceFiles may look like
  //    1, 4, 3, 1.
  //
  // We need to update the indices of files to 0, 1, 2, 0 for case 3.
  unsigned Index = 0;
  for (auto &File : Children) {
    unsigned OldIndex = File.getIndex();
    if (IndexToFile.find(OldIndex) == IndexToFile.end()) {
      IndexToFile.insert({OldIndex, &File});
      File.setIndex(Index++);
    } else {
      File.setIndex(IndexToFile[OldIndex]->getIndex());
    }
  }

  IndexToFile.clear();
  for (const auto &File : Children)
    IndexToFile.insert({File.getIndex(), &File});

  IsEnded = true;
}
