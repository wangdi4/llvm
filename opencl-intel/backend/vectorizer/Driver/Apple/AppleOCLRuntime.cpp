/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "AppleOCLRuntime.h"
#include "Mangler.h"
#include "Logger.h"
#include "SpecialCaseFuncs.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Module.h"

#include <string.h>

namespace intel {


const char RTModuleStr [] = 
"\
\n\
; ModuleID = 'vectorizer_inner_rt.ll'\n\
target datalayout = \"e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a64:64:64-f80:128:128-n8:16:32\"\n\
target triple = \"i386-applecl-darwin11\"\n\
\n\
%struct._image2d_t = type opaque\n\
%struct._image3d_t = type opaque\n\
\n\
;----------------------------------------------------------------------\n\
;  geometric fake declaration\n\
;----------------------------------------------------------------------\n\
declare <3 x float> @_f_v._Z5crossDv3_fS_(<3 x float>, <3 x float>) nounwind readnone\n\
declare <4 x float> @_f_v._Z5crossDv4_fS_(<4 x float>, <4 x float>) nounwind readnone\n\
declare float @_f_v._Z13fast_distanceff(float, float) nounwind readnone\n\
declare float @_f_v._Z13fast_distanceDv2_fS_(<2 x float>, <2 x float>) nounwind readnone\n\
declare float @_f_v._Z13fast_distanceDv3_fS_(<3 x float>, <3 x float>) nounwind readnone\n\
declare float @_f_v._Z13fast_distanceDv4_fS_(<4 x float>, <4 x float>) nounwind readnone\n\
declare float @_f_v._Z8distanceff(float, float) nounwind readnone\n\
declare float @_f_v._Z6lengthf(float) nounwind readnone\n\
declare float @_f_v._Z8distanceDv2_fS_(<2 x float>, <2 x float>) nounwind readnone\n\
declare float @_f_v._Z6lengthDv2_f(<2 x float>) nounwind readnone\n\
declare float @_f_v._Z8distanceDv3_fS_(<3 x float>, <3 x float>) nounwind readnone\n\
declare float @_f_v._Z6lengthDv3_f(<3 x float>) nounwind readnone\n\
declare float @_f_v._Z8distanceDv4_fS_(<4 x float>, <4 x float>) nounwind readnone\n\
declare float @_f_v._Z6lengthDv4_f(<4 x float>) nounwind readnone\n\
declare float @_f_v._Z3dotff(float, float) nounwind readnone\n\
declare float @_f_v._Z3dotDv2_fS_(<2 x float>, <2 x float>) nounwind readnone\n\
declare float @_f_v._Z3dotDv3_fS_(<3 x float>, <3 x float>) nounwind readnone\n\
declare float @_f_v._Z3dotff4(<4 x float>, <4 x float>) nounwind readnone\n\
declare float @_f_v._Z11fast_lengthf(float) nounwind readnone\n\
declare float @_f_v._Z11fast_lengthDv2_f(<2 x float>) nounwind readnone\n\
declare float @_f_v._Z11fast_lengthDv3_f(<3 x float>) nounwind readnone\n\
declare float @_f_v._Z11fast_lengthDv4_f(<4 x float>) nounwind readnone\n\
declare float @_f_v._Z14fast_normalizef(float) nounwind readnone\n\
declare <2 x float> @_f_v._Z14fast_normalizeDv2_f(<2 x float>) nounwind readnone\n\
declare <3 x float> @_f_v._Z14fast_normalizeDv3_f(<3 x float>) nounwind readnone\n\
declare <4 x float> @_f_v._Z14fast_normalizef4(<4 x float>) nounwind readnone\n\
declare float @_f_v._Z9normalizef(float) nounwind readnone\n\
declare <2 x float> @_f_v._Z9normalizeDv2_f(<2 x float>) nounwind readnone\n\
declare <3 x float> @_f_v._Z9normalizeDv3_f(<3 x float>) nounwind readnone\n\
declare <4 x float> @_f_v._Z9normalizeDv4_f(<4 x float>) nounwind readnone\n\
\n\
\n\
\n\
;----------------------------------------------------------------------------------------------------------------------------\n\
; ci gamma\n\
;----------------------------------------------------------------------------------------------------------------------------\n\
\n\
declare <3 x float> @_f_v.__ci_gamma_scalar_SPI(<3 x float>, float %y) nounwind readnone\n\
declare [3 x <4 x float>] @_f_v.__ci_gamma_SPI(<4 x float>, <4 x float>, <4 x float>, <4 x float>) nounwind readnone\n\
declare [3 x <8 x float>] @_f_v.__ci_gamma_SPI_8(<8 x float>, <8 x float> , <8 x float> , <8 x float> %y) nounwind readnone\n\
\n\
\n\
;----------------------------------------------------------------------------------------------------------------------------\n\
; read write image\n\
;----------------------------------------------------------------------------------------------------------------------------\n\
declare <4 x float> @_f_v._Z11read_imagefPU3AS110_image2d_tuSamplerDv2_f(%struct._image2d_t addrspace(1)* %image, i32 %sampler, <2 x float>) nounwind\n\
declare <4 x float> @_f_v._Z11read_imagefPU3AS110_image3d_tuSamplerDv4_f(%struct._image3d_t addrspace(1)* %image, i32 %sampler, <3 x float>) nounwind\n\
declare void @_f_v._Z12write_imagefPU3AS110_image2d_tDv2_iDv4_f(%struct._image2d_t addrspace(1)* %image, i32 %x, i32 %y, <4 x float>) nounwind\n\
\n\
\n\
;----------------------------------------------------------------------------------------------------------------------------\n\
; fract\n\
;----------------------------------------------------------------------------------------------------------------------------\n\
declare float @_f_v._Z5fractfPf(float) nounwind readnone\n\
declare <4 x float> @_f_v._Z5fractDv2_fPS_(<2 x float>) nounwind readnone\n\
declare <4 x float> @_f_v._Z5fractDv3_fPS_(<3 x float>) nounwind readnone\n\
declare <4 x float> @_f_v._Z5fractDv4_fPS_(<4 x float>) nounwind readnone\n\
declare <4 x double> @_f_v._Z5fractDv8_fPS_(<8 x float>) nounwind readnone\n\
declare <16 x float> @_f_v._Z5fractDv16_fPS_(<16 x float>) nounwind readnone\n\
\n\
\0";

const char *appleNeedPreVectorization[] = {
  "_Z11read_imagefPU3AS110_image2d_tuSamplerDv2_f","_Z11read_imagefPU3AS110_image3d_tuSamplerDv4_f",
  "__ci_gamma_scalar_SPI",
  //TODO: geometric special cases, handle separately
/*
  "_Z5crossDv3_fS_","_Z5crossDv4_fS_",
  "_Z13fast_distanceff", "_Z13fast_distanceDv2_fS_", "_Z13fast_distanceDv3_fS_", "_Z13fast_distanceDv4_fS_",
  "_Z8distanceff", "_Z8distanceDv2_fS_", "_Z8distanceDv3_fS_", "_Z8distanceDv4_fS_",
  "_Z6lengthf", "_Z6lengthDv2_f", "_Z6lengthDv3_f", "_Z6lengthDv4_f",
  "_Z3dotff", "_Z3dotDv2_fS_", "_Z3dotDv3_fS_", "_Z3dotff4",
  "_Z11fast_lengthf", "_Z11fast_lengthDv2_f", "_Z11fast_lengthDv3_f", "_Z11fast_lengthDv4_f",
  "_Z14fast_normalizef", "_Z14fast_normalizeDv2_f", "_Z14fast_normalizeDv3_f", "_Z14fast_normalizef4",
  "_Z9normalizef", "_Z9normalizeDv2_f", "_Z9normalizeDv3_f", "_Z9normalizeDv4_f",
*/
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


const char *APPLE_WRITE_IMG_NAME = "_Z12write_imagefPU3AS110_image2d_tDv2_iDv4_f";
const char *APPLE_READ_IMG_NAME = "_Z11read_imagefPU3AS110_image2d_tuSamplerDv2_f";

// On volcano sets the __i386 manually
//TODO: Check whether we need to define __i386

const char *APPLE_STREAM_READ_IMG_NAME_32 = "_Z36__async_work_group_stream_from_imagePU3AS110_image2d_tuSamplerDv2_fS1_jPDv4_fS3_S3_S3_";
const char *APPLE_STREAM_READ_IMG_NAME_64 = "_Z36__async_work_group_stream_from_imagePU3AS110_image2d_tuSamplerDv2_fS1_mPDv4_fS3_S3_S3_";

const char *APPLE_STREAM_WRITE_IMG_NAME_32 = "_Z34__async_work_group_stream_to_imagePU3AS110_image2d_tjjjPKDv4_fS2_S2_S2_";
const char *APPLE_STREAM_WRITE_IMG_NAME_64 = "_Z34__async_work_group_stream_to_imagePU3AS110_image2d_tmmmPKDv4_fS2_S2_S2_";


AppleOpenclRuntime::AppleOpenclRuntime(const Module *runtimeModule):
    OpenclRuntime(runtimeModule, appleScalarSelect)    
{
  const char **preVecPtr = appleNeedPreVectorization;
  while (*preVecPtr) {
    m_needPreVectorizationSet.insert(*preVecPtr);
    ++preVecPtr;
  }

  MemoryBuffer *MB = MemoryBuffer::getMemBuffer(RTModuleStr);
  SMDiagnostic Err1;
  LLVMContext &Context1 = runtimeModule->getContext();
  m_innerRTModule = ParseIR(MB, Err1, Context1);

  std::string fakeReadImgName = Mangler::getFakeBuiltinName(APPLE_READ_IMG_NAME);
  m_readImageEntry = findBuiltinFunction(fakeReadImgName);

  std::string fakeWriteImgName = Mangler::getFakeBuiltinName(APPLE_WRITE_IMG_NAME);
  m_writeImageEntry = findBuiltinFunction(fakeWriteImgName);
}

AppleOpenclRuntime::~AppleOpenclRuntime() {}

Function * AppleOpenclRuntime::findInRuntimeModule(StringRef Name) const {
   Function *rtFunc = OpenclRuntime::findInRuntimeModule(Name);
   if (!rtFunc) rtFunc = m_innerRTModule->getFunction(Name);
   return rtFunc;
}

bool AppleOpenclRuntime::isStreamFunc(const std::string &funcName) const {
  return funcName.find("async_work_group_stream") != std::string::npos;
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

bool AppleOpenclRuntime::isTransposedReadImg(const std::string &func_name) const {
  // check only the soa entries of read_image
  for (unsigned i=1; i<6; ++i) {
    if (func_name.compare(m_readImageEntry->getVersion(i)) == 0) return true;
  }
  return false;
}

Function *AppleOpenclRuntime::getWriteStream(bool isPointer64Bit) const {
  if(isPointer64Bit) {
    return m_runtimeModule->getFunction(APPLE_STREAM_WRITE_IMG_NAME_64);
  } else {
    return m_runtimeModule->getFunction(APPLE_STREAM_WRITE_IMG_NAME_32);
  }
}

bool AppleOpenclRuntime::isTransposedWriteImg(const std::string &func_name) const {
  // check only the soa entries of write_image
  for (unsigned i=1; i<6; ++i) {
    if (func_name.compare(m_writeImageEntry->getVersion(i)) == 0) return true;
  }
  return false;
}

Function *AppleOpenclRuntime::getReadStream(bool isPointer64Bit) const {
  if(isPointer64Bit) {
    return m_runtimeModule->getFunction(APPLE_STREAM_READ_IMG_NAME_64);
  } else {
    return m_runtimeModule->getFunction(APPLE_STREAM_READ_IMG_NAME_32);
  }
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
