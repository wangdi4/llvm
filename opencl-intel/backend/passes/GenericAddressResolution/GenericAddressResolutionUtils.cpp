/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "GenericAddressResolution.h"

#include <NameMangleAPI.h>
#include <FunctionDescriptor.h>
#include <ParameterType.h>
#include <MetaDataApi.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Debug.h>
#include <set>
#include <assert.h>

#define DEBUG_TYPE "GenericAddressResolutionUtils"

namespace Intel { namespace OpenCL { namespace DeviceBackend { namespace Passes { namespace GenericAddressSpace {
  using namespace llvm;

  /// @brief Names of Address Space Qualifier OpenCL BI functions
  static const char *addrQualifierFunctions[] = {
                      "to_global",
                      "to_local",
                      "to_private",
                      "get_fence"
  };

  /// @brief Names of OpenCL BI functions which may use GAS pointers
  static const char *genericAddrBiFunctions[] = {
                      "fract",
                      "frexp",
                      "lgamma_r",
                      "modf",
                      "remquo",
                      "sincos",
                      "vload",
                      "vstore",
                      "atomic_init",
                      "atomic_store",
                      "atomic_store_explicit",
                      "atomic_load",
                      "atomic_load_explicit",
                      "atomic_exchange",
                      "atomic_exchange_explicit",
                      "atomic_compare_exchange_strong",
                      "atomic_compare_exchange_strong_explicit",
                      "atomic_compare_exchange_weak",
                      "atomic_compare_exchange_weak_explicit",
                      "atomic_fetch_add",
                      "atomic_fetch_add_explicit",
                      "atomic_fetch_sub",
                      "atomic_fetch_sub_explicit",
                      "atomic_fetch_or",
                      "atomic_fetch_or_explicit",
                      "atomic_fetch_xor",
                      "atomic_fetch_xor_explicit",
                      "atomic_fetch_and",
                      "atomic_fetch_and_explicit",
                      "atomic_fetch_min",
                      "atomic_fetch_min_explicit",
                      "atomic_fetch_max",
                      "atomic_fetch_max_explicit",
                      "atomic_flag_test_and_set",
                      "atomic_flag_test_and_set_explicit",
                      "atomic_flag_clear",
                      "atomic_flag_clear_explicit",
                      "enqueue_kernel",
                      "enqueue_marker",
                      "read_pipe",
                      "wait_group_events",
                      "write_pipe"
  };

  bool isAddressQualifierBI(const Function *pFunc) {
    StringRef funcName = pFunc->getName();
    std::string tmp = funcName.str();
    const char *funcNameStr = tmp.c_str();
    funcName = isMangledName(funcNameStr)? stripName(funcNameStr) : funcName;
    for (unsigned idx = 0; idx < sizeof(addrQualifierFunctions)/sizeof(char*); idx++) {
      if (funcName == addrQualifierFunctions[idx]) {
        return true;
      }
    }
    return false;
  }

  bool isGenericAddrBI(const Function *pFunc) {
    StringRef funcName = pFunc->getName();
    std::string tmp = funcName.str();
    const char *funcNameStr = tmp.c_str();
    funcName = isMangledName(funcNameStr)? stripName(funcNameStr) : funcName;
    for (unsigned idx = 0; idx < sizeof(genericAddrBiFunctions)/sizeof(char*); idx++) {
      if (std::string::npos != funcName.find(genericAddrBiFunctions[idx])) {
        return true;
      }
    }
    return false;
  }

  void sortFunctionsInCGOrder(Module *pModule, TFunctionList &orderedList, bool isTopDownOrder) {

    typedef std::set<Function*> TFunctionSet;
    TFunctionSet functionCache;

    // Function list to sort
    for (Module::iterator func_it = pModule->begin(),
                          func_it_end = pModule->end();
                          func_it != func_it_end; func_it++) {
      if (!func_it->isDeclaration()) {
        functionCache.insert(func_it);
      }
    }

    // Sort the functions according to the call-graph
    while (!functionCache.empty()) {
      bool hasNoRootFunction = true;
      for (TFunctionSet::iterator func_it = functionCache.begin(),
                                  func_end = functionCache.end();
                                  func_it != func_end; func_it++) {

        Function *pFunc = *func_it;
        bool isRoot = true;
        // Looking for callers to the function
        for (Value::user_iterator user_it = pFunc->user_begin(),
                                  user_end = pFunc->user_end();
                                  user_it != user_end; user_it++) {

          CallInst *pInstr = dyn_cast<CallInst>(*user_it);
          if (pInstr) {
            // Only direct Call to the function matters for ordering
            // (because only that call is to be resolved later)
            Function *pCallerFunc = pInstr->getParent()->getParent();
            if (functionCache.count(pCallerFunc)) {
              isRoot = false;
              break;
            }
          }
        }

        // If no one calls this function - have it bubbled-up in the ordered list
        if (isRoot) {
          hasNoRootFunction = false;
          if (isTopDownOrder) {
            // caller-first
            orderedList.push_back(pFunc);
          } else {
            // caller-last
            orderedList.push_front(pFunc);
          }
          functionCache.erase(func_it);
          break;
        }
      } // iterate through functions
      if (hasNoRootFunction) {
        assert(0 && "Detected recursive function call!");
        break;
      }
    }   // repeat until all functions are sorted
  }

  void assocDebugLocWith(Instruction *pNew, const Instruction *pOld) {
    if (!pOld->getDebugLoc().isUnknown()) {
      pNew->setDebugLoc(pOld->getDebugLoc());
    }
  }

  static reflection::TypeAttributeEnum translateAddrSpaceToAttr(OCLAddressSpace::spaces space) {
    reflection::TypeAttributeEnum attr = reflection::ATTR_NONE;
    switch (space) {
      case OCLAddressSpace::Private :
        attr = reflection::ATTR_PRIVATE;
        break;
      case OCLAddressSpace::Global :
        attr = reflection::ATTR_GLOBAL;
        break;
      case OCLAddressSpace::Local :
        attr = reflection::ATTR_LOCAL;
        break;
      case OCLAddressSpace::Constant :
        attr = reflection::ATTR_CONSTANT;
        break;
      case OCLAddressSpace::Generic :
        attr = reflection::ATTR_GENERIC;
        break;
      default:
        assert(0 && "Unsupported type of address space!");
        break;
    }
    return attr;
  }

  std::string getResolvedMangledName(std::string origMangledName,
                                     const SmallVector<OCLAddressSpace::spaces, 8> &resolvedSpaces,
                                     const SmallVector<OCLAddressSpace::spaces, 8> *originalSpaces) {

    assert(isMangledName(origMangledName.c_str()) && "Function name is expected to be mangled!");

    reflection::FunctionDescriptor fd = demangle(origMangledName.c_str());
    //if (fd.parameters.size() != resolvedSpaces.size()) {
    //  dbgs() << origMangledName << "\n";
    // }

    reflection::ParamType * lastArg = fd.parameters.back();
    reflection::PrimitiveType *lastArgTy = reflection::dyn_cast<reflection::PrimitiveType>(lastArg);
    if(lastArgTy && lastArgTy->getPrimitive() == reflection::PRIMITIVE_VAR_ARG){
      // in case of 0 number of variadic arguments mangler still detect variadic argument
      // in callinst variadic argument may not be generated, we should exclude it from computations
      // and align number of arguments to resolvedSpaces.size()
      assert(fd.parameters.size() - 1  <= resolvedSpaces.size() &&
         "in case of variadic # arguments should be less or equal to # of arguments detected by mangler");
    } else {
      assert(resolvedSpaces.size() == fd.parameters.size() &&
        "Mismatch between mangled name and amount of parameters");
    }

    assert((!originalSpaces || originalSpaces->size() == resolvedSpaces.size()) && "Invalid parameters!");

    for (unsigned idx = 0; idx < resolvedSpaces.size(); idx++) {
      OCLAddressSpace::spaces resolvedSpace = resolvedSpaces[idx];
      if (!IS_ADDR_SPACE_GENERIC(resolvedSpace)) {
        // This parameter is a pointer which can be replaced with named addr-space
        reflection::PointerType *ptrParam =
              reflection::dyn_cast<reflection::PointerType>(fd.parameters[idx]);
        assert(ptrParam && "The parameter's mangling encoding should be of pointer type!");
        // Replace mangling encoding with its named-space version
        reflection::TypeAttributeEnum newAttr = translateAddrSpaceToAttr(resolvedSpace);
        reflection::TypeAttributeEnum origAttr = originalSpaces?
                                                   translateAddrSpaceToAttr(
                                                            (*originalSpaces)[idx]) :
                                                   reflection::ATTR_GENERIC;
        if (!ptrParam->convertAddrSpaceAttribute(origAttr, newAttr)) {
          assert(0 && "Addr-space mangling attribute replace failed!");
        }
      }
    }

    return mangle(fd);
  }

  std::string getSpecializedFunctionName(std::string functionName, const SmallVector<Type*,  8> &argTypes) {

    reflection::FunctionDescriptor fdSpecialized;
    fdSpecialized.name = functionName;

    // Collect mangling names of the function's pointer arguments
    // (including address space attribute)
    for (unsigned idx = 0; idx < argTypes.size(); idx++) {
      Type *argType = argTypes[idx];
      if (!argType->isPointerTy()) {
        // Filter-out non-pointer arguments
        continue;
      }
      // For simplicity reason - we use int* as a pointer primitive type
      reflection::PointerType *ptrParam = new reflection::PointerType(
                                            new reflection::PrimitiveType(
                                                  reflection::PRIMITIVE_INT));
      // Add address space attribute according to the argument's addres space
      reflection::TypeAttributeEnum attr =
                    translateAddrSpaceToAttr(
                        (OCLAddressSpace::spaces)cast<PointerType>(argType)->getAddressSpace());
      ptrParam->addAttribute(attr);
      // Add the mangling name of the argument to the function descriptor
      reflection::RefParamType refParam(ptrParam);
      fdSpecialized.parameters.push_back(refParam);
    }

    return mangle(fdSpecialized);
  }

  void getIntrinsicOverload(Function *pFunc, const SmallVector<Type*, 8> &argTypes,
                                             SmallVector<Type*, 8> &overloadableArgTypes) {
    assert(pFunc->isIntrinsic() &&
           Intrinsic::isOverloaded((Intrinsic::ID)pFunc->getIntrinsicID()) &&
           "Overloadable intrinsic function is expected at this point!");

    FunctionType *pFuncType = pFunc->getFunctionType();
    SmallVector<Intrinsic::IITDescriptor, 8> intrinsicInfo;
    Intrinsic::getIntrinsicInfoTableEntries((Intrinsic::ID)pFunc->getIntrinsicID(),
                                            intrinsicInfo);
    // Check intrinsic info for parameter types and fill the signature with
    // overloadable types only (starting from 1, as return type is not a
    // part of the overloading signature)
    for (unsigned idx = 0; idx < pFuncType->getNumParams(); idx++) {
      if (intrinsicInfo[idx+1].Kind == Intrinsic::IITDescriptor::Argument) {
        overloadableArgTypes.push_back(argTypes[idx]);
      }
    }
  }

  void emitWarning(std::string warning, Instruction *pInstr,
                   Module *pModule, LLVMContext *pLLVMContext) {

    assert(pModule && pLLVMContext && "emitWarning parameters are invalid!");

    Intel::MetaDataUtils mdUtils(pModule);
    if (mdUtils.empty_ModuleInfoList()) {
        mdUtils.addModuleInfoListItem(Intel::ModuleInfoMetaDataHandle(Intel::ModuleInfoMetaData::get()));
    }

    Intel::ModuleInfoMetaDataHandle handle = mdUtils.getModuleInfoListItem(0);

    // Print-out the message ...
    DEBUG(
      dbgs() << "WARNING: " << warning << ": line# ";
      if (pInstr && !pInstr->getDebugLoc().isUnknown()) {
        unsigned lineNo = pInstr->getDebugLoc().getLine();
        dbgs() << lineNo << "\n";
        // ... and record to metadata
        handle->addGASwarningsItem(lineNo);
      }
      else {
        dbgs() << "unknown" << "\n";
      }
    );

    mdUtils.save(*pLLVMContext);
  }

  bool isSinglePtr(const Type *pPtrType) {
    Type *pPtrElementType = pPtrType->getPointerElementType();
    return !pPtrElementType->isArrayTy() && !pPtrElementType->isPointerTy();
  }

  Value *getFoldedAddrSpaceCall(CallInst *pCallInstr, Value *pSrcPtr) {
    // Fetching function to fold
    const Function *pCallee = pCallInstr->getCalledFunction();
    assert(pCallee && "Call instruction doesn't have a callee!");
    StringRef funcName = pCallee->getName();
    std::string tmp = funcName.str();
    const char *funcNameStr = tmp.c_str();
    funcName = isMangledName(funcNameStr)? stripName(funcNameStr) : funcName;

    // Generating bitcast instruction or constant value
    Value *pFolded = NULL;
    if (funcName == "get_fence") {
      // get_fence - always generates CLK_GLOBAL_MEM_FENCE for CPU
      pFolded = ConstantInt::get(pCallee->getReturnType(), CLK_GLOBAL_MEM_FENCE);
    } else {
      // to_XX - always returns XX cast of the input parameter for CPU
      pFolded = CastInst::CreatePointerCast(pSrcPtr, pCallee->getReturnType(),
                                "ToNamedPtr", pCallInstr);
    }

    return pFolded;
  }



} // namespace GenericAddressSpace
} // namespace Passes
} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
