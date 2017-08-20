/*********************************************************************************************
 * Copyright © 2010-2012, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#include "DXRuntime.h"
#include "Mangler.h"
#include "Logger.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"

namespace intel {

DXRuntime::DXRuntime(
  SmallVector<Module*, 2> runtimeModuleList,
  unsigned packetWidth) :
  m_packetizationWidth(packetWidth),
  m_vfh(DXEntryDB) {
    m_runtimeModulesList = runtimeModuleList;
}

Function* DXRuntime::findInRuntimeModule(StringRef Name) const {
  for (SmallVector<Module*, 2>::const_iterator it = m_runtimeModulesList.begin();
    it != m_runtimeModulesList.end(); ++it)
  {
    Function* ret_function = (*it)->getFunction(Name);
    if (ret_function != nullptr)
        return ret_function;
  }
  return nullptr;
}

std::auto_ptr<VectorizerFunction>
DXRuntime::findBuiltinFunction(StringRef inp_name) const {
  std::string strName = inp_name.str();
  funcEntry fe = m_vfh.findFunctionInHash(strName);
  return std::auto_ptr<VectorizerFunction>(new funcEntry(fe));
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

bool DXRuntime::isSyncFunc(const std::string &func_name) const {
  // TODO: ComputeShader is defined with SYNC() function
  return false;
}

bool DXRuntime::isFakedFunction(StringRef fname) const {
   return false;
}

bool DXRuntime::hasNoSideEffect(const std::string &func_name) const {
  return true;
}

bool DXRuntime::isExpensiveCall(const std::string &func_name) const {
  return false;
}

bool DXRuntime::isMaskedFunctionCall(const std::string &func_name) const {
  return func_name.find("dx_soa") == 0;
}

} // Namespace

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  intel::RuntimeServices* createDXRuntimeSupport(SmallVector<Module*, 2> runtimeModuleList) {
    return new intel::DXRuntime(runtimeModuleList, 4);
  }
}
