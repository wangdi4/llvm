#include "Mangler.h"
#include "Logger.h"
#include <stdlib.h>

#include <cassert>
#include <sstream>

const std::string Mangler::mask_delim           = "_";
const std::string Mangler::mask_prefix_func     = "maskedf_";
const std::string Mangler::mask_prefix_load     = "masked_load_align";
const std::string Mangler::mask_prefix_store    = "masked_store_align";
const std::string Mangler::prefix_gather        = "internal.gather";
const std::string Mangler::prefix_scatter       = "internal.scatter";
const std::string Mangler::name_allOne          = "allOne";
const std::string Mangler::name_allZero         = "allZero";
const std::string Mangler::fake_builtin_prefix  = "_f_v.";
const std::string Mangler::fake_prefix_extract  = "fake.extract.element";
const std::string Mangler::fake_prefix_insert   = "fake.insert.element";

static const char* getGatherScatterTypeName(Type *ElemTy) {
  if (ElemTy->isIntegerTy(8)) return "i8";
  if (ElemTy->isIntegerTy(16)) return "i16";
  if (ElemTy->isIntegerTy(32)) return "i32";
  if (ElemTy->isIntegerTy(64)) return "i64";
  if (ElemTy->isFloatTy()) return "f32";
  if (ElemTy->isDoubleTy()) return "f64";
  return "unknown";
}

template <class T>
inline std::string toString (const T& elem) {
  std::stringstream ss;
  ss << elem;
  return ss.str();
}

std::string Mangler::mangle(const std::string& name) {
  static unsigned int serial = 0;
  // Attach a serial number to each function decleration
  std::string suffix = toString(serial++);
  return mask_prefix_func+suffix+mask_delim+name;
}

std::string Mangler::getLoadName(unsigned align) {
  static unsigned int serial = 0;
  std::string suffix = toString(serial++);
  std::string alignStr = toString(align);
  return mask_prefix_load + alignStr + "_" + suffix;
}

std::string Mangler::getStoreName(unsigned align) {
  static unsigned int serial = 0;
  std::string suffix = toString(serial++);
  std::string alignStr = toString(align);
  return mask_prefix_store + alignStr + "_" + suffix;
}

std::string Mangler::getGatherScatterName(bool isMasked, bool isGather, VectorType *VecTy) {
  std::stringstream result;
  unsigned numElements = VecTy->getNumElements();
  const char* elemTyName = getGatherScatterTypeName(VecTy->getScalarType());
  // Format:
  // [masked_]gather|scatter.v16<i|f><32|64>

  if (isMasked)
    result << "masked_";
  result << (isGather ? "gather.v" : "scatter.v");
  result << numElements << elemTyName;
  return result.str();
}

std::string Mangler::getGatherScatterInternalName(bool isGather, Type *maskType, VectorType *retDataVecTy, Type *indexType) {
  std::stringstream result;
  unsigned numElements = retDataVecTy->getNumElements();
  const char* elemRetTyName = getGatherScatterTypeName(retDataVecTy->getScalarType());
  const char* elemIndexTyName = getGatherScatterTypeName(indexType->getScalarType());
  // Format:
  // internal.gather|scatter.v16<i|f><32|64>.i[<32|64>].m[1|16]

  result << (isGather ? prefix_gather : prefix_scatter);
  result << ".v" << numElements << elemRetTyName << "[" << elemIndexTyName << "].m";
  result << (isa<VectorType>(maskType) ? cast<VectorType>(maskType)->getNumElements() : 1);
  return result.str();
}

std::string Mangler::getFakeExtractName() {
  static unsigned int serial = 0;
  std::string suffix = toString(serial++);
  return fake_prefix_extract+suffix;
}

std::string Mangler::getFakeInsertName() {
  static unsigned int serial = 0;
  std::string suffix = toString(serial++);
  return fake_prefix_insert+suffix;
}


std::string Mangler::demangle(const std::string& name) {
  V_ASSERT(name.find(mask_prefix_func) != name.npos && "not a mangled function");
  // Format:
  // masked_83_function
  size_t start = name.find(mask_prefix_func)
    + std::string(mask_prefix_func).length() + 1;
  size_t orig = name.find(mask_delim, start);
  V_ASSERT(orig != std::string::npos && "unable to find original name");
  return name.substr(orig + 1);
}


bool Mangler::isMangledLoad(const std::string& name) {
  return name.find(mask_prefix_load) != std::string::npos;
}

bool Mangler::isMangledStore(const std::string& name) {
  return name.find(mask_prefix_store) != std::string::npos;
}

bool Mangler::isMangledCall(const std::string& name) {
  return name.find(mask_prefix_func) != std::string::npos;
}

bool Mangler::isMangledGather(const std::string& name) {
  return name.find(prefix_gather) != std::string::npos;
}

bool Mangler::isMangledScatter(const std::string& name) {
  return name.find(prefix_scatter) != std::string::npos;
}

bool Mangler::isFakeExtract(const std::string& name) {
  return name.find(fake_prefix_extract) != std::string::npos;
}

bool Mangler::isFakeInsert(const std::string& name) {
  return name.find(fake_prefix_insert) != std::string::npos;
}

bool Mangler::isFakeBuiltin(const std::string& name) {
  return name.find(fake_builtin_prefix) != std::string::npos;
}

std::string Mangler::getFakeBuiltinName(const std::string& name) {
  return fake_builtin_prefix+name;
}

unsigned Mangler::getMangledStoreAlignment(const std::string& name) {
  V_ASSERT(isMangledStore(name) && "not a mangled store");
  unsigned alignStart = name.find(mask_prefix_store) + mask_prefix_store.length();
  unsigned alignLen = name.find("_", alignStart) - alignStart;
  unsigned value = atoi(name.substr(alignStart, alignLen).c_str());
  return value;
}

unsigned Mangler::getMangledLoadAlignment(const std::string& name) {
  V_ASSERT(isMangledLoad(name) && "not a mangled store");
  unsigned alignStart = name.find(mask_prefix_load) + mask_prefix_load.length();
  unsigned alignLen = name.find("_", alignStart) - alignStart;
  unsigned value = atoi(name.substr(alignStart, alignLen).c_str());
  return value;
}

std::string Mangler::demangle_fake_builtin(const std::string& name) {
  V_ASSERT(isFakeBuiltin(name) && "not a mangled fake builtin function");
  // Format:
  // _f_v.function
  size_t start = name.find(fake_builtin_prefix);
  // when demangle_fake_builtin is called fake_builtin_pefix should be at start 
  V_ASSERT(start == 0);
  start += fake_builtin_prefix.length();
  return name.substr(start);
}

