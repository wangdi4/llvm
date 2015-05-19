/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "DriverVectorizerFunction.h"
#include "OpenclRuntime.h"
#include "VectorizerFunction.h"
#include "BuiltinKeeper.h"
#include "NameMangleAPI.h"
#include "Mangler.h"
#include "ParameterType.h"
#include "Logger.h"
#include "CompilationUtils.h"
#include "common_dev_limits.h"

#include "llvm/IR/Constants.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
#include "llvm/ADT/StringRef.h"

using namespace reflection;
namespace intel {
//
//VectorizerFunctionBridge
//Purpose: a bridg of interfaces
//
class OpenClVFunction: public DriverVectorizerFunction{
public:
  OpenClVFunction(const std::string& s):DriverVectorizerFunction(s) {}

  ~OpenClVFunction() { }
};

struct dotProdInlineData {
  const char *name;
  unsigned opWidth;
};

const dotProdInlineData dotInlineTable [] = {
  {"_Z3dotff",1},
  {"_Z3dotdd",1},
  {"_Z3dotDv2_fS_",2},
  {"_Z3dotDv2_dS_",2},
  {"_Z3dotDv3_fS_",3},
  {"_Z3dotDv3_dS_",3},
  {"_Z3dotDv4_fS_",4},
  {"_Z3dotDv4_dS_",4},
  {NULL,0}
};

const char* BuiltinReturnByPtr[] = {
  "fract",
  "modf",
  "native_fract",
  "native_sincos",
  "sincos",
};
const size_t BuiltinReturnByPtrLength = sizeof(BuiltinReturnByPtr) / sizeof(BuiltinReturnByPtr[0]);


/// @brief Constructor which get arbitraty table as input
OpenclRuntime::OpenclRuntime(SmallVector<Module*, 2> runtimeModuleList,
                             const char  **scalarSelects) :
m_packetizationWidth(0) {
  m_runtimeModulesList = runtimeModuleList;
  initScalarSelectSet(scalarSelects);
  initDotMap();
}

void OpenclRuntime::initDotMap() {
  const dotProdInlineData *entryPtr = dotInlineTable;
  while (entryPtr->name) {
    m_dotOpWidth[entryPtr->name] = entryPtr->opWidth;
    ++entryPtr;
  }
}

void OpenclRuntime::initScalarSelectSet(const char **scalarSelects) {
  while (*scalarSelects) {
    m_scalarSelectSet.insert(*scalarSelects);
    ++scalarSelects;
  }
}

Function * OpenclRuntime::findInRuntimeModule(StringRef Name) const {
  for (SmallVector<Module*, 2>::const_iterator it = m_runtimeModulesList.begin();
      it != m_runtimeModulesList.end();
      ++it)
  {
    assert(*it != NULL && "OpenclRuntime::findInRuntimeModule Encountered NULL ptr in m_runtimeModulesList");
    Function* ret_function = (*it)->getFunction(Name);
    if (ret_function != NULL)
       return ret_function;
  }
  return NULL;
}

std::auto_ptr<VectorizerFunction>
OpenclRuntime::findBuiltinFunction(StringRef mangledName) const {
  std::auto_ptr<VectorizerFunction> ret(new OpenClVFunction(mangledName));
  return ret;
}

bool OpenclRuntime::orderedWI() const { return true; }

bool OpenclRuntime::isTIDGenerator(const Instruction * inst, bool * err, unsigned *dim) const {
  using namespace Intel::OpenCL::DeviceBackend;
  *err = false; // By default, no error expected..
  const CallInst * CI = dyn_cast<CallInst>(inst);
  if (!CI) return false; // ID generator is a function call.
  std::string funcName = CI->getCalledFunction()->getName().str();
  unsigned dimensionIndex = 0; // Index of argument to CALL instruction, which is the dimension
  if (Mangler::isMangledCall(funcName)) {
    funcName = Mangler::demangle(funcName); // Remove mangling (masking) prefix
    ++dimensionIndex; // There is a mask/predicate argument before the dimension
  }
  if (!CompilationUtils::isGetGlobalId(funcName) && !CompilationUtils::isGetLocalId(funcName))
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

unsigned OpenclRuntime::getPacketizationWidth() const {
  return m_packetizationWidth;
}

unsigned OpenclRuntime::getNumJitDimensions() const {
  return MAX_WORK_DIM;
}

const char *OpenclRuntime::getBaseGIDName() const {
  return "get_base_global_id.";
}

void OpenclRuntime::setPacketizationWidth(unsigned width) {
  m_packetizationWidth = width;
}

bool OpenclRuntime::isSyncFunc(const std::string &func_name) const {
  return isSyncWithNoSideEffect(func_name) || isSyncWithSideEffect(func_name);
}

static bool isNdrange_ndBuiltin(StringRef func_name) {
  return func_name.startswith("_Z10ndrange_");
}

bool OpenclRuntime::hasNoSideEffect(const std::string &func_name) const {
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
  // OpenCL 2.0
  // ndrange_1D/ndrange_2D/ndrange_3D built-ins have an sret argument, so
  // doesNotAccessMemory() returns false.
  return isNdrange_ndBuiltin(func_name);
}

bool OpenclRuntime::isSafeToSpeculativeExecute(const std::string &func_name) const {
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

bool OpenclRuntime::isExpensiveCall(const std::string &func_name) const {
  if (!isMangledName(func_name.c_str()))
    return false;
  StringRef stripped = stripName(func_name.c_str());
  return stripped.startswith("read_image") || stripped.startswith("write_image");
}

bool OpenclRuntime::isWorkItemBuiltin(const std::string &name) const {
  using namespace Intel::OpenCL::DeviceBackend;
  return CompilationUtils::isGetGlobalId(name) ||
    CompilationUtils::isGetLocalId(name)   ||
    CompilationUtils::isGetLocalSize(name) ||
    CompilationUtils::isGetGlobalSize(name)||
    CompilationUtils::isGetGroupId(name)   ||
    CompilationUtils::isGetWorkDim(name)   ||
    CompilationUtils::isGlobalOffset(name) ||
    CompilationUtils::isGetNumGroups(name) ||
    (0 == name.compare("get_base_global_id.")) ||
    // The following is applicabble for OpenCL 2.0 or more recent versions.
    CompilationUtils::isGetEnqueuedLocalSize(name);
}

bool OpenclRuntime::isSyncWithSideEffect(const std::string &func_name) const {
  using namespace Intel::OpenCL::DeviceBackend;
  if (CompilationUtils::isAsyncWorkGroupCopy(func_name)  ||
      CompilationUtils::isAsyncWorkGroupStridedCopy(func_name) ||
      CompilationUtils::isWorkGroupReserveReadPipe(func_name) ||
      CompilationUtils::isWorkGroupCommitReadPipe(func_name) ||
      CompilationUtils::isWorkGroupReserveWritePipe(func_name) ||
      CompilationUtils::isWorkGroupCommitWritePipe(func_name))
    return true;

  return false;
}

bool OpenclRuntime::isSyncWithNoSideEffect(const std::string &func_name) const {
  using namespace Intel::OpenCL::DeviceBackend;
  if (func_name == CompilationUtils::mangledBarrier() ||
      func_name == CompilationUtils::mangledWGBarrier(CompilationUtils::WG_BARRIER_NO_SCOPE) ||
      func_name == CompilationUtils::mangledWGBarrier(CompilationUtils::WG_BARRIER_WITH_SCOPE) )
    return true;

  if (CompilationUtils::isWaitGroupEvents(func_name))
    return true;

  const char* Fname = func_name.c_str();
  bool IsMangled = isMangledName(Fname);

  //builtin functions are always mangeled
  if (!IsMangled)
    return false;

  llvm::StringRef Stripped = stripName(Fname);
  if (Stripped == "mem_fence")
    return true;

  if (Stripped  == "read_mem_fence")
    return true;

  if (Stripped == "write_mem_fence")
    return true;

  return false;
}

bool OpenclRuntime::isImageDescBuiltin(const std::string &func_name) const {
  if (0 == func_name.find("_Z16get_image_height")) return true;
  if (0 == func_name.find("_Z15get_image_width")) return true;
  if (0 == func_name.find("_Z15get_image_depth")) return true;
  if (0 == func_name.find("_Z27get_image_channel")) return true;
  if (0 == func_name.find("_Z13get_image_dim_")) return true;
  return false;
}

bool OpenclRuntime::isSafeLLVMIntrinsic(const std::string &func_name) const {
  if (0 == func_name.compare("llvm.var.annotation")) return true;
  if (0 == func_name.compare("llvm.dbg.declare")) return true;
  if (0 == func_name.compare("llvm.dbg.value")) return true;
  return false;
}

bool OpenclRuntime::isReturnByPtrBuiltin(const std::string &func_name) const {
  if (!findInRuntimeModule(func_name)) return false;
  FunctionDescriptor ret = demangle(func_name.c_str());
  if (ret == FunctionDescriptor::null()) return false;
  llvm::ArrayRef<const char*> A(BuiltinReturnByPtr);
  for (size_t i=0; i < A.size(); ++i)
    if (llvm::StringRef(A[i]) == ret.name)
      return true;
  return false;
}


bool OpenclRuntime::needSpecialCaseResolving(const std::string &funcName) const{
  return Mangler::isFakeBuiltin(funcName);
}

bool OpenclRuntime::isScalarSelect(const std::string &funcName) const{
  if (m_scalarSelectSet.count(funcName)) return true;
  return false;
}

bool OpenclRuntime::isMaskedFunctionCall(const std::string &func_name) const{
  return false;
}

bool OpenclRuntime::isFakedFunction(StringRef fname)const{
  bool isFake = Mangler::isFakeInsert(fname) ||
    Mangler::isFakeExtract(fname) || Mangler::isFakeBuiltin(fname);
  if (isFake)
    return true;
  Function* pMaskedFunction = findInRuntimeModule(fname);
  //Since not all faked builtins can be discovered by name, we need to make
  //the function resides within the runtime module.
  return (pMaskedFunction == NULL);
}

unsigned OpenclRuntime::isInlineDot(const std::string &funcName) const{
  std::map<std::string, unsigned>::const_iterator it = m_dotOpWidth.find(funcName);
  if (it != m_dotOpWidth.end()) {
    return it->second;
  }
  return 0;
}

bool OpenclRuntime::isAtomicBuiltin(const std::string &func_name) const {
  if (!findInRuntimeModule(func_name)) return false;
  return (func_name.find("atom") != std::string::npos);
}

bool OpenclRuntime::isWorkItemPipeBuiltin(const std::string &func_name) const {
  // pipe built-in names must be mangled
  if (!isMangledName(func_name.c_str())) return false;
  // and be inside the built-in RT library
  if (!findInRuntimeModule(func_name)) return false;
  StringRef name = stripName(func_name.c_str());
  return !name.startswith("work_group") && name.endswith("_pipe");
}

bool OpenclRuntime::isScalarMinMaxBuiltin(StringRef funcName, bool &isMin,
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
