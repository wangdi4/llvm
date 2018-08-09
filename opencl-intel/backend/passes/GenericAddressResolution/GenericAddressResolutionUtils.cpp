// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#include "GenericAddressResolution.h"

#include <FunctionDescriptor.h>
#include <CompilationUtils.h>
#include <NameMangleAPI.h>
#include <ParameterType.h>

#include <llvm/ADT/StringSwitch.h>
#include "llvm/IR/Constants.h"
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/Type.h>

#include <llvm/Support/Debug.h>
#include <llvm/Support/raw_ostream.h>

#include <assert.h>
#include <set>

#define DEBUG_TYPE "GenericAddressResolutionUtils"

namespace Intel { namespace OpenCL { namespace DeviceBackend { namespace Passes { namespace GenericAddressSpace {
  using namespace llvm;

  /// @brief Names of Address Space Qualifier OpenCL BI functions
  constexpr StringLiteral addrQualifierFunctions[] = {
    "__to_global", "__to_local", "__to_private", "get_fence"
  };

  /// @brief Names of OpenCL BI functions which may use GAS pointers
  constexpr StringLiteral genericAddrBiFunctions[] = {
    "fract",                                   "frexp",
    "lgamma_r",                                "modf",
    "remquo",                                  "sincos",
    "vload2",                                  "vload8",
    "vload3",                                  "vload16",
    "vload4",                                  "vstore2",
    "vstore3",                                 "vstore8",
    "vstore4",                                 "vstore16",
    "vloada_half",                             "vloada_half2",
    "vloada_half3",                            "vloada_half4",
    "vloada_half8",                            "vloada_half16",
    "vstore_half",                             "vstore_half2",
    "vstore_half3",                            "vstore_half4",
    "vstore_half8",                            "vstore_half16",
    "atomic_init",                             "atomic_store",
    "atomic_store_explicit",                   "atomic_load",
    "atomic_load_explicit",                    "atomic_exchange",
    "atomic_exchange_explicit",                "atomic_compare_exchange_strong",
    "atomic_compare_exchange_strong_explicit", "atomic_compare_exchange_weak",
    "atomic_compare_exchange_weak_explicit",   "atomic_fetch_add",
    "atomic_fetch_add_explicit",               "atomic_fetch_sub",
    "atomic_fetch_sub_explicit",               "atomic_fetch_or",
    "atomic_fetch_or_explicit",                "atomic_fetch_xor",
    "atomic_fetch_xor_explicit",               "atomic_fetch_and",
    "atomic_fetch_and_explicit",               "atomic_fetch_min",
    "atomic_fetch_min_explicit",               "atomic_fetch_max",
    "atomic_fetch_max_explicit",               "atomic_flag_test_and_set",
    "atomic_flag_test_and_set_explicit",       "atomic_flag_clear",
    "atomic_flag_clear_explicit",              "enqueue_marker",
    "wait_group_events",
  };

  bool isAddressQualifierBI(const Function *pFunc) {
    StringRef FName = pFunc->getName();
    FName = isMangledName(FName.data()) ? stripName(FName.data()) : FName;
    return std::find(std::begin(addrQualifierFunctions),
                     std::end(addrQualifierFunctions),
                     FName) != std::end(addrQualifierFunctions);
  }

  bool isGenericAddrBI(const Function *pFunc) {
    StringRef FName = pFunc->getName();
    FName = isMangledName(FName.data()) ? stripName(FName.data()) : FName;
    return std::find(std::begin(genericAddrBiFunctions),
                     std::end(genericAddrBiFunctions),
                     FName) != std::end(genericAddrBiFunctions);
  }

  bool needToSkipResolution(const Function *F) {
    StringRef FName = F->getName();
    using namespace Intel::OpenCL::DeviceBackend;
    // It isn't needed to process this extended execution BIs
    if (CompilationUtils::isEnqueueKernel(FName.str()) ||
        FName.equals("__get_kernel_work_group_size_impl") ||
        FName.equals("__get_kernel_preferred_work_group_size_multiple_impl"))
      return true;
    // It isn't needed to process pipes BIs
    if (CompilationUtils::isPipeBuiltin(FName))
      return true;

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
        functionCache.insert(&*func_it);
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
    if (pOld->getDebugLoc()) {
      pNew->setDebugLoc(pOld->getDebugLoc());
    }
  }

  static reflection::TypeAttributeEnum
  translateAddrSpaceToAttr(OCLAddressSpace::spaces space) {
    reflection::TypeAttributeEnum attr = reflection::ATTR_NONE;
    switch (space) {
    case OCLAddressSpace::Private:
      attr = reflection::ATTR_PRIVATE;
      break;
    case OCLAddressSpace::Global:
      attr = reflection::ATTR_GLOBAL;
      break;
    case OCLAddressSpace::Local:
      attr = reflection::ATTR_LOCAL;
      break;
    case OCLAddressSpace::Constant:
      attr = reflection::ATTR_CONSTANT;
      break;
    case OCLAddressSpace::Generic:
      attr = reflection::ATTR_GENERIC;
      break;
    default:
      assert(0 && "Unsupported type of address space!");
      break;
    }
    return attr;
  }

  std::string getResolvedMangledName(
      std::string origMangledName,
      const SmallVector<OCLAddressSpace::spaces, 8> &resolvedSpaces,
      const SmallVector<OCLAddressSpace::spaces, 8> *originalSpaces) {
    if (!isMangledName(origMangledName.c_str()))
      return origMangledName;

    reflection::FunctionDescriptor fd = demangle(origMangledName.c_str());

    reflection::ParamType *lastArg = fd.parameters.back();
    reflection::PrimitiveType *lastArgTy =
        reflection::dyn_cast<reflection::PrimitiveType>(lastArg);
    if (lastArgTy &&
        lastArgTy->getPrimitive() == reflection::PRIMITIVE_VAR_ARG) {
      // in case of 0 number of variadic arguments mangler still detect variadic
      // argument in callinst variadic argument may not be generated, we should
      // exclude it from computations and align number of arguments to
      // resolvedSpaces.size()
      assert(fd.parameters.size() - 1 <= resolvedSpaces.size() &&
             "in case of variadic # arguments should be less or equal to # of "
             "arguments detected by mangler");
    } else {
      assert(resolvedSpaces.size() == fd.parameters.size() &&
             "Mismatch between mangled name and amount of parameters");
    }

    assert(
        (!originalSpaces || originalSpaces->size() == resolvedSpaces.size()) &&
        "Invalid parameters!");

    for (unsigned idx = 0; idx < resolvedSpaces.size(); idx++) {
      OCLAddressSpace::spaces resolvedSpace = resolvedSpaces[idx];
      if (IS_ADDR_SPACE_GENERIC(resolvedSpace))
        continue;
      // This parameter is a pointer which can be replaced with named
      // addr-space
      reflection::PointerType *ptrParam =
          reflection::dyn_cast<reflection::PointerType>(fd.parameters[idx]);
      assert(ptrParam &&
             "The parameter's mangling encoding should be of pointer type!");
      // Replace mangling encoding with its named-space version
      reflection::TypeAttributeEnum newAttr =
          translateAddrSpaceToAttr(resolvedSpace);
      reflection::TypeAttributeEnum origAttr =
          originalSpaces ? translateAddrSpaceToAttr((*originalSpaces)[idx])
                         : reflection::ATTR_GENERIC;
      if (!ptrParam->convertAddrSpaceAttribute(origAttr, newAttr)) {
        assert(0 && "Addr-space mangling attribute replace failed!");
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
                   llvm::SmallVectorImpl<int> &GASWarnings, LLVMContext *pLLVMContext) {

    // Print-out the message ...
    LLVM_DEBUG(
      dbgs() << "WARNING: " << warning << ": line# ";
      if (pInstr && pInstr->getDebugLoc()) {
        unsigned lineNo = pInstr->getDebugLoc().getLine();
        dbgs() << lineNo << "\n";
        // ... and record to metadata
        GASWarnings.push_back(lineNo);
      }
      else {
        dbgs() << "unknown" << "\n";
      }
    );
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
    Value *pFolded = nullptr;
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
