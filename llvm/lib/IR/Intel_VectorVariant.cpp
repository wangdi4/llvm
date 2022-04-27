//===------- Intel_VectorVariant.cpp - Vector function ABI -*- C++ -*------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the VectorVariant class and corresponding utilities.
/// VectorVariant objects are associated with a scalar function and are used
/// to generate new functions that can be vectorized. VectorVariants are
/// determined by inspecting the function attributes associated with the scalar
/// function. When a mangled function name is found in the attributes (indicated
/// as "_ZGV"), a VectorVariant object is created. The class and utilities
/// in this file follow the standards defined in the vector function ABI.
///
//===----------------------------------------------------------------------===//

#include "llvm/IR/Intel_VectorVariant.h"
#include "llvm/IR/Type.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "VectorVariant"

using namespace llvm;

/// \brief Generate a vector variant by decoding the mangled string for the
/// variant contained in the original scalar function's attributes. For
/// example: "_ZGVxN4". The name mangling is defined in the vector function
/// ABI. Based on this string, the parameter kinds (uniform, linear, vector),
/// vector length, parameter alignment, and masking are determined.
VectorVariant::VectorVariant(StringRef MangledVariantName) {
  size_t AliasStart = MangledVariantName.find_first_of('(');
  StringRef FuncName = MangledVariantName.take_front(AliasStart);
  if (AliasStart != MangledVariantName.npos) {
    assert(MangledVariantName[MangledVariantName.size() - 1] == ')' &&
           "No matching parenthesis!");
    this->Alias = std::string(MangledVariantName.slice(
        AliasStart + 1, MangledVariantName.size() - 1));
  }

  assert(isVectorVariant(FuncName) && "invalid vector variant format");

  std::stringstream SST(std::string(FuncName.drop_front(prefix().size())));

  // mandatory annotations
  char EncodedISA;
  SST.get(EncodedISA);
  Isa = decodeISAClass(EncodedISA);

  char EncodedMask;
  SST.get(EncodedMask);
  Mask = decodeMask(EncodedMask);
  SST >> Vlen;

  // optional parameter annotations
  while (SST.peek() != '_') {
    char Kind;

    // Get parameter kind
    SST.get(Kind);

    auto ExtractOptionalAlignment =
        [](std::stringstream &SST) -> unsigned {
      if (SST.peek() == 'a') {
        SST.ignore(1);
        unsigned Alignment;
        SST >> Alignment;
        return Alignment;
      }
      return 0;
    };

    // Default stride for linear is 1. If the stride for a parameter is 1,
    // then the front-end will not encode it and we will not have set the
    // correct stride below.
    switch (Kind) {
    case UNIFORM_KIND: {
      Parameters.push_back(VectorKind::uniform(ExtractOptionalAlignment(SST)));
      break;
    }
    case VECTOR_KIND: {
      Parameters.push_back(VectorKind::vector(ExtractOptionalAlignment(SST)));
      break;
    }
    case LINEAR_KIND: {
      bool IsNegativeStride = false;

      if (SST.peek() == 's') {
        // Stride in the variable.
        SST.ignore(1);
        assert(std::isdigit(SST.peek()) &&
               "Expected position number of linear_step argument!");
        unsigned StrideArgumentPosition;
        SST >> StrideArgumentPosition;
        Parameters.push_back(VectorKind::variableStrided(
            StrideArgumentPosition, ExtractOptionalAlignment(SST)));
        break;
      }

      int Stride = 1;
      // Handle optional stride.
      if (SST.peek() == 'n') {
        // Stride is negative.
        SST.ignore(1);
        IsNegativeStride = true;
        // Is the following allowed by the mangling scheme?
        assert(std::isdigit(SST.peek()) &&
               "Negative stride without actual value?");
      }
      if (std::isdigit(SST.peek()))
        // Extract constant stride
        SST >> Stride;

      if (IsNegativeStride)
        Stride = -Stride;

      Parameters.push_back(
          VectorKind::linear(Stride, ExtractOptionalAlignment(SST)));
      break;
    }
    default:
      // "R/U/L"
      llvm_unreachable("Not Implemented yet!");
    }
  }

  // Ignore the underscore.
  SST.ignore(1);

  // The remaining symbols are the base name.
  SST >> BaseName;

  if (Mask) {
    // Masked variants will have an additional mask parameter
    Parameters.push_back(VectorKind::vector());
  }
}

/// \brief Determine the vector variant's vector length based on the
/// characteristic data type defined in the vector function ABI and target
/// vector register width.
unsigned int VectorVariant::calcVlen(ISAClass I,
				     Type* CharacteristicDataType) {
  assert(CharacteristicDataType &&
	 CharacteristicDataType->getPrimitiveSizeInBits() != 0 &&
	 "expected characteristic data type to have a primitive size in bits");

  unsigned int VectorRegisterSize =
    maximumSizeofISAClassVectorRegister(I, CharacteristicDataType);

  return VectorRegisterSize / CharacteristicDataType->getPrimitiveSizeInBits();
}

std::string VectorVariant::encodeVectorKind(const VectorKind VK) {
    std::stringstream SST;
    if (VK.isVector())
      SST << static_cast<char>(VECTOR_KIND);
    else if (VK.isUniform())
      SST << static_cast<char>(UNIFORM_KIND);
    else if (VK.isLinear()) {
      SST << static_cast<char>(LINEAR_KIND);
      if (VK.isVariableStride()) {
        SST << 's' << VK.getStrideArgumentPosition();
      } else if (VK.isConstantNonUnitStride()) {
        int Stride = VK.getStride();
        if (Stride >= 0)
          SST << Stride;
        else
          SST << "n" << -Stride;
      } else {
        assert(VK.isUnitStride() && "Expected unit-stride linear!");
      }
    } else {
      llvm_unreachable("Unknown Kind!");
    }

    if (VK.isAligned())
      SST << 'a' << VK.getAlignment();

    return SST.str();
}

/// \brief Determine the maximum vector register width based on the ISA classes
/// defined in the vector function ABI.
unsigned int VectorVariant::maximumSizeofISAClassVectorRegister(ISAClass I,
                                                                Type* Ty)
{
  assert((Ty->isIntegerTy() || Ty->isFloatTy() || Ty->isDoubleTy() ||
          Ty->isPointerTy()) && "unsupported type");

  unsigned int VectorRegisterSize = 0;

  switch (I) {
    case XMM:
      VectorRegisterSize = 128;
      break;
    case YMM1:
      if (Ty->isIntegerTy() || Ty->isPointerTy())
	VectorRegisterSize = 128;
      else
	VectorRegisterSize = 256;
      break;
    case YMM2:
      if (Ty->isIntegerTy(8))
	VectorRegisterSize = 128;
      else
	VectorRegisterSize = 256;
      break;
    case ZMM:
      VectorRegisterSize = 512;
      break;
    default:
      llvm_unreachable("unknown isa class");
      return 0;
  }

  assert(VectorRegisterSize != 0 && "unsupported ISA/type combination");
  return VectorRegisterSize;
}

int VectorVariant::matchParameters(VectorVariant &Other, int &MaxArg,
                                   const Module *M) {
  int ParamScore = 0;
  // 'this' refers to the variant for the call. Match parameters with Other,
  // which represents some available variant.
  std::vector<VectorKind> OtherParms = Other.getParameters();

  assert(Parameters.size() == OtherParms.size() &&
         "Number of parameters do not match");

  Function *F = M->getFunction(BaseName);
  assert(F && "Function not found in module");

  LLVM_DEBUG(dbgs() << "Attempting parameter matching of " << encode()
                    << " with " << Other.encode() << "\n");

  std::vector<int> ArgScores;
  unsigned ArgIdx = (F->getName().startswith("__intel_indirect_call")) ? 1 : 0;
  for (unsigned I = 0; I < OtherParms.size(); ++I, ++ArgIdx) {
    // Linear and uniform arguments can always safely be put into vectors, but
    // reduce score in those cases because scalar is optimal.
    int ArgScore;
    if (OtherParms[I].isVector()) {
      if (Parameters[I].isVector())
        ArgScore = Vector2VectorScore;
      else
        ArgScore = Scalar2VectorScore; // uniform/linear -> vector
      ArgScores.push_back(ArgScore);
      ParamScore += ArgScore;
      continue;
    }

    // linear->linear matches occur when both args are linear and have same
    // stride.
    if (OtherParms[I].isLinear() && Parameters[I].isLinear() &&
        OtherParms[I].isConstantStrideLinear() &&
        Parameters[I].isConstantStrideLinear() &&
        OtherParms[I].getStride() == Parameters[I].getStride()) {
      ArgScore = Linear2LinearScore;
      ArgScores.push_back(ArgScore);
      ParamScore += ArgScore;
      continue;
    }

    if (OtherParms[I].isUniform() && Parameters[I].isUniform()) {
      // Uniform ptr arguments are more beneficial for performance, so weight
      // them accordingly.
      if (isa<PointerType>(F->getArg(ArgIdx)->getType()))
        ArgScore = UniformPtr2UniformPtrScore;
      else
        ArgScore = Uniform2UniformScore;
      ArgScores.push_back(ArgScore);
      ParamScore += ArgScore;
      continue;
    }

    LLVM_DEBUG(dbgs() << "Arg did not match variant parameter!\n");
    return NoMatch;
  }

  LLVM_DEBUG(dbgs() << "Args matched variant parameters\n");
  // If two args have the same max score, the 1st is selected.
  MaxArg =
      std::max_element(ArgScores.begin(), ArgScores.end()) - ArgScores.begin();
  LLVM_DEBUG(dbgs() << "MaxArg: " << MaxArg << "\n");
  LLVM_DEBUG(dbgs() << "Score: " << ParamScore << "\n");
  return ParamScore;
}

int VectorVariant::getMatchingScore(VectorVariant &Other, int &MaxArg,
                                    const Module *M) {
  if (getVlen() != Other.getVlen())
    return NoMatch;
  if (isMasked() != Other.isMasked())
    return NoMatch;
  return matchParameters(Other, MaxArg, M);
}
