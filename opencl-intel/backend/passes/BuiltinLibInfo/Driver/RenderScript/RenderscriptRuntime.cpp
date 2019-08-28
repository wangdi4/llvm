// INTEL CONFIDENTIAL
//
// Copyright 2011-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "DriverVectorizerFunction.h"
#include "RenderscriptRuntime.h"
#include "VectorizerFunction.h"
#include "BuiltinKeeper.h"
#include "NameMangleAPI.h"
#include "Mangler.h"
#include "Logger.h"

#include "llvm/IR/Constants.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"

using namespace reflection;

namespace intel {
//
//VectorizerFunctionBridge
//Purpose: a bridg of interfaces
//
class RenderscriptVFunction: public DriverVectorizerFunction{
public:
  RenderscriptVFunction(const std::string& s):DriverVectorizerFunction(s) {}

  ~RenderscriptVFunction() { }
};

struct dotProdInlineData {
  const char *name;
  unsigned opWidth;
};

const dotProdInlineData g_rs_dotInlineTable [] = {
  {"_Z3dotff",1},
  {"_Z3dotdd",1},
  {"_Z3dotDv2_fS_",2},
  {"_Z3dotDv2_dS_",2},
  {"_Z3dotDv3_fS_",3},
  {"_Z3dotDv3_dS_",3},
  {"_Z3dotDv4_fS_",4},
  {"_Z3dotDv4_dS_",4},
  {nullptr,0}
};
  
const char* g_rs_BuiltinReturnByPtr[] = {
  "fract",
  "modf",
  "sincos",
};
const size_t g_rs_BuiltinReturnByPtrLength = sizeof(g_rs_BuiltinReturnByPtr) / sizeof(g_rs_BuiltinReturnByPtr[0]);
  
// TODO[MA]: need to modify the content to match RS
const char *g_rs_scalarSelects[] = {
  nullptr
};

/// @brief Constructor which get arbitraty table as input
RenderscriptRuntime::RenderscriptRuntime(SmallVector<Module*, 2> runtimeModulesList) :
m_packetizationWidth(0) {
  m_runtimeModulesList = runtimeModulesList;
  initScalarSelectSet(g_rs_scalarSelects);
  initDotMap();
}

void RenderscriptRuntime::initScalarSelectSet(const char **scalarSelects) {
  while (*scalarSelects) {
    m_scalarSelectSet.insert(*scalarSelects);
    ++scalarSelects;
  }
}
  
void RenderscriptRuntime::initDotMap() {
  const dotProdInlineData *entryPtr = g_rs_dotInlineTable;
  while (entryPtr->name) {
    m_dotOpWidth[entryPtr->name] = entryPtr->opWidth;
    ++entryPtr;
  }
}

Function * RenderscriptRuntime::findInRuntimeModule(StringRef Name) const {
  for (SmallVector<Module*, 2>::const_iterator it = m_runtimeModulesList.begin();
      it != m_runtimeModulesList.end();
      ++it)
  {
    Function* ret_function = (*it)->getFunction(Name);
    if (ret_function != nullptr)
      return ret_function;
  }
  return nullptr;
}

bool RenderscriptRuntime::needPreVectorizationFakeFunction(const std::string &funcName) const{
  return false;
}

bool RenderscriptRuntime::isWriteImage(const std::string &funcName) const {
  return false;
}

bool RenderscriptRuntime::isFakeWriteImage(const std::string &funcName) const {
  return false;
}

bool RenderscriptRuntime::isTransposedReadImg(const std::string &func_name) const {
  return false;
}

bool RenderscriptRuntime::isTransposedWriteImg(const std::string &func_name) const {
  return false;
}

Function *RenderscriptRuntime::getReadStream() const {
  return nullptr;
}

Function *RenderscriptRuntime::getWriteStream() const {
  return nullptr;
}

bool RenderscriptRuntime::isStreamFunc(const std::string &funcName) const {
  return false;
}

std::auto_ptr<VectorizerFunction>
RenderscriptRuntime::findBuiltinFunction(StringRef mangledName) const {
  std::auto_ptr<VectorizerFunction> ret(new RenderscriptVFunction(mangledName));
  return ret;
}

bool RenderscriptRuntime::orderedWI() const { return true; }

bool RenderscriptRuntime::isTIDGenerator(const Instruction * inst, bool * err, unsigned *dim) const {
  *err = false; // By default, no error expected..
  const CallInst * CI = dyn_cast<CallInst>(inst);
  if (!CI) return false; // ID generator is a function call.
  assert(CI->getCalledFunction() && "Unexpected indirect function invocation");
  std::string funcName = CI->getCalledFunction()->getName().str();

  if (Mangler::isMangledCall(funcName)) {
    funcName = Mangler::demangle(funcName); // Remove mangling (masking) prefix
  }

  if (funcName != GET_ID_NAME) return false;

  // Report the dimention of the request (in rs it's always 1 dim)
  *dim = 0;

  // This is indeed a TID generator
  return true;
}

unsigned RenderscriptRuntime::getPacketizationWidth() const {
  return m_packetizationWidth;
}

unsigned RenderscriptRuntime::getNumJitDimensions() const {
  return 1;
}

const char *RenderscriptRuntime::getBaseGIDName() const {
  return "_INVALID._INVALID";
}

void RenderscriptRuntime::setPacketizationWidth(unsigned width) {
  m_packetizationWidth = width;
}

bool RenderscriptRuntime::isSyncFunc(const std::string &func_name) const {
  return isSyncWithNoSideEfffect(func_name) || isSyncWithSideEfffect(func_name);
}

bool RenderscriptRuntime::hasNoSideEffect(const std::string &func_name) const {
  // Work item builtins and llvm intrinsics are not in runtime module so check
  // them first.
  if (isWorkItemBuiltin(func_name))  return true;
  if (isSafeLLVMIntrinsic(func_name)) return true;
  if (Mangler::isFakeExtract(func_name)) return true;
  if (Mangler::isFakeInsert(func_name)) return true;
  if (Mangler::isRetByVectorBuiltin(func_name)) return true;

  // If it is not a built-in, don't know if it has side effect.
  Function *funcRT = findInRuntimeModule(func_name);
  if (!funcRT) return false;

  // Special case built-ins that access memory but has no side effects.
  if (isSyncWithNoSideEfffect(func_name)) return true;
  if (isImageDescBuiltin(func_name)) return true;

  // All built-ins that does not access memory and does not throw
  // have no side effects.
  return (funcRT->doesNotAccessMemory() && funcRT->doesNotThrow());
}

bool RenderscriptRuntime::isSafeToSpeculativeExecute(const std::string &func_name) const {
  // Work item builtins are not in runtime module so check them first.
  if (isWorkItemBuiltin(func_name)) return true;

  // Can not say anything on non - builtin function.
  Function *funcRT = findInRuntimeModule(func_name);
  if (!funcRT) return false;

  // Special case built-ins that access memory but can be speculatively executed.
  if (isImageDescBuiltin(func_name)) return true;

  // All built-ins that does not access memory and does not throw
  // can be speculatively executed.
  return (funcRT->doesNotAccessMemory() && funcRT->doesNotThrow());
}

bool RenderscriptRuntime::isExpensiveCall(const std::string &func_name) const {
  return false;
}

bool RenderscriptRuntime::isWorkItemBuiltin(const std::string &func_name) const {
  if (0 == func_name.compare(GET_ID_NAME)) return true;
  return false;
}

bool RenderscriptRuntime::isSyncWithSideEfffect(const std::string &func_name) const {
  return false;
}

bool RenderscriptRuntime::isSyncWithNoSideEfffect(const std::string &func_name) const {
  return false;
}

bool RenderscriptRuntime::isImageDescBuiltin(const std::string &func_name) const {
  return false;
}

bool RenderscriptRuntime::isSafeLLVMIntrinsic(const std::string &func_name) const {
  if (0 == func_name.compare("llvm.var.annotation")) return true;
  if (0 == func_name.compare("llvm.dbg.declare")) return true;
  if (0 == func_name.compare("llvm.dbg.value")) return true;
  return false;
}

bool RenderscriptRuntime::isReturnByPtrBuiltin(const std::string &func_name) const {
  if (!findInRuntimeModule(func_name)) return false;
  FunctionDescriptor ret = demangle(func_name.c_str());
  if (ret == FunctionDescriptor::null()) return false;
  llvm::ArrayRef<const char*> A(g_rs_BuiltinReturnByPtr);
  for (size_t i=0; i < A.size(); ++i)
    if (llvm::StringRef(A[i]) == ret.name)
      return true;
  return false;
}


bool RenderscriptRuntime::needSpecialCaseResolving(const std::string &funcName) const{
  return Mangler::isFakeBuiltin(funcName);
}

bool RenderscriptRuntime::isScalarSelect(const std::string &funcName) const{
  if (m_scalarSelectSet.count(funcName)) return true;
  return false;
}

bool RenderscriptRuntime::isMaskedFunctionCall(const std::string &func_name) const{
  return false;
}

bool RenderscriptRuntime::needsVPlanStyleMask(StringRef name) const {
  return false;
}

// TODO[MA]: revisit
bool RenderscriptRuntime::isFakedFunction(StringRef fname)const{
  bool isFake = Mangler::isFakeInsert(fname) ||
    Mangler::isFakeExtract(fname) || Mangler::isFakeBuiltin(fname);
  if (isFake)
    return true;
  Function* pMaskedFunction = findInRuntimeModule(fname);
  //Since not all faked builtins can be discovered by name, we need to make
  //the function resides within the runtime module.
  return (pMaskedFunction == nullptr);
}
  
unsigned RenderscriptRuntime::isInlineDot(const std::string &funcName) const{
  std::map<std::string, unsigned>::const_iterator it = m_dotOpWidth.find(funcName);
  if (it != m_dotOpWidth.end()) {
    return it->second;
  }
  return 0;
}
  
// TODO[MA]: need to revisit
bool RenderscriptRuntime::isAtomicBuiltin(const std::string &func_name) const {
  Function *bltn = findInRuntimeModule(func_name);
  if (!bltn) return false;
  return (func_name.find("atom") != std::string::npos);
}

bool RenderscriptRuntime::isScalarMinMaxBuiltin(StringRef funcName, bool &isMin,
                                          bool &isSigned) const {
  // funcName need to be mangled min or max.
  if (!isMangledName(funcName.data())) return false;
  std::string strippedName = stripName(funcName.data());
  isMin = (strippedName == "min");
  if (!isMin && strippedName != "max") return false;

  // Now that we know that this is min or max demnagle the builtin.
  FunctionDescriptor desc = demangle(funcName.data());
  assert(desc.parameters.size() == 2 && "min, max should have two parameters");
  // The argument type should be (u)int/(u)long
  RefParamType argTy = desc.parameters[0];
  const PrimitiveType *pPrimitive = reflection::dyn_cast<PrimitiveType>(argTy);
  if (!pPrimitive) return false;
  TypePrimitiveEnum basicType = pPrimitive->getPrimitive();
  isSigned = (basicType == PRIMITIVE_INT ||
              basicType == PRIMITIVE_LONG);
  if (!isSigned && basicType != PRIMITIVE_UINT &&
      basicType != PRIMITIVE_ULONG)  return false;
  return true;
}
  
} // Namespace

/// Support for static linking of modules
extern "C" {
  intel::RuntimeServices* createRenderscriptRuntimeSupport(SmallVector<Module*, 2> runtimeModuleList) {
    return new intel::RenderscriptRuntime(runtimeModuleList);
  }
}

