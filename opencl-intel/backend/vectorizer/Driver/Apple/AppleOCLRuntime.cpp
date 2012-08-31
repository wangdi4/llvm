/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "AppleOCLRuntime.h"
#include "EnvAdapt.h"
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
  "_Z5crossDv3_fS_","_Z5crossDv4_fS_",
  "_Z13fast_distanceff", "_Z13fast_distanceDv2_fS_", "_Z13fast_distanceDv3_fS_", "_Z13fast_distanceDv4_fS_",
  "_Z8distanceff", "_Z8distanceDv2_fS_", "_Z8distanceDv3_fS_", "_Z8distanceDv4_fS_",
  "_Z6lengthf", "_Z6lengthDv2_f", "_Z6lengthDv3_f", "_Z6lengthDv4_f",
  "_Z3dotff", "_Z3dotDv2_fS_", "_Z3dotDv3_fS_", "_Z3dotff4",
  "_Z11fast_lengthf", "_Z11fast_lengthDv2_f", "_Z11fast_lengthDv3_f", "_Z11fast_lengthDv4_f",
  "_Z14fast_normalizef", "_Z14fast_normalizeDv2_f", "_Z14fast_normalizeDv3_f", "_Z14fast_normalizef4",
  "_Z9normalizef", "_Z9normalizeDv2_f", "_Z9normalizeDv3_f", "_Z9normalizeDv4_f",
  NULL
};

const char *appleScalarSelect[] = {
  "_Z6selectccc", "_Z6selectsss", "_Z6selectiii", "_Z6selectlll",
  "_Z6selectcch", "_Z6selectsst", "_Z6selectiij", "_Z6selectllm",
  "_Z6selecthhc", "_Z6selecttts", "_Z6selectjji", "_Z6selectmml",
  "_Z6selecthhh", "_Z6selectttt", "_Z6selectjjj", "_Z6selectmmm",
  "_Z6selectffi", "_Z6selectffj",
  "_Z6selectddl", "_Z6selectddm",
  NULL
};


const char *APPLE_WRITE_IMG_NAME = "__write_imagef_2d";
const char *APPLE_READ_IMG_NAME = "_Z11read_imagefPU3AS110_image2d_tuSamplerDv2_f";

// On volcano sets the __i386 manually
#if VOLCANO_ENV 
//#define __i386 1
#endif 

#if defined(__i386)
const char *APPLE_STREAM_READ_IMG_NAME = "_Z36__async_work_group_stream_from_imagePU3AS110_image2d_tuSamplerDv2_fS1_jPDv4_fS3_S3_S3_";
#else
const char *APPLE_STREAM_READ_IMG_NAME = "_Z36__async_work_group_stream_from_imagePU3AS110_image2d_tuSamplerDv2_fS1_mPDv4_fS3_S3_S3_";
#endif

#if defined(__i386)
const char *APPLE_STREAM_WRITE_IMG_NAME = "_Z34__async_work_group_stream_to_imagePU3AS110_image2d_tjjjPKDv4_fS2_S2_S2_";
#else
const char *APPLE_STREAM_WRITE_IMG_NAME = "_Z34__async_work_group_stream_to_imagePU3AS110_image2d_tmmmPKDv4_fS2_S2_S2_";
#endif



AppleOpenclRuntime::AppleOpenclRuntime(const Module *runtimeModule):
    OpenclRuntime(runtimeModule, AppleOCLEntryDB, appleScalarSelect)
{ 
  const char **preVecPtr = appleNeedPreVectorization;
  while (*preVecPtr) {
    m_needPreVectorizationSet.insert(*preVecPtr);
    ++preVecPtr;
  }

  std::string fakeReadImgName = Mangler::getFakeBuiltinName(APPLE_READ_IMG_NAME);
  funcEntry readImgFuncEntry= findBuiltinFunction(fakeReadImgName);
  m_readImageEntry = readImgFuncEntry.first;

  std::string fakeWriteImgName = Mangler::getFakeBuiltinName(APPLE_WRITE_IMG_NAME);
  funcEntry writeImgFuncEntry= findBuiltinFunction(fakeWriteImgName);
  m_writeImageEntry = writeImgFuncEntry.first;
}

unsigned AppleOpenclRuntime::getNumJitDimensions() const {
  return 1;
}

const char *AppleOpenclRuntime::getBaseGIDName() const {
  return GET_GID_NAME;
}

bool AppleOpenclRuntime::needPreVectorizationFakeFunction(const std::string &funcName) const{
  if(m_needPreVectorizationSet.count(funcName)) return true;
  return false;
}

bool AppleOpenclRuntime::isWriteImage(const std::string &funcName) const{
  // check that function has select builtin prefix
  if (funcName.compare(APPLE_WRITE_IMG_NAME) != 0) return false;
  return true;
}

bool AppleOpenclRuntime::isFakeWriteImage(const std::string &funcName) const{
  if (Mangler::isFakeBuiltin(funcName) ) {
    std::string resolvedName = Mangler::demangle_fake_builtin(funcName);
    if (resolvedName.compare(APPLE_WRITE_IMG_NAME) == 0) 
      return true;
  }
  return false;
}

unsigned AppleOpenclRuntime::isInlineDot(const std::string &funcName) const{
  return 0;
}

bool AppleOpenclRuntime::isTransposedReadImg(const std::string &func_name) const {
  // check only the soa entries of read_image
  for (unsigned i=1; i<6; ++i) {
    if (func_name.compare(m_readImageEntry->funcs[i]) == 0) return true;
  }
  return false;
}

Function *AppleOpenclRuntime::getWriteStream() const {
  return m_runtimeModule->getFunction(APPLE_STREAM_WRITE_IMG_NAME);
}

bool AppleOpenclRuntime::isTransposedWriteImg(const std::string &func_name) const {
  // check only the soa entries of write_image
  for (unsigned i=1; i<6; ++i) {
    if (func_name.compare(m_writeImageEntry->funcs[i]) == 0) return true;
  }
  return false;
}

Function *AppleOpenclRuntime::getReadStream() const {
  return m_runtimeModule->getFunction(APPLE_STREAM_READ_IMG_NAME);
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
