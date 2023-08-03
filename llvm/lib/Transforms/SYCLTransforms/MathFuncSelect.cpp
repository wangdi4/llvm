//===-- MathFuncSelect.cpp - Select math builtin for required accruacy ---===//
//
// Copyright (C) 2023 Intel Corporation
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
//===---------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/MathFuncSelect.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Transforms/SYCLTransforms/Utils/NameMangleAPI.h"
#include <cmath>
#include <limits>
#include <unordered_set>

using namespace llvm;

#define DEBUG_TYPE "sycl-kernel-math-func-select"

static constexpr const char *MaxErrorAttr = "fpbuiltin-max-error";
static constexpr double DoubleNAN = std::numeric_limits<double>::quiet_NaN();

DiagnosticKind RequiredAccuracyDiagInfo::Kind =
    static_cast<DiagnosticKind>(getNextAvailablePluginDiagnosticKind());

// Here we assume all OCL math builtin functions are directly mapped to the
// corresponding OCL SVML functions.
// For example,
// acos --> __ocl_svml_<target>_acosf*
// native_acos --> __ocl_svml_<target>_acos*_native
//
// There're several low precision variants in OCL SVML libs: _rm, _half, _native
// but they all share the same assembly implementation. So we also assume that
// low precision OCL builtin variants (e.g. native_acos, half_acos) are
// identical.
//
// Since we do NOT have a comprehensive list of accuracies for each OCL SVML
// function, we use the "Relative Error as ULPs" table from OpenCL Spec
// (https://registry.khronos.org/OpenCL/specs/3.0-unified/html/OpenCL_C.html#relative-error-as-ulps)
// to estimate the error upper bound for the corresponding high-precision OCL
// builtin function.
// TODO: Some of our implementations may have a better accuracy than the Spec
// requirement. We should update the map case-by-case.
//
// For low-precision variants, we assume all results will have at least half
// correct mantissa bits. Translating into ULPS, it's 4096 for single precision
// and 6.71089e+07 for double precision. So we don't maintain a dedicated map
// for low-precision errors for now.
using PrecisionPerType =
    std::tuple<float /*ULPS for FP32*/, float /*ULPS for FP64*/>;
// Indicies for tuple elements.
enum PrecisionType { FP32 = 0, FP64 = 1 };
using FuncErrorMapTy = StringMap<PrecisionPerType>;

static const PrecisionPerType LowPrecisionUlpsPerType = {4096.0f, 67108900.0f};
static const FuncErrorMapTy HighPrecisionMap = {
    // TODO: handle commented out lines
    // {"x + y", Correctly rounded},
    // {"x - y", Correctly rounded},
    // {"x * y", Correctly rounded},
    {"divide", {2.5f, 0.5f}}, // for "x / y"
    {"acos", {4.f, 4.f}},
    {"acospi", {5.f, 5.f}},
    {"asin", {4.f, 4.f}},
    {"asinpi", {5.f, 5.f}},
    {"atan", {5.f, 5.f}},
    {"atan2", {6.f, 6.f}},
    {"atanpi", {5.f, 5.f}},
    {"atan2pi", {6.f, 6.f}},
    {"acosh", {4.f, 4.f}},
    {"asinh", {4.f, 4.f}},
    {"atanh", {5.f, 5.f}},
    {"cbrt", {2.f, 2.f}},
    {"ceil", {0.5f, 0.5f}},
    {"clamp", {0.f, 0.f}},
    {"copysign", {0.f, 0.f}},
    {"cos", {4.f, 4.f}},
    {"cosh", {4.f, 4.f}},
    {"cospi", {4.f, 4.f}},
    // {"cross", absolute error tolerance of 'max * max * (3 * FLT_EPSILON)' per
    // vector component, where max is the maximum input operand magnitude},
    {"degrees", {2.f, 2.f}},
    // {"distance", ≤ 2.5 + 2n ulp for gentype with vector width n, ≤ 5.5 + 2n
    // ulp for gentype with vector width n},
    // {"dot", absolute error tolerance of 'max * max * (2n - 1) * FLT_EPSILON',
    // for vector width n and maximum input operand magnitude max across all
    // vector components},
    {"erfc", {16.f, 16.f}},
    {"erf", {16.f, 16.f}},
    {"exp", {3.f, 3.f}},
    {"exp2", {3.f, 3.f}},
    {"exp10", {3.f, 3.f}},
    {"expm1", {3.f, 3.f}},
    {"fabs", {0.f, 0.f}},
    {"fdim", {0.5f, 0.5f}},
    {"floor", {0.5f, 0.5f}},
    {"fma", {0.5f, 0.5f}},
    {"fmax", {0.f, 0.f}},
    {"fmin", {0.f, 0.f}},
    {"fmod", {0.f, 0.f}},
    {"fract", {0.5f, 0.5f}},
    {"frexp", {0.f, 0.f}},
    {"hypot", {4.f, 4.f}},
    {"ilogb", {0.f, 0.f}},
    // {"length", ≤ 2.75 + 0.5n ulp for gentype with vector width n, ≤ 5.5 + n
    // ulp for gentype with vector width n},
    {"ldexp", {0.5f, 0.5f}},
    // {"lgamma", Undefined},
    // {"lgamma_r", Undefined},
    {"log", {3.f, 3.f}},
    {"log2", {3.f, 3.f}},
    {"log10", {3.f, 3.f}},
    {"log1p", {2.f, 2.f}},
    {"logb", {0.f, 0.f}},
    {"mad", {0.5f, std::numeric_limits<float>::infinity()}},
    {"max", {0.f, 0.f}},
    {"maxmag", {0.f, 0.f}},
    {"min", {0.f, 0.f}},
    {"minmag", {0.f, 0.f}},
    // {"mix", absolute error tolerance of 1e-3, Implementation-defined},
    {"modf", {0.f, 0.f}},
    {"nan", {0.f, 0.f}},
    {"nextafter", {0.f, 0.f}},
    // {"normalize", ≤ 2 + n ulp for gentype with vector width n, ≤ 4.5 + n ulp
    // for gentype with vector width n},
    {"pow", {16.f, 16.f}},
    {"pown", {16.f, 16.f}},
    {"powr", {16.f, 16.f}},
    {"radians", {2.f, 2.f}},
    {"remainder", {0.f, 0.f}},
    {"remquo", {0.f, 0.f}},
    {"rint", {0.5f, 0.5f}},
    {"rootn", {16.f, 16.f}},
    {"round", {0.5f, 0.5f}},
    {"rsqrt", {2.f, 2.f}},
    {"sign", {0.f, 0.f}},
    {"sin", {4.f, 4.f}},
    {"sincos", {4.f, 4.f}},
    {"sinh", {4.f, 4.f}},
    {"sinpi", {4.f, 4.f}},
    // {"smoothstep", absolute error tolerance of 1e-5, Implementation-defined},
    {"sqrt", {3.f, 3.f}},
    {"step", {0.f, 0.f}},
    // {"fsqrt", Undefined, Correctly rounded},
    {"tan", {5.f, 5.f}},
    {"tanh", {5.f, 5.f}},
    {"tanpi", {6.f, 6.f}},
    {"tgamma", {16.f, 16.f}},
    {"trunc", {0.5f, 0.5f}},
};

// Functions whose "native_" variants have been implemented
static const std::unordered_set<std::string> NativeSupportedFuncs = {
    "acos",   "acosh",   "acospi", "asin",   "asinh", "asinpi", "atan",
    "atan2",  "atan2pi", "atanh",  "atanpi", "cbrt",  "cos",    "cosh",
    "cospi",  "divide",  "erf",    "erfc",   "exp",   "exp10",  "exp2",
    "expm1",  "fdim",    "fmax",   "fmin",   "fmod",  "fract",  "hypot",
    "ilogb",  "log",     "log10",  "log1p",  "log2",  "logb",   "pow",
    "pown",   "powr",    "rcbrt",  "recip",  "rootn", "rsqrt",  "sin",
    "sincos", "sinh",    "sinpi",  "sqrt",   "tan",   "tanh",   "tanpi",
};

static double getUlpsFromAttribute(Attribute Attr) {
  StringRef S = Attr.getValueAsString();
  double Ulps = DoubleNAN;
  if (S.getAsDouble(Ulps))
    LLVM_DEBUG(dbgs() << "Invalid 'fpbuiltin-max-error' attribute value: " << S
                      << '\n');
  return Ulps;
}

static double getUlpsFromAttribute(Function *F) {
  return F->hasFnAttribute(MaxErrorAttr)
             ? getUlpsFromAttribute(F->getFnAttribute(MaxErrorAttr))
             : DoubleNAN;
}

static double getUlpsFromAttribute(CallInst *CI) {
  return CI->hasFnAttr(MaxErrorAttr)
             ? getUlpsFromAttribute(CI->getFnAttr(MaxErrorAttr))
             : DoubleNAN;
}

static StringRef
stripLowPrecisionAnnotation(StringRef LowPrecisionBuiltinName) {
  StringRef S = LowPrecisionBuiltinName;
  if (S.consume_front("half_"))
    return S;
  if (S.consume_front("native_"))
    return S;
  if (S.consume_back("_rm"))
    return S;
  return S;
}

template <int Type = PrecisionType::FP32>
static std::string selectFuncForRequiredAccuracy(CallInst *CI,
                                                 StringRef BuiltinName,
                                                 float RequiredUlps) {
  // Check the current callee accuracy version.
  bool IsLowPrecision = BuiltinName.starts_with("half_") ||
                        BuiltinName.starts_with("native_") ||
                        BuiltinName.ends_with("_rm");
  StringRef HighPrecisionBuiltinName = stripLowPrecisionAnnotation(BuiltinName);
  // If the ulps of high precision version is not well defined, we have to use
  // the lower precision ulps as the fallback.
  float LowPrecisionUlps = std::get<Type>(LowPrecisionUlpsPerType);
  float HighPrecisionUlps = LowPrecisionUlps;
  auto It = HighPrecisionMap.find(HighPrecisionBuiltinName);
  if (It != HighPrecisionMap.end())
    HighPrecisionUlps = std::get<Type>(It->second);

  // No matter we replace the callee function or not, always issue a warning if
  // the high precision version still cannot meet the ulps requirement.
  if (RequiredUlps < HighPrecisionUlps) {
    CI->getContext().diagnose(RequiredAccuracyDiagInfo(
        *CI->getFunction(),
        "The highest precision version of " + BuiltinName +
            " provided by the implementation has a max error of " +
            std::to_string(HighPrecisionUlps) +
            ", while fpbuiltin-max-error requires " +
            std::to_string(RequiredUlps) + " ulps."));
    // Try to emit the high precision version even it cannot meet the
    // requirement -- anyway, it's a better candidate.
    return HighPrecisionBuiltinName.str();
  }

  if (IsLowPrecision && RequiredUlps < LowPrecisionUlps)
    return stripLowPrecisionAnnotation(BuiltinName).str();

  if (!IsLowPrecision &&
      NativeSupportedFuncs.count(HighPrecisionBuiltinName.str()) &&
      RequiredUlps >= LowPrecisionUlps)
    return std::string("native_") + BuiltinName.str();

  return BuiltinName.str();
}

static bool runOnFunction(Function *F) {
  bool Changed = false;
  auto FD = NameMangleAPI::demangle(F->getName());
  // Explicitly make a copy of builtin name, since we will modify the FD object
  // while generating new calls.
  std::string CalledBuiltinName = FD.Name;
  std::string HighPrecisionBuiltinName =
      stripLowPrecisionAnnotation(CalledBuiltinName).str();

  double FuncUlps = getUlpsFromAttribute(F);
  for (auto *U : F->users()) {
    if (auto *CI = dyn_cast<CallInst>(U)) {
      // If the attribute presents on the call site, then it would take
      // precedence over the function attribute.
      double CallUlps = getUlpsFromAttribute(CI);
      double Ulps = std::isnan(CallUlps) ? FuncUlps : CallUlps;
      // Attribute invalid or not found. Do nothing.
      if (std::isnan(Ulps))
        continue;

      // Found "fpbuiltin-max-error" attribute but we don't have well defined
      // ulps for it.
      // FIXME: Ideally we should have all implementation ulps defined, at least
      // an estimated upper bound?
      if (HighPrecisionMap.find(HighPrecisionBuiltinName) ==
          HighPrecisionMap.end()) {
        F->getContext().diagnose(RequiredAccuracyDiagInfo(
            *CI->getFunction(),
            "The implementation ulps for " + CalledBuiltinName +
                " is not well defined, so the fpbuiltin-max-error "
                "requirement (" +
                std::to_string(Ulps) + " ulps) may not apply."));
        return false;
      }

      // Check the scalar type of the first fp argument.
      Type *FPType = nullptr;
      for (auto &Arg : F->args()) {
        if (Arg.getType()->isFPOrFPVectorTy())
          FPType = Arg.getType()->getScalarType();
      }
      assert(FPType && "Function with fpbuiltin-max-error attribute has no "
                       "floating-point arg!");
      // Only handle fp32 and fp64 type here.
      // FIXME: OpenCL Spec doesn't provide accuracy requirements for
      // other fp types.
      bool IsSinglePrecision = FPType->isFloatTy();
      bool IsDoublePrecision = FPType->isDoubleTy();
      if (!IsSinglePrecision && !IsDoublePrecision)
        continue;

      std::string NewName =
          IsSinglePrecision
              ? selectFuncForRequiredAccuracy<PrecisionType::FP32>(
                    CI, CalledBuiltinName, Ulps)
              : selectFuncForRequiredAccuracy<PrecisionType::FP64>(
                    CI, CalledBuiltinName, Ulps);
      if (NewName == CalledBuiltinName)
        continue;

      LLVM_DEBUG(dbgs() << "Rewriting " << *CI << '\n');
      FD.Name = NewName;
      Function *NewF = dyn_cast<Function>(
          F->getParent()
              ->getOrInsertFunction(NameMangleAPI::mangle(FD),
                                    F->getFunctionType(), F->getAttributes())
              .getCallee());
      CI->replaceUsesOfWith(F, NewF);
      LLVM_DEBUG(dbgs() << " to " << *CI << '\n');
      Changed = true;
    }
  }

  return Changed;
}

PreservedAnalyses MathFuncSelectPass::run(Module &M, ModuleAnalysisManager &) {
  bool Changed = false;
  SmallPtrSet<Function *, 32> WorkList;
  for (auto &F : M)
    if (F.isDeclaration() && NameMangleAPI::isMangledName(F.getName()))
      WorkList.insert(&F);

  for (auto *F : WorkList)
    Changed |= runOnFunction(F);

  return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
