//===--- LoopOptReport.cpp ---------------------------------------*- C++ -*-==//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements LoopOptReport class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_OptReport/LoopOptReport.h"

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
  assert(LoopOptReport::isOptReportMetadata(OptReport) && "Bad OptReport");

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

  LLVMContext &Context = OptReport->getContext();
  MDTuple *NewImpl = MDTuple::get(Context, Ops);

  assert(OptReport->isDistinct() &&
         "Operands can be safely replaced only in distinct metadata");
  OptReport->replaceOperandWith(1, NewImpl);
}

/// Add a new Key-Value pair (tuple) to OptReport. It is an error to add a key
/// that is already present in the OptReport.
static void addOptReportSingleValue(MDTuple *OptReport, StringRef Key,
                                    Metadata *Value) {
  assert(LoopOptReport::isOptReportMetadata(OptReport) && "Bad OptReport");

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
  assert(LoopOptReport::isOptReportMetadata(OptReport) && "Bad OptReport");

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
  assert(LoopOptReport::isOptReportMetadata(OptReport) && "Bad OptReport");

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
static LoopOptReport::op_range findOptReportMultiValue(const MDTuple *OptReport,
                                                       StringRef Key) {
  assert(LoopOptReport::isOptReportMetadata(OptReport) && "Bad OptReport");

  using op_iterator = LoopOptReport::op_iterator;
  using op_range = LoopOptReport::op_range;
  op_range Empty{op_iterator(), op_iterator()};

  MDTuple *OptReportImpl = cast<MDTuple>(OptReport->getOperand(1));
  int Idx = findNamedTupleField(OptReportImpl, Key);
  if (Idx < 0)
    return Empty;

  MDTuple *OriginTuple = cast<MDTuple>(OptReportImpl->getOperand(Idx));
  assert(OriginTuple->getNumOperands() > 1 && "Field without values");
  return op_range(std::next(OriginTuple->op_begin()), OriginTuple->op_end());
}

LoopOptReport LoopOptReport::findOptReportInLoopID(MDNode *LoopID) {
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

MDNode *LoopOptReport::eraseOptReportFromLoopID(MDNode *LoopID,
                                                LLVMContext &C) {
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

MDNode *LoopOptReport::addOptReportToLoopID(MDNode *LoopID, LoopOptReport OR,
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

LoopOptReport LoopOptReport::createEmptyOptReport(LLVMContext &Context) {
  MDString *ProxyTitle = MDString::get(Context, LoopOptReportTag::Proxy);
  MDString *RootTitle = MDString::get(Context, LoopOptReportTag::Root);

  // No need for distinct !{!"intel.loop.optreport"}, as no replaceOperandWith
  // will be ever called on it.
  MDTuple *Empty = MDTuple::get(Context, {ProxyTitle});
  // Root node must be distinct, as we are going to replace its second operand.
  MDTuple *Root = MDTuple::getDistinct(Context, {RootTitle, Empty});
  return Root;
}

void LoopOptReport::addOrigin(LoopOptRemark Origin) const {
  assert(Origin && "Null Origin");
  addOptReportMultiValue(OptReport, LoopOptReportTag::Origin, Origin.get());
}

void LoopOptReport::setDebugLoc(DILocation *Location) const {
  assert(Location && "Null Location");
  addOptReportSingleValue(OptReport, LoopOptReportTag::DebugLoc, Location);
}

void LoopOptReport::addRemark(LoopOptRemark Remark) const {
  assert(Remark && "Null Remark");
  addOptReportMultiValue(OptReport, LoopOptReportTag::Remarks, Remark.get());
}

void LoopOptReport::addChild(LoopOptReport Child) const {
  assert(Child && "Null Child");
  if (LoopOptReport Next = firstChild()) {
    Next.addSibling(Child);
    return;
  }
  addOptReportSingleValue(OptReport, LoopOptReportTag::FirstChild, Child.get());
}

void LoopOptReport::addSibling(LoopOptReport Sibling) const {
  assert(Sibling && "Null Sibling");
  if (LoopOptReport Next = nextSibling()) {
    Next.addSibling(Sibling);
    return;
  }
  addOptReportSingleValue(OptReport, LoopOptReportTag::NextSibling,
                          Sibling.get());
}

void LoopOptReport::eraseSiblings() const {
  removeOptReportField(OptReport, LoopOptReportTag::NextSibling);
}

LoopOptReport::op_range LoopOptReport::origin() const {
  return findOptReportMultiValue(OptReport, LoopOptReportTag::Origin);
}

const DILocation *LoopOptReport::debugLoc() const {
  const Metadata *MDVal =
      findOptReportSingleValue(OptReport, LoopOptReportTag::DebugLoc);
  return cast_or_null<DILocation>(MDVal);
}

LoopOptReport::op_range LoopOptReport::remarks() const {
  return findOptReportMultiValue(OptReport, LoopOptReportTag::Remarks);
}

const LoopOptReport LoopOptReport::nextSibling() const {
  Metadata *MDVal =
      findOptReportSingleValue(OptReport, LoopOptReportTag::NextSibling);
  return cast_or_null<MDTuple>(MDVal);
}

const LoopOptReport LoopOptReport::firstChild() const {
  Metadata *MDVal =
      findOptReportSingleValue(OptReport, LoopOptReportTag::FirstChild);
  return cast_or_null<MDTuple>(MDVal);
}

bool LoopOptRemark::isOptRemarkMetadata(const Metadata *R) {
  const MDTuple *T = dyn_cast<MDTuple>(R);
  if (!T)
    return false;

  if (T->getNumOperands() == 0)
    return false;

  const MDString *S = dyn_cast<MDString>(T->getOperand(0));
  if (!S)
    return false;

  return S->getString() == LoopOptReportTag::Remark;
}

const Metadata *LoopOptRemark::getOperand(unsigned Idx) const {
  // The remark tag (argument #0) is skipped.
  unsigned NewIdx = Idx + 1;
  return Remark->getOperand(NewIdx);
}

unsigned LoopOptRemark::getNumOperands() const {
  return Remark->getNumOperands() - 1;
}
