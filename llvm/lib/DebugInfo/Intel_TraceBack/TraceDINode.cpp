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
  // Due to the implementation details of iplist, we need to check if a node has
  // a parent before call getPrevNode on it.
  auto getPrevNode = [](auto Node) -> decltype(Node->getPrevNode()) {
    if (!Node->getParent())
      return nullptr;
    return Node->getPrevNode();
  };

  // We need 2 steps to search the previous line number
  //  - Step1 Check if we have a previous line in the current routine, if yes,
  //  get its line number and search completes.
  //  - Step2 Check if we have a previous line in the previousroutine, if yes,
  //  get its line number otherwise assume 0.
  unsigned PrevLineNo;
  auto *PrevLine = getPrevNode(this);
  if (PrevLine)
    PrevLineNo = PrevLine->getLine();
  else if (getPrevNode(Parent) && !getPrevNode(Parent)->empty())
    PrevLineNo = getPrevNode(Parent)->back().getLine();
  else
    PrevLineNo = 0;

  return computeSignedDeltaLine(PrevLineNo, Line);
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
  OS.emitValueToAlignment(PointerSize);
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
  if (getIndex() != 0) {
    // Don't emit the index for the first file.
    emitTag(OS, getTag());
    emitIntAttribute(OS, traceback::TB_AT_FileIdx, getIndex());
  }
  emitChildren(OS);
}

TraceFile *TraceModule::getLastFile() {
  return Children.empty() ? nullptr : &(Children.back());
}

// Get the last child from the parent \p Node and its previous node.
template <typename NodeTy>
static auto getLastChildFrom(NodeTy *Node) -> decltype(&Node->back()) {
  if (!Node)
    return nullptr;
  if (!Node->empty())
    return &(Node->back());
  auto *PrevNode = Node->getPrevNode();
  if (!PrevNode)
    return nullptr;
  assert(!PrevNode->empty() &&
         "Empty node should be removed before a new node is inserted!");
  return &(PrevNode->back());
}

TraceRoutine *TraceModule::getLastRoutine() {
  return getLastChildFrom(getLastFile());
}

TraceLine *TraceModule::getLastLine() {
  return getLastChildFrom(getLastRoutine());
}

const TraceLine *TraceModule::getLastLine() const {
  return const_cast<TraceModule *>(this)->getLastLine();
}

unsigned TraceModule::getLastLineOrZero() const {
  if (getLastLine())
    return getLastLine()->getLine();
  return 0;
}

Optional<unsigned> TraceModule::getLastLineNo() const {
  if (getLastLine())
    return getLastLine()->getLine();
  return None;
}

void TraceModule::selfComplete(MCStreamer &OS) {
  // Base PC of .text section.
  // Don't use getBeginSymbol() for .text section here, the begin symbol is
  // the section's name to ELF while it is empty by default to COFF/MachO. The
  // begin symbol is emitted in MCStreamer::SwitchSection when a section begins
  // and here we already lost the chance to set and emit it.
  TextBegin = Children.front().front().getBegin();

  // .trace section range
  auto &Context = OS.getContext();
  MCSection *TraceSection = Context.getObjectFileInfo()->getTraceSection();

  if (!TraceSection->getBeginSymbol())
    TraceSection->setBeginSymbol(Context.createTempSymbol("sec_begin", true));
  TraceBegin = TraceSection->getBeginSymbol();
  TraceEnd = TraceSection->getEndSymbol(Context);
}

void TraceModule::emit(MCStreamer &OS) const {
  // Set alignment for .trace section, 8-byte align for x86_64 and 4-byte align
  // for ia32
  MCSection *TraceSection =
      OS.getContext().getObjectFileInfo()->getTraceSection();
  TraceSection->setAlignment(Align(PointerSize));

  // Switch to .trace for writing
  OS.SwitchSection(TraceSection);

  emitTag(OS, getTag());
  // Format version
  emitIntAttribute(OS, traceback::TB_AT_MajorV, getMajorID());
  emitIntAttribute(OS, traceback::TB_AT_MinorV, getMinorID());
  // .trace section size
  emitRangeAttribute(OS, traceback::TB_AT_TraceSize, TraceBegin, TraceEnd);
  // Base PC of .text section
  emitReferenceAttribute(OS, traceback::TB_AT_TextBegin, TextBegin,
                         PointerSize);
  // File count
  emitIntAttribute(OS, traceback::TB_AT_NumOfFiles,
                   Children.back().getIndex() + 1);
  // .text section size
  emitRangeAttribute(OS, traceback::TB_AT_TextSize, TextBegin,
                     Children.back().back().getEnd());
  // Module name
  // Module name is always empty in ICC's implementation.
  emitIntAttribute(OS, traceback::TB_AT_NameLength, getName().size());
  if (getName().size())
    emitNameAttribute(OS, traceback::TB_AT_ModuleName, getName());
  // File name
  for (const auto &File : Children) {
    const std::string &Filename = File.getName();
    emitIntAttribute(OS, traceback::TB_AT_NameLength, Filename.size());
    emitNameAttribute(OS, traceback::TB_AT_FileName, Filename);
  }

  emitChildren(OS);

  // Emit a label at the end of .trace function, which is used to determine the
  // size of the section
  assert(TraceEnd && "End label should not be null");
  OS.emitLabel(TraceEnd);
}

void TraceModule::removeEmptyFile() {
  if (!Children.empty() && getLastFile()->empty())
    Children.pop_back();
}

void TraceModule::addFile(const std::string &Name) {
  removeEmptyFile();

  // Don't call size() on iplist, it's not a constant time method.
  unsigned FileIndex = (Children.empty() ? 0 : getLastFile()->getIndex() + 1);
  push_back(new TraceFile(Name, FileIndex));
}

void TraceModule::addRoutine(const std::string &Name, unsigned Line,
                             MCSymbol *Begin) {
  assert(getLastFile() && "No file is added!");

  // Add a new routine
  getLastFile()->push_back(new TraceRoutine(PointerSize, Name, Line, Begin));
}

void TraceModule::addLine(unsigned Line, MCSymbol *Begin) {
  assert(getLastRoutine() && "No routine is added!");

  auto *LastRoutine = getLastRoutine();

  // Add a initial line record only if the prologue is not null.
  auto *RoutineBegin = LastRoutine->getBegin();
  if (LastRoutine->empty() && Begin != RoutineBegin)
    LastRoutine->push_back(new TraceLine(getLastLineOrZero(),
                                         LastRoutine->getLine(), RoutineBegin));

  LastRoutine->push_back(new TraceLine(getLastLineOrZero(), Line, Begin));
}

void TraceModule::endRoutine(MCSymbol *End) {
  assert(getLastRoutine() && "No routine is added!");
  assert(!getLastRoutine()->getEnd() && "Routine already has end label!");

  if (getLastRoutine()->empty())
    getLastFile()->pop_back();
  else
    getLastRoutine()->setEnd(End);
}

void TraceModule::finish(MCStreamer &OS) {
  removeEmptyFile();

  // Nothing to be emitted, return directly.
  if (Children.empty())
    return;

  selfComplete(OS);
  emit(OS);
}
