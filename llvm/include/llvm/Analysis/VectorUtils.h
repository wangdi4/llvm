//===- llvm/Analysis/VectorUtils.h - Vector utilities -----------*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines some vectorizer utilities.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VECTORUTILS_H
#define LLVM_ANALYSIS_VECTORUTILS_H

#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopAccessAnalysis.h"
#include "llvm/IR/IRBuilder.h" // INTEL
#include "llvm/Support/CheckedArithmetic.h"
#include <optional> // INTEL

namespace llvm {
class TargetLibraryInfo;

#if INTEL_CUSTOMIZATION
namespace VectorUtils {
// The attribute provides a list of vector variants for a scalar function.
// When attached to a function definition the list specifies vector variants
// need to be generated from the original (scalar) version.
// When attached to a call, tells loop vectorizer about available vector
// variants for the call.
constexpr const char *VectorVariantsAttrName = "vector-variants";
} // namespace VectorUtils
#endif // INTEL_CUSTOMIZATION
/// Describes the type of Parameters
enum class VFParamKind {
  Vector,            // No semantic information.
  OMP_Linear,        // declare simd linear(i)
  OMP_LinearRef,     // declare simd linear(ref(i))
  OMP_LinearVal,     // declare simd linear(val(i))
  OMP_LinearUVal,    // declare simd linear(uval(i))
  OMP_LinearPos,     // declare simd linear(i:c) uniform(c)
  OMP_LinearValPos,  // declare simd linear(val(i:c)) uniform(c)
  OMP_LinearRefPos,  // declare simd linear(ref(i:c)) uniform(c)
  OMP_LinearUValPos, // declare simd linear(uval(i:c)) uniform(c)
  OMP_Uniform,       // declare simd uniform(i)
  GlobalPredicate,   // Global logical predicate that acts on all lanes
                     // of the input and output mask concurrently. For
                     // example, it is implied by the `M` token in the
                     // Vector Function ABI mangled name.
  Unknown
};

/// Describes the type of Instruction Set Architecture
enum class VFISAKind {
  AdvancedSIMD, // AArch64 Advanced SIMD (NEON)
  SVE,          // AArch64 Scalable Vector Extension
  SSE,          // x86 SSE
  AVX,          // x86 AVX
  AVX2,         // x86 AVX2
  AVX512,       // x86 AVX512
  LLVM,         // LLVM internal ISA for functions that are not
  // attached to an existing ABI via name mangling.
  Unknown // Unknown ISA
};

#if INTEL_CUSTOMIZATION
inline StringRef toString(VFISAKind Kind) {
  switch (Kind) {
  case VFISAKind::SSE:    return "XMM";
  case VFISAKind::AVX:    return "YMM1";
  case VFISAKind::AVX2:   return "YMM2";
  case VFISAKind::AVX512: return "ZMM";

  case VFISAKind::SVE:
  case VFISAKind::AdvancedSIMD:
  case VFISAKind::LLVM:
  case VFISAKind::Unknown: llvm_unreachable("not supported!");
  }
}
#endif // INTEL_CUSTOMIZATION

/// Encapsulates information needed to describe a parameter.
///
/// The description of the parameter is not linked directly to
/// OpenMP or any other vector function description. This structure
/// is extendible to handle other paradigms that describe vector
/// functions and their parameters.
struct VFParameter {
  unsigned ParamPos;         // Parameter Position in Scalar Function.
  VFParamKind ParamKind;     // Kind of Parameter.
  int LinearStepOrPos = 0;   // Step or Position of the Parameter.
#if INTEL_CUSTOMIZATION
  MaybeAlign Alignment = std::nullopt; // Optional alignment in bytes.
#endif // INTEL_CUSTOMIZATION

  // Comparison operator.
  bool operator==(const VFParameter &Other) const {
    return std::tie(ParamPos, ParamKind, LinearStepOrPos, Alignment) ==
           std::tie(Other.ParamPos, Other.ParamKind, Other.LinearStepOrPos,
                    Other.Alignment);
  }

#if INTEL_CUSTOMIZATION
  // Create a vector parameter at the given position, with a possible alignment.
  static VFParameter vector(unsigned Pos, MaybeAlign Alignment = std::nullopt) {
    return VFParameter{Pos, VFParamKind::Vector, 0, Alignment};
  }

  // Create a linear parameter at the given position, with the given stride,
  // with a possible alignment.
  static VFParameter linear(unsigned Pos, int Stride,
                            MaybeAlign Alignment = std::nullopt) {
    return VFParameter{Pos, VFParamKind::OMP_Linear, Stride, Alignment};
  }

  // Create a linear ref parameter at the given position, with the given stride,
  // with a possible alignment.
  static VFParameter linearRef(unsigned Pos, int Stride,
                               MaybeAlign Alignment = std::nullopt) {
    return VFParameter{Pos, VFParamKind::OMP_LinearRef, Stride, Alignment};
  }

  // Create a linear uval parameter at the given position, with the given
  // stride, with a possible alignment.
  static VFParameter linearUVal(unsigned Pos, int Stride,
                                MaybeAlign Alignment = std::nullopt) {
    return VFParameter{Pos, VFParamKind::OMP_LinearUVal, Stride, Alignment};
  }

  // Create a linear val parameter at the given position, with the given stride,
  // with a possible alignment.
  static VFParameter linearVal(unsigned Pos, int Stride,
                               MaybeAlign Alignment = std::nullopt) {
    return VFParameter{Pos, VFParamKind::OMP_LinearVal, Stride, Alignment};
  }

  // Create a linear parameter at the given position, with the given variable
  // position, with a possible alignment.
  static VFParameter linearPos(unsigned Pos, int LinearPos,
                               MaybeAlign Alignment = std::nullopt) {
    return VFParameter{Pos, VFParamKind::OMP_LinearPos, LinearPos, Alignment};
  }

  // Create a linearRef parameter at the given position, with the given variable
  // position, with a possible alignment.
  static VFParameter linearRefPos(unsigned Pos, int LinearPos,
                                  MaybeAlign Alignment = std::nullopt) {
    return VFParameter{Pos, VFParamKind::OMP_LinearRefPos, LinearPos,
                       Alignment};
  }

  // Create a linearUVal parameter at the given position, with the given
  // variable position, with a possible alignment.
  static VFParameter linearUValPos(unsigned Pos, int LinearPos,
                                   MaybeAlign Alignment = std::nullopt) {
    return VFParameter{Pos, VFParamKind::OMP_LinearUValPos, LinearPos,
                       Alignment};
  }

  // Create a linearVal parameter at the given position, with the given variable
  // position, with a possible alignment.
  static VFParameter linearValPos(unsigned Pos, int LinearPos,
                                  MaybeAlign Alignment = std::nullopt) {
    return VFParameter{Pos, VFParamKind::OMP_LinearValPos, LinearPos,
                       Alignment};
  }

  // Create a uniform parameter at the given position, with a possible
  // alignment.
  static VFParameter uniform(unsigned Pos,
                             MaybeAlign Alignment = std::nullopt) {
    return VFParameter{Pos, VFParamKind::OMP_Uniform, 0, Alignment};
  }

  // Create a mask parameter at the given position.
  static VFParameter mask(unsigned Pos) {
    return VFParameter{Pos, VFParamKind::GlobalPredicate, 0, std::nullopt};
  }

  /// Is this a non-reference linear parameter? This includes linear integral
  /// parameters and linear pointers.
  bool isLinear() const {
    return ParamKind == VFParamKind::OMP_Linear ||
           ParamKind == VFParamKind::OMP_LinearPos;
  }

  /// Is this a linear reference parameter with ref modifier?
  bool isLinearRef() const {
    return ParamKind == VFParamKind::OMP_LinearRef ||
           ParamKind == VFParamKind::OMP_LinearRefPos;
  }

  /// Is this a linear reference parameter with uval modifer?
  bool isLinearUVal() const {
    return ParamKind == VFParamKind::OMP_LinearUVal ||
           ParamKind == VFParamKind::OMP_LinearUValPos;
  }

  /// Is this a linear reference parameter with val modifer?
  bool isLinearVal() const {
    return ParamKind == VFParamKind::OMP_LinearVal ||
           ParamKind == VFParamKind::OMP_LinearValPos;
  }

  /// Is this a uniform parameter?
  bool isUniform() const { return ParamKind == VFParamKind::OMP_Uniform; }

  /// Is this a vector parameter?
  ///
  /// NOTE: for convenience, we consider the mask parameter to be a vector
  /// parameter as well, since it is always vector in nature (see
  /// `llvm::createVectorMaskArg`)
  bool isVector() const {
    return ParamKind == VFParamKind::Vector ||
           ParamKind == VFParamKind::GlobalPredicate;
  }

  /// Is this a mask parameter?
  bool isMask() const { return ParamKind == VFParamKind::GlobalPredicate; }

  /// Is the stride for a linear parameter a uniform variable? (i.e.,
  /// the stride is stored in a variable but is uniform)
  bool isVariableStride() const {
    return ParamKind == VFParamKind::OMP_LinearPos ||
           ParamKind == VFParamKind::OMP_LinearRefPos ||
           ParamKind == VFParamKind::OMP_LinearUValPos ||
           ParamKind == VFParamKind::OMP_LinearValPos;
  }

  /// Is the stride for a linear parameter a compile-time constant?
  bool isConstantStrideLinear() const {
    return ParamKind == VFParamKind::OMP_Linear ||
           ParamKind == VFParamKind::OMP_LinearRef ||
           ParamKind == VFParamKind::OMP_LinearUVal ||
           ParamKind == VFParamKind::OMP_LinearVal;
  }

  /// Is the stride for a linear variable non-unit stride?
  bool isConstantNonUnitStride() const {
    return isConstantStrideLinear() && LinearStepOrPos != 1;
  }

  /// Is the stride for a linear variable unit stride?
  bool isUnitStride() const {
    return isConstantStrideLinear() && LinearStepOrPos == 1;
  }

  /// Is the parameter aligned?
  bool isAligned() const { return Alignment.has_value(); }

  /// Get the stride associated with a linear parameter.
  int getStride() const {
    assert(isConstantStrideLinear() && "This is not constant-stride linear!");
    return LinearStepOrPos;
  }

  int getStrideArgumentPosition() const {
    assert(isVariableStride() && "This is not variable-stride linear!");
    return LinearStepOrPos;
  }

  /// Get the alignment associated with a parameter.
  unsigned getAlignment() const {
    assert(isAligned() && "It is not aligned!");
    return Alignment->value();
  }
#endif // INTEL_CUSTOMIZATION
};

/// Contains the information about the kind of vectorization
/// available.
///
/// This object in independent on the paradigm used to
/// represent vector functions. in particular, it is not attached to
/// any target-specific ABI.
struct VFShape {
  ElementCount VF;                        // Vectorization factor.
  SmallVector<VFParameter, 8> Parameters; // List of parameter information.
  // Comparison operator.
  bool operator==(const VFShape &Other) const {
    return std::tie(VF, Parameters) == std::tie(Other.VF, Other.Parameters);
  }

  /// Update the parameter in position P.ParamPos to P.
  void updateParam(VFParameter P) {
    assert(P.ParamPos < Parameters.size() && "Invalid parameter position.");
    Parameters[P.ParamPos] = P;
    assert(hasValidParameterList() && "Invalid parameter list");
  }

  // Retrieve the VFShape that can be used to map a (scalar) function to itself,
  // with VF = 1.
  static VFShape getScalarShape(const CallInst &CI) {
    return VFShape::get(CI, ElementCount::getFixed(1),
                        /*HasGlobalPredicate*/ false);
  }

  // Retrieve the basic vectorization shape of the function, where all
  // parameters are mapped to VFParamKind::Vector with \p EC
  // lanes. Specifies whether the function has a Global Predicate
  // argument via \p HasGlobalPred.
  static VFShape get(const CallInst &CI, ElementCount EC, bool HasGlobalPred) {
    SmallVector<VFParameter, 8> Parameters;
    for (unsigned I = 0; I < CI.arg_size(); ++I)
      Parameters.push_back(VFParameter({I, VFParamKind::Vector}));
    if (HasGlobalPred)
      Parameters.push_back(
          VFParameter({CI.arg_size(), VFParamKind::GlobalPredicate}));

    return {EC, Parameters};
  }
#if INTEL_CUSTOMIZATION
  /// Validation check on the Parameters in the VFShape,
  /// with an added parameter that controls whether to accept variable-strided
  /// params which point to themselves. This is necessary to allow certain
  /// vector variants that we generate during call vec decisions.
  /// (TODO: remove once this behavior has been properly implemented.)
  bool hasValidParameterList(bool Permissive = false) const;
#endif // INTEL_CUSTOMIZATION
};

/// Holds the VFShape for a specific scalar to vector function mapping.
struct VFInfo {
  VFShape Shape;          /// Classification of the vector function.
  std::string ScalarName; /// Scalar Function Name.
  std::string VectorName; /// Vector Function Name associated to this VFInfo.
  VFISAKind ISA;          /// Instruction Set Architecture.

#if INTEL_CUSTOMIZATION
  // Notes:
  //   - `VectorName` holds either the encoded vector name, or the aliased
  //      vector name.
  //   - `FullName` holds the entire (original) specification (including
  //      possible alias.)
  //   - Special care should be taken to not modify properties of this class
  //     (e.g. ISA) without updating the latter two values accordingly. If
  //     modifications are necessary, do so using corresponding setter(s) if
  //     they exist, or make the necessary changes and then call `recomputeNames()` to update their values.

  /// The full variant name (including possible alias.)
  std::string FullName;

  /// VFABI prefix
  static constexpr const char *PREFIX = "_ZGV";

public:
  /// \brief Check to see if \p FuncName is a valid vector variant.
  static bool isVectorVariant(StringRef FuncName) {
    return FuncName.startswith(VFInfo::PREFIX);
  }

  /// \brief Get the ISA of the vector variant.
  VFISAKind getISA() const {
    return ISA;
  }

  /// \brief Set the ISA of the vector variant.
  /// NOTE: this should *always* be used instead of directly setting ISA, as
  /// this updates the `VectorName` and `FullName` fields.
  void setISA(VFISAKind ISA) {
    this->ISA = ISA;
    recomputeNames();
  }

  /// \brief Get the vector length of the vector variant.
  unsigned getVF() const { return Shape.VF.getFixedValue(); }

  /// \brief Set the vector length of the vector variant.
  /// NOTE: this should *always* be used instead of directly setting VF, as
  /// this updates the `VectorName` and `FullName` fields.
  void setVF(unsigned VF) {
    this->Shape.VF = ElementCount::getFixed(VF);
    recomputeNames();
  }

  /// \brief Get the parameters of the vector variant.
  ArrayRef<VFParameter> getParameters() const { return Shape.Parameters; }

  /// \brief Get the variant's prefix (vector name without the scalar name.)
  std::string prefix() const {
    return encodeFromParts(getISA(), isMasked(), getVF(), getParameters(), "");
  }

  /// After making changes to any property of this class, calling
  /// `recomputeNames()` will recompute the values of `VectorName` and
  /// `FullName` to reflect the updated properties.
  void recomputeNames() {
    // Note that we do mangle with gcc specific name mangling.
    // So rise alarm if we are about to switch VFABI encoding.
    assert(!isIntelVFABIMangling(VectorName) &&
           "Changing Vector ABI is not allowed");

    std::string EncodedName = encodeFromParts(getISA(), isMasked(), getVF(),
                                              getParameters(), ScalarName);
    if (VFInfo::isVectorVariant(VectorName)) {
      VectorName = std::move(EncodedName);
      FullName = VectorName;
    } else {
      FullName = (std::move(EncodedName) + "(" + VectorName + ")");
    }
  }

  /// Return true if given vector variant name is encoded with Intel VFABI.
  /// Specifically means that ISA class encoded with 'x', 'y', 'Y' or 'Z'.
  static bool isIntelVFABIMangling(StringRef MangledName) {

    if (!MangledName.consume_front(PREFIX))
      return false;
    if (MangledName.empty())
      return false;
    switch (MangledName.front()) {
    case 'x':
    case 'y':
    case 'Y':
    case 'Z':
      return true;
    default:
      break;
    }
    return false;
  }

  // Return CPU name which is baseline for a vector variant ABI.
  StringRef getBaseCPU() const {
    switch (ISA) {
    case VFISAKind::SSE:
      return "x86-64";
    case VFISAKind::AVX:
      return "corei7-avx";
    case VFISAKind::AVX2:
      return "core-avx2";
    case VFISAKind::AVX512:
      return "skylake-avx512";
    case VFISAKind::SVE:
    case VFISAKind::AdvancedSIMD:
    case VFISAKind::LLVM:
    case VFISAKind::Unknown:
      return StringRef();
    }
  }

private:
  /// Encode the full mangled name of a vector variant from its constituent
  /// parts (e.g. '_ZGVbM4v_foo')
  static std::string encodeFromParts(VFISAKind Isa, bool Mask, unsigned VF,
                                     ArrayRef<VFParameter> Parameters,
                                     StringRef ScalarName);

public:
  /// Get a new VFInfo with the given ISA, Mask, VF, parameters and
  /// scalar name (and possible alias).
  static VFInfo get(VFISAKind ISA, bool Masked, unsigned VF,
                    SmallVector<VFParameter, 8> Parameters,
                    StringRef ScalarName, StringRef Alias = "") {

    assert(llvm::none_of(Parameters,
                         [](const VFParameter &P) { return P.isMask(); }) &&
           "Mask parameters should not be passed directly");

    // Add mask param to masked variants
    if (Masked)
      Parameters.push_back(VFParameter::mask(Parameters.size()));

    std::string EncodedName =
        encodeFromParts(ISA, Masked, VF, Parameters, ScalarName);

    std::string VectorName, FullName;
    if (Alias.empty()) {
      VectorName = std::move(EncodedName);
      FullName = VectorName;
    } else {
      VectorName = std::string(Alias);
      FullName = (std::move(EncodedName) + "(" + Alias + ")").str();
    }

    auto Shape = VFShape{ElementCount::getFixed(VF), std::move(Parameters)};
    assert(Shape.hasValidParameterList(/*Permissive=*/true) &&
           "Invalid parameter list");

    return VFInfo{std::move(Shape), std::string(ScalarName),
                  std::move(VectorName), ISA, std::move(FullName)};
  }

  /// Get a new VFInfo with the given ISA, Mask, VF, parameters and
  /// scalar name (and possible alias).
  static VFInfo get(VFISAKind ISA, bool Masked, unsigned VF,
                    ArrayRef<VFParameter> Parameters, StringRef ScalarName,
                    StringRef Alias = "") {
    return VFInfo::get(
        ISA, Masked, VF,
        SmallVector<VFParameter, 8>(Parameters.begin(), Parameters.end()),
        ScalarName, Alias);
  }

  /// Get a new VFInfo with the given ISA, Mask, VF, parameter kinds and
  /// scalar name (and possible alias).
  ///
  /// NOTE: This is a convenience function only used to create variants with
  /// Vector or Uniform parameter kinds. To generate vector variants with other
  /// kinds of parameters, use the other overload.
  static VFInfo get(VFISAKind ISA, bool Masked, unsigned VF,
                    ArrayRef<VFParamKind> ParamKinds, StringRef ScalarName,
                    StringRef Alias = "") {
    SmallVector<VFParameter, 8> Parameters(map_range(
        enumerate(ParamKinds), [](const auto &IndexedKind) -> VFParameter {
          unsigned ParamPos = IndexedKind.index();
          VFParamKind ParamKind = IndexedKind.value();
          assert((ParamKind == VFParamKind::Vector ||
                  ParamKind == VFParamKind::OMP_Uniform) &&
                 // Can only add vector or uniform parameters -- other kinds
                 // require additional data.
                 "Can only pass vector or uniform param kinds!");
          return VFParameter{ParamPos, ParamKind};
        }));

    return VFInfo::get(ISA, Masked, VF, std::move(Parameters), ScalarName,
                       Alias);
  }

  /// Returns the score of the vector variant matching between 'this' and \p
  /// Other. Returns score of 0 if no proper match was found. Places the
  /// position of the highest scoring arg in \p MaxArg.
  int getMatchingScore(
      const VFInfo &Other, int &MaxArg, const Module *M,
      const ArrayRef<bool> ArgIsLinearPrivateMem) const;
#endif // INTEL_CUSTOMIZATION
  /// Returns the index of the first parameter with the kind 'GlobalPredicate',
  /// if any exist.
  std::optional<unsigned> getParamIndexForOptionalMask() const {
    unsigned ParamCount = Shape.Parameters.size();
    for (unsigned i = 0; i < ParamCount; ++i)
      if (Shape.Parameters[i].ParamKind == VFParamKind::GlobalPredicate)
        return i;

    return std::nullopt;
  }

  /// Returns true if at least one of the operands to the vectorized function
  /// has the kind 'GlobalPredicate'.
  bool isMasked() const { return getParamIndexForOptionalMask().has_value(); }
};

namespace VFABI {
/// LLVM Internal VFABI ISA token for vector functions.
static constexpr char const *_LLVM_ = "_LLVM_";
/// Prefix for internal name redirection for vector function that
/// tells the compiler to scalarize the call using the scalar name
/// of the function. For example, a mangled name like
/// `_ZGV_LLVM_N2v_foo(_LLVM_Scalarize_foo)` would tell the
/// vectorizer to vectorize the scalar call `foo`, and to scalarize
/// it once vectorization is done.
static constexpr char const *_LLVM_Scalarize_ = "_LLVM_Scalarize_";
#if INTEL_CUSTOMIZATION
/// Intel-specific VFABI ISA token for vector functions that indicates
/// incomplete (i.e. yet to be defined) ISA class specification.
static constexpr char const *_Unknown_ = "_unknown_";
#endif // INTEL_CUSTOMIZATION

/// Function to construct a VFInfo out of a mangled names in the
/// following format:
///
/// <VFABI_name>{(<redirection>)}
///
/// where <VFABI_name> is the name of the vector function, mangled according
/// to the rules described in the Vector Function ABI of the target vector
/// extension (or <isa> from now on). The <VFABI_name> is in the following
/// format:
///
/// _ZGV<isa><mask><vlen><parameters>_<scalarname>[(<redirection>)]
///
/// This methods support demangling rules for the following <isa>:
///
/// * AArch64: https://developer.arm.com/docs/101129/latest
///
/// * x86 (libmvec): https://sourceware.org/glibc/wiki/libmvec and
///  https://sourceware.org/glibc/wiki/libmvec?action=AttachFile&do=view&target=VectorABI.txt
///
/// \param MangledName -> input string in the format
/// _ZGV<isa><mask><vlen><parameters>_<scalarname>[(<redirection>)].
/// \param M -> Module used to retrieve informations about the vector
/// function that are not possible to retrieve from the mangled
/// name. At the moment, this parameter is needed only to retrieve the
/// Vectorization Factor of scalable vector functions from their
/// respective IR declarations.
std::optional<VFInfo> tryDemangleForVFABI(StringRef MangledName,
                                          const Module &M);

#if INTEL_CUSTOMIZATION
VFInfo demangleForVFABI(StringRef MangledName);
std::optional<VFInfo> tryDemangleForVFABI(StringRef MangledName,
                                          const Module *M = nullptr);

/// Tells whether parameters and/or return value legalization supported
/// for the vector variant function described by \p Variant.
/// \p ArgTys and \p RetTy describe type of function arguments and return
/// value respectively.
bool supportedVectorVariantLegalization(const VFInfo &Variant,
                                        ArrayRef<Type *> ArgTys, Type *RetTy);

/// Fixup \p ArgChunks and \p RetChunks with the actual number of parts
/// required to pass each logical parameter and return value of a vector
/// variant function described by \p Variant. Logical types of the variant
/// return value and parametes described by \p RetTy and \p ArgTys respectively.
void calcVectorVariantParamChunks(MutableArrayRef<int> ArgChunks,
                                  int &RetChunks, ArrayRef<Type *> ArgTys,
                                  Type *RetTy, const VFInfo &Variant,
                                  bool PtrSize64);
// Return true if variant has mask which should be packed into a bitset.
bool hasPackedMask(const VFInfo &V);

// Return integer type used to pass mask via GPR.
Type *getPackedMaskArgumentTy(LLVMContext &C, unsigned MaskSize);

#endif // INTEL_CUSTOMIZATION

/// Retrieve the `VFParamKind` from a string token.
VFParamKind getVFParamKindFromString(const StringRef Token);

// Name of the attribute where the variant mappings are stored.
static constexpr char const *MappingsAttrName = "vector-function-abi-variant";

/// Populates a set of strings representing the Vector Function ABI variants
/// associated to the CallInst CI. If the CI does not contain the
/// vector-function-abi-variant attribute, we return without populating
/// VariantMappings, i.e. callers of getVectorVariantNames need not check for
/// the presence of the attribute (see InjectTLIMappings).
void getVectorVariantNames(const CallInst &CI,
                           SmallVectorImpl<std::string> &VariantMappings);
} // end namespace VFABI

/// The Vector Function Database.
///
/// Helper class used to find the vector functions associated to a
/// scalar CallInst.
class VFDatabase {
  /// The Module of the CallInst CI.
  const Module *M;
  /// The CallInst instance being queried for scalar to vector mappings.
  const CallInst &CI;
  /// List of vector functions descriptors associated to the call
  /// instruction.
  const SmallVector<VFInfo, 8> ScalarToVectorMappings;

  /// Retrieve the scalar-to-vector mappings associated to the rule of
  /// a vector Function ABI.
  static void getVFABIMappings(const CallInst &CI,
                               SmallVectorImpl<VFInfo> &Mappings) {
    if (!CI.getCalledFunction())
      return;

    const StringRef ScalarName = CI.getCalledFunction()->getName();

    SmallVector<std::string, 8> ListOfStrings;
    // The check for the vector-function-abi-variant attribute is done when
    // retrieving the vector variant names here.
    VFABI::getVectorVariantNames(CI, ListOfStrings);
    if (ListOfStrings.empty())
      return;
    for (const auto &MangledName : ListOfStrings) {
      const std::optional<VFInfo> Shape =
          VFABI::tryDemangleForVFABI(MangledName, *(CI.getModule()));
      // A match is found via scalar and vector names, and also by
      // ensuring that the variant described in the attribute has a
      // corresponding definition or declaration of the vector
      // function in the Module M.
      if (Shape && (Shape->ScalarName == ScalarName)) {
        assert(CI.getModule()->getFunction(Shape->VectorName) &&
               "Vector function is missing.");
        Mappings.push_back(*Shape);
      }
    }
  }

public:
  /// Retrieve all the VFInfo instances associated to the CallInst CI.
  static SmallVector<VFInfo, 8> getMappings(const CallInst &CI) {
    SmallVector<VFInfo, 8> Ret;

    // Get mappings from the Vector Function ABI variants.
    getVFABIMappings(CI, Ret);

    // Other non-VFABI variants should be retrieved here.

    return Ret;
  }

  static bool hasMaskedVariant(const CallInst &CI,
                               std::optional<ElementCount> VF = std::nullopt) {
    // Check whether we have at least one masked vector version of a scalar
    // function. If no VF is specified then we check for any masked variant,
    // otherwise we look for one that matches the supplied VF.
    auto Mappings = VFDatabase::getMappings(CI);
    for (VFInfo Info : Mappings)
      if (!VF || Info.Shape.VF == *VF)
        if (Info.isMasked())
          return true;

    return false;
  }

  /// Constructor, requires a CallInst instance.
  VFDatabase(CallInst &CI)
      : M(CI.getModule()), CI(CI),
        ScalarToVectorMappings(VFDatabase::getMappings(CI)) {}
  /// \defgroup VFDatabase query interface.
  ///
  /// @{
  /// Retrieve the Function with VFShape \p Shape.
  Function *getVectorizedFunction(const VFShape &Shape) const {
    if (Shape == VFShape::getScalarShape(CI))
      return CI.getCalledFunction();

    for (const auto &Info : ScalarToVectorMappings)
      if (Info.Shape == Shape)
        return M->getFunction(Info.VectorName);

    return nullptr;
  }
  /// @}
};

template <typename T> class ArrayRef;
class DemandedBits;
template <typename InstTy> class InterleaveGroup;
class IRBuilderBase;
class Loop;
class ScalarEvolution;
class TargetTransformInfo;
class Type;
class Value;
class SCEV; // INTEL
class Attribute; // INTEL

namespace Intrinsic {
typedef unsigned ID;
}

/// A helper function for converting Scalar types to vector types. If
/// the incoming type is void, we return void. If the EC represents a
/// scalar, we return the scalar type.
inline Type *ToVectorTy(Type *Scalar, ElementCount EC) {
  if (Scalar->isVoidTy() || Scalar->isMetadataTy() || EC.isScalar())
    return Scalar;
  return VectorType::get(Scalar, EC);
}

inline Type *ToVectorTy(Type *Scalar, unsigned VF) {
  return ToVectorTy(Scalar, ElementCount::getFixed(VF));
}

/// Identify if the intrinsic is trivially vectorizable.
/// This method returns true if the intrinsic's argument types are all scalars
/// for the scalar form of the intrinsic and all vectors (or scalars handled by
/// isVectorIntrinsicWithScalarOpAtArg) for the vector form of the intrinsic.
bool isTriviallyVectorizable(Intrinsic::ID ID);

/// Identifies if the vector form of the intrinsic has a scalar operand.
bool isVectorIntrinsicWithScalarOpAtArg(Intrinsic::ID ID,
                                        unsigned ScalarOpdIdx);

/// Identifies if the vector form of the intrinsic is overloaded on the type of
/// the operand at index \p OpdIdx, or on the return type if \p OpdIdx is -1.
bool isVectorIntrinsicWithOverloadTypeAtArg(Intrinsic::ID ID, int OpdIdx);

/// Returns intrinsic ID for call.
/// For the input call instruction it finds mapping intrinsic and returns
/// its intrinsic ID, in case it does not found it return not_intrinsic.
Intrinsic::ID getVectorIntrinsicIDForCall(const CallInst *CI,
                                          const TargetLibraryInfo *TLI);

/// Given a vector and an element number, see if the scalar value is
/// already around as a register, for example if it were inserted then extracted
/// from the vector.
Value *findScalarElement(Value *V, unsigned EltNo);

/// If all non-negative \p Mask elements are the same value, return that value.
/// If all elements are negative (undefined) or \p Mask contains different
/// non-negative values, return -1.
int getSplatIndex(ArrayRef<int> Mask);

/// Get splat value if the input is a splat vector or return nullptr.
/// The value may be extracted from a splat constants vector or from
/// a sequence of instructions that broadcast a single value into a vector.
Value *getSplatValue(const Value *V);

/// Return true if each element of the vector value \p V is poisoned or equal to
/// every other non-poisoned element. If an index element is specified, either
/// every element of the vector is poisoned or the element at that index is not
/// poisoned and equal to every other non-poisoned element.
/// This may be more powerful than the related getSplatValue() because it is
/// not limited by finding a scalar source value to a splatted vector.
bool isSplatValue(const Value *V, int Index = -1, unsigned Depth = 0);

/// Transform a shuffle mask's output demanded element mask into demanded
/// element masks for the 2 operands, returns false if the mask isn't valid.
/// Both \p DemandedLHS and \p DemandedRHS are initialised to [SrcWidth].
/// \p AllowUndefElts permits "-1" indices to be treated as undef.
bool getShuffleDemandedElts(int SrcWidth, ArrayRef<int> Mask,
                            const APInt &DemandedElts, APInt &DemandedLHS,
                            APInt &DemandedRHS, bool AllowUndefElts = false);

/// Replace each shuffle mask index with the scaled sequential indices for an
/// equivalent mask of narrowed elements. Mask elements that are less than 0
/// (sentinel values) are repeated in the output mask.
///
/// Example with Scale = 4:
///   <4 x i32> <3, 2, 0, -1> -->
///   <16 x i8> <12, 13, 14, 15, 8, 9, 10, 11, 0, 1, 2, 3, -1, -1, -1, -1>
///
/// This is the reverse process of widening shuffle mask elements, but it always
/// succeeds because the indexes can always be multiplied (scaled up) to map to
/// narrower vector elements.
void narrowShuffleMaskElts(int Scale, ArrayRef<int> Mask,
                           SmallVectorImpl<int> &ScaledMask);

/// Try to transform a shuffle mask by replacing elements with the scaled index
/// for an equivalent mask of widened elements. If all mask elements that would
/// map to a wider element of the new mask are the same negative number
/// (sentinel value), that element of the new mask is the same value. If any
/// element in a given slice is negative and some other element in that slice is
/// not the same value, return false (partial matches with sentinel values are
/// not allowed).
///
/// Example with Scale = 4:
///   <16 x i8> <12, 13, 14, 15, 8, 9, 10, 11, 0, 1, 2, 3, -1, -1, -1, -1> -->
///   <4 x i32> <3, 2, 0, -1>
///
/// This is the reverse process of narrowing shuffle mask elements if it
/// succeeds. This transform is not always possible because indexes may not
/// divide evenly (scale down) to map to wider vector elements.
bool widenShuffleMaskElts(int Scale, ArrayRef<int> Mask,
                          SmallVectorImpl<int> &ScaledMask);

/// Repetitively apply `widenShuffleMaskElts()` for as long as it succeeds,
/// to get the shuffle mask with widest possible elements.
void getShuffleMaskWithWidestElts(ArrayRef<int> Mask,
                                  SmallVectorImpl<int> &ScaledMask);

/// Splits and processes shuffle mask depending on the number of input and
/// output registers. The function does 2 main things: 1) splits the
/// source/destination vectors into real registers; 2) do the mask analysis to
/// identify which real registers are permuted. Then the function processes
/// resulting registers mask using provided action items. If no input register
/// is defined, \p NoInputAction action is used. If only 1 input register is
/// used, \p SingleInputAction is used, otherwise \p ManyInputsAction is used to
/// process > 2 input registers and masks.
/// \param Mask Original shuffle mask.
/// \param NumOfSrcRegs Number of source registers.
/// \param NumOfDestRegs Number of destination registers.
/// \param NumOfUsedRegs Number of actually used destination registers.
void processShuffleMasks(
    ArrayRef<int> Mask, unsigned NumOfSrcRegs, unsigned NumOfDestRegs,
    unsigned NumOfUsedRegs, function_ref<void()> NoInputAction,
    function_ref<void(ArrayRef<int>, unsigned, unsigned)> SingleInputAction,
    function_ref<void(ArrayRef<int>, unsigned, unsigned)> ManyInputsAction);

/// Compute a map of integer instructions to their minimum legal type
/// size.
///
/// C semantics force sub-int-sized values (e.g. i8, i16) to be promoted to int
/// type (e.g. i32) whenever arithmetic is performed on them.
///
/// For targets with native i8 or i16 operations, usually InstCombine can shrink
/// the arithmetic type down again. However InstCombine refuses to create
/// illegal types, so for targets without i8 or i16 registers, the lengthening
/// and shrinking remains.
///
/// Most SIMD ISAs (e.g. NEON) however support vectors of i8 or i16 even when
/// their scalar equivalents do not, so during vectorization it is important to
/// remove these lengthens and truncates when deciding the profitability of
/// vectorization.
///
/// This function analyzes the given range of instructions and determines the
/// minimum type size each can be converted to. It attempts to remove or
/// minimize type size changes across each def-use chain, so for example in the
/// following code:
///
///   %1 = load i8, i8*
///   %2 = add i8 %1, 2
///   %3 = load i16, i16*
///   %4 = zext i8 %2 to i32
///   %5 = zext i16 %3 to i32
///   %6 = add i32 %4, %5
///   %7 = trunc i32 %6 to i16
///
/// Instruction %6 must be done at least in i16, so computeMinimumValueSizes
/// will return: {%1: 16, %2: 16, %3: 16, %4: 16, %5: 16, %6: 16, %7: 16}.
///
/// If the optional TargetTransformInfo is provided, this function tries harder
/// to do less work by only looking at illegal types.
MapVector<Instruction*, uint64_t>
computeMinimumValueSizes(ArrayRef<BasicBlock*> Blocks,
                         DemandedBits &DB,
                         const TargetTransformInfo *TTI=nullptr);

#if INTEL_CUSTOMIZATION
/// @brief Contains the names of the declared vector function variants
typedef std::vector<std::string> DeclaredVariants;

/// @brief Contains a mapping of a function to its vector function variants
typedef std::map<Function*, DeclaredVariants> FunctionVariants;

/// \brief Determine the characteristic type of the vector function as
/// specified according to the vector function ABI.
Type *calcCharacteristicType(Function &F, const VFInfo &Variant);

/// Determine the characteristic type using the \pReturnType and argument list
/// passed by \p ArgBegin and \p ArgEnd of the vector function as specified
/// according to the vector function ABI.
/// Note: For pointers, the integer type of the pointer size width is returned.
template <typename RangeIterator>
Type *calcCharacteristicType(Type *ReturnType, RangeIterator Args,
                             const VFInfo &Variant, const DataLayout &DL) {
  Type *CharacteristicDataType = nullptr;

  if (!ReturnType->isVoidTy())
    CharacteristicDataType = ReturnType;

  if (!CharacteristicDataType) {
    const auto *VKIt = Variant.getParameters().begin();
    for (auto ArgBegin = Args.begin(), ArgEnd = Args.end(); ArgBegin != ArgEnd;
         ++ArgBegin, ++VKIt) {
      if (VKIt->isVector()) {
        CharacteristicDataType = ArgBegin->getType();
        break;
      }
    }
  }

  // TODO except Clang's ComplexType
  if (!CharacteristicDataType || !CharacteristicDataType->isSingleValueType() ||
      CharacteristicDataType->isX86_MMXTy() ||
      CharacteristicDataType->isVectorTy()) {
    CharacteristicDataType = Type::getInt32Ty(ReturnType->getContext());
  }

  if (CharacteristicDataType->isPointerTy()) {
    unsigned CharacteristicTypeSize =
        DL.getPointerTypeSizeInBits(CharacteristicDataType);
    CharacteristicDataType = IntegerType::get(
        CharacteristicDataType->getContext(), CharacteristicTypeSize);
  }

  return CharacteristicDataType;
}

/// Promote provided boolean (i1) mask, \p MaskToUse, to a type suitable
/// for the \p VecVariant call. Return mask value.
Value *createVectorMaskArg(IRBuilder<> &Builder, Type *CharacteristicType,
                           const VFInfo &VecVariant, Value *MaskToUse);

/// Helper function that returns widened type of given type \p Ty.
inline FixedVectorType *getWidenedType(const Type *Ty, unsigned VF) {
  unsigned NumElts =
      Ty->isVectorTy() ? cast<FixedVectorType>(Ty)->getNumElements() * VF : VF;
  return FixedVectorType::get(Ty->getScalarType(), NumElts);
}

/// Helper function that returns widened type of given type \p Ty.
/// When PromoteI1 is true i1 type promoted to i8 before widening.
inline FixedVectorType *getWidenedType(Type *Ty, unsigned VF, bool PromoteI1) {
  // Promote i1 return type to i8 before widening if requested;
  Type *VecEltTy =
      PromoteI1 && Ty->isIntOrIntVectorTy(1) ? Ty->getWithNewBitWidth(8) : Ty;
  return getWidenedType(VecEltTy, VF);
}

/// Helper function that returns widened type of given type \p RetTy
/// Type is widened with function return specifics.
inline Type *getWidenedReturnType(Type *RetTy, unsigned VF) {
  if (RetTy->isVoidTy())
    return RetTy;

  Type *VecEltRetTy = RetTy;
  // Promote i1 return type to i8 before widening
  if (RetTy->isIntegerTy(1))
    VecEltRetTy = Type::getInt8Ty(RetTy->getContext());
  return getWidenedType(VecEltRetTy, VF);
}

/// Using the \p Args, the arguments of actual call or a function declaration,
/// and information from \p Variant, build the signature of the vector variant.
/// Output is \p LogicalArgTypes array populated with types of arguments and
/// \p LogicalRetType is the vector variant return type.
template <typename ArgsItT>
void buildVectorVariantLogicalSignature(
    Type *RetTy, iterator_range<ArgsItT> Args, const VFInfo &Variant,
    Type *MaskEltType, SmallVectorImpl<Type *> &LogicalArgTypes,
    Type *&LogicalRetType) {

  LogicalArgTypes.clear();
  unsigned VF = Variant.getVF();
  const auto *VKIt = Variant.getParameters().begin();
  for (auto ArgIt = Args.begin(), ArgEnd = Args.end(); ArgIt != ArgEnd;
       ++ArgIt, ++VKIt) {
    Type *ParmType = ArgIt->getType();
    if (VKIt->isVector() || VKIt->isLinearVal())
      ParmType = getWidenedType(ParmType, VF, true /*Promote i1*/);

    LogicalArgTypes.push_back(ParmType);
  }

  if (Variant.isMasked()) {
    assert(MaskEltType && "Mask type not provided for masked variant");
    Type *MaskType = getWidenedType(MaskEltType, VF, true /*Promote i1*/);
    LogicalArgTypes.push_back(MaskType);
  }

  LogicalRetType = getWidenedReturnType(RetTy, VF);
}

/// Helper function to instantiate buildVectorVariantLogicalSignature template
/// specialized for Function.
void buildVectorVariantLogicalSignature(
    Function &OrigF, const VFInfo &Variant, Type *MaskEltType,
    SmallVectorImpl<Type *> &LogicalArgTypes, Type *&LogicalRetType);

/// Build new AttributeList (Fn, return and Parameters) for a vector variant
/// taking into account type legalization information and update the attributes
/// of the \p VectorF. The attributes derived from original function \p OrigF.
void updateVectorVariantAttributes(Function &VectorF, const Function &OrigF,
                                   const VFInfo &Variant,
                                   ArrayRef<Type *> ArgTys,
                                   ArrayRef<int> ArgNumParts);

/// This function will insert functions for simd declared functions.
/// If does not exist already the function creates a vector function
/// variant type using information from \p ArgTys and \p RetTy for their logical
/// types respectively. The original function \p OrigF and variant information
/// \p Variant is used to set proper vector variant attributes.
/// Actual function type is returned via \p FTy. Note that it may be different
/// than logical one if any of the arguments or return value would require
/// target ISA class legalization (in order to be VFABI compliant), i.e. these
/// arguments are passed/returned as chunks (subvectors). This information is
/// provided via \p ArgChunks and \p RetChunks.
Value *getOrInsertVectorVariantFunction(FunctionType *&FTy, Function &OrigF,
                                        const VFInfo &Variant,
                                        ArrayRef<Type *> ArgTys, Type *RetTy,
                                        ArrayRef<int> ArgChunks = std::nullopt,
                                        int RetChunks = 1);

/// \brief Widens the call to function \p OrigF  using a vector length of \p VL
/// and inserts the appropriate function declaration if not already created.
/// This function will insert functions for library calls, intrinsics.
/// The call site instruction is not strictly required here. It is
/// used only for OpenCL read/write channel functions.
Function *getOrInsertVectorLibFunction(Function *OrigF, unsigned VL,
                                       ArrayRef<Type *> ArgTys,
                                       TargetLibraryInfo *TLI,
                                       const TargetTransformInfo *TTI,
                                       Intrinsic::ID ID, bool Masked,
                                       const CallInst *Call = nullptr);

/// Update \p CI call to use calling convention from a \p Callee.
void setCallCallingConvention(CallInst *CI, Value *Callee);

/// \brief Return true if \p FnName is an OpenCL SinCos function
bool isOpenCLSinCos(StringRef FcnName);

/// \brief Return true if \p FnName is an OpenCL read channel function
bool isOpenCLReadChannel(StringRef FnName);

/// \brief Return true if \p FnName is an OpenCL write channel function
bool isOpenCLWriteChannel(StringRef FnName);

/// \brief Return true if the argument at \p Idx is the read destination for
/// an OpenCL read channel call.
bool isOpenCLReadChannelDest(StringRef FnName, unsigned Idx);

/// \brief Return true if the argument at \p Idx is the write source for an
/// OpenCL write channel call.
bool isOpenCLWriteChannelSrc(StringRef FnName, unsigned Idx);

/// \brief Returns the alloca associated with an OpenCL read/write channel call.
AllocaInst* getOpenCLReadWriteChannelAlloc(const CallInst *Call);

/// \brief Returns a string representation of Type \p Ty.
std::string typeToString(Type *Ty);

/// \brief Returns true if \p VFnName is a SVML vector function for a given
/// vectorizable scalar function \p FnName.
bool isSVMLFunction(const TargetLibraryInfo *TLI, StringRef FnName,
                    StringRef VFnName);

/// \brief Returns true if \p FnName is a name of a scalar version of SVML
/// device function
bool isSVMLDeviceScalarFunctionName(StringRef FnName);
/// \brief Returns true if \p VFnName is a device version of a SVML vector
/// function for a given vectorizable scalar function \p FnName.
bool isSVMLDeviceFunction(const TargetLibraryInfo *TLI, StringRef FnName,
                          StringRef VFnName);

/// Determine if scalar call \p CB should be vectorized by pumping
/// feature for the chosen \p VF. If yes, then the factor to pump by is
/// returned, 1 otherwise.
unsigned getPumpFactor(const CallBase &CB, bool IsMasked, unsigned VF,
                       const TargetLibraryInfo *TLI,
                       const TargetTransformInfo *TTI);

/// \brief A helper function that returns value after skipping 'bitcast' and
/// 'addrspacecast' on pointers.
template <typename CastInstTy> Value *getPtrThruCast(Value *Ptr);

/// We need to set call-site attributes, except the ones "consumed" by the
/// vectorizer itself (like vector-variants). Set ones that should be preserved
/// from \p Attrs to \p VecCall. All attributes in the list of \p Attrs is
/// copied one by one to \p VecCall.
void setRequiredAttributes(AttributeList Attrs, CallInst *VecCall);

/// Set attributes of function and return value from \p Attrs to \p VecCall
/// (except the ones "consumed" by the vectorizer itself (like
/// vector-variants)). Set attributes of parameters in \p VecCall to
/// \p AttrArgs.
void setRequiredAttributes(AttributeList Attrs, CallInst *VecCall,
                           ArrayRef<AttributeSet> AttrArgs);

// Common utilities to manipulate vectors

/// \brief Join a set of vectors into a single vector.
Value *joinVectors(ArrayRef<Value *> VectorsToJoin, IRBuilderBase &Builder,
                   Twine Name = "");

/// \brief Extend the length of incoming vector \p OrigVal to \p TargetLength
/// using undefs. Example -
/// {0, 1, 2, 3} -> TargetLen = 8 -> { 0, 1, 2, 3, undef, undef, undef, undef}
Value *extendVector(Value *OrigVal, unsigned TargetLength, IRBuilderBase &Builder,
                    const Twine &Name = "");

/// \brief Replicate elements of vector by \p OriginalVL times in consecutive
/// locations. Example - {0, 1, 2, 3} -> { 0, 0, 1, 1, 2, 2, 3, 3}
Value *replicateVectorElts(Value *OrigVal, unsigned OriginalVL,
                           IRBuilderBase &Builder, const Twine &Name = "");

/// \brief Replicate the entire vector \p OrigVal by \p OriginalVL times.
/// Example - {0, 1, 2, 3} -> { 0, 1, 2, 3, 0, 1, 2, 3}
Value *replicateVector(Value *OrigVal, unsigned OriginalVL,
                       IRBuilderBase &Builder, const Twine &Name = "");

/// Create vector which contains \p V broadcasted \p VF times. \p V can be
/// either another vector or a scalar value. So the resulting vector is
/// - in case \p V is a scalar: {V, V,..,V}, vector of VF elements
/// - in case \p V is vector: {v1,v2,...vN, v1,v2,...vN,...,v1,v2,...vN},
///   vector of NxVF elements
Value *createVectorSplat(Value *V, unsigned VF, IRBuilderBase &Builder,
                         const Twine &Name = "");

/// Generate code to extract a subvector of vector value \p V. The number of
/// parts that vector should be divided into is \p NumParts and \p Part defines
/// the position of the part to extract i.e. starts from Part*(subvector
/// size)-th element of the vector. Subvector size is determined by given vector
/// size and number of parts to be divided into.
Value *generateExtractSubVector(Value *V, unsigned Part, unsigned NumParts,
                                IRBuilderBase &Builder, const Twine &Name = "");
#endif // INTEL_CUSTOMIZATION

/// Compute the union of two access-group lists.
///
/// If the list contains just one access group, it is returned directly. If the
/// list is empty, returns nullptr.
MDNode *uniteAccessGroups(MDNode *AccGroups1, MDNode *AccGroups2);

/// Compute the access-group list of access groups that @p Inst1 and @p Inst2
/// are both in. If either instruction does not access memory at all, it is
/// considered to be in every list.
///
/// If the list contains just one access group, it is returned directly. If the
/// list is empty, returns nullptr.
MDNode *intersectAccessGroups(const Instruction *Inst1,
                              const Instruction *Inst2);

/// Specifically, let Kinds = [MD_tbaa, MD_alias_scope, MD_noalias, MD_fpmath,
/// MD_nontemporal, MD_access_group].
/// For K in Kinds, we get the MDNode for K from each of the
/// elements of VL, compute their "intersection" (i.e., the most generic
/// metadata value that covers all of the individual values), and set I's
/// metadata for M equal to the intersection value.
///
/// This function always sets a (possibly null) value for each K in Kinds.
Instruction *propagateMetadata(Instruction *I, ArrayRef<Value *> VL);

/// Create a mask that filters the members of an interleave group where there
/// are gaps.
///
/// For example, the mask for \p Group with interleave-factor 3
/// and \p VF 4, that has only its first member present is:
///
///   <1,0,0,1,0,0,1,0,0,1,0,0>
///
/// Note: The result is a mask of 0's and 1's, as opposed to the other
/// create[*]Mask() utilities which create a shuffle mask (mask that
/// consists of indices).
Constant *createBitMaskForGaps(IRBuilderBase &Builder, unsigned VF,
                               const InterleaveGroup<Instruction> &Group);

/// Create a mask with replicated elements.
///
/// This function creates a shuffle mask for replicating each of the \p VF
/// elements in a vector \p ReplicationFactor times. It can be used to
/// transform a mask of \p VF elements into a mask of
/// \p VF * \p ReplicationFactor elements used by a predicated
/// interleaved-group of loads/stores whose Interleaved-factor ==
/// \p ReplicationFactor.
///
/// For example, the mask for \p ReplicationFactor=3 and \p VF=4 is:
///
///   <0,0,0,1,1,1,2,2,2,3,3,3>
llvm::SmallVector<int, 16> createReplicatedMask(unsigned ReplicationFactor,
                                                unsigned VF);

/// Create an interleave shuffle mask.
///
/// This function creates a shuffle mask for interleaving \p NumVecs vectors of
/// vectorization factor \p VF into a single wide vector. The mask is of the
/// form:
///
///   <0, VF, VF * 2, ..., VF * (NumVecs - 1), 1, VF + 1, VF * 2 + 1, ...>
///
/// For example, the mask for VF = 4 and NumVecs = 2 is:
///
///   <0, 4, 1, 5, 2, 6, 3, 7>.
llvm::SmallVector<int, 16> createInterleaveMask(unsigned VF, unsigned NumVecs);

/// Create a stride shuffle mask.
///
/// This function creates a shuffle mask whose elements begin at \p Start and
/// are incremented by \p Stride. The mask can be used to deinterleave an
/// interleaved vector into separate vectors of vectorization factor \p VF. The
/// mask is of the form:
///
///   <Start, Start + Stride, ..., Start + Stride * (VF - 1)>
///
/// For example, the mask for Start = 0, Stride = 2, and VF = 4 is:
///
///   <0, 2, 4, 6>
llvm::SmallVector<int, 16> createStrideMask(unsigned Start, unsigned Stride,
                                            unsigned VF);

#if INTEL_CUSTOMIZATION
/// Create an interleave shuffle mask for a "vector of vectors".
///
/// When vectorizing an IR with incoming vector types (e.g. float4), we have to
/// flatten the resulting widened type. For example, ater applying VF=8 to
/// float4, instead of <8 x <4 x float>> we have to generate <32 x float>. That
/// means that masks produced by createInterleaveMask are not applicable to such
/// widened values. This function adapts createInterleaveMask to be usable for
/// vectors of size \p VecWidth.
///
/// For example, a mask to interleave 3 adjacent <4 x <3 x float>> vectors
/// (VF = 4, NumVecs = 3, VecWidth = 3) is:
///
///     <(0, 1, 2), (12, 13, 14), (24, 25, 26),
///      (3, 4, 5), (15, 16, 17), (27, 28, 29),
///      (6, 7, 8), (18, 19, 20), (30, 31, 32),
///      (9, 10, 11), (21, 22, 23), (33, 34, 35)>.
SmallVector<int, 64> createVectorInterleaveMask(unsigned VF, unsigned NumVecs,
                                                unsigned VecWidth);

/// Create a stride shuffle mask for a "vector of vectors".
///
/// When vectorizing an IR with incoming vector types (e.g. float4), we have to
/// flatten the resulting widened type. For example, ater applying VF=8 to
/// float4, instead of <8 x <4 x float>> we have to generate <32 x float>. That
/// means that masks produced by createStrideMask are not applicable to such
/// widened values. This function adapts createStrideMask to be usable for
/// vectors of size \p VecWidth.
///
/// For example, a mask with Stride=3 to extract 4 elements (VF=4) from vector
/// <12 x <3 x float>> starting with the second element (Start=1) is:
///
///     <(3, 4, 5), (12, 13, 14), (21, 22, 23), (30, 31, 32)>.
SmallVector<int, 64> createVectorStrideMask(unsigned Start, unsigned Stride,
                                            unsigned VF, unsigned VecWidth);

/// Return true if given Type \p Ty is a FP type or uses FP type. Arrays and
/// identical element-type Structs are accounted for in this utility.
bool isOrUsesFPTy(Type *Ty);
#endif // INTEL_CUSTOMIZATION

/// Create a sequential shuffle mask.
///
/// This function creates shuffle mask whose elements are sequential and begin
/// at \p Start.  The mask contains \p NumInts integers and is padded with \p
/// NumUndefs undef values. The mask is of the form:
///
///   <Start, Start + 1, ... Start + NumInts - 1, undef_1, ... undef_NumUndefs>
///
/// For example, the mask for Start = 0, NumInsts = 4, and NumUndefs = 4 is:
///
///   <0, 1, 2, 3, undef, undef, undef, undef>
llvm::SmallVector<int, 16>
createSequentialMask(unsigned Start, unsigned NumInts, unsigned NumUndefs);

/// Given a shuffle mask for a binary shuffle, create the equivalent shuffle
/// mask assuming both operands are identical. This assumes that the unary
/// shuffle will use elements from operand 0 (operand 1 will be unused).
llvm::SmallVector<int, 16> createUnaryMask(ArrayRef<int> Mask,
                                           unsigned NumElts);

/// Concatenate a list of vectors.
///
/// This function generates code that concatenate the vectors in \p Vecs into a
/// single large vector. The number of vectors should be greater than one, and
/// their element types should be the same. The number of elements in the
/// vectors should also be the same; however, if the last vector has fewer
/// elements, it will be padded with undefs.
Value *concatenateVectors(IRBuilderBase &Builder, ArrayRef<Value *> Vecs);

/// Given a mask vector of i1, Return true if all of the elements of this
/// predicate mask are known to be false or undef.  That is, return true if all
/// lanes can be assumed inactive.
bool maskIsAllZeroOrUndef(Value *Mask);

/// Given a mask vector of i1, Return true if all of the elements of this
/// predicate mask are known to be true or undef.  That is, return true if all
/// lanes can be assumed active.
bool maskIsAllOneOrUndef(Value *Mask);

/// Given a mask vector of the form <Y x i1>, return an APInt (of bitwidth Y)
/// for each lane which may be active.
APInt possiblyDemandedEltsInMask(Value *Mask);

/// The group of interleaved loads/stores sharing the same stride and
/// close to each other.
///
/// Each member in this group has an index starting from 0, and the largest
/// index should be less than interleaved factor, which is equal to the absolute
/// value of the access's stride.
///
/// E.g. An interleaved load group of factor 4:
///        for (unsigned i = 0; i < 1024; i+=4) {
///          a = A[i];                           // Member of index 0
///          b = A[i+1];                         // Member of index 1
///          d = A[i+3];                         // Member of index 3
///          ...
///        }
///
///      An interleaved store group of factor 4:
///        for (unsigned i = 0; i < 1024; i+=4) {
///          ...
///          A[i]   = a;                         // Member of index 0
///          A[i+1] = b;                         // Member of index 1
///          A[i+2] = c;                         // Member of index 2
///          A[i+3] = d;                         // Member of index 3
///        }
///
/// Note: the interleaved load group could have gaps (missing members), but
/// the interleaved store group doesn't allow gaps.
template <typename InstTy> class InterleaveGroup {
public:
  InterleaveGroup(uint32_t Factor, bool Reverse, Align Alignment)
      : Factor(Factor), Reverse(Reverse), Alignment(Alignment),
        InsertPos(nullptr) {}

  InterleaveGroup(InstTy *Instr, int32_t Stride, Align Alignment)
      : Alignment(Alignment), InsertPos(Instr) {
    Factor = std::abs(Stride);
    assert(Factor > 1 && "Invalid interleave factor");

    Reverse = Stride < 0;
    Members[0] = Instr;
  }

  bool isReverse() const { return Reverse; }
  uint32_t getFactor() const { return Factor; }
  Align getAlign() const { return Alignment; }
  uint32_t getNumMembers() const { return Members.size(); }

  /// Try to insert a new member \p Instr with index \p Index and
  /// alignment \p NewAlign. The index is related to the leader and it could be
  /// negative if it is the new leader.
  ///
  /// \returns false if the instruction doesn't belong to the group.
  bool insertMember(InstTy *Instr, int32_t Index, Align NewAlign) {
    // Make sure the key fits in an int32_t.
    std::optional<int32_t> MaybeKey = checkedAdd(Index, SmallestKey);
    if (!MaybeKey)
      return false;
    int32_t Key = *MaybeKey;

    // Skip if the key is used for either the tombstone or empty special values.
    if (DenseMapInfo<int32_t>::getTombstoneKey() == Key ||
        DenseMapInfo<int32_t>::getEmptyKey() == Key)
      return false;

    // Skip if there is already a member with the same index.
    if (Members.contains(Key))
      return false;

    if (Key > LargestKey) {
      // The largest index is always less than the interleave factor.
      if (Index >= static_cast<int32_t>(Factor))
        return false;

      LargestKey = Key;
    } else if (Key < SmallestKey) {

      // Make sure the largest index fits in an int32_t.
      std::optional<int32_t> MaybeLargestIndex = checkedSub(LargestKey, Key);
      if (!MaybeLargestIndex)
        return false;

      // The largest index is always less than the interleave factor.
      if (*MaybeLargestIndex >= static_cast<int64_t>(Factor))
        return false;

      SmallestKey = Key;
    }

    // It's always safe to select the minimum alignment.
    Alignment = std::min(Alignment, NewAlign);
    Members[Key] = Instr;
    return true;
  }

  /// Get the member with the given index \p Index
  ///
  /// \returns nullptr if contains no such member.
  InstTy *getMember(uint32_t Index) const {
    int32_t Key = SmallestKey + Index;
    return Members.lookup(Key);
  }

  /// Get the index for the given member. Unlike the key in the member
  /// map, the index starts from 0.
  uint32_t getIndex(const InstTy *Instr) const {
    for (auto I : Members) {
      if (I.second == Instr)
        return I.first - SmallestKey;
    }

    llvm_unreachable("InterleaveGroup contains no such member");
  }

  InstTy *getInsertPos() const { return InsertPos; }
  void setInsertPos(InstTy *Inst) { InsertPos = Inst; }

  /// Add metadata (e.g. alias info) from the instructions in this group to \p
  /// NewInst.
  ///
  /// FIXME: this function currently does not add noalias metadata a'la
  /// addNewMedata.  To do that we need to compute the intersection of the
  /// noalias info from all members.
  void addMetadata(InstTy *NewInst) const;

  /// Returns true if this Group requires a scalar iteration to handle gaps.
  bool requiresScalarEpilogue() const {
    // If the last member of the Group exists, then a scalar epilog is not
    // needed for this group.
    if (getMember(getFactor() - 1))
      return false;

    // We have a group with gaps. It therefore can't be a reversed access,
    // because such groups get invalidated (TODO).
    assert(!isReverse() && "Group should have been invalidated");

    // This is a group of loads, with gaps, and without a last-member
    return true;
  }

private:
  uint32_t Factor; // Interleave Factor.
  bool Reverse;
  Align Alignment;
  DenseMap<int32_t, InstTy *> Members;
  int32_t SmallestKey = 0;
  int32_t LargestKey = 0;

  // To avoid breaking dependences, vectorized instructions of an interleave
  // group should be inserted at either the first load or the last store in
  // program order.
  //
  // E.g. %even = load i32             // Insert Position
  //      %add = add i32 %even         // Use of %even
  //      %odd = load i32
  //
  //      store i32 %even
  //      %odd = add i32               // Def of %odd
  //      store i32 %odd               // Insert Position
  InstTy *InsertPos;
};

/// Drive the analysis of interleaved memory accesses in the loop.
///
/// Use this class to analyze interleaved accesses only when we can vectorize
/// a loop. Otherwise it's meaningless to do analysis as the vectorization
/// on interleaved accesses is unsafe.
///
/// The analysis collects interleave groups and records the relationships
/// between the member and the group in a map.
class InterleavedAccessInfo {
public:
  InterleavedAccessInfo(PredicatedScalarEvolution &PSE, Loop *L,
                        DominatorTree *DT, LoopInfo *LI,
                        const LoopAccessInfo *LAI)
      : PSE(PSE), TheLoop(L), DT(DT), LI(LI), LAI(LAI) {}

  ~InterleavedAccessInfo() { invalidateGroups(); }

  /// Analyze the interleaved accesses and collect them in interleave
  /// groups. Substitute symbolic strides using \p Strides.
  /// Consider also predicated loads/stores in the analysis if
  /// \p EnableMaskedInterleavedGroup is true.
  void analyzeInterleaving(bool EnableMaskedInterleavedGroup);

  /// Invalidate groups, e.g., in case all blocks in loop will be predicated
  /// contrary to original assumption. Although we currently prevent group
  /// formation for predicated accesses, we may be able to relax this limitation
  /// in the future once we handle more complicated blocks. Returns true if any
  /// groups were invalidated.
  bool invalidateGroups() {
    if (InterleaveGroups.empty()) {
      assert(
          !RequiresScalarEpilogue &&
          "RequiresScalarEpilog should not be set without interleave groups");
      return false;
    }

    InterleaveGroupMap.clear();
    for (auto *Ptr : InterleaveGroups)
      delete Ptr;
    InterleaveGroups.clear();
    RequiresScalarEpilogue = false;
    return true;
  }

  /// Check if \p Instr belongs to any interleave group.
  bool isInterleaved(Instruction *Instr) const {
    return InterleaveGroupMap.contains(Instr);
  }

  /// Get the interleave group that \p Instr belongs to.
  ///
  /// \returns nullptr if doesn't have such group.
  InterleaveGroup<Instruction> *
  getInterleaveGroup(const Instruction *Instr) const {
    return InterleaveGroupMap.lookup(Instr);
  }

  iterator_range<SmallPtrSetIterator<llvm::InterleaveGroup<Instruction> *>>
  getInterleaveGroups() {
    return make_range(InterleaveGroups.begin(), InterleaveGroups.end());
  }

  /// Returns true if an interleaved group that may access memory
  /// out-of-bounds requires a scalar epilogue iteration for correctness.
  bool requiresScalarEpilogue() const { return RequiresScalarEpilogue; }

  /// Invalidate groups that require a scalar epilogue (due to gaps). This can
  /// happen when optimizing for size forbids a scalar epilogue, and the gap
  /// cannot be filtered by masking the load/store.
  void invalidateGroupsRequiringScalarEpilogue();

  /// Returns true if we have any interleave groups.
  bool hasGroups() const { return !InterleaveGroups.empty(); }

private:
  /// A wrapper around ScalarEvolution, used to add runtime SCEV checks.
  /// Simplifies SCEV expressions in the context of existing SCEV assumptions.
  /// The interleaved access analysis can also add new predicates (for example
  /// by versioning strides of pointers).
  PredicatedScalarEvolution &PSE;

  Loop *TheLoop;
  DominatorTree *DT;
  LoopInfo *LI;
  const LoopAccessInfo *LAI;

  /// True if the loop may contain non-reversed interleaved groups with
  /// out-of-bounds accesses. We ensure we don't speculatively access memory
  /// out-of-bounds by executing at least one scalar epilogue iteration.
  bool RequiresScalarEpilogue = false;

  /// Holds the relationships between the members and the interleave group.
  DenseMap<Instruction *, InterleaveGroup<Instruction> *> InterleaveGroupMap;

  SmallPtrSet<InterleaveGroup<Instruction> *, 4> InterleaveGroups;

  /// Holds dependences among the memory accesses in the loop. It maps a source
  /// access to a set of dependent sink accesses.
  DenseMap<Instruction *, SmallPtrSet<Instruction *, 2>> Dependences;

  /// The descriptor for a strided memory access.
  struct StrideDescriptor {
    StrideDescriptor() = default;
    StrideDescriptor(int64_t Stride, const SCEV *Scev, uint64_t Size,
                     Align Alignment)
        : Stride(Stride), Scev(Scev), Size(Size), Alignment(Alignment) {}

    // The access's stride. It is negative for a reverse access.
    int64_t Stride = 0;

    // The scalar expression of this access.
    const SCEV *Scev = nullptr;

    // The size of the memory object.
    uint64_t Size = 0;

    // The alignment of this access.
    Align Alignment;
  };

  /// A type for holding instructions and their stride descriptors.
  using StrideEntry = std::pair<Instruction *, StrideDescriptor>;

  /// Create a new interleave group with the given instruction \p Instr,
  /// stride \p Stride and alignment \p Align.
  ///
  /// \returns the newly created interleave group.
  InterleaveGroup<Instruction> *
  createInterleaveGroup(Instruction *Instr, int Stride, Align Alignment) {
    assert(!InterleaveGroupMap.count(Instr) &&
           "Already in an interleaved access group");
    InterleaveGroupMap[Instr] =
        new InterleaveGroup<Instruction>(Instr, Stride, Alignment);
    InterleaveGroups.insert(InterleaveGroupMap[Instr]);
    return InterleaveGroupMap[Instr];
  }

  /// Release the group and remove all the relationships.
  void releaseGroup(InterleaveGroup<Instruction> *Group) {
    for (unsigned i = 0; i < Group->getFactor(); i++)
      if (Instruction *Member = Group->getMember(i))
        InterleaveGroupMap.erase(Member);

    InterleaveGroups.erase(Group);
    delete Group;
  }

  /// Collect all the accesses with a constant stride in program order.
  void collectConstStrideAccesses(
      MapVector<Instruction *, StrideDescriptor> &AccessStrideInfo,
      const DenseMap<Value *, const SCEV *> &Strides);

  /// Returns true if \p Stride is allowed in an interleaved group.
  static bool isStrided(int Stride);

  /// Returns true if \p BB is a predicated block.
  bool isPredicated(BasicBlock *BB) const {
    return LoopAccessInfo::blockNeedsPredication(BB, TheLoop, DT);
  }

  /// Returns true if LoopAccessInfo can be used for dependence queries.
  bool areDependencesValid() const {
    return LAI && LAI->getDepChecker().getDependences();
  }

  /// Returns true if memory accesses \p A and \p B can be reordered, if
  /// necessary, when constructing interleaved groups.
  ///
  /// \p A must precede \p B in program order. We return false if reordering is
  /// not necessary or is prevented because \p A and \p B may be dependent.
  bool canReorderMemAccessesForInterleavedGroups(StrideEntry *A,
                                                 StrideEntry *B) const {
    // Code motion for interleaved accesses can potentially hoist strided loads
    // and sink strided stores. The code below checks the legality of the
    // following two conditions:
    //
    // 1. Potentially moving a strided load (B) before any store (A) that
    //    precedes B, or
    //
    // 2. Potentially moving a strided store (A) after any load or store (B)
    //    that A precedes.
    //
    // It's legal to reorder A and B if we know there isn't a dependence from A
    // to B. Note that this determination is conservative since some
    // dependences could potentially be reordered safely.

    // A is potentially the source of a dependence.
    auto *Src = A->first;
    auto SrcDes = A->second;

    // B is potentially the sink of a dependence.
    auto *Sink = B->first;
    auto SinkDes = B->second;

    // Code motion for interleaved accesses can't violate WAR dependences.
    // Thus, reordering is legal if the source isn't a write.
    if (!Src->mayWriteToMemory())
      return true;

    // At least one of the accesses must be strided.
    if (!isStrided(SrcDes.Stride) && !isStrided(SinkDes.Stride))
      return true;

    // If dependence information is not available from LoopAccessInfo,
    // conservatively assume the instructions can't be reordered.
    if (!areDependencesValid())
      return false;

    // If we know there is a dependence from source to sink, assume the
    // instructions can't be reordered. Otherwise, reordering is legal.
    return !Dependences.contains(Src) || !Dependences.lookup(Src).count(Sink);
  }

  /// Collect the dependences from LoopAccessInfo.
  ///
  /// We process the dependences once during the interleaved access analysis to
  /// enable constant-time dependence queries.
  void collectDependences() {
    if (!areDependencesValid())
      return;
    auto *Deps = LAI->getDepChecker().getDependences();
    for (auto Dep : *Deps)
      Dependences[Dep.getSource(*LAI)].insert(Dep.getDestination(*LAI));
  }
};

} // llvm namespace

#endif
