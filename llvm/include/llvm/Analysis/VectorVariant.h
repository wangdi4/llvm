#ifndef __VECTOR_VARIANT__
#define __VECTOR_VARIANT__

#include <vector>
#include <sstream>
#include <cctype>
#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/ADT/StringRef.h"

using namespace llvm;

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

enum ISAClass {
  XMM,  // (SSE2)
  YMM1, // (AVX1)
  YMM2, // (AVX2)
  ZMM,  // (MIC)
};

#define STRIDE_KIND  's'
#define LINEAR_KIND  'l'
#define UNIFORM_KIND 'u'
#define VECTOR_KIND  'v'

#define NOT_ALIGNED 1

#define POSITIVE 1
#define NEGATIVE -1

class VectorKind {

public:

  VectorKind(char K, int S, int A = NOT_ALIGNED) {

    assert((S == NA() || K == STRIDE_KIND || K == LINEAR_KIND) &&
	   "only linear vectors have strides");

    assert((K != LINEAR_KIND || S != NA()) &&
	   "linear vectors must have a stride");

    assert((K != STRIDE_KIND || S != NA()) &&
	   "variable stride vectors must have a stride");

    assert((K != STRIDE_KIND || S >= 0) &&
	   "variable stride position must be non-negative");

    assert(A > 0 && "alignment must be positive");

    Kind = K;
    Stride = S;
    Alignment = A;
  }

  VectorKind(const VectorKind& Other) {
    Kind = Other.Kind;
    Stride = Other.Stride;
    Alignment = Other.Alignment;
  }

  bool isVariableStride() { return Kind == STRIDE_KIND; }

  bool isNonUnitStride()  { return Kind == LINEAR_KIND && Stride != 1; }

  bool isUnitStride()     { return Kind == LINEAR_KIND && Stride == 1;}

  bool isLinear() {
    return isVariableStride() || isNonUnitStride() || isUnitStride();
  }

  bool isUniform()   { return Kind == UNIFORM_KIND; }

  bool isVector()    { return Kind == VECTOR_KIND; }

  bool isAligned()   { return Alignment != NOT_ALIGNED; }

  int getStride()    { return Stride; }

  int getAlignment() { return Alignment; }

  static int NA()    { return -1; }

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

  char Kind;
  int Stride;
  int Alignment;
};

class VectorVariant {

public:

  VectorVariant(ISAClass I,
                bool M,
                unsigned int V,
                const std::vector<VectorKind>& P) :
                Isa(I),
                Mask(M),
                Vlen(V),
                Parameters(P) {
    if (Mask) {
      // Masked variants will have an additional mask parameter
      VectorKind vectorKind(VECTOR_KIND, VectorKind::NA());
      Parameters.push_back(vectorKind);
    }
  }

  VectorVariant(const VectorVariant& Other) :
                Isa(Other.Isa),
		Mask(Other.Mask),
		Vlen(Other.Vlen),
		Parameters(Other.Parameters) {}

  VectorVariant(StringRef FuncName);

  ISAClass getISA() { return Isa; }

  bool isMasked() { return Mask; }

  unsigned int getVlen() { return Vlen; }

  std::vector<VectorKind>& getParameters() { return Parameters; }

  std::string encode() {

    std::stringstream SST;
    SST << PREFIX() << encodeISAClass(Isa) << encodeMask(Mask) << Vlen;

    std::vector<VectorKind>::iterator It = Parameters.begin();
    std::vector<VectorKind>::iterator End = Parameters.end();

    if (isMasked())
      End--; // mask parameter is not encoded

    for (; It != End; ++It)
      SST << (*It).encode();

    SST << "_";

    return SST.str();
  }

  std::string generateFunctionName(StringRef ScalarFuncName) {

    static StringRef MANGLING_PREFIX("_Z");
    std::string Name = encode();

    if (ScalarFuncName.startswith(MANGLING_PREFIX))
      return Name + ScalarFuncName.drop_front(MANGLING_PREFIX.size()).str();
    else
      return Name + ScalarFuncName.str();
  }

  Type* promoteToSupportedType(Type* Ty) {
    return promoteToSupportedType(Ty, getISA());
  }

  static bool isVectorVariant(StringRef FuncName) {
    return FuncName.startswith(PREFIX());
  }

  static Type* promoteToSupportedType(Type* Ty,
                                      ISAClass I) {
    // On ZMM promote char and short to int
    if (I == ISAClass::ZMM && (Ty->isIntegerTy(8) || Ty->isIntegerTy(16))) {
      return Type::getInt32Ty(Ty->getContext());
    }

    return Ty;
  }

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

  static char encodeISAClass(ISAClass IsaClass) {

    switch(IsaClass) {
      case XMM:
	return 'x';
      case YMM1:
	return 'y';
      case YMM2:
	return 'Y';
      case ZMM:
	return 'z';
    }

    assert(false && "unsupported ISA class");
    return '?';
  }

  static ISAClass decodeISAClass(char IsaClass) {

    switch(IsaClass) {
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

  static char encodeMask(bool EncodeMask) {

    switch(EncodeMask) {
      case true:
	return 'M';
      case false:
	return 'N';
    }

    llvm_unreachable("unsupported mask");
  }

  static bool decodeMask(char MaskToDecode) {

    switch(MaskToDecode) {
      case 'M':
	return true;
      case 'N':
	return false;
    }

    llvm_unreachable("unsupported mask");
  }

  static unsigned int calcVlen(ISAClass I, Type* Ty);

private:

  ISAClass Isa;
  bool Mask;
  unsigned int Vlen;
  std::vector<VectorKind> Parameters;

  static std::string PREFIX() {return "_ZGV";}

  static unsigned int maximumSizeofISAClassVectorRegister(ISAClass I,
                                                          Type* Ty);
};

#endif // __VECTOR_VARIANT__
