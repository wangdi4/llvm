#include "VectorVariant.h"

namespace intel {

VectorVariant::VectorVariant(llvm::StringRef funcName) {
  assert(funcName.startswith(PREFIX()) && "WI requires vector parameter annotation");
  std::stringstream sst(funcName.drop_front(PREFIX().size()));
  // mandatory annotations
  char encodedISA;
  sst.get(encodedISA);
  isa = decodeISAClass(encodedISA);
  char encodedMask;
  sst.get(encodedMask);
  mask = decodeMask(encodedMask);
  sst >> vlen;
  // optional parameter annotations
  while (sst.peek() != '_') {
    char kind;
    int stride = VectorKind::NA();
    int strideSign = 1;
    int alignment = VectorKind::NA();
    sst.get(kind);
    if (sst.peek() == 'n') {
      sst.ignore(1);
      strideSign = -1;
    }
    if (std::isdigit(sst.peek()))
      sst >> stride;
    assert(kind != 's' || stride >= 0);
    stride *= strideSign;
    if (sst.peek() == 'a') {
      sst.ignore(1);
      sst >> alignment;
    }
    VectorKind vectorKind(kind, stride, alignment);
    parameters.push_back(vectorKind);
  }
  if (mask) {
    // Masked variants will have an additional mask parameter
    VectorKind vectorKind('v', VectorKind::NA(), VectorKind::NA());
    parameters.push_back(vectorKind);
  }
}

unsigned int VectorVariant::calcVlen(ISAClass isa,
				     llvm::Type* characteristicDataType) {
  assert(characteristicDataType &&
	 characteristicDataType->getPrimitiveSizeInBits() != 0 &&
	 "expected characteristic data type to have a primitive size in bits");
  unsigned int vector_register_size =
    maximum_sizeof_ISA_Class_vector_register(isa, characteristicDataType);
  return vector_register_size / characteristicDataType->getPrimitiveSizeInBits();
}

unsigned int
VectorVariant::maximum_sizeof_ISA_Class_vector_register(ISAClass isa,
							llvm::Type* type) {
  assert((type->isIntegerTy() ||
	  type->isFloatTy() ||
	  type->isDoubleTy() ||
	  type->isPointerTy()) &&
	 "unsupported type");
  unsigned int vectorRegisterSize = 0;
  switch (isa) {
    case XMM:
      vectorRegisterSize = 128;
      break;
    case YMM1:
      if (type->isIntegerTy() || type->isPointerTy())
	vectorRegisterSize = 128;
      else
	vectorRegisterSize = 256;
    case YMM2:
      if (type->isIntegerTy(8))
	vectorRegisterSize = 128;
      else
	vectorRegisterSize = 256;
      break;
    case ZMM:
      vectorRegisterSize = 512;
      break;
    default:
      assert(false && "unknown isa class");
      return 0;
  }
  assert(vectorRegisterSize != 0 && "unsupported ISA/type combination");
  return vectorRegisterSize;
}

}
