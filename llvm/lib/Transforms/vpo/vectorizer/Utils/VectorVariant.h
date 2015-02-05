#include <vector>
#include <sstream>
#include <cctype>
#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/ADT/StringRef.h"

namespace intel {

enum TargetProcessor {
  pentium_4,
  pentium_4_sse3,
  core_2_duo_ssse3,
  core_2_duo_sse4_1,
  core_i7_sse4_2,
  core_2nd_gen_avx,
  core_3nd_gen_avx,
  core_4nd_gen_avx,
  mic,
};

enum ISAClass {
  XMM,  // (SSE2)
  YMM1, // (AVX1)
  YMM2, // (AVX2)
  ZMM,  // (mic)
};

class VectorKind {
public:
  VectorKind(char k, int s, int a = -1) {
    assert((s == NA() || k == 's' || k == 'l') &&
	   "only linear vectors have strides");
    assert((k != 's' || s != NA()) &&
	   "variable-stride vectors must have a stride");
    assert((k != 's' || s >= 0) &&
	   "variable stride position must be non-negative");
    assert((a == NA() || a > 0) &&
	   "alignment must be positive");
    kind = k;
    stride = s;
    alignment = a;
  }
  VectorKind(const VectorKind& other) {
    kind = other.kind;
    stride = other.stride;
    alignment = other.alignment;
  }
  bool isVariableStride() {return kind == 's';}
  bool isNonUnitStride() {return kind == 'l' && stride != 1;}
  bool isUnitStride() {return kind == 'l' && stride == 1;}
  bool isLinear() {return isVariableStride() || isNonUnitStride() || isUnitStride();}
  bool isUniform() {return kind == 'u';}
  bool isVector() {return kind == 'v';}
  bool isAligned() {return alignment != NA();}
  int getStride() {return stride;}
  int getAlignment() {return alignment;}
  static int NA() {return -1;}
  std::string encode() {
    std::stringstream sst;
    sst << kind;
    if (isLinear())
      if (stride != NA()) {
	if (stride >= 0)
	  sst << stride;
	else
	  sst << "n" << -stride;
      }
    if (isAligned())
      sst << 'a' << alignment;
    return sst.str();
  }
private:
  char kind;
  int stride;
  int alignment;
};

class VectorVariant {
public:
  VectorVariant(ISAClass i,
		bool m,
		unsigned int v,
		const std::vector<VectorKind>& p) :
                isa(i),
		mask(m),
		vlen(v),
		parameters(p) {
    if (mask) {
      // Masked variants will have an additional mask parameter
      VectorKind vectorKind('v', VectorKind::NA(), VectorKind::NA());
      parameters.push_back(vectorKind);
    }
  }

  VectorVariant(const VectorVariant& other) :
                isa(other.isa),
		mask(other.mask),
		vlen(other.vlen),
		parameters(other.parameters) {}

  VectorVariant(llvm::StringRef funcName);

  ISAClass getISA() {return isa;}
  bool isMasked() {return mask;}
  int getVlen() {return vlen;}
  std::vector<VectorKind>& getParameters() {return parameters;}

  std::string encode() {
    std::stringstream sst;
    sst << PREFIX() << encodeISAClass(isa) << encodeMask(mask) << vlen;
    std::vector<VectorKind>::iterator it = parameters.begin();
    std::vector<VectorKind>::iterator end = parameters.end();
    if (isMasked())
      end--; // mask parameter is not encoded
    for (; it != end; it++)
      sst << (*it).encode();
    sst << "_";
    return sst.str();
  }

  llvm::Type* promoteToSupportedType(llvm::Type* type) {
    return promoteToSupportedType(type, getISA());
  }

  static llvm::Type* promoteToSupportedType(llvm::Type* type, ISAClass isa) {
    // On ZMM promote char and short to int
    if (isa == ISAClass::ZMM && (type->isIntegerTy(8) ||
				 type->isIntegerTy(16))) {
      return llvm::Type::getInt32Ty(type->getContext());
    }
    return type;
  }

  static ISAClass targetProcessorISAClass(TargetProcessor targetProcessor) {
    switch (targetProcessor) {
      case pentium_4:
      case pentium_4_sse3:
      case core_2_duo_ssse3:
      case core_2_duo_sse4_1:
      case core_i7_sse4_2:
	return XMM;
      case core_2nd_gen_avx:
      case core_3nd_gen_avx:
	return YMM1;
      case core_4nd_gen_avx:
	return YMM2;
      case mic:
	return ZMM;
      default:
	assert(false && "unsupported target processor");
	return XMM;
    }
  }

  static char encodeISAClass(ISAClass isa_class) {
    switch(isa_class) {
      case XMM:
	return 'z';
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

  static ISAClass decodeISAClass(char isa_class) {
    switch(isa_class) {
      case 'x':
	return XMM;
      case 'y':
	return YMM1;
      case 'Y':
	return YMM2;
      case 'z':
	return ZMM;
      default:
	assert(false && "unsupported ISA class");
	return XMM;
    }
  }

  static char encodeMask(bool mask) {
    switch(mask) {
      case true:
	return 'M';
      case false:
	return 'N';
    }
  }

  static bool decodeMask(char mask) {
    switch(mask) {
      case 'M':
	return true;
      case 'N':
	return false;
    }
    assert(false && "unsupported mask");
    return false;
  }

  static unsigned int calcVlen(ISAClass isa, llvm::Type* type);

private:
  ISAClass isa;
  bool mask;
  int vlen;
  std::vector<VectorKind> parameters;

  static std::string PREFIX() {return "_ZGV";}

  static unsigned int
  maximum_sizeof_ISA_Class_vector_register(ISAClass isa,
					   llvm::Type* type);

};

}
