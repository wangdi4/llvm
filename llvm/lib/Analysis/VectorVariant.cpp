#include "llvm/Analysis/VectorVariant.h"
#include "llvm/IR/Type.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

VectorVariant::VectorVariant(StringRef FuncName) {

  assert(isVectorVariant(FuncName) && "Invalid vector variant format");

  std::stringstream SST(FuncName.drop_front(prefix().size()));

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
    int Stride = VectorKind::notAValue();
    int StrideSign = POSITIVE;
    int Alignment = NOT_ALIGNED;

    // Get parameter kind
    SST.get(Kind);

    // Default stride for linear is 1. If the stride for a parameter is 1,
    // then the front-end will not encode it and we will not have set the
    // correct stride below.
    if (Kind == LINEAR_KIND)
      Stride = 1;

    // Handle optional stride
    if (SST.peek() == 'n') {
      // Stride is negative
      SST.ignore(1);
      StrideSign = NEGATIVE;
    }

    if (std::isdigit(SST.peek())) {
      // Extract constant stride
      SST >> Stride;
      assert((Kind != STRIDE_KIND || Stride >= 0) &&
             "Variable stride argument index cannot be negative");
    }

    Stride *= StrideSign;
    // Handle optional alignment
    if (SST.peek() == 'a') {
      SST.ignore(1);
      SST >> Alignment;
    }

    VectorKind VecKind(Kind, Stride, Alignment);
    Parameters.push_back(VecKind);
  }

  if (Mask) {
    // Masked variants will have an additional mask parameter
    VectorKind VecKind(VECTOR_KIND, VectorKind::notAValue());
    Parameters.push_back(VecKind);
  }
}

unsigned int VectorVariant::calcVlen(ISAClass I, Type *CharacteristicDataType) {
  assert(CharacteristicDataType &&
         CharacteristicDataType->getPrimitiveSizeInBits() != 0 &&
         "Expected characteristic data type to have a primitive size in bits");

  unsigned int VectorRegisterSize =
      maximumSizeofISAClassVectorRegister(I, CharacteristicDataType);

  return VectorRegisterSize / CharacteristicDataType->getPrimitiveSizeInBits();
}

unsigned int VectorVariant::maximumSizeofISAClassVectorRegister(ISAClass I,
                                                                Type *Ty) {
  assert((Ty->isIntegerTy() || Ty->isFloatTy() || Ty->isDoubleTy() ||
          Ty->isPointerTy()) &&
         "Unsupported type");

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
    llvm_unreachable("Unknown ISA class");
  }

  assert(VectorRegisterSize != 0 && "Unsupported ISA/type combination");
  return VectorRegisterSize;
}
