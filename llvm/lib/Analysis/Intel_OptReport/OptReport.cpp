//===--- OptReport.cpp ------------------------------------------*- C++ -*-==//
//
// Copyright (C) 2017-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements OptReport class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_OptReport/OptReport.h"

#include "llvm/IR/DebugInfoMetadata.h"

using namespace llvm;

/// \brief Find an element tagged with \p Key in metadata dictionary \p Dict.
///
/// Metadata dictionary is a tuple of tagged tuples. A tagged tuple is a tuple
/// whose first element is MDString, which is considered to be the tag.
/// If an element was found, returns its index. Otherwise, returns -1.
static int findNamedTupleField(MDTuple *Dict, StringRef Key) {
  int NumOps = Dict->getNumOperands();
  for (int i = 0; i < NumOps; ++i) {
    if (MDTuple *T = dyn_cast<MDTuple>(Dict->getOperand(i)))
      if (T->getNumOperands() > 0)
        if (MDString *S = dyn_cast<MDString>(T->getOperand(0)))
          if (S->getString() == Key)
            return i;
  }
  return -1;
}

/// Erase field tagged with \p Key from \p OptReport.
static void removeOptReportField(MDTuple *OptReport, StringRef Key) {
  assert(OptReport::isOptReportMetadata(OptReport) && "Bad OptReport");

  MDTuple *OptReportImpl = cast<MDTuple>(OptReport->getOperand(1));
  SmallVector<Metadata *, 4> Ops;
  auto It = OptReportImpl->op_begin();
  // Always store first Tag of Opt Report into new Operands.
  Ops.push_back(*It++);
  std::copy_if(It, OptReportImpl->op_end(), std::back_inserter(Ops),
               [Key](Metadata *M) {
                 Metadata *Name = cast<MDTuple>(M)->getOperand(0);
                 return cast<MDString>(Name)->getString() != Key;
               });

  // No need to modify OptReport if there's nothing to be removed (no \p Key
  // field in the report).
  if (OptReportImpl->getNumOperands() == Ops.size())
    return;

  // Empty impl node doesn't need to be distinct (nothing to be modified
  // in-place with replaceOperandWith).
  LLVMContext &Context = OptReport->getContext();
  MDTuple *NewImpl = (Ops.size() > 1) ? MDTuple::getDistinct(Context, Ops)
                                      : MDTuple::get(Context, Ops);

  assert(OptReport->isDistinct() &&
         "Operands can be safely replaced only in distinct metadata");
  OptReport->replaceOperandWith(1, NewImpl);
}

/// Add a new Key-Value pair (tuple) to OptReport. It is an error to add a key
/// that is already present in the OptReport.
static void addOptReportSingleValue(MDTuple *OptReport, StringRef Key,
                                    Metadata *Value) {
  assert(OptReport::isOptReportMetadata(OptReport) && "Bad OptReport");

  LLVMContext &Context = OptReport->getContext();
  MDString *MDKey = MDString::get(Context, Key);
  MDTuple *KeyValue = MDTuple::get(Context, {MDKey, Value});
  MDTuple *OptReportImpl = cast<MDTuple>(OptReport->getOperand(1));
  assert(findNamedTupleField(OptReportImpl, Key) == -1 &&
         "A field with the same key already exists in OptReport");
  SmallVector<Metadata *, 4> Ops(OptReportImpl->op_begin(),
                                 OptReportImpl->op_end());
  Ops.push_back(KeyValue);
  // Non-empty impl node must be distinct, as some of its data may be replaced
  // (e.g. remarks tuple after adding a new remark).
  MDTuple *NewImpl = MDTuple::getDistinct(Context, Ops);

  assert(OptReport->isDistinct() &&
         "Operands can be safely replaced only in distinct metadata");
  OptReport->replaceOperandWith(1, NewImpl);
}

/// Associate a new \p Value with \p Key in \p OptReport. Multiple values can be
/// associated with a single key. The key and the values are stored in a single
/// MDTuple, where first entry is the key, and the rest are values.
static void addOptReportMultiValue(MDTuple *OptReport, StringRef Key,
                                   Metadata *Value) {
  assert(OptReport::isOptReportMetadata(OptReport) && "Bad OptReport");

  MDTuple *OptReportImpl = cast<MDTuple>(OptReport->getOperand(1));
  int Idx = findNamedTupleField(OptReportImpl, Key);
  if (Idx < 0) {
    addOptReportSingleValue(OptReport, Key, Value);
    return;
  }

  LLVMContext &Context = OptReport->getContext();
  MDTuple *OldTuple = cast<MDTuple>(OptReportImpl->getOperand(Idx));
  SmallVector<Metadata *, 4> Ops(OldTuple->op_begin(), OldTuple->op_end());
  Ops.push_back(Value);
  MDTuple *NewTuple = MDTuple::get(Context, Ops);

  assert(OptReportImpl->isDistinct() &&
         "Operands can be safely replaced only in distinct metadata");
  OptReportImpl->replaceOperandWith(Idx, NewTuple);
}

/// Look for a value associated with \p Key in \p OptReport. It is expected that
/// the value was previously pushed into OptReport with addOptReportSingleValue
/// function.
static Metadata *findOptReportSingleValue(const MDTuple *OptReport,
                                          StringRef Key) {
  assert(OptReport::isOptReportMetadata(OptReport) && "Bad OptReport");

  MDTuple *OptReportImpl = cast<MDTuple>(OptReport->getOperand(1));
  int Idx = findNamedTupleField(OptReportImpl, Key);
  if (Idx < 0)
    return nullptr;
  MDTuple *OriginTuple = cast<MDTuple>(OptReportImpl->getOperand(Idx));
  assert(OriginTuple->getNumOperands() == 2 && "Unsupported field format");
  return cast<Metadata>(OriginTuple->getOperand(1));
}

/// Look for values associated with \p Key in \p OptReport. It is expected that
/// the values were previously pushed into OptReport with addOptReportMultiValue
/// function. Range (possibly empty) of found values is returned.
static OptReport::op_range findOptReportMultiValue(const MDTuple *OptReport,
                                                   StringRef Key) {
  assert(OptReport::isOptReportMetadata(OptReport) && "Bad OptReport");

  using op_iterator = OptReport::op_iterator;
  using op_range = OptReport::op_range;
  op_range Empty{op_iterator(), op_iterator()};

  MDTuple *OptReportImpl = cast<MDTuple>(OptReport->getOperand(1));
  int Idx = findNamedTupleField(OptReportImpl, Key);
  if (Idx < 0)
    return Empty;

  MDTuple *OriginTuple = cast<MDTuple>(OptReportImpl->getOperand(Idx));
  assert(OriginTuple->getNumOperands() > 1 && "Field without values");
  return op_range(std::next(OriginTuple->op_begin()), OriginTuple->op_end());
}

OptReport OptReport::findOptReportInLoopID(MDNode *LoopID) {
  if (!LoopID)
    return nullptr;

  assert(!isOptReportMetadata(LoopID->getOperand(0)) && "Bad LoopID metadata");

  for (int I = 1, IE = LoopID->getNumOperands(); I < IE; ++I) {
    Metadata *MD = LoopID->getOperand(I);
    if (isOptReportMetadata(MD))
      return cast<MDTuple>(MD);
  }

  return nullptr;
}

MDNode *OptReport::eraseOptReportFromLoopID(MDNode *LoopID, LLVMContext &C) {
  if (!LoopID)
    return nullptr;

  SmallVector<Metadata *, 4> Ops;
  Ops.push_back(nullptr);
  std::copy_if(std::next(LoopID->op_begin()), LoopID->op_end(),
               std::back_inserter(Ops),
               [](Metadata *M) { return !isOptReportMetadata(M); });
  if (Ops.size() == 1) {
    return nullptr;
  }

  MDTuple *NewLoopID = MDTuple::get(C, Ops);
  NewLoopID->replaceOperandWith(0, NewLoopID);
  return NewLoopID;
}

MDNode *OptReport::addOptReportToLoopID(MDNode *LoopID, OptReport OR,
                                        LLVMContext &C) {
  assert(!findOptReportInLoopID(LoopID) && "The loop already has OptReport");
  assert(OR && "Null OptReport");

  SmallVector<Metadata *, 4> Ops;
  Ops.push_back(nullptr);
  if (LoopID)
    std::copy(std::next(LoopID->op_begin()), LoopID->op_end(),
              std::back_inserter(Ops));
  Ops.push_back(OR.get());
  MDTuple *NewLoopID = MDTuple::get(C, Ops);
  NewLoopID->replaceOperandWith(0, NewLoopID);

  return NewLoopID;
}

OptReport OptReport::createEmptyOptReport(LLVMContext &Context) {
  MDString *ProxyTitle = MDString::get(Context, OptReportTag::Proxy);
  MDString *RootTitle = MDString::get(Context, OptReportTag::Root);

  // No need for distinct !{!"intel.optreport"}, as no replaceOperandWith
  // will be ever called on it.
  MDTuple *Empty = MDTuple::get(Context, {ProxyTitle});
  // Root node must be distinct, as we are going to replace its second operand.
  MDTuple *Root = MDTuple::getDistinct(Context, {RootTitle, Empty});
  return Root;
}

void OptReport::addOrigin(OptRemark Origin) const {
  assert(Origin && "Null Origin");
  addOptReportMultiValue(OptReportMD, OptReportTag::Origin, Origin.get());
}

void OptReport::setTitle(StringRef Title) const {
  assert(!Title.empty() && "Empty Title");
  if (Title != "LOOP") {
    MDString *MDTitle = MDString::get(OptReportMD->getContext(), Title);
    addOptReportSingleValue(OptReportMD, OptReportTag::Title, MDTitle);
  }
}

void OptReport::setDebugLoc(DILocation *Location) const {
  assert(Location && "Null Location");
  addOptReportSingleValue(OptReportMD, OptReportTag::DebugLoc, Location);
}

void OptReport::addRemark(OptRemark Remark) const {
  assert(Remark && "Null Remark");
  addOptReportMultiValue(OptReportMD, OptReportTag::Remarks, Remark.get());
}

void OptReport::addChild(OptReport Child) const {
  assert(Child && "Null Child");
  if (Child.OptReportMD == this->OptReportMD)
    report_fatal_error("Found a parent/child cycle when generating opt-report. "
                       "Proceeding will cause an infinite loop.",
                       false /*no crash diag*/);

  if (OptReport Next = firstChild()) {
    Next.addSibling(Child);
    return;
  }
  addOptReportSingleValue(OptReportMD, OptReportTag::FirstChild, Child.get());
}

void OptReport::addSibling(OptReport Sibling) const {
  assert(Sibling && "Null Sibling");
  OptReport Current = *this;
  if (Current.OptReportMD == Sibling.OptReportMD) {
    llvm_unreachable("Duplicate nodes in optreport list");
    // out of spec, but still possible to continue compilation
    return;
  }
  while (Current.nextSibling()) {
    Current = Current.nextSibling();
    if (Current.OptReportMD == Sibling.OptReportMD) {
      llvm_unreachable("Duplicate nodes in optreport list");
      return;
    }
  }
  addOptReportSingleValue(Current.OptReportMD, OptReportTag::NextSibling,
                          Sibling.get());
}

void OptReport::eraseSiblings() const {
  removeOptReportField(OptReportMD, OptReportTag::NextSibling);
}

OptReport::op_range OptReport::origin() const {
  return findOptReportMultiValue(OptReportMD, OptReportTag::Origin);
}

const DILocation *OptReport::debugLoc() const {
  const Metadata *MDVal =
      findOptReportSingleValue(OptReportMD, OptReportTag::DebugLoc);
  return cast_or_null<DILocation>(MDVal);
}

StringRef OptReport::title() const {
  if (OptReportMD) {
    const Metadata *MDVal =
        findOptReportSingleValue(OptReportMD, OptReportTag::Title);
    if (auto *MDTitle = cast_or_null<MDString>(MDVal))
      return MDTitle->getString();
  }
  // Default title is "LOOP".
  return "LOOP";
}

OptReport::op_range OptReport::remarks() const {
  return findOptReportMultiValue(OptReportMD, OptReportTag::Remarks);
}

const OptReport OptReport::nextSibling() const {
  Metadata *MDVal =
      findOptReportSingleValue(OptReportMD, OptReportTag::NextSibling);
  return cast_or_null<MDTuple>(MDVal);
}

const OptReport OptReport::firstChild() const {
  Metadata *MDVal =
      findOptReportSingleValue(OptReportMD, OptReportTag::FirstChild);
  return cast_or_null<MDTuple>(MDVal);
}

OptReport OptReport::copy() const {
  if (!OptReportMD)
    return nullptr;

  // Create an empty OptReport objects and fill in all fields.
  OptReport Copy = OptReport::createEmptyOptReport(OptReportMD->getContext());
  for (auto &Origin : origin())
    Copy.addOrigin(Origin);
  StringRef Title = title();
  if (!Title.empty())
    Copy.setTitle(title());
  if (const DILocation *DL = debugLoc())
    Copy.setDebugLoc(const_cast<DILocation *>(DL));
  for (auto &Remark : remarks())
    Copy.addRemark(Remark);

  // Then replicate all children.
  for (OptReport Child = firstChild(); Child; Child = Child.nextSibling())
    Copy.addChild(Child.copy());
  return Copy;
}

bool OptRemark::isOptRemarkMetadata(const Metadata *R) {
  const MDTuple *T = dyn_cast<MDTuple>(R);
  if (!T)
    return false;

  if (T->getNumOperands() == 0)
    return false;

  const MDString *S = dyn_cast<MDString>(T->getOperand(0));
  if (!S)
    return false;

  return S->getString() == OptReportTag::Remark;
}

const Metadata *OptRemark::getOperand(unsigned Idx) const {
  // The remark tag (argument #0) is skipped.
  unsigned NewIdx = Idx + 1;
  return Remark->getOperand(NewIdx);
}

unsigned OptRemark::getNumOperands() const {
  return Remark->getNumOperands() - 1;
}
