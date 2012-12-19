#include "Mangler.h"
#include "Logger.h"
#include <stdlib.h>

#include <cassert>
#include <sstream>

//name mangled related
#include "NameMangleAPI.h"
#include "FunctionDescriptor.h"
#include "Type.h"
#include "TypeCast.h"

const std::string Mangler::mask_delim           = "_";
const std::string Mangler::mask_prefix_func     = "maskedf_";
const std::string Mangler::mask_prefix_load     = "masked_load_align";
const std::string Mangler::mask_prefix_store    = "masked_store_align";
const std::string Mangler::prefix_gather        = "internal.gather";
const std::string Mangler::prefix_scatter       = "internal.scatter";
const std::string Mangler::prefix_gather_prefetch = "internal.prefetch.gather";
const std::string Mangler::prefix_scatter_prefetch = "internal.prefetch.scatter";
const std::string Mangler::prefetch             = "prefetch";
const std::string Mangler::name_allOne          = "allOne";
const std::string Mangler::name_allZero         = "allZero";
const std::string Mangler::fake_builtin_prefix  = "_f_v.";
const std::string Mangler::retbyarray_builtin_prefix  = "_retbyarray_";
const std::string Mangler::fake_prefix_extract  = "fake.extract.element";
const std::string Mangler::fake_prefix_insert   = "fake.insert.element";

llvm::StringRef imageFunctions[] = {
    "read_imagei",
    "read_imageui",
    "write_imagei",
    "write_imageui"
};

const char* IMG_MASK_PREFIX = "mask_";

unsigned IMG_SIZE = sizeof(imageFunctions)/sizeof(imageFunctions[0]);

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
  if (::isMangledName(name.c_str())){
    llvm::StringRef stripped = stripName(name.c_str()); 
    for (unsigned i=0 ; i<IMG_SIZE ; ++i){
      if (imageFunctions[i] == stripped){
        reflection::FunctionDescriptor fdesc = ::demangle(name.c_str());
        //Currently, we only support two dimension images to be masked.
        if ( fdesc.parameters[0]->toString() != "image2d_t" )
          return name;
        fdesc.name = IMG_MASK_PREFIX + fdesc.name;
        std::vector<reflection::Type*>& params = fdesc.parameters;
        reflection::Type intType(reflection::primitives::INT);
        params.insert(params.begin(), &intType);
        return ::mangle(fdesc);
      }
    }
  }
  // Attach a serial number to each function decleration
  static unsigned int serial = 0;
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

std::string Mangler::getGatherScatterName(bool isMasked, GatherScatterType gatherType, VectorType *VecTy) {
  std::stringstream result;
  unsigned numElements = VecTy->getNumElements();
  const char* elemTyName = getGatherScatterTypeName(VecTy->getScalarType());
  // Format:
  // [masked_]gather|scatter.v16<i|f><32|64>

  if (isMasked)
    result << "masked_";

  const char *type = 0;
  
  switch (gatherType) {
  case Gather: 
    type = "gather.v";
    break;
  case Scatter:
    type = "scatter.v";
    break;
  case GatherPrefetch:
    type = "gatherpf.v";
    break;
  case ScatterPrefetch:
    type = "scatterpf.v";
    break;
  default:
    V_ASSERT(false && "Invalid GatherScatter type");
  };

  result << type;
  result << numElements << elemTyName;
  return result.str();
}

std::string Mangler::getGatherScatterInternalName(GatherScatterType gatherType, Type *maskType, VectorType *retDataVecTy, Type *indexType) {
  std::stringstream result;
  unsigned numElements = retDataVecTy->getNumElements();
  const char* elemRetTyName = getGatherScatterTypeName(retDataVecTy->getScalarType());
  const char* elemIndexTyName = getGatherScatterTypeName(indexType->getScalarType());
  // Format:
  // internal.gather|scatter.v16<i|f><32|64>.i[<32|64>].m[1|16]

  const char *prefix = 0;

  switch (gatherType) {
  case Gather: 
    prefix = prefix_gather.c_str();
    break;
  case Scatter:
    prefix = prefix_scatter.c_str();
    break;
  case GatherPrefetch:
    prefix = prefix_gather_prefetch.c_str();
    break;
  case ScatterPrefetch:
    prefix = prefix_scatter_prefetch.c_str();
    break;
  default:
    V_ASSERT(false && "Invalid GatherScatter type");
  };
  
  result << prefix;
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

std::string Mangler::getVectorizedPrefetchName(const std::string& name, int packetWidth) {
  std::string mangledName = name;
  // First remove masked function prefix.
  if (isMangledCall(mangledName))
    mangledName = demangle(mangledName);

  reflection::FunctionDescriptor prefetchDesc = ::demangle(mangledName.c_str());
  // First argument of prefetch built-in must be pointer.
  V_ASSERT(reflection::cast<reflection::Pointer>(prefetchDesc.parameters[0]) && "First argument of prefetch built-in is expected to a pointer.");
  const reflection::Type* scalarType = static_cast<reflection::Pointer*>(prefetchDesc.parameters[0])->getPointee();
  V_ASSERT(scalarType->isPrimiteTy() && "Primitive type is expected.");
  // create vectorized data type for packed prefetch
  reflection::Vector vectorizedType(scalarType, packetWidth);
  reflection::Pointer vectorizedPrt(&vectorizedType);
  prefetchDesc.parameters[0] = &vectorizedPrt;
  return ::mangle(prefetchDesc);
}

std::string Mangler::demangle(const std::string& name) {
  if (::isMangledName(name.c_str())){
    llvm::StringRef stripped = stripName(name.c_str());
    if (stripped.startswith(IMG_MASK_PREFIX)){
      for (unsigned i=0 ; i<IMG_SIZE ; ++i){
        //in the case of image functions, the name should be unchanged, since
        //masked functions are 'real functions' to be looked in the reflection module
        if (stripped.endswith(imageFunctions[i]))
          return name;
      }
    }
  }
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

bool Mangler::isMangeledGatherPrefetch(const std::string& name) {
  return name.find(prefix_gather_prefetch) != std::string::npos;
}

bool Mangler::isMangledPrefetch(const std::string& name) {
  std::string mangledName = name;
  // First remove masked function prefix.
  if (isMangledCall(mangledName))
    mangledName = demangle(mangledName);

  reflection::FunctionDescriptor fdesc = ::demangle(mangledName.c_str());
  return fdesc.name == prefetch;
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

std::string Mangler::getRetByArrayBuiltinName(const std::string& name) {
  reflection::FunctionDescriptor ret = ::demangle(name.c_str());
  V_ASSERT(ret.parameters.size() == 2 && "Expected exactly two arguments");
  // Remove the pointer argument
  ret.parameters.resize(1);
  // Create the name of the builtin function we will be replacing with.
  // If the orginal function was scalar, use the same function that will
  // planted by the Scalarizer
  ret.name = reflection::cast<reflection::Vector>(ret.parameters[0]) ?
    retbyarray_builtin_prefix+ret.name :
    ret.name + "_scalarized";
  return ::mangle(ret);
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

std::string Mangler::getTransposeBuiltinName(bool isLoad, bool isScatterGather, bool isMasked,
                          VectorType * origVecType, unsigned int packetWidth) {

  // Is this a masked operation?
  std::string maskedName = "";
  if (isMasked)
    maskedName = "masked_";

  // Determine load or store
  std::string baseFuncName;
  if (isLoad) {
    if (isScatterGather)
      baseFuncName = "gather_transpose_";
    else
      baseFuncName = "load_transpose_";
  } else { // isStore
    if (isScatterGather)
      baseFuncName = "transpose_scatter_";
    else
      baseFuncName = "transpose_store_";
  }

  // Determine vector element type
  std::string typeName;
  if (origVecType->getScalarSizeInBits() == 8) {
    typeName = "char";
  } else if ((origVecType->getScalarSizeInBits() == 32) && origVecType->getElementType()->isIntegerTy()) {
    typeName = "int";
  } else if ((origVecType->getScalarSizeInBits() == 32) && origVecType->getElementType()->isFloatTy()) {
    typeName = "float";
  }

  std::stringstream funcName;
  funcName << maskedName << baseFuncName << typeName << origVecType->getNumElements() << "x" << packetWidth;

  return funcName.str();
}

