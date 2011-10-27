/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "DXRuntime.h"
#include "Mangler.h"
#include "Logger.h"

namespace intel {

Function* DXRuntime::findInRuntimeModule(StringRef Name) const {
   return m_runtimeModule->getFunction(Name);
}

RuntimeServices::funcEntry
DXRuntime::findBuiltinFunction(std::string &inp_name) const {
    return m_vfh.findFunctionInHash(inp_name);
}

bool DXRuntime::orderedWI() const { return false; }

bool DXRuntime::isTIDGenerator(const Instruction * inst, bool * err, unsigned *dim) const {
  return false;// Everything is a TID generator.
}

unsigned DXRuntime::getPacketizationWidth() const {
  return m_packetizationWidth;
}

void DXRuntime::setPacketizationWidth(unsigned width) {
  m_packetizationWidth = width;
}

bool DXRuntime::isKnownUniformFunc(std::string &func_name) const {
  return false;
}

bool DXRuntime::isSyncFunc(const std::string &func_name) const {
  // TODO: ComputeShader is defined with SYNC() function
  return false;
}

bool DXRuntime::hasNoSideEffect(std::string &func_name) const{
  return true;
}

bool DXRuntime::isMaskedFunctionCall(std::string &func_name) const{
  return func_name.find("dx_soa") == 0;
}

} // Namespace

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  void* createDXRuntimeSupport(const Module *runtimeModule,
    unsigned packetizationWidth)
  {
    V_ASSERT(NULL == intel::RuntimeServices::get() && "Trying to re-create singleton!");
    intel::DXRuntime * rt =
      new intel::DXRuntime(runtimeModule, packetizationWidth);
    intel::RuntimeServices::set(rt);
    return (void*)(rt);
  }
}
