/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "TestRuntimeServices.h"
#include "VectorizerFunction.h"
#include "NameMangleAPI.h"
#include "Mangler.h"
#include "ParameterType.h"
#include "Logger.h"
#include "CompilationUtils.h"

#include "llvm/IR/Constants.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
#include "llvm/ADT/StringRef.h"

#include <sstream>

using namespace std;
using namespace reflection;
namespace intel {

class TestVFunction: public VectorizerFunction{
private:
  unsigned m_width;
public:
  TestVFunction(const std::string& s) : m_name(s) {
    size_t last_underscore = m_name.find_last_of("_v");
    if (last_underscore == string::npos) {
      m_width = FunctionDescriptor::SCALAR;
      return;
    }
    string widthSuffix = m_name.substr(last_underscore + 2);
    if (widthSuffix.empty()) {
      m_width = FunctionDescriptor::SCALAR;
      return;
    }
    std::stringstream sst(widthSuffix);
    sst >> m_width;
    if (sst.fail())
      m_width = FunctionDescriptor::SCALAR;
  }

  ~TestVFunction() {}

  virtual unsigned getWidth() const {
    return m_width;
  }

  virtual bool isPacketizable() const {
    // for now, all functions have vector versions
    return true;
  }

  virtual bool isScalarizable() const {
    // for now, all functions have a scalar version
    return true;
  }

  virtual std::string getVersion(unsigned index) const {
    // simple scheme - base name + "_v" + index
    if (index == 0U)
      return m_name;
    return m_name + "_v" + ::to_string(index);
  }

  virtual bool isNull() const {
    return false;
  }
private:
  const std::string m_name;
};

const char* BuiltinReturnByPtr[] = {
  "fract",
  "modf",
  "native_fract",
};
const size_t BuiltinReturnByPtrLength = sizeof(BuiltinReturnByPtr) / sizeof(BuiltinReturnByPtr[0]);

/// @brief Constructor which get arbitraty table as input
TestRuntime::TestRuntime(const Module *runtimeModule):
m_runtimeModule(runtimeModule),
m_packetizationWidth(0) {
}

Function * TestRuntime::findInRuntimeModule(StringRef Name) const {
   return m_runtimeModule->getFunction(Name);
}

std::unique_ptr<VectorizerFunction>
TestRuntime::findBuiltinFunction(StringRef mangledName) const {
  std::unique_ptr<VectorizerFunction> ret(new TestVFunction(mangledName));
  return ret;
}

bool TestRuntime::orderedWI() const { return true; }

bool TestRuntime::isTIDGenerator(const Instruction * inst, bool * err, unsigned *dim) const {
  using namespace Intel;
  *err = false; // By default, no error expected..
  const CallInst * CI = dyn_cast<CallInst>(inst);
  if (!CI) return false; // ID generator is a function call.
  std::string funcName = CI->getCalledFunction()->getName().str();
  unsigned dimensionIndex = 0; // Index of argument to CALL instruction, which is the dimension
  if (Mangler::isMangledCall(funcName)) {
    funcName = Mangler::demangle(funcName); // Remove mangling (masking) prefix
    ++dimensionIndex; // There is a mask/predicate argument before the dimension
  }
  if (!CompilationUtils::isGetGlobalId(funcName))
    return false; // not a get_***_id function
  Value * inputValue = CI->getArgOperand(dimensionIndex);

  // Check if the argument is constant - if not, we cannot determine if
  // the call will generate different IDs per different vectorization lanes
  if (!isa<ConstantInt>(inputValue))
  {
    *err = true; // return error
    return false;
  }

  // Report the dimention of the request
  *dim = static_cast<unsigned>(
    cast<ConstantInt>(inputValue)->getValue().getZExtValue());

  // This is indeed a TID generator
  return true;
}

unsigned TestRuntime::getPacketizationWidth() const {
  return m_packetizationWidth;
}

unsigned TestRuntime::getNumJitDimensions() const {
#define MAX_WORK_DIM (3) /* xmain */
  return MAX_WORK_DIM;
}

void TestRuntime::setPacketizationWidth(unsigned width) {
  m_packetizationWidth = width;
}

bool TestRuntime::isSyncFunc(const std::string &func_name) const {
  return isSyncWithNoSideEffect(func_name) || isSyncWithSideEffect(func_name);
}

static bool isNdrange_ndBuiltin(StringRef func_name) {
  return false;
}

bool TestRuntime::hasNoSideEffect(const std::string &func_name) const {
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
  if (isSyncWithNoSideEffect(func_name)) return true;
  if (isImageDescBuiltin(func_name)) return true;

  // All built-ins that does not access memory and does not throw
  // have no side effects.
  if (funcRT->doesNotAccessMemory() && funcRT->doesNotThrow())
    return true;

  return isNdrange_ndBuiltin(func_name);
}

bool TestRuntime::isSafeToSpeculativeExecute(const std::string &func_name) const {
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

bool TestRuntime::isExpensiveCall(const std::string &func_name) const {
  return false;
}

bool TestRuntime::isWorkItemBuiltin(const std::string &name) const {
  using namespace Intel;
  return (CompilationUtils::isGetGlobalId(name) ||
	  CompilationUtils::isGetGlobalSize(name));
}

bool TestRuntime::isSyncWithSideEffect(const std::string &func_name) const {
  return false;
}

bool TestRuntime::isSyncWithNoSideEffect(const std::string &func_name) const {
  return false;
}

bool TestRuntime::isImageDescBuiltin(const std::string &func_name) const {
  return false;
}

bool TestRuntime::isSafeLLVMIntrinsic(const std::string &func_name) const {
  if (0 == func_name.compare("llvm.var.annotation")) return true;
  if (0 == func_name.compare("llvm.dbg.declare")) return true;
  if (0 == func_name.compare("llvm.dbg.value")) return true;
  return false;
}

bool TestRuntime::isReturnByPtrBuiltin(const std::string &func_name) const {
  if (!findInRuntimeModule(func_name)) return false;
  FunctionDescriptor ret = demangle(func_name.c_str());
  if (ret == FunctionDescriptor::null()) return false;
  llvm::ArrayRef<const char*> A(BuiltinReturnByPtr);
  for (size_t i=0; i < A.size(); ++i)
    if (llvm::StringRef(A[i]) == ret.name)
      return true;
  return false;
}


bool TestRuntime::needSpecialCaseResolving(const std::string &funcName) const{
  return Mangler::isFakeBuiltin(funcName);
}

bool TestRuntime::isScalarSelect(const std::string &funcName) const{
  return false;
}

bool TestRuntime::isMaskedFunctionCall(const std::string &func_name) const{
  return false;
}

bool TestRuntime::isFakedFunction(StringRef fname)const{
  bool isFake = Mangler::isFakeInsert(fname) ||
    Mangler::isFakeExtract(fname) || Mangler::isFakeBuiltin(fname);
  if (isFake)
    return true;
  Function* pMaskedFunction = findInRuntimeModule(fname);
  //Since not all faked builtins can be discovered by name, we need to make
  //the function resides within the runtime module.
  return (pMaskedFunction == NULL);
}

unsigned TestRuntime::isInlineDot(const std::string &funcName) const{
  return 0;
}

bool TestRuntime::isAtomicBuiltin(const std::string &func_name) const {
  return false;
}

bool TestRuntime::isWorkItemPipeBuiltin(const std::string &func_name) const {
  return false;
}

bool TestRuntime::isScalarMinMaxBuiltin(StringRef funcName, bool &isMin,
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

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  intel::RuntimeServices* createTestRuntimeSupport(const Module *runtimeModule) {
    return new intel::TestRuntime(runtimeModule);
  }
}
