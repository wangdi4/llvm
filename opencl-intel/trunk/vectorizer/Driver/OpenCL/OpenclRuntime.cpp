/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "OpenclRuntime.h"
#include "Mangler.h"
#include "Logger.h"

namespace intel {


typedef struct dotProdInlineData {
  const char *name;
  unsigned opWidth;
} dotProdInlineData;

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

const char *volacanoScalarSelect[] = {
  "_Z6selectccc", "_Z6selectcch", "_Z6selecthhc", "_Z6selecthhh",
  "_Z6selectsss", "_Z6selectsst", "_Z6selecttts", "_Z6selectttt",
  "_Z6selectiii", "_Z6selectiij", "_Z6selectjji", "_Z6selectjjj",
  "_Z6selectlll", "_Z6selectllm", "_Z6selectmml", "_Z6selectmmm",
  "_Z6selectffi", "_Z6selectffj",
  "_Z6selectddl", "_Z6selectddm",
  NULL
};

/// @brief Constructor which get arbitraty table as input
OpenclRuntime::OpenclRuntime(const Module *runtimeModule,
                             VFH::hashEntry *DB,
                             const char  **scalarSelects):
m_runtimeModule(runtimeModule),
m_packetizationWidth(0),
m_vfh(DB) { 
  initDotMap();
  initScalarSelectSet(scalarSelects);
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
   return m_runtimeModule->getFunction(Name);
}

RuntimeServices::funcEntry
OpenclRuntime::findBuiltinFunction(std::string &inp_name) const {
    return m_vfh.findFunctionInHash(inp_name);
}

bool OpenclRuntime::orderedWI() const { return true; }

bool OpenclRuntime::isTIDGenerator(const Instruction * inst, bool * err, unsigned *dim) const {
  *err = false; // By default, no error expected..
  const CallInst * CI = dyn_cast<CallInst>(inst);
  if (!CI) return false; // ID generator is a function call.
  std::string funcName = CI->getCalledFunction()->getName();
  unsigned dimensionIndex = 0; // Index of argument to CALL instruction, which is the dimension
  if (Mangler::isMangledCall(funcName)) {
    funcName = Mangler::demangle(funcName); // Remove mangling (masking) prefix
    ++dimensionIndex; // There is a mask/predicate argument before the dimension
  }
  if (funcName != GET_GID_NAME && funcName != GET_LID_NAME) return false; // not a get_***_id function
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

void OpenclRuntime::setPacketizationWidth(unsigned width) {
  m_packetizationWidth = width;
}

bool OpenclRuntime::isSyncFunc(const std::string &func_name) const {
  // TODO: Maybe need to add OpenCL's "sync_***" functions as well
  return (0 == func_name.compare("barrier"));
}

bool OpenclRuntime::isKnownUniformFunc(std::string &func_name) const {
  if (0 == func_name.compare("get_local_size")) return true;
  if (0 == func_name.compare("get_global_size")) return true;
  if (0 == func_name.compare("get_group_id")) return true;
  if (0 == func_name.compare("get_work_dim")) return true;
  if (0 == func_name.compare("barrier")) return true;
  return false;
}

bool OpenclRuntime::hasNoSideEffect(std::string &func_name) const {
  const RuntimeServices::funcEntry foundFunction = m_vfh.findFunctionInHash(func_name);
  if (foundFunction.first) return true;
  return false;
}

bool OpenclRuntime::needSpecialCaseResolving(std::string &funcName) const{
  return Mangler::isFakeBuiltin(funcName);
}

bool OpenclRuntime::needPreVectorizationFakeFunction(std::string &funcName) const{
  return false;
}

bool OpenclRuntime::isScalarSelect(std::string &funcName) const{
  if (m_scalarSelectSet.count(funcName)) return true;
  return false;
}

bool OpenclRuntime::isMaskedFunctionCall(std::string &func_name) const{
  return false;
}

bool OpenclRuntime::isWriteImage(std::string &funcName) const{
  return false;
}

unsigned OpenclRuntime::isInlineDot(std::string &funcName) const{
  std::map<std::string, unsigned>::const_iterator it = m_dotOpWidth.find(funcName);
  if (it != m_dotOpWidth.end()) {
	return it->second;
  }
  return 0;
}


} // Namespace

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  void* createOpenclRuntimeSupport(const Module *runtimeModule) {
    V_ASSERT(NULL == intel::RuntimeServices::get() && "Trying to re-create singleton!");
    intel::OpenclRuntime * rt = 
      new intel::OpenclRuntime(runtimeModule);
    intel::RuntimeServices::set(rt);
    return (void*)(rt);
  }

  void* destroyOpenclRuntimeSupport() {
    delete intel::RuntimeServices::get();
    intel::RuntimeServices::set(0);
    return 0;
  }
}
