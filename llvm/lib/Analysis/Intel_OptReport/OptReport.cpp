//===--- OptReport.cpp ------------------------------------------*- C++ -*-==//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
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
/// If an element was found, returns its index. Otherwise, returns nullopt.
static std::optional<unsigned> findNamedTupleField(const MDTuple *Dict,
                                                   StringRef Key) {
  const unsigned NumOps = Dict->getNumOperands();
  for (unsigned Idx = 0; Idx < NumOps; ++Idx) {
    if (MDTuple *T = dyn_cast<MDTuple>(Dict->getOperand(Idx)))
      if (T->getNumOperands() > 0)
        if (MDString *S = dyn_cast<MDString>(T->getOperand(0)))
          if (S->getString() == Key)
            return Idx;
  }
  return std::nullopt;
}

/// Erase field tagged with \p Key from \p OptReport.
static void removeOptReportField(MDTuple *OptReport, StringRef Key) {
  assert(OptReport::isOptReportMetadata(OptReport) && "Bad OptReport");

  // Look for the key; bail if it isn't found.
  const std::optional<unsigned> IdxToDelete =
      findNamedTupleField(OptReport, Key);
  if (!IdxToDelete)
    return;

  // If it is found, delete it by shifting all later operands and removing the
  // last one.
  for (unsigned Idx = *IdxToDelete; Idx + 1 < OptReport->getNumOperands();
       ++Idx)
    OptReport->replaceOperandWith(Idx, OptReport->getOperand(Idx + 1));
  OptReport->pop_back();
}

/// Add a new Key-Value pair (tuple) to OptReport. It is an error to add a key
/// that is already present in the OptReport.
static void addOptReportSingleValue(MDTuple *OptReport, StringRef Key,
                                    Metadata *Value) {
  assert(OptReport::isOptReportMetadata(OptReport) && "Bad OptReport");

  LLVMContext &Context = OptReport->getContext();
  MDString *const MDKey = MDString::get(Context, Key);
  MDTuple *const KeyValue = MDTuple::get(Context, {MDKey, Value});
  assert(!findNamedTupleField(OptReport, Key) &&
         "A field with the same key already exists in OptReport");
  OptReport->push_back(KeyValue);
}

/// Associate a new \p Value with \p Key in \p OptReport. Multiple values can be
/// associated with a single key. The key and the values are stored in a single
/// MDTuple, where first entry is the key, and the rest are values.
static void addOptReportMultiValue(MDTuple *OptReport, StringRef Key,
                                   Metadata *Value) {
  assert(OptReport::isOptReportMetadata(OptReport) && "Bad OptReport");

  const std::optional<unsigned> Idx = findNamedTupleField(OptReport, Key);
  if (!Idx)
    return addOptReportSingleValue(OptReport, Key, Value);

  LLVMContext &Context = OptReport->getContext();
  const MDTuple *const OldTuple = cast<MDTuple>(OptReport->getOperand(*Idx));
  SmallVector<Metadata *, 4> Ops(OldTuple->op_begin(), OldTuple->op_end());
  Ops.push_back(Value);
  MDTuple *const NewTuple = MDTuple::get(Context, Ops);

  assert(OptReport->isDistinct() &&
         "Operands can be safely replaced only in distinct metadata");
  OptReport->replaceOperandWith(*Idx, NewTuple);
}

/// Look for a value associated with \p Key in \p OptReport. It is expected that
/// the value was previously pushed into OptReport with addOptReportSingleValue
/// function.
static Metadata *findOptReportSingleValue(const MDTuple *OptReport,
                                          StringRef Key) {
  assert(OptReport::isOptReportMetadata(OptReport) && "Bad OptReport");

  const std::optional<unsigned> Idx = findNamedTupleField(OptReport, Key);
  if (!Idx)
    return nullptr;
  const MDTuple *const OriginTuple = cast<MDTuple>(OptReport->getOperand(*Idx));
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

  const std::optional<unsigned> Idx = findNamedTupleField(OptReport, Key);
  if (!Idx)
    return Empty;

  const MDTuple *const OriginTuple = cast<MDTuple>(OptReport->getOperand(*Idx));
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
  return MDTuple::getDistinct(Context,
                              MDString::get(Context, OptReportTag::Report));
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

OptRemarkID OptRemark::getRemarkID() const {
  assert(getNumOperands() > 0 && "No remark ID present!");
  // Avoid crash on release build.
  if (getNumOperands() == 0)
    return OptRemarkID::InvalidRemarkID;
  const Metadata *MD = getOperand(0);
  const ConstantAsMetadata *ConstMD = cast_or_null<ConstantAsMetadata>(MD);
  assert(ConstMD && "Remark ID is not a constant!");
  // Avoid crash on release build.
  uint64_t Val =
      ConstMD ? cast<ConstantInt>(ConstMD->getValue())->getZExtValue() : 0;
  return static_cast<OptRemarkID>(Val);
}
