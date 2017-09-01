//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2016 Intel Corporation. All rights reserved.
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

using namespace llvm;

#define STRIDE_KIND 's'
#define LINEAR_KIND 'l'
#define UNIFORM_KIND 'u'
#define VECTOR_KIND 'v'

#define NOT_ALIGNED 1

#define POSITIVE 1
#define NEGATIVE -1

class VectorKind {

public:
  VectorKind(char K, int S, int A = NOT_ALIGNED) {

    assert((S == notAValue() || K == STRIDE_KIND || K == LINEAR_KIND) &&
           "only linear vectors have strides");

    assert((K != LINEAR_KIND || S != notAValue()) &&
           "linear vectors must have a stride");

    assert((K != STRIDE_KIND || S != notAValue()) &&
           "variable stride vectors must have a stride");

    assert((K != STRIDE_KIND || S >= 0) &&
           "variable stride position must be non-negative");

    assert(A > 0 && "alignment must be positive");

    Kind = K;
    Stride = S;
    Alignment = A;
  }

  VectorKind(const VectorKind &Other) {
    Kind = Other.Kind;
    Stride = Other.Stride;
    Alignment = Other.Alignment;
  }

  /// \brief Is the stride for a linear parameter a uniform variable? (i.e.,
  /// the stride is stored in a variable but is uniform)
  bool isVariableStride() { return Kind == STRIDE_KIND; }

  /// \brief Is the stride for a linear variable non-unit stride?
  bool isNonUnitStride() { return Kind == LINEAR_KIND && Stride != 1; }

  /// \brief Is the stride for a linear variable unit stride?
  bool isUnitStride() { return Kind == LINEAR_KIND && Stride == 1; }

  /// \brief Is this a linear parameter?
  bool isLinear() {
    return isVariableStride() || isNonUnitStride() || isUnitStride();
  }

  /// \brief Is this a uniform parameter?
  bool isUniform() { return Kind == UNIFORM_KIND; }

  /// \brief Is this a vector parameter?
  bool isVector() { return Kind == VECTOR_KIND; }

  /// \brief Is the parameter aligned?
  bool isAligned() { return Alignment != NOT_ALIGNED; }

  /// \brief Get the stride associated with a linear parameter.
  int getStride() { return Stride; }

  /// \brief Get the alignment associated with a linear parameter.
  int getAlignment() { return Alignment; }

  /// \brief Represents a don't care value for strides of parameters other
  /// than linear parameters.
  static int notAValue() { return -1; }

  /// \brief Encode the parameter information into a mangled string
  /// corresponding to the standards defined in the vector function ABI.
  std::string encode() {
    std::stringstream SST;
    SST << Kind;

    if (isNonUnitStride()) {
      if (Stride >= 0)
        SST << Stride;
      else
        SST << "n" << -Stride;
    }

    if (isVariableStride())
      SST << Stride;

    if (isAligned())
      SST << 'a' << Alignment;

    return SST.str();
  }

private:
  char Kind;      // linear, uniform, vector
  int  Stride;
  int  Alignment;
};

class VectorVariant {
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

  // ISA classes defined in the vector function ABI.
  enum ISAClass {
    XMM,  // (SSE2)
    YMM1, // (AVX1)
    YMM2, // (AVX2)
    ZMM,  // (MIC)
    ISA_CLASSES_NUM
  };

  ISAClass Isa;
  bool Mask;
  unsigned int Vlen;
  std::vector<VectorKind> Parameters;

  static std::string prefix() { return "_ZGV"; }

  /// \brief Determine the maximum vector register width based on the ISA classes
  /// defined in the vector function ABI.
  static unsigned int maximumSizeofISAClassVectorRegister(ISAClass I, Type *Ty);

public:
  VectorVariant(ISAClass I, bool M, unsigned int V,
                const std::vector<VectorKind> &P)
      : Isa(I), Mask(M), Vlen(V), Parameters(P) {
    if (Mask) {
      // Masked variants will have an additional mask parameter
      VectorKind VKind(VECTOR_KIND, VectorKind::notAValue());
      Parameters.push_back(VKind);
    }
  }

  VectorVariant(const VectorVariant &Other)
      : Isa(Other.Isa), Mask(Other.Mask), Vlen(Other.Vlen),
        Parameters(Other.Parameters) {}

  VectorVariant(StringRef FuncName);

  /// \brief Get the ISA corresponding to this vector variant.
  ISAClass getISA() { return Isa; }

  /// \brief Is this a masked vector function variant?
  bool isMasked() { return Mask; }

  /// \brief Get the vector length of the vector variant.
  unsigned int getVlen() { return Vlen; }

  /// \brief Get the parameters of the vector variant.
  std::vector<VectorKind> &getParameters() { return Parameters; }

  /// \brief Build the mangled name for the vector variant. This function
  /// builds a mangled name by including the encodings for the ISA class,
  /// mask information, and all parameters.
  std::string encode() {

    std::stringstream SST;
    SST << prefix() << encodeISAClass(Isa) << encodeMask(Mask) << Vlen;

    std::vector<VectorKind>::iterator It = Parameters.begin();
    std::vector<VectorKind>::iterator End = Parameters.end();

    if (isMasked())
      End--; // mask parameter is not encoded

    for (; It != End; ++It)
      SST << (*It).encode();

    SST << "_";

    return SST.str();
  }

  /// \brief Generate a function name corresponding to a vector variant.
  std::string generateFunctionName(StringRef ScalarFuncName) {

    static StringRef ManglingPrefix("_Z");
    std::string Name = encode();

    if (ScalarFuncName.startswith(ManglingPrefix))
      return Name + ScalarFuncName.drop_front(ManglingPrefix.size()).str();
    else
      return Name + ScalarFuncName.str();
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
    default:
      llvm_unreachable("unsupported target processor");
      return XMM;
    }
  }

  /// \brief Get an ISA class string based on ISA class enum.
  static std::string ISAClassToString(ISAClass isa_class) {
    switch (isa_class) {
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
  static ISAClass ISAClassFromString(std::string isa_class) {
    if (isa_class == "XMM")
      return XMM;
    if (isa_class == "YMM1")
      return YMM1;
    if (isa_class == "YMM2")
      return YMM2;
    if (isa_class == "ZMM")
      return ZMM;
    assert(false && "unsupported ISA class");
    return ISA_CLASSES_NUM;
  }

  /// \brief Encode the ISA class for the mangled variant name.
  static char encodeISAClass(ISAClass IsaClass) {

    switch (IsaClass) {
    case XMM:
      return 'x';
    case YMM1:
      return 'y';
    case YMM2:
      return 'Y';
    case ZMM:
      return 'z';
    default:
      break;
    }

    assert(false && "unsupported ISA class");
    return '?';
  }

  /// \brief Decode the ISA class from the mangled variant name.
  static ISAClass decodeISAClass(char IsaClass) {

    switch (IsaClass) {
    case 'x':
      return XMM;
    case 'y':
      return YMM1;
    case 'Y':
      return YMM2;
    case 'z':
      return ZMM;
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

#endif // LLVM_TRANSFORMS_UTILS_INTEL_VECTORVARIANT_H
