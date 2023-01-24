//==-- SpecializeConstant.cpp - Resolve __spirv_SpecConstant calls -- C++ -==//
//
// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// ===--------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/SpecializeConstant.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include <unordered_set>

using namespace llvm;

cl::list<std::string>
    SYCLSpecConstantOpt("sycl-spec-constant", cl::Hidden,
                         cl::desc("Specialize specific constant."),
                         cl::value_desc("id:type:value"));

#define DEBUG_TYPE "sycl-kernel-spec-constant"

/// Emitting warning messages if specified -sycl-spec-constant is invalid.
class SpecializeConstantDiagInfo : public DiagnosticInfo {
  const StringRef OptionValue;
  const Twine &Msg;

public:
  static DiagnosticKind Kind;

  SpecializeConstantDiagInfo(const StringRef OptionValue, const Twine &Msg)
      : DiagnosticInfo(Kind, DS_Warning), OptionValue(OptionValue), Msg(Msg) {}

  void print(DiagnosticPrinter &DP) const override {
    DP << "Option --sycl-spec-constant=" << OptionValue
       << " is ignored because " << Msg;
  }
};

DiagnosticKind SpecializeConstantDiagInfo::Kind =
    static_cast<DiagnosticKind>(getNextAvailablePluginDiagnosticKind());

/// Maps spec constant id to a tuple of (expected type, specialized value,
/// original option value)
using SpecConstantMap =
    DenseMap<uint32_t, std::tuple<Type *, Constant *, StringRef>>;

static Type *parseTypeName(LLVMContext &C, StringRef Name) {
  unsigned BitWidth = 0;
  if (Name.consume_front("i")) {
    if (Name.getAsInteger(10, BitWidth))
      return nullptr;
    return Type::getIntNTy(C, BitWidth);
  }

  if (!Name.consume_front("f"))
    return nullptr;

  if (Name.getAsInteger(10, BitWidth))
    return nullptr;

  switch (BitWidth) {
  case 16:
    return Type::getHalfTy(C);
  case 32:
    return Type::getFloatTy(C);
  case 64:
    return Type::getDoubleTy(C);
  default:
    return nullptr;
  }
}

static void collectExternalSpecializations(LLVMContext &C,
                                           SpecConstantMap &ExtSpecConstants) {
  SmallVector<StringRef, 3> Params;
  for (StringRef SpecConstOpt : SYCLSpecConstantOpt) {
    Params.clear();
    SpecConstOpt.split(Params, ':', 2);
    if (Params.size() != 3) {
      C.diagnose(SpecializeConstantDiagInfo(
          SpecConstOpt,
          "the format is invalid. The expected format is id:type:value"));
      continue;
    }

    uint32_t SpecId = 0;
    if (Params[0].getAsInteger(10, SpecId)) {
      C.diagnose(SpecializeConstantDiagInfo(SpecConstOpt,
                                            "the id must be an unsigned int"));
      continue;
    }

    Type *Ty = parseTypeName(C, Params[1]);
    if (!Ty) {
      C.diagnose(SpecializeConstantDiagInfo(
          SpecConstOpt, "the type is invalid. The expected type is one of i1, "
                        "i8, i16, i32, i64, f16, f32 and f64"));
      continue;
    }
    Constant *Value = nullptr;
    if (auto *ITy = dyn_cast<IntegerType>(Ty))
      Value = ConstantInt::get(ITy, Params[2], 10);
    else
      Value = ConstantFP::get(Ty, Params[2]);

    auto It = ExtSpecConstants.find(SpecId);
    if (It != ExtSpecConstants.end())
      C.diagnose(SpecializeConstantDiagInfo(
          std::get<2>(It->getSecond()),
          "it is overrided by the later option with same id (" + SpecConstOpt +
              ")"));
    ExtSpecConstants[SpecId] = {Ty, Value, SpecConstOpt};
  }
}

static void resolveSpecConstantCalls(Function &SpecConstantFunc,
                                     const SpecConstantMap &ExtSpecConstants) {
  std::unordered_set<uint32_t> UsedExtIds;
  for (auto *U : make_early_inc_range(SpecConstantFunc.users())) {
    if (auto *CI = dyn_cast<CallInst>(U)) {
      ConstantInt *SpecId = cast<ConstantInt>(CI->getArgOperand(0));
      Constant *Value = cast<Constant>(CI->getArgOperand(1));
      assert(Value->getType() == CI->getType() &&
             "The default value doesn't match the return type of "
             "__spirv_SpecConstant");
      auto It = ExtSpecConstants.find(SpecId->getZExtValue());
      if (It != ExtSpecConstants.end()) {
        UsedExtIds.insert(It->getFirst());
        Type *Ty;
        Constant *C;
        StringRef OptValue;
        std::tie(Ty, C, OptValue) = It->getSecond();
        if (Ty != CI->getType())
          SpecConstantFunc.getContext().diagnose(SpecializeConstantDiagInfo(
              OptValue, "the specified type doesn't match "
                        "with the spec constant type defined "
                        "in the module"));
        else
          Value = C;
      }
      LLVM_DEBUG(dbgs() << "Resolving " << *CI << " as " << *Value << '\n');
      CI->replaceAllUsesWith(Value);
      CI->eraseFromParent();
    }
  }
  // Emit warning for unused options.
  for (auto &Entry : ExtSpecConstants)
    if (!UsedExtIds.count(Entry.getFirst()))
      SpecConstantFunc.getContext().diagnose(SpecializeConstantDiagInfo(
          std::get<2>(Entry.getSecond()),
          "there's no matched spec constant in the module of the given id"));

  SpecConstantFunc.eraseFromParent();
}

PreservedAnalyses SpecializeConstantPass::run(Module &M,
                                              ModuleAnalysisManager &AM) {
  auto *F = M.getFunction("__spirv_SpecConstant");
  if (!F)
    return PreservedAnalyses::all();

  SpecConstantMap ExtSpecConstants;
  collectExternalSpecializations(M.getContext(), ExtSpecConstants);
  resolveSpecConstantCalls(*F, ExtSpecConstants);
  return PreservedAnalyses::none();
}
