//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2016-2017 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This header file defines the VectorVariant class and implements the encoding
/// and decoding utilities for VectorVariant objects. Multiple VectorVariant
/// objects can be created (masked, non-masked, etc.) and associated with the
/// original scalar function. These objects are then used to clone new functions
/// that can be vectorized. This class follows the standards defined in the
/// vector function ABI.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_UTILS_INTEL_VECTORVARIANT_H
#define LLVM_TRANSFORMS_UTILS_INTEL_VECTORVARIANT_H

#include <vector>
#include <sstream>
#include <cctype>
#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/CommandLine.h"


#if INTEL_CUSTOMIZATION
extern llvm::cl::opt<bool> Usei1MaskForSimdFunctions;
#endif

namespace llvm {

class VectorKind {
  enum class Kind { LINEAR, UNIFORM, VECTOR };
  unsigned char KindData : 2;
  unsigned char IsVariableStride : 1;

  int StrideOrStrideArgumentPosition;
  unsigned Alignment;

  Kind getKind() const { return static_cast<Kind>(KindData); }

  VectorKind(Kind Kind, bool IsVariableStride,
             int StrideOrStrideArgumentPosition, unsigned Alignment)
      : KindData(static_cast<unsigned char>(Kind)),
        IsVariableStride(IsVariableStride),
        StrideOrStrideArgumentPosition(StrideOrStrideArgumentPosition),
        Alignment(Alignment) {}

public:
  static VectorKind linear(unsigned Stride, unsigned Alignment = 0) {
    return VectorKind(Kind::LINEAR, false, Stride, Alignment);
  }

  static VectorKind variableStrided(int StrideArgumentPosition,
                                    unsigned Alignment = 0) {
    assert(StrideArgumentPosition >= 0 && "Negative argument position?");
    return VectorKind{Kind::LINEAR, true, StrideArgumentPosition, Alignment};
  }

  static VectorKind vector(unsigned Alignment = 0) {
    return VectorKind{Kind::VECTOR, false, 0, Alignment};
  }

  static VectorKind uniform(unsigned Alignment = 0) {
    return VectorKind{Kind::UNIFORM, false, 0, Alignment};
  }

  VectorKind(const VectorKind &Other) = default;

  /// Is this a linear parameter?
  bool isLinear() const { return getKind() == Kind::LINEAR; }

  /// Is this a uniform parameter?
  bool isUniform() const { return getKind() == Kind::UNIFORM; }

  /// Is this a vector parameter?
  bool isVector() const { return getKind() == Kind::VECTOR; }

  /// Is the stride for a linear parameter a uniform variable? (i.e.,
  /// the stride is stored in a variable but is uniform)
  bool isVariableStride() const { return isLinear() && IsVariableStride; }

  /// Is the stride for a linear parameter a compile-time constant?
  bool isConstantStrideLinear() const {
    return isLinear() && !IsVariableStride;
  }

  /// Is the stride for a linear variable non-unit stride?
  bool isConstantNonUnitStride() const {
    return isConstantStrideLinear() && StrideOrStrideArgumentPosition != 1;
  }

  /// Is the stride for a linear variable unit stride?
  bool isUnitStride() const {
    return isConstantStrideLinear() && StrideOrStrideArgumentPosition == 1;
  }

  /// Is the parameter aligned?
  // TODO: Should we conside Alignment == 1 as aligned or not?
  bool isAligned() const { return Alignment != 0; }

  /// Get the stride associated with a linear parameter.
  int getStride() const {
    assert(isConstantStrideLinear() && "This is not constant-stride linear!");
    return StrideOrStrideArgumentPosition;
  }

  int getStrideArgumentPosition() const {
    assert(isVariableStride() && "This is not variable-stride linear!");
    return StrideOrStrideArgumentPosition;
  }

  /// Get the alignment associated with a linear parameter.
  unsigned getAlignment() const {
    assert(isAligned() && "It is not aligned!");
    return Alignment;
  }
};

class VectorVariant {

public:
  // ISA classes defined in the vector function ABI.
  enum ISAClass {
    XMM,  // (SSE2)
    YMM1, // (AVX1)
    YMM2, // (AVX2)
    ZMM,  // (MIC)
    OTHER, // Sometimes the ISA is not known and it is marked with 'x'
    ISA_CLASSES_NUM
  };

private:
  // Targets defined in the vector function ABI.
  enum TargetProcessor {
    PENTIUM_4,         // ISA extension = SSE2,     ISA class = XMM
    PENTIUM_4_SSE3,    // ISA extension = SSE3,     ISA class = XMM
    CORE_2_DUO_SSSE3,  // ISA extension = SSSE3,    ISA class = XMM
    CORE_2_DUO_SSE4_1, // ISA extension = SSE4_1,   ISA class = XMM
    CORE_I7_SSE4_2,    // ISA extension = SSE4_2,   ISA class = XMM
    CORE_2ND_GEN_AVX,  // ISA extension = AVX,      ISA class = YMM1
    CORE_3RD_GEN_AVX,  // ISA extension = AVX,      ISA class = YMM1
    CORE_4TH_GEN_AVX,  // ISA extension = AVX2,     ISA class = YMM2
    MIC,               // ISA extension = Xeon Phi, ISA class = MIC(ZMM)
    FUTURE_CPU_22,     // ISA extension = AVX512,   ISA class = ZMM
    FUTURE_CPU_23,     // ISA extension = AVX512,   ISA class = ZMM
  };

  ISAClass Isa;
  bool Mask;
  unsigned int Vlen;
  std::vector<VectorKind> Parameters;

  std::string BaseName;
  std::string Alias;

  enum KindEncodings {
    LINEAR_KIND = 'l',
    UNIFORM_KIND = 'u',
    VECTOR_KIND = 'v'
  };
  static std::string prefix() { return "_ZGV"; }
  static std::string encodeVectorKind(const VectorKind VK);

  /// \brief Determine the maximum vector register width based on the ISA classes
  /// defined in the vector function ABI.
  static unsigned int maximumSizeofISAClassVectorRegister(ISAClass I, Type *Ty);

public:
  VectorVariant(ISAClass I, bool M, unsigned int V,
                const std::vector<VectorKind> &P, std::string BaseName,
                std::string Alias)
      : Isa(I), Mask(M), Vlen(V), Parameters(P), BaseName(std::move(BaseName)),
        Alias(std::move(Alias)) {
    if (Mask) {
      // Masked variants will have an additional mask parameter
      Parameters.push_back(VectorKind::vector());
    }
  }

  VectorVariant(const VectorVariant &Other) = default;

  VectorVariant(StringRef MangledVariantName);

  /// \brief Get the ISA corresponding to this vector variant.
  ISAClass getISA() const { return Isa; }

  /// \brief Set the ISA corresponding to this vector variant.
  void setISA(ISAClass ISA) { Isa = ISA; }

  /// \brief Is this a masked vector function variant?
  bool isMasked() const { return Mask; }

  /// \brief Get the vector length of the vector variant.
  unsigned int getVlen() const { return Vlen; }

  /// \brief Set the vector length of the vector variant.
  void setVlen(unsigned int VL) { Vlen = VL; }

  /// \brief Get the parameters of the vector variant.
  std::vector<VectorKind> &getParameters() { return Parameters; }

  /// \brief Build the mangled name for the vector variant. This function
  /// builds a mangled name by including the encodings for the ISA class,
  /// mask information, and all parameters.
  std::string encode() const {

    std::stringstream SST;
    SST << prefix() << encodeISAClass(Isa) << encodeMask(Mask) << Vlen;

    std::vector<VectorKind>::const_iterator It = Parameters.begin();
    std::vector<VectorKind>::const_iterator End = Parameters.end();

    if (isMasked())
      End--; // mask parameter is not encoded

    for (; It != End; ++It)
      SST << encodeVectorKind(*It);

    SST << "_";

    return SST.str();
  }

  /// \brief Generate a function name corresponding to a vector variant.
  std::string generateFunctionName(StringRef ScalarFuncName) const {
    return encode() + ScalarFuncName.str();
  }

  Optional<std::string> getName() const {
    if (!Alias.empty())
      return Alias;

    if (BaseName.empty())
      return {};

    return generateFunctionName(BaseName);
  }

  std::string toString() const {
    std::string Result = generateFunctionName(BaseName);
    if (!Alias.empty())
      Result += '(' + Alias + ')';

    return Result;
  }

  /// \brief Some targets do not support particular types, so promote to a type
  /// that is supported.
  Type *promoteToSupportedType(Type *Ty) {
    return promoteToSupportedType(Ty, *this);
  }

  /// \brief Check to see if this is a vector variant based on the function
  /// name.
  static bool isVectorVariant(StringRef FuncName) {
    return FuncName.startswith(prefix());
  }

  /// \brief Some targets do not support particular types, so promote to a type
  /// that is supported.
  static Type *promoteToSupportedType(Type *Ty, VectorVariant &Variant) {
    ISAClass I;

    I = Variant.getISA();
    // On ZMM promote char and short to int
    if (I == ISAClass::ZMM && (Ty->isIntegerTy(8) || Ty->isIntegerTy(16)))
      return Type::getInt32Ty(Ty->getContext());

    return Ty;
  }

  /// \brief Get the ISA class corresponding to a particular target processor.
  static ISAClass targetProcessorISAClass(TargetProcessor Target) {

    switch (Target) {
    case PENTIUM_4:
    case PENTIUM_4_SSE3:
    case CORE_2_DUO_SSSE3:
    case CORE_2_DUO_SSE4_1:
    case CORE_I7_SSE4_2:
      return XMM;
    case CORE_2ND_GEN_AVX:
    case CORE_3RD_GEN_AVX:
      return YMM1;
    case CORE_4TH_GEN_AVX:
      return YMM2;
    case MIC:
    case FUTURE_CPU_22:
    case FUTURE_CPU_23:
      return ZMM;
    }

    llvm_unreachable("unsupported target processor");
  }

  /// \brief Get the maximum vector register width for the ISA class.
  static unsigned ISAClassMaxRegisterWidth(ISAClass IsaClass) {
    switch (IsaClass) {
      case XMM:
        return 128;
      case YMM1:
      case YMM2:
        return 256;
      case ZMM:
        return 512;
      default:
        llvm_unreachable("unsupported ISA class");
    }
  }

  /// \brief Get an ISA class string based on ISA class enum.
  static std::string ISAClassToString(ISAClass IsaClass) {
    switch (IsaClass) {
    case XMM:
      return "XMM";
    case YMM1:
      return "YMM1";
    case YMM2:
      return "YMM2";
    case ZMM:
      return "ZMM";
    default:
      assert(false && "unsupported ISA class");
      return "?";
    }
  }

  /// \brief Get an ISA class enum based on ISA class string.
  static ISAClass ISAClassFromString(std::string IsaClass) {
    if (IsaClass == "XMM")
      return XMM;
    if (IsaClass == "YMM1")
      return YMM1;
    if (IsaClass == "YMM2")
      return YMM2;
    if (IsaClass == "ZMM")
      return ZMM;
    assert(false && "unsupported ISA class");
    return ISA_CLASSES_NUM;
  }

  /// \brief Encode the ISA class for the mangled variant name.
  static char encodeISAClass(ISAClass IsaClass) {

    // Front-end seems to have changed the default implementation to conform to
    // the GCC compatible vector ABI. TODO: To maintain ICC compatibility, we
    // need to be able to switch between the GCC/ICC vector ABIs. Once this
    // switch is supported by the Driver, the proper encodings for each mode
    // can be selected here.
    switch (IsaClass) {
    case XMM:
      return 'b';
    case YMM1:
      return 'c';
    case YMM2:
      return 'd';
    case ZMM:
      return 'e';
    case OTHER:
      return 'x';
    default:
      break;
    }

    assert(false && "unsupported ISA class");
    return '?';
  }

  /// \brief Decode the ISA class from the mangled variant name.
  static ISAClass decodeISAClass(char IsaClass) {

    // Front-end seems to have changed the default implementation to conform to
    // the GCC compatible vector ABI. TODO: To maintain ICC compatibility, we
    // need to be able to switch between the GCC/ICC vector ABIs. Once this
    // switch is supported by the Driver, the proper decoding for each mode
    // can be selected here.
    switch (IsaClass) {
    case 'b':
      return XMM;
    case 'c':
      return YMM1;
    case 'd':
      return YMM2;
    case 'e':
      return ZMM;
    case 'x':
      return OTHER;
    default:
      llvm_unreachable("unsupported ISA class");
      return XMM;
    }
  }

  /// \brief Encode the mask information for the mangled variant name.
  static char encodeMask(bool EncodeMask) {

    return EncodeMask ? 'M' : 'N';
  }

  /// \brief Decode the mask information from the mangled variant name.
  static bool decodeMask(char MaskToDecode) {

    switch (MaskToDecode) {
    case 'M':
      return true;
    case 'N':
      return false;
    }

    llvm_unreachable("unsupported mask");
  }

  /// \brief Calculate the vector length for the vector variant.
  static unsigned int calcVlen(ISAClass I, Type *Ty);
};

} // namespace llvm
#endif // LLVM_TRANSFORMS_UTILS_INTEL_VECTORVARIANT_H
