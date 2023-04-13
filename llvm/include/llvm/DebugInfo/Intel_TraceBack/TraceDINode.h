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
/// This file defines the data structures used to store the debug information
/// for traceback in a compact and easy-to-debug format.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_DEBUGINFO_TRACEBACK_TRACEDINODE_H
#define LLVM_DEBUGINFO_TRACEBACK_TRACEDINODE_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/ilist.h"
#include "llvm/ADT/ilist_node.h"
#include "llvm/BinaryFormat/Intel_Trace.h"
#include "llvm/MC/MCSymbol.h"

namespace llvm {
class TraceRoutine;
class TraceFile;
class TraceModule;
class MCStreamer;
class MCObjectStreamer;
class MCExpr;

/// A structured debug information node. This class and the derived classes
/// should not have any virtual member function for space saving.
class TraceDINode {
private:
  /// Tag of this node.
  traceback::Tag Tag;

protected:
  TraceDINode(traceback::Tag Tag, TraceDINode *Parent = nullptr) : Tag(Tag) {}

  /// Emit the tag of record.
  static void emitTag(MCStreamer &OS, traceback::Tag Tag);

  /// Emit the attribute when it is a integer.
  static void emitIntAttribute(MCStreamer &OS, traceback::Attribute Attr,
                               int Val);
  /// Emit the attribute when it is a name.
  static void emitNameAttribute(MCStreamer &OS, traceback::Attribute Attr,
                                const std::string &Name);
  /// Emit the attribute when it refers a position in any section.
  static void emitReferenceAttribute(MCStreamer &OS, traceback::Attribute Attr,
                                     MCSymbol *Ref, unsigned PointerSize);
  /// Emit the attribute when it represents a range.
  static void emitRangeAttribute(MCStreamer &OS, traceback::Attribute Attr,
                                 MCSymbol *Begin, MCSymbol *End);

public:
  TraceDINode() = delete;
  TraceDINode(const TraceDINode &) = delete;
  TraceDINode &operator=(const TraceDINode &) = delete;

  /// \returns the tag of this node.
  traceback::Tag getTag() const { return Tag; }
};

/// A data structure used to maintain the debug information for a line record,
/// including the tag and attribute for the line, PC, correlation.
class TraceLine : public TraceDINode,
                  public ilist_node_with_parent<TraceLine, TraceRoutine> {
private:
  /// Line number of previous record.
  unsigned PrevLine;
  /// Line number of this record.
  unsigned Line;
  /// Label before the first of the instructions that this line covers.
  MCSymbol *Begin;
  /// Parent routine.
  TraceRoutine *Parent;

  /// Utility to determine the optimal line tag.
  static traceback::Tag getOptimalTag(unsigned PrevLine, unsigned CurrLine);

public:
  TraceLine(unsigned PrevLine, unsigned CurrLine, MCSymbol *Begin,
            TraceRoutine *Parent = nullptr)
      : TraceDINode(getOptimalTag(PrevLine, CurrLine)), PrevLine(PrevLine),
        Line(CurrLine), Begin(Begin), Parent(Parent) {}

  /// \returns the 1-to-1 source line number.
  unsigned getLine() const { return Line; }
  /// \returns the begin label.
  MCSymbol *getBegin() const { return Begin; }
  /// \returns the parent routine.
  TraceRoutine *getParent() const { return Parent; }
  /// Set the parent of this line.
  void setParent(TraceRoutine *Val) { Parent = Val; }

  /// \returns the delta line between this line node and the previous one.
  int getDeltaLine() const;
  /// \returns the delta PC between the end PC and the start PC of the line
  /// node, then minus one.
  const MCExpr *getDeltaPCMinusOne(MCContext &Context) const;

  /// Emit this line.
  void emit(MCStreamer &OS) const;

  static bool classof(const TraceDINode *Node) {
    return Node->getTag() == traceback::TB_TAG_LN1 ||
           Node->getTag() == traceback::TB_TAG_LN2 ||
           Node->getTag() == traceback::TB_TAG_LN4;
  }
};

/// A debug information node that may have children, it is used to reduce the
/// duplicate code for TraceRoutine, TraceFile and TraceModule.
template <typename ChildTy> class TraceDINodeWithChildren : public TraceDINode {
protected:
  using ChildListType = iplist<ChildTy>;
  using const_iterator = typename ChildListType::const_iterator;
  using iterator = typename ChildListType::iterator;
  using const_reverse_iterator = typename ChildListType::const_reverse_iterator;
  using reverse_iterator = typename ChildListType::reverse_iterator;

private:
  std::string Name;

protected:
  /// Children nodes.
  ChildListType Children;

  TraceDINodeWithChildren(traceback::Tag Tag, const std::string &Name)
      : TraceDINode(Tag), Name(Name) {}

public:
  /// Traverse the children and emit them.
  void emitChildren(MCStreamer &OS) const {
    for (const ChildTy &Child : Children)
      Child.emit(OS);
  }

  const std::string &getName() const { return Name; }

  /// Support for ChildTy::getNextNode(), getPrevNode().
  static ChildListType TraceDINodeWithChildren::*getSublistAccess(ChildTy *) {
    return &TraceDINodeWithChildren<ChildTy>::Children;
  }

  /// Child forwarding functions
  iterator begin() { return Children.begin(); }
  const_iterator begin() const { return Children.begin(); }
  iterator end() { return Children.end(); }
  const_iterator end() const { return Children.end(); }
  bool empty() const { return Children.empty(); }
  const ChildTy &front() const { return Children.front(); }
  ChildTy &front() { return Children.front(); }
  const ChildTy &back() const { return Children.back(); }
  ChildTy &back() { return Children.back(); }
  void pop_back() { Children.pop_back(); }
};

/// A data structure used to maintain the debug information for a routine
/// record, including the tag and attribute for the 32/64-bit routine.
class TraceRoutine : public TraceDINodeWithChildren<TraceLine>,
                     public ilist_node_with_parent<TraceRoutine, TraceFile> {
private:
  /// Begin line.
  unsigned Line;
  /// Routine begin label.
  MCSymbol *Begin;
  /// Routine end label.
  MCSymbol *End = nullptr;
  /// Parent file.
  TraceFile *Parent;

  /// Utility to determine the routine tag according to the pointer size of the
  /// target.
  static traceback::Tag determineRoutineTag(unsigned PointerSize) {
    return PointerSize == 4 ? traceback::TB_TAG_RTN32 : traceback::TB_TAG_RTN64;
  }

public:
  TraceRoutine(unsigned PointerSize, const std::string &Name, unsigned Line,
               MCSymbol *Begin, TraceFile *Parent = nullptr)
      : TraceDINodeWithChildren<TraceLine>(determineRoutineTag(PointerSize),
                                           Name),
        Line(Line), Begin(Begin), Parent(Parent) {}

  /// \returns the begin line.
  unsigned getLine() const { return Line; }
  /// \returns the begin label.
  MCSymbol *getBegin() const { return Begin; }
  /// \returns the end label.
  MCSymbol *getEnd() const { return End; }
  /// Set the end label.
  void setEnd(MCSymbol *Sym) { End = Sym; }
  /// \returns the parent file.
  TraceFile *getParent() const { return Parent; }
  /// Set the parent file.
  void setParent(TraceFile *Val) { Parent = Val; }

  /// Add a child line \p Child.
  void push_back(TraceLine *Child) {
    Children.push_back(Child);
    Child->setParent(this);
  }

  /// Emit this routine.
  void emit(MCStreamer &OS) const;

  static bool classof(const TraceDINode *Node) {
    return Node->getTag() == traceback::TB_TAG_RTN32 ||
           Node->getTag() == traceback::TB_TAG_RTN64;
  }
};

/// A data structure used to maintain the debug information for a file record.
class TraceFile : public TraceDINodeWithChildren<TraceRoutine>,
                  public ilist_node_with_parent<TraceFile, TraceModule> {
private:
  /// The zero-based index of this file.
  unsigned Index;
  /// Parent module.
  TraceModule *Parent;

public:
  TraceFile(const std::string &Name, unsigned Index,
            TraceModule *Parent = nullptr)
      : TraceDINodeWithChildren(traceback::TB_TAG_File, Name), Index(Index),
        Parent(Parent) {}

  /// \returns the index.
  unsigned getIndex() const { return Index; }
  /// Set the index.
  void setIndex(unsigned Val) { Index = Val; }

  /// \returns the parent module.
  TraceModule *getParent() const { return Parent; }
  /// Set the parent module.
  void setParent(TraceModule *Val) { Parent = Val; }

  /// Add a child routine \p Child.
  void push_back(TraceRoutine *Child) {
    Children.push_back(Child);
    Child->setParent(this);
  }

  /// Emit this file.
  void emit(MCStreamer &OS) const;

  static bool classof(const TraceDINode *Node) {
    return Node->getTag() == traceback::TB_TAG_File;
  }
};

/// Represents a module in .trace section.
/// In this module, there may be multiple TraceFile corrsponding to the same
/// source file, and the file/routine without any routines/lines will be removed
/// at appropriate time.
class TraceModule : public TraceDINodeWithChildren<TraceFile>,
                    public ilist_node<TraceModule> {
private:
  /// The pointer size of the target machine.
  unsigned PointerSize;
  /// The format version of the trace module.
  unsigned Version;

  /// Map index to TraceFile.
  DenseMap<unsigned, const TraceFile *> IndexToFile;
  /// Is this module already ended?
  bool IsEnded = false;

private:
  /// \returns the major version.
  unsigned getMajorID() const { return Version / 100; }
  /// \returns the minor version.
  unsigned getMinorID() const { return Version % 100; }

  /// Add a child file \p Child.
  void push_back(TraceFile *Child) {
    Children.push_back(Child);
    Child->setParent(this);
  }

  /// \returns the last file in this module.
  TraceFile *getLastFile();
  const TraceFile *getLastFile() const;
  /// \returns the last routine in the last file.
  TraceRoutine *getLastRoutine();
  const TraceRoutine *getLastRoutine() const;
  /// \returns the last line in the last routine.
  TraceLine *getLastLine();
  const TraceLine *getLastLine() const;

  /// \returns the last line in this module if exists, otherwise returns 0.
  unsigned getLastLineNoInModuleOrZero() const;

  /// Clear the file that doesn't have any routine.
  void removeEmptyFile();

public:
  // Module name(always empty) and format version (always 2.00) in ICC's
  // current implementation. Unlike DWARF, we only want to support the latest
  // version of traceback. So the version info is used to debug only for
  // developers.
  TraceModule(unsigned PointerSize, unsigned Version = 200,
              const std::string &Name = "");

  /// Add a new file, the indices of two file should be same if and only if
  /// they corresponds to the same source/header file. The indices don't need to
  /// be continuous or start from 0.
  /// \param Name filename.
  /// \param Index index of file.
  void addFile(const std::string &Name, unsigned Index);
  /// Add a new routine.
  /// \param Name routine name.
  /// \param Line start line of the routine.
  /// \param Begin begin label of the routine.
  void addRoutine(const std::string &Name, unsigned Line, MCSymbol *Begin);
  /// Add a new line.
  /// \param Line line number in the file.
  /// \param Begin begin label of the line.
  void addLine(unsigned Line, MCSymbol *Begin);

  /// \returns the line number of the last line record in the last file.
  std::optional<unsigned> getLastLineNo() const;

  /// \returns true if the last routine is empty.
  bool isLastRoutineEmpty() const;

  /// Hint the current routine ends with label \p End.
  void endRoutine(MCSymbol *End);

  /// End the module, the module is immutable indeed after being ended.
  void endModule();

  /// Emit this module.
  void emit(MCStreamer &OS) const;

  static bool classof(const TraceDINode *Node) {
    return Node->getTag() == traceback::TB_TAG_Module;
  }
};

} // namespace llvm

#endif // LLVM_DEBUGINFO_TRACEBACK_TRACEDINODE_H
