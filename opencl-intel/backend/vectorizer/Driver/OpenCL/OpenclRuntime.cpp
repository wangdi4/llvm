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
#include "llvm/Constants.h"

#include "VectorizerFunction.h"
#include "BuiltinKeeper.h"
#include "NameMangleAPI.h"

using namespace reflection;
namespace intel {
//
//VectorizerFunctionBridge
//Purpose: a bridg of interfaces
//
class OpenClVFunction: public VectorizerFunction{
public:

  OpenClVFunction(const std::string& s): m_name(s){
  }

  unsigned getWidth()const{
    assert(!isNull() && "Null function");
    const BuiltinKeeper* pKeeper = reflection::BuiltinKeeper::instance();
    if (!pKeeper->isBuiltin(m_name))
      return width::NONE;
    width::V allWidth[] = {width::SCALAR, width::TWO, width::THREE, width::FOUR,
    width::EIGHT, width::SIXTEEN};
    for (size_t i=0 ; i<width::OCL_VERSIONS ; ++i){
      PairSW sw = pKeeper->getVersion(m_name, allWidth[i]);
      if (m_name == sw.first)
        return sw.second;
    }
    assert (isMangled() && "not a mangled name, cannot determine function width");
    //if we reached here, that means that function cannot be versioned, so our
    //best option is to apply the automatic width detection.
    FunctionDescriptor ret = demangle(m_name.c_str());
    ret.assignAutomaticWidth();
    return ret.width;
  }

  bool isPacketizable()const{
    //all builtin version has a width 4 version is they are packetizable
    const BuiltinKeeper* pKeeper = BuiltinKeeper::instance();
    if (!pKeeper->isBuiltin(m_name))
      return false;
    PairSW version4 = pKeeper->getVersion(m_name, width::FOUR);
    return !isNullPair(version4);
  }

  bool isScalarizable()const{
    const BuiltinKeeper* pKeeper = reflection::BuiltinKeeper::instance();
    if (!pKeeper->isBuiltin(m_name))
      return false;
    PairSW sw = pKeeper->getVersion(m_name, width::SCALAR);
    return !isNullPair(sw);
  }

  std::string getVersion(unsigned index)const{
    //we need to comply with the 'wiered' indexing system of the interface
    const BuiltinKeeper* pKeeper = BuiltinKeeper::instance();
    if (!pKeeper->isBuiltin(m_name))
      return FunctionDescriptor::nullString();
    width::V w;
    switch(index){
      case 0U: w = width::SCALAR; break;
      case 1U: w = width::TWO; break;
      case 2U: w = width::FOUR; break;
      case 3U: w = width::EIGHT; break;
      case 4U: w = width::SIXTEEN; break;
      case 5U: w = width::THREE; break;
      default:
        assert(false && "invalid index");
        return FunctionDescriptor::nullString();
    }
    PairSW sw = pKeeper->getVersion(m_name, w);
    assert(sw.second == w && "requested width doesn't match");
    return sw.first;
  }

  bool isNull()const{
    const BuiltinKeeper* pKeeper = BuiltinKeeper::instance();
    return !pKeeper->isBuiltin(m_name);
  }

private:

  bool isMangled()const{
    const std::string prefix = "_Z";
    return ( prefix == m_name.substr(0, prefix.size()) );
  }

  const std::string& m_name;
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
                             const char  **scalarSelects):
m_runtimeModule(runtimeModule),
m_packetizationWidth(0) {
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

std::auto_ptr<VectorizerFunction>
OpenclRuntime::findBuiltinFunction(std::string &mangledName) const {
  std::auto_ptr<VectorizerFunction> ret(new OpenClVFunction(mangledName));
  return ret;
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

unsigned OpenclRuntime::getNumJitDimensions() const {
  return 3;
}

const char *OpenclRuntime::getBaseGIDName() const {
  return "get_base_global_id.";
}

void OpenclRuntime::setPacketizationWidth(unsigned width) {
  m_packetizationWidth = width;
}

bool OpenclRuntime::isSyncFunc(const std::string &func_name) const {
  return isSyncWithNoSideEfffect(func_name) || isSyncWithSideEfffect(func_name);
}

bool OpenclRuntime::hasNoSideEffect(const std::string &func_name) const {
  // Work item builtins and llvm intrinsics are not in runtime module so check
  // them first.
  if (isWorkItemBuiltin(func_name))  return true;
  if (isSafeLLVMIntrinsic(func_name)) return true;

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

bool OpenclRuntime::isWorkItemBuiltin(const std::string &func_name) const {
  if (0 == func_name.compare("get_global_id")) return true;
  if (0 == func_name.compare("get_local_id")) return true;
  if (0 == func_name.compare("get_base_global_id.")) return true;
  if (0 == func_name.compare("get_local_size")) return true;
  if (0 == func_name.compare("get_global_size")) return true;
  if (0 == func_name.compare("get_group_id")) return true;
  if (0 == func_name.compare("get_num_groups")) return true;
  if (0 == func_name.compare("get_work_dim")) return true;
  if (0 == func_name.compare("get_global_offset")) return true;
  return false;
}

bool OpenclRuntime::isSyncWithSideEfffect(const std::string &func_name) const {
  if (std::string::npos != func_name.find("async_work_group_copy")) return true;
  if (std::string::npos != func_name.find("async_work_group_strided_copy")) return true;
  if (0 == func_name.compare("_Z17wait_group_eventsiPj")) return true;
  return false;
}

bool OpenclRuntime::isSyncWithNoSideEfffect(const std::string &func_name) const {
  if (0 == func_name.compare("barrier")) return true;
  if (0 == func_name.compare("mem_fence")) return true;
  if (0 == func_name.compare("read_mem_fence")) return true;
  if (0 == func_name.compare("write_mem_fence")) return true;
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


bool OpenclRuntime::needSpecialCaseResolving(const std::string &funcName) const{
  return Mangler::isFakeBuiltin(funcName);
}

bool OpenclRuntime::needPreVectorizationFakeFunction(const std::string &funcName) const{
  return false;
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

bool OpenclRuntime::isWriteImage(const std::string &funcName) const{
  return false;
}

unsigned OpenclRuntime::isInlineDot(const std::string &funcName) const{
  std::map<std::string, unsigned>::const_iterator it = m_dotOpWidth.find(funcName);
  if (it != m_dotOpWidth.end()) {
	return it->second;
  }
  return 0;
}

bool OpenclRuntime::isAtomicBuiltin(const std::string &func_name) const {
  Function *bltn = findInRuntimeModule(func_name);
  if (!bltn) return false;
  return (func_name.find("atom") != std::string::npos);
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
