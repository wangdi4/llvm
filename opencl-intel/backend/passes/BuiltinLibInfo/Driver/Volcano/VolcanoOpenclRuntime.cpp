/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/

#include "VolcanoOpenclRuntime.h"
#include "Logger.h"

namespace intel {
  const char *volacanoScalarSelect[] = {
    "_Z6selectccc", "_Z6selectcch", "_Z6selecthhc", "_Z6selecthhh",
    "_Z6selectsss", "_Z6selectsst", "_Z6selecttts", "_Z6selectttt",
    "_Z6selectiii", "_Z6selectiij", "_Z6selectjji", "_Z6selectjjj",
    "_Z6selectlll", "_Z6selectllm", "_Z6selectmml", "_Z6selectmmm",
    "_Z6selectffi", "_Z6selectffj",
    "_Z6selectddl", "_Z6selectddm",
    NULL
  };
  
  bool VolcanoOpenclRuntime::needPreVectorizationFakeFunction(const std::string &funcName) const{
    return false;
  }
  
  bool VolcanoOpenclRuntime::isWriteImage(const std::string &funcName) const{
    return false;
  }
  
  bool VolcanoOpenclRuntime::isFakeWriteImage(const std::string &funcName) const{
    return false;
  }
  
  VolcanoOpenclRuntime::VolcanoOpenclRuntime(const Module *runtimeModule):
  OpenclRuntime(runtimeModule, volacanoScalarSelect)
  {
    
  }
  
  bool VolcanoOpenclRuntime::isTransposedReadImg(const std::string &func_name) const {
    return false;
  }
  
  Function *VolcanoOpenclRuntime::getWriteStream(bool isPointer64Bit) const {
    return NULL;
  }
  
  bool VolcanoOpenclRuntime::isTransposedWriteImg(const std::string &func_name) const {
    return false;
  }
  
  Function *VolcanoOpenclRuntime::getReadStream(bool isPointer64Bit) const {
    return NULL;
  }
  
  bool VolcanoOpenclRuntime::isStreamFunc(const std::string &funcName) const {
    return false;
  }
  
}


/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  intel::RuntimeServices* createVolcanoOpenclRuntimeSupport(const Module *runtimeModule) {
    return new intel::VolcanoOpenclRuntime(runtimeModule);
  }
}
