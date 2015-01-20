#include <vector>
#include <sstream>
#include <cctype>
#include "llvm/ADT/StringRef.h"

namespace intel {

class VectorKind {
public:
  VectorKind(char k, int s, int a = -1) {
    assert((s == NA() || k == 's' || k == 'l') && "only linear vectors have strides");
    assert((k != 's' || s != NA()) && "variable-stride vectors must have a stride");
    assert((k != 's' || s >= 0) && "variable stride position must be non-negative");
    assert((a == NA() || a > 0) && "alignment must be positive");
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
  VectorVariant(llvm::StringRef funcName) {
    assert(funcName.startswith(PREFIX()) && "WI requires vector parameter annotation");
    std::stringstream sst(funcName.drop_front(PREFIX().size()));
    // mandatory annotations
    sst.get(isa);
    sst.get(mask);
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
  }

  char getISA() {return isa;}
  char getMask() {return mask;}
  int getVlen() {return vlen;}
  std::vector<VectorKind>& getParameters() {return parameters;}

  std::string encode() {
    std::stringstream sst;
    sst << PREFIX() << isa << mask << vlen;
    std::vector<VectorKind>::iterator it, end;
    for (it = parameters.begin(), end = parameters.end(); it != end; it++)
      sst << (*it).encode();
    sst << "_";
    return sst.str();
  }

private:
  char isa;
  char mask;
  int vlen;
  std::vector<VectorKind> parameters;

  static std::string PREFIX() {return "_ZGV";}
};

}
