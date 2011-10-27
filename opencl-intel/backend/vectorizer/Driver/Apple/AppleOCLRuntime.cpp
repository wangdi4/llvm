/*********************************************************************************************
 * Copyright � 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "AppleOCLRuntime.h"
#include "Mangler.h"
#include "Logger.h"
#include "SpecialCaseFuncs.h"
#include <string.h>



namespace intel {

const char *appleNeedPreVectorization[] = {
  // readimage
  "read_2d_ff","read_3d_ff",
  // ci_gamma
  "__ci_gamma_scalar_SPI",
  // geometric special case
  "__crossf3","__crossf4",
  "__fast_distancef", "__fast_distancef2", "__fast_distancef3", "__fast_distancef4",
  "__distancef", "__distancef2", "__distancef3", "__distancef4",
  "__lengthf", "__lengthf2", "__lengthf3", "__lengthf4",
  "__dotf", "__dotf2", "__dotf3", "__dotf4",
  "__fast_lengthf", "__fast_lengthf2", "__fast_lengthf3", "__fast_lengthf4",
  "__fast_normalizef", "__fast_normalizef2", "__fast_normalizef3", "__fast_normalizef4",
  "__normalizef", "__normalizef2", "__normalizef3", "__normalizef4",
  NULL
};

const char *appleScalarSelect[] = {
  "__select_1i8", "__select_1i16", "__select_1i32", "__select_1i64",
  "__select_1i8u", "__select_1i16u", "__select_1i32u", "__select_1i64u",
  "__select_1u8", "__select_1u16", "__select_1u32", "__select_1u64",
  "__select_1u8u", "__select_1u16u", "__select_1u32u", "__select_1u64u",
  "__select_ffi", "__select_ffu",
  "__select_ddi", "__select_ddu",
  NULL
};


const char *APPLE_WRITE_IMG_NAME = "__write_imagef_2d";


AppleOpenclRuntime::AppleOpenclRuntime(const Module *runtimeModule):
    OpenclRuntime(runtimeModule, AppleOCLEntryDB, appleScalarSelect)
{ 
  const char **preVecPtr = appleNeedPreVectorization;
  while (*preVecPtr) {
    m_needPreVectorizationSet.insert(*preVecPtr);
    ++preVecPtr;
  }
}

bool AppleOpenclRuntime::needPreVectorizationFakeFunction(std::string &funcName) const{
  if(m_needPreVectorizationSet.count(funcName)) return true;
  return false;
}

bool AppleOpenclRuntime::hasNoSideEffect(std::string &func_name) const {
  Function *func = findInRuntimeModule(func_name);
  // if function is not in runtime module we dont want to say anything about it
  if (!func) return false;
  // if function says it does not access memory than it has no side effects
  if (func->doesNotAccessMemory()) return true;
  // if function has no pointers arguments than it is also known not to have side effects
  bool hasNoPointerArg = true;
  const FunctionType *fType = func->getFunctionType();
  unsigned nParams = fType->getNumParams();
  for (unsigned i=0; i<nParams && hasNoPointerArg; ++i)
  {
    hasNoPointerArg &= !(fType->getParamType(i)->isPointerTy());
  }
  if (hasNoPointerArg) return true;
  // possible side effects
  return false;
}

bool AppleOpenclRuntime::isWriteImage(std::string &funcName) const{
  // check that function has select builtin prefix
  if (funcName.compare(APPLE_WRITE_IMG_NAME) != 0) return false;
  return true;
}

bool AppleOpenclRuntime::isFakeWriteImage(std::string &funcName) const{
  if (Mangler::isFakeBuiltin(funcName) ) {
	std::string resolvedName = Mangler::demangle_fake_builtin(funcName);
	if (resolvedName.compare(APPLE_WRITE_IMG_NAME) == 0) 
      return true;
  }
  return false;
}

unsigned AppleOpenclRuntime::isInlineDot(std::string &funcName) const{
  return 0;
}


} // Namespace


/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
    void* createAppleOpenclRuntimeSupport(const Module *runtimeModule) {
    V_ASSERT(NULL == intel::RuntimeServices::get() && "Trying to re-create singleton!");
    intel::AppleOpenclRuntime * rt = 
      new intel::AppleOpenclRuntime(runtimeModule);
    intel::RuntimeServices::set(rt);
    return (void*)(rt);
  }

  void* destroyAppleOpenclRuntimeSupport() {
	  delete intel::RuntimeServices::get();
	  intel::RuntimeServices::set(0);
	  return 0;
  }
}