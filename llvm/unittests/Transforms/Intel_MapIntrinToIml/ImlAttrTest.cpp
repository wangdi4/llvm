//===- ImlAttr.cpp - Tests for libiml_attr ----------------------*- C++ -*-===//
//
//   Copyright (C) 2021 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/StringRef.h"
#include "llvm/Transforms/Intel_MapIntrinToIml/iml_accuracy_interface.h"
#include "gtest/gtest.h"

using namespace llvm;

namespace {

class ImlAttrTest : public testing::Test {};

static ImfAttr *createImfAttr(const char *Name, const char *Value,
                              ImfAttr *Next) {
  ImfAttr *Attr = new ImfAttr();
  Attr->name = Name;
  Attr->value = Value;
  Attr->next = Next;
  return Attr;
}

class ImfAttrListWrapper {
  ImfAttr *List;

public:
  explicit ImfAttrListWrapper(ImfAttr *List) : List(List) {}
  ImfAttrListWrapper(const ImfAttrListWrapper &) = delete;
  ImfAttrListWrapper(ImfAttrListWrapper &&Other) {
    List = Other.List;
    Other.List = nullptr;
  }
  ~ImfAttrListWrapper() {
    ImfAttr *Node = List;
    while (Node) {
      ImfAttr *Next = Node->next;
      delete Node;
      Node = Next;
    }
  }
  ImfAttr *getImfAttrList() { return List; }
};

enum PrecisionEnum {
  Low,    // ep
  Medium, // la
  High,   // ha
};

const char *precisionEnumToString(PrecisionEnum Value) {
  switch (Value) {
  case Low:
    return "low";
  case Medium:
    return "medium";
  case High:
    return "high";
  }
  llvm_unreachable("Fully covered switch");
}

struct ImfFuncInfo {
  PrecisionEnum Precision = High;
  bool FuSa = false;

  ImfFuncInfo setPrecision(PrecisionEnum Value) const {
    ImfFuncInfo Ret(*this);
    Ret.Precision = Value;
    return Ret;
  }

  ImfFuncInfo setFuSa(bool Value) const {
    ImfFuncInfo Ret(*this);
    Ret.FuSa = Value;
    return Ret;
  }

  ImfAttrListWrapper createImfAttrList() const {
    ImfAttr *Tail = nullptr;

    Tail = createImfAttr("precision", precisionEnumToString(Precision), Tail);

    if (FuSa)
      Tail = createImfAttr("fusa", "true", Tail);

    return std::move(ImfAttrListWrapper(Tail));
  }
};

static StringRef getLibraryFunctionName(const char *BaseName,
                                        const ImfFuncInfo &Attrs) {
  return get_library_function_name(BaseName,
                                   Attrs.createImfAttrList().getImfAttrList());
}

TEST_F(ImlAttrTest, FuSaTest) {
  ImfFuncInfo InfoNoFuSa;

  ImfFuncInfo InfoFuSa = InfoNoFuSa.setFuSa(true);

  EXPECT_EQ(getLibraryFunctionName("fmaxf", InfoNoFuSa), "fmaxf");
  EXPECT_EQ(getLibraryFunctionName("logf", InfoNoFuSa), "logf");
  EXPECT_EQ(getLibraryFunctionName("fmaxf", InfoFuSa), "");
  EXPECT_EQ(getLibraryFunctionName("logf", InfoFuSa), "logf");
}

} // namespace
