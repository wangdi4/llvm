/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "Mangler.h"
#include "Logger.h"

//name mangled related
#include "NameMangleAPI.h"
#include "FunctionDescriptor.h"
#include "ParameterType.h"

#include <stdlib.h>
#include <cassert>
#include <sstream>

const std::string Mangler::mask_delim           = "_";
const std::string Mangler::mask_prefix_func     = "maskedf_";
const std::string Mangler::mask_prefix_load     = "masked_load_align";
const std::string Mangler::mask_prefix_store    = "masked_store_align";
const std::string Mangler::prefix_gather        = "internal.gather";
const std::string Mangler::prefix_scatter       = "internal.scatter";
const std::string Mangler::prefix_gather_prefetch = "internal.prefetch.gather";
const std::string Mangler::prefix_scatter_prefetch = "internal.prefetch.scatter";
const std::string Mangler::prefetch             = "prefetch";
const std::string Mangler::name_allOne          = "__ocl_allOne";
const std::string Mangler::name_allZero         = "__ocl_allZero";
const std::string Mangler::fake_builtin_prefix  = "_f_v.";
const std::string Mangler::retbyarray_builtin_prefix  = "__retbyarray_";
const std::string Mangler::retbyvector_builtin_prefix  = "__retbyvector_";
const std::string Mangler::fake_prefix_extract  = "fake.extract.element";
const std::string Mangler::fake_prefix_insert   = "fake.insert.element";


llvm::StringRef imageFunctions[] = {
    "read_imagei",
    "read_imageui",
    "write_imagei",
    "write_imageui"
};

unsigned IMG_SIZE = sizeof(imageFunctions)/sizeof(imageFunctions[0]);

const char* IMG_MASK_PREFIX = "mask_";

static const char* getScalarTypeName(Type *ElemTy) {
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

bool is2DImage(reflection::TypePrimitiveEnum image)
{
  return (
    image == reflection::PRIMITIVE_IMAGE_2D_T ||
    image == reflection::PRIMITIVE_IMAGE_2D_RO_T ||
    image == reflection::PRIMITIVE_IMAGE_2D_WO_T ||
    image == reflection::PRIMITIVE_IMAGE_2D_RW_T);
}

std::string Mangler::mangle(const std::string& name) {
  if (::isMangledName(name.c_str())){
    llvm::StringRef stripped = stripName(name.c_str());
    for (unsigned i=0 ; i<IMG_SIZE ; ++i){
      if (imageFunctions[i] == stripped){
        reflection::FunctionDescriptor fdesc = ::demangle(name.c_str());
        V_ASSERT(!fdesc.isNull() && "demangle operation failed!");
        //Currently, we only support two dimension images masked built-ins.
        reflection::PrimitiveType *pTy = reflection::dyn_cast<reflection::PrimitiveType>(fdesc.parameters[0]);
        bool isImage2d = pTy && is2DImage(pTy->getPrimitive());
        if (!isImage2d) {
          //Do not have implementation for masked built-ins, break to use create fake mask function.
          break;
        }
        fdesc.name = IMG_MASK_PREFIX + fdesc.name;
        reflection::TypeVector& params = fdesc.parameters;
        reflection::RefParamType intType(new reflection::PrimitiveType(reflection::PRIMITIVE_INT));
        params.insert(params.begin(), intType);
        return ::mangle(fdesc);
      }
    }
  }
  // Attach a serial number to each function declaration
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

std::string Mangler::getGatherScatterName(bool isMasked, GatherScatterType gatherType,
                                          VectorType *DataTy, VectorType *IndexTy) {
  std::stringstream result;
  unsigned numElements = DataTy->getNumElements();
  const char* elemTyName = getScalarTypeName(DataTy->getScalarType());
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
  if (IndexTy) {
    result << "_ind_v" << IndexTy->getNumElements() <<
      getScalarTypeName(IndexTy->getScalarType());
  }
  return result.str();
}

std::string Mangler::getGatherScatterInternalName(GatherScatterType gatherType, Type *maskType, VectorType *retDataVecTy, Type *indexType) {
  std::stringstream result;
  unsigned numElements = retDataVecTy->getNumElements();
  const char* elemRetTyName = getScalarTypeName(retDataVecTy->getScalarType());
  const char* elemIndexTyName = getScalarTypeName(indexType->getScalarType());
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
  V_ASSERT(reflection::dyn_cast<reflection::PointerType>(prefetchDesc.parameters[0])
    && "First argument of prefetch built-in is expected to a pointer.");
  reflection::PointerType* pPtrTy =
    reflection::dyn_cast<reflection::PointerType>(prefetchDesc.parameters[0]);
  assert (pPtrTy && "not a pointer");
  reflection::RefParamType scalarType = pPtrTy->getPointee();
  V_ASSERT(scalarType->getTypeId() == reflection::PrimitiveType::enumTy && "Primitive type is expected.");
  // create vectorized data type for packed prefetch
  reflection::VectorType *vectorizedType = new reflection::VectorType(scalarType, packetWidth);
  reflection::PointerType* vectorizedPtr = new reflection::PointerType(vectorizedType);
  prefetchDesc.parameters[0] = vectorizedPtr;
  return ::mangle(prefetchDesc);
}

std::string Mangler::demangle(const std::string& name, bool masked) {
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
    if (! masked)
      return stripped;
  }

  if (! masked)
    return name;

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

bool Mangler::isAllZero(const std::string& name) {
  return name.find(Mangler::name_allZero) == 0;
}

bool Mangler::isAllOne(const std::string& name) {
  return name.find(Mangler::name_allOne) == 0;
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

bool Mangler::isRetByVectorBuiltin(const std::string& name) {
  reflection::FunctionDescriptor desc = ::demangle(name.c_str());
  if(desc.isNull()) return false;
  return desc.name.find(retbyvector_builtin_prefix) != std::string::npos;
}

std::string Mangler::getRetByArrayBuiltinName(const std::string& name) {
  reflection::FunctionDescriptor ret = ::demangle(name.c_str());
  V_ASSERT(ret.parameters.size() == 2 && "Expected exactly two arguments");
  // Remove the pointer argument
  ret.parameters.resize(1);
  // Create the name of the builtin function we will be replacing with.
  // If the orginal function was scalar, use the same function that will
  // planted by the Scalarizer
  ret.name = reflection::dyn_cast<reflection::VectorType>(ret.parameters[0]) ?
    retbyarray_builtin_prefix + ret.name :
    retbyvector_builtin_prefix + ret.name;
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

std::string Mangler::get_original_scalar_name_from_retbyvector_builtin(const std::string& name) {
  V_ASSERT(isRetByVectorBuiltin(name) && "not a mangled ret-by-vector builtin function");
  reflection::FunctionDescriptor desc = ::demangle(name.c_str());
  //Add second operand that is pointer of first operand type
  //It is always pointer to address space 0 (should not really matter)!
  desc.parameters.push_back(new reflection::PointerType(desc.parameters[0]));
  //Fix the name
  size_t start = desc.name.find(retbyvector_builtin_prefix);
  // when get_original_scalar_name_from_retbyvector_builtin
  // is called retbyvector_builtin_prefix should be at start
  V_ASSERT(start == 0);
  start += retbyvector_builtin_prefix.length();
  desc.name = desc.name.substr(start);
  return ::mangle(desc);
}

std::string Mangler::getTransposeBuiltinName(bool isLoad, bool isScatterGather, bool isMasked,
                          VectorType * origVecType, unsigned int packetWidth) {

  // Is this a masked operation?
  std::string maskedName = "";
  if (isMasked)
    maskedName = "masked_";

  // Determine load or store
  std::string baseFuncName = "unknown";
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
  std::string typeName = "unknown";
  if (origVecType->getScalarSizeInBits() == 8) {
    typeName = "char";
  } else if ((origVecType->getScalarSizeInBits() == 16) && origVecType->getElementType()->isIntegerTy()) {
    typeName = "short";
  } else if ((origVecType->getScalarSizeInBits() == 32) && origVecType->getElementType()->isIntegerTy()) {
    typeName = "int";
  } else if ((origVecType->getScalarSizeInBits() == 32) && origVecType->getElementType()->isFloatTy()) {
    typeName = "float";
  }

  std::stringstream funcName;
  funcName << "__ocl_" << maskedName << baseFuncName << typeName << "_" << origVecType->getNumElements() << "x" << packetWidth;

  return funcName.str();
}

std::string Mangler::getMaskedLoadStoreBuiltinName(bool isLoad, VectorType * vecType, bool isBitMask) {

  // Determine load or store
  std::string baseFuncName;
  if (isLoad) {
    baseFuncName = "load_";
  } else { // isStore
    baseFuncName = "store_";
  }

  // Determine vector element type
  std::string typeName = "unknown";
  if (vecType->getScalarSizeInBits() == 8) {
    typeName = "char";
  } else if ((vecType->getScalarSizeInBits() == 16) && vecType->getElementType()->isIntegerTy()) {
    typeName = "short";
  } else if ((vecType->getScalarSizeInBits() == 32) && vecType->getElementType()->isIntegerTy()) {
    typeName = "int";
  } else if ((vecType->getScalarSizeInBits() == 64) && vecType->getElementType()->isIntegerTy()) {
    typeName = "long";
  } else if (vecType->getElementType()->isFloatTy()) {
    typeName = "float";
  } else if (vecType->getElementType()->isDoubleTy()) {
    typeName = "double";
  }

  std::stringstream funcName;
  std::string prefix = isBitMask ? "__ocl_imasked_" : "__ocl_masked_";
  funcName << prefix << baseFuncName << typeName << vecType->getNumElements();

  return funcName.str();
}
