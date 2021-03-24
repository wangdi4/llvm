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
  Low,       // ep
  Medium,    // la
  High,      // ha
  Reference, // rf
};

const char *precisionEnumToString(PrecisionEnum Value) {
  switch (Value) {
  case Low:
    return "low";
  case Medium:
    return "medium";
  case High:
    return "high";
  case Reference:
    return "reference";
  }
  llvm_unreachable("Fully covered switch");
}

struct ImfFuncInfo {
  PrecisionEnum Precision = High;
  bool FuSa = false;
  const char *ISASet = nullptr;

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

  ImfFuncInfo setISASet(const char *Value) const {
    ImfFuncInfo Ret(*this);
    Ret.ISASet = Value;
    return Ret;
  }

  ImfAttrListWrapper createImfAttrList() const {
    ImfAttr *Tail = nullptr;

    Tail = createImfAttr("precision", precisionEnumToString(Precision), Tail);

    if (FuSa)
      Tail = createImfAttr("fusa", "true", Tail);
    if (ISASet)
      Tail = createImfAttr("isa-set", ISASet, Tail);

    return ImfAttrListWrapper(Tail);
  }
};

/// Wrapper for libiml_attr's get_library_function_name. Search for a variant of
/// function specified in \p BaseName with constraints \p Attrs for a given \p
/// OS (default Linux). Returns the function name for IA-32 and Intel 64
/// architectures in a pair.
static std::pair<StringRef, StringRef>
getLibraryFunctionName(const char *BaseName, const ImfFuncInfo &Attrs,
                       llvm::Triple::OSType OS = llvm::Triple::Linux) {
  return std::make_pair(
      get_library_function_name(BaseName,
                                Attrs.createImfAttrList().getImfAttrList(),
                                llvm::Triple::x86, OS),
      get_library_function_name(BaseName,
                                Attrs.createImfAttrList().getImfAttrList(),
                                llvm::Triple::x86_64, OS));
}

// Helper function to build a pair of function names to compare with the pair
// returned by getLibraryFunctionName in tests. This is needed to force the
// compiler to build a pair of StringRefs, instead of const char *.
static std::pair<StringRef, StringRef> makeFuncNamePair(StringRef IA32Name,
                                                        StringRef Intel64Name) {
  return std::make_pair(IA32Name, Intel64Name);
}

TEST_F(ImlAttrTest, FuSaTest) {
  ImfFuncInfo InfoNoFuSa;

  ImfFuncInfo InfoFuSa = InfoNoFuSa.setFuSa(true);

  EXPECT_EQ(getLibraryFunctionName("fmaxf", InfoNoFuSa),
            makeFuncNamePair("fmaxf", "fmaxf"));
  EXPECT_EQ(getLibraryFunctionName("logf", InfoNoFuSa),
            makeFuncNamePair("logf", "logf"));
  EXPECT_EQ(getLibraryFunctionName("fmaxf", InfoFuSa, llvm::Triple::Linux),
            makeFuncNamePair("fmaxf", "__libm_fmaxf_ex"));
  EXPECT_EQ(getLibraryFunctionName("logf", InfoFuSa, llvm::Triple::Linux),
            makeFuncNamePair("logf", "__libm_logf_ex"));
  EXPECT_EQ(getLibraryFunctionName("fmaxf", InfoFuSa, llvm::Triple::Win32),
            makeFuncNamePair("fmaxf", "fmaxf"));
  EXPECT_EQ(getLibraryFunctionName("logf", InfoFuSa, llvm::Triple::Win32),
            makeFuncNamePair("logf", "logf"));
}

TEST_F(ImlAttrTest, ISASetTest) {
  ImfFuncInfo Default;

  ImfFuncInfo SSE42 = Default.setISASet("sse42");
  ImfFuncInfo AVX = Default.setISASet("avx");
  ImfFuncInfo AVX2 = Default.setISASet("avx2");
  ImfFuncInfo AVX512ZMMLow = Default.setISASet("coreavx512zmmlow");
  ImfFuncInfo AVX512ZMMHigh = Default.setISASet("coreavx512");

  EXPECT_EQ(getLibraryFunctionName("__svml_sinf4", SSE42),
            makeFuncNamePair("__svml_sinf4_ha", "__svml_sinf4_ha"));
  EXPECT_EQ(getLibraryFunctionName("__svml_sinf8", AVX),
            makeFuncNamePair("__svml_sinf8_ha_g9", "__svml_sinf8_ha_e9"));
  EXPECT_EQ(getLibraryFunctionName("__svml_sinf8", AVX2),
            makeFuncNamePair("__svml_sinf8_ha_s9", "__svml_sinf8_ha_l9"));
  EXPECT_EQ(getLibraryFunctionName("__svml_sinf8", AVX512ZMMLow),
            makeFuncNamePair("__svml_sinf8_ha_s9", "__svml_sinf8_ha_l9"));
  EXPECT_EQ(getLibraryFunctionName("__svml_sinf8", AVX512ZMMHigh),
            makeFuncNamePair("__svml_sinf8_ha_x0", "__svml_sinf8_ha_z0"));
}

TEST_F(ImlAttrTest, PrecisionTest) {
  ImfFuncInfo Default;

  ImfFuncInfo LowAccuracy = Default.setPrecision(Low);
  ImfFuncInfo MediumAccuracy = Default.setPrecision(Medium);
  ImfFuncInfo HighAccuracy = Default.setPrecision(High);
  ImfFuncInfo ReferenceAccuracy = Default.setPrecision(Reference);

  EXPECT_EQ(getLibraryFunctionName("powf", LowAccuracy),
            makeFuncNamePair("powf", "powf"));
  EXPECT_EQ(getLibraryFunctionName("powf", MediumAccuracy),
            makeFuncNamePair("powf", "powf"));
  EXPECT_EQ(getLibraryFunctionName("powf", HighAccuracy),
            makeFuncNamePair("powf", "powf"));
  EXPECT_EQ(
      getLibraryFunctionName("powf", ReferenceAccuracy, llvm::Triple::Linux),
      makeFuncNamePair("powf", "__libm_powf_rf"));
  EXPECT_EQ(
      getLibraryFunctionName("powf", ReferenceAccuracy, llvm::Triple::Win32),
      makeFuncNamePair("powf", "powf"));

  EXPECT_EQ(getLibraryFunctionName("__svml_powf8", LowAccuracy),
            makeFuncNamePair("__svml_powf8_ep", "__svml_powf8_ep"));
  EXPECT_EQ(getLibraryFunctionName("__svml_powf8", LowAccuracy),
            makeFuncNamePair("__svml_powf8_ep", "__svml_powf8_ep"));
  EXPECT_EQ(getLibraryFunctionName("__svml_powf8", MediumAccuracy),
            makeFuncNamePair("__svml_powf8", "__svml_powf8"));
  EXPECT_EQ(getLibraryFunctionName("__svml_powf8", HighAccuracy),
            makeFuncNamePair("__svml_powf8_ha", "__svml_powf8_ha"));
  EXPECT_EQ(getLibraryFunctionName("__svml_powf8", ReferenceAccuracy,
                                   llvm::Triple::Linux),
            makeFuncNamePair("__svml_powf8_ha", "__svml_powf8_rf_e9"));
  EXPECT_EQ(getLibraryFunctionName("__svml_powf8", ReferenceAccuracy,
                                   llvm::Triple::Win32),
            makeFuncNamePair("__svml_powf8_ha", "__svml_powf8_ha"));
}

#if INTEL_FEATURE_ISA_FP16
TEST_F(ImlAttrTest, FP16Test) {
  ImfFuncInfo HighAccuracy = ImfFuncInfo().setPrecision(High);
  ImfFuncInfo GLC = ImfFuncInfo().setISASet("coreavx512glc");
  ImfFuncInfo GLCLowAccuracy = GLC.setPrecision(Low);
  ImfFuncInfo GLCMediumAccuracy = GLC.setPrecision(Medium);
  ImfFuncInfo GLCHighAccuracy = GLC.setPrecision(High);
  EXPECT_EQ(getLibraryFunctionName("__svml_sins16", HighAccuracy),
            makeFuncNamePair("__svml_sins16_ha", "__svml_sins16_ha"));
  EXPECT_EQ(getLibraryFunctionName("__svml_sins16", GLCLowAccuracy),
            makeFuncNamePair("__svml_sins16_ep_x1", "__svml_sins16_ep_z1"));
  EXPECT_EQ(getLibraryFunctionName("__svml_sins16", GLCMediumAccuracy),
            makeFuncNamePair("__svml_sins16_x1", "__svml_sins16_z1"));
  EXPECT_EQ(getLibraryFunctionName("__svml_sins16", GLCHighAccuracy),
            makeFuncNamePair("__svml_sins16_ha_x1", "__svml_sins16_ha_z1"));
}
#endif // INTEL_FEATURE_ISA_FP16

} // namespace
