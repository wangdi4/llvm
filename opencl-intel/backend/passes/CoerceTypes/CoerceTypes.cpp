// INTEL CONFIDENTIAL
//
// Copyright 2019 Intel Corporation.
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

#include "CoerceTypes.h"
#include "CompilationUtils.h"
#include "InitializePasses.h"
#include "OCLPassSupport.h"
#include "MetadataAPI.h"

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/IRBuilder.h"

#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

using namespace llvm;
using namespace Intel::MetadataAPI;
using namespace Intel::OpenCL::DeviceBackend;

extern "C" {
ModulePass *createCoerceTypesPass() { return new intel::CoerceTypes(); }
}

namespace intel {
char CoerceTypes::ID = 0;

OCL_INITIALIZE_PASS(CoerceTypes, "coerce-types",
                    "Performs function argument and return value type coercion "
                    "to ensure ABI compliance",
                    false, false)

CoerceTypes::CoerceTypes() : ModulePass(ID) {}

bool CoerceTypes::runOnModule(Module &M) {
  m_pModule = &M;
  m_pDataLayout = &M.getDataLayout();
  m_FunctionMap.clear();
  bool Changed = false;

  // Leave kernel signatures intact
  KernelList KL(m_pModule);
  SmallPtrSet<Function *, 16> Kernels(KL.begin(), KL.end());

  // Store the original functions since some of them will be replaced with new
  // ones
  std::vector<Function *> FuncsToHandle;
  for (auto &Func : M) {
    if (!Func.isIntrinsic() && isFunctionSupported(&Func) &&
        !Kernels.count(&Func))
      FuncsToHandle.push_back(&Func);
  }

  for (auto *Func : FuncsToHandle)
    Changed |= runOnFunction(Func);

  CompilationUtils::updateFunctionMetadata(m_pModule, m_FunctionMap);

  // TODO handle indirect calls
  return Changed;
}

bool CoerceTypes::runOnFunction(Function *F) {
  // Leave intrinsic functions unchanged
  if (F->isIntrinsic())
    return false;

  // TODO handle only the needed calling conventions
  bool Changed = false;
  SmallVector<Type *, 16> OldArgTypes;
  SmallVector<Type *, 16> NewArgTypes;
  SmallVector<TypePair, 16> NewArgTypePairs;
  // Track the number of registers left, structs might require two when only one
  // is available
  unsigned NFreeIntRegs = 6;
  unsigned NFreeSSERegs = 8;

  // Coerce function argument types
  for (auto &Arg : F->args()) {
    TypePair TP = getCoercedType(&Arg, NFreeIntRegs, NFreeSSERegs);
    OldArgTypes.push_back(Arg.getType());
    NewArgTypePairs.push_back(TP);
    NewArgTypes.push_back(TP.first);
    Changed |= (Arg.getType() != TP.first);
    if (TP.second)
      NewArgTypes.push_back(TP.second);
  }
  // TODO coerce return value type as well
  Type *RetType = F->getReturnType();

  // Replace function with a new one
  // TODO eliminate code duplication with CompilationUtils and AddImplicitArgs
  if (Changed) {
    FunctionType *FuncType =
        FunctionType::get(RetType, NewArgTypes, /*isVarArg*/ false);
    std::string Name = F->getName().str();
    F->setName("__" + F->getName() + "_before.CoerceTypes");
    Function *NewF =
        Function::Create(FuncType, F->getLinkage(), Name, m_pModule);
    m_FunctionMap[F] = NewF;

    NewF->copyMetadata(F, 0);
    copyAttributesAndArgNames(F, NewF, NewArgTypePairs);
    NewF->setSubprogram(F->getSubprogram());
    NewF->setComdat(F->getComdat());
    if (!F->isDeclaration()) {
      moveFunctionBody(F, NewF, NewArgTypePairs);
      // F becomes a declaration and it should not contain Comdat.
      F->setComdat(nullptr);
    }
    // Replace F with NewF in KernelList module Metadata (if any)
    llvm::Module *M = F->getParent();
    assert(M && "Module is NULL");
    auto Kernels = KernelList(M).getList();
    std::replace_if(std::begin(Kernels), std::end(Kernels),
                    [F](llvm::Function *Func) { return F == Func; }, NewF);
    KernelList(M).set(Kernels);

    // Patch users of the old function
    for (auto UI = F->user_begin(), UE = F->user_end(); UI != UE;) {
      // Increment UI before eraseFromParent() to avoid operating on freed memory.
      if (CallInst *CI = dyn_cast<CallInst>(*UI++)) {
        // replace call instruction
        SmallVector<Value *, 16> Args;
        size_t I = 0;
        IRBuilder<> Builder(CI);
        for (const auto &NewArgTypePair : NewArgTypePairs) {
          if (NewArgTypePair.first == OldArgTypes[I]) {
            Args.push_back(CI->getArgOperand(I));
            ++I;
            continue;
          }

          auto OldArgT = cast<PointerType>(OldArgTypes[I]);
          auto OldStructT = cast<StructType>(OldArgT->getElementType());
          // Bitcast the original structure to the new coerced type (a struct if
          // there are 2 eightbytes), load values of the coerced type from it
          // and pass to the new function. Example for struct.coerced = {i64,
          // double}
          // %1 = bitcast %struct.original* %0 to %struct.coerced*
          // %2 = getelementptr %struct.coerced, %1, i32 0, i32 0
          // %3 = load i64, i64* %2
          // %4 = getelementptr %struct.coerced, %1, i32 0, i32 1
          // %5 = load double, double* %4
          // call void @foo(i64 %3, double %5)
          Value *BC = Builder.CreateBitCast(
              CI->getArgOperand(I),
              PointerType::get(
                  getCombinedCoercedType(NewArgTypePair, OldStructT->getName()),
                  OldArgT->getAddressSpace()));
          Value *LoadSrc = BC;
          SmallVector<Value *, 2> Indices(
              2, ConstantInt::get(IntegerType::get(m_pModule->getContext(), 32),
                                  0));
          if (NewArgTypePair.second)
            LoadSrc = Builder.CreateGEP(BC, Indices);

          Value *Load = Builder.CreateLoad(NewArgTypePair.first, LoadSrc);
          Args.push_back(Load);
          if (NewArgTypePair.second) {
            Indices[1] = ConstantInt::get(
                IntegerType::get(m_pModule->getContext(), 32), 1);
            LoadSrc = Builder.CreateGEP(BC, Indices);
            Load = Builder.CreateLoad(NewArgTypePair.second, LoadSrc);
            Args.push_back(Load);
          }
          ++I;
        }
        CallInst *NewCI = CallInst::Create(NewF, Args, "", CI);
        if (CI->hasMetadata())
          NewCI->setDebugLoc(CI->getDebugLoc());
        CI->replaceAllUsesWith(NewCI);
        CI->eraseFromParent();
      } else
        llvm_unreachable("Unhandled function reference");
    }
  }
  return Changed;
}

bool CoerceTypes::isFunctionSupported(Function *F) {
  // Leave functions with users that are not supported yet (function pointer
  // related) unchanged
  for (User *U : F->users()) {
    if (!dyn_cast<CallInst>(U))
      return false;
  }
  return true;
}

CoerceTypes::TypePair CoerceTypes::getCoercedType(Argument *Arg,
                                                  unsigned &FreeIntRegs,
                                                  unsigned &FreeSSERegs) const {
  Type *T = Arg->getType();
  if (auto *PtrT = dyn_cast<PointerType>(T)) {
    // TODO Handle empty structs by excluding them from the new argument list
    if (auto *StructT = dyn_cast<StructType>(PtrT->getElementType())) {
      if (!Arg->hasByValAttr())
        return {T, nullptr};
      ClassPair Classes = classifyStruct(StructT);
      // Leave MEMORY as-is to be passed on stack
      // TODO NO_CLASS is currently returned for empty structs, such structs
      // should be omitted in the argument list instead
      if (Classes.first == TypeClass::MEMORY ||
          Classes.first == TypeClass::NO_CLASS)
        return {T, nullptr};

      unsigned RequiredIntRegs = (Classes.first == TypeClass::INTEGER) +
                                 (Classes.second == TypeClass::INTEGER);
      unsigned RequiredSSERegs = (Classes.first == TypeClass::SSE) +
                                 (Classes.second == TypeClass::SSE);
      // Pass on stack if not enough registers for the whole type
      if (RequiredIntRegs > FreeIntRegs || RequiredSSERegs > FreeSSERegs)
        return {T, nullptr};

      FreeIntRegs -= RequiredIntRegs;
      FreeSSERegs -= RequiredSSERegs;
      return {getCoercedType(StructT, 0, Classes.first),
              getCoercedType(StructT, 8, Classes.second)};
    }
  }
  // If the type is not coerced, just update the number of free registers
  // TODO handle vector types here as well
  if (!T->isVectorTy()) {
    switch (classifyScalar(T)) {
    case TypeClass::INTEGER:
      FreeIntRegs = FreeIntRegs ? FreeIntRegs - 1 : 0;
      break;
    case TypeClass::SSE:
      FreeSSERegs = FreeSSERegs ? FreeSSERegs - 1 : 0;
      break;
    default:
      break;
    }
  }
  return {T, nullptr};
}

Type *CoerceTypes::getCoercedType(StructType *StructT, unsigned Offset,
                                  TypeClass Class) const {
  assert((Offset == 0 || Offset == 8) && "Unexpected offset");
  switch (Class) {
  case TypeClass::NO_CLASS:
    return nullptr;
  case TypeClass::INTEGER:
    return getIntegerType(StructT, Offset);
  case TypeClass::SSE:
    return getSSEType(StructT, Offset);
  default:
    llvm_unreachable("Unexpected classification");
    return nullptr;
  }
}

Type *CoerceTypes::getIntegerType(StructType *T, unsigned Offset) const {
  Type *FieldT = getNonCompositeTypeAtExactOffset(T, Offset);
  if (FieldT && (FieldT->isPointerTy() || FieldT->isIntegerTy(64)))
    return FieldT;

  const StructLayout *Layout = m_pDataLayout->getStructLayout(T);
  unsigned Size = Layout->getSizeInBytes();

  // TODO can do better here, no need to always extend the first eightbyte type
  // of a two-eightbyte struct to 64 bits
  return IntegerType::get(m_pModule->getContext(), std::min(Size, 8U) * 8);
}

Type *CoerceTypes::getSSEType(StructType *T, unsigned Offset) const {
  Type *TyAt0 = getNonCompositeTypeAtExactOffset(T, Offset);
  if (TyAt0 && TyAt0->isFloatTy()) {
    // Since the class is SSE, the other 4 bytes are either float or empty
    // Coerce to <2 x float> in the first case
    Type *TyAt4 = getNonCompositeTypeAtExactOffset(T, Offset + 4);
    if (TyAt4 && TyAt4->isFloatTy())
      return VectorType::get(Type::getFloatTy(m_pModule->getContext()), 2);
    // Coerce to float in the second
    return Type::getFloatTy(m_pModule->getContext());
  }

  // The only other usable type is double
  return Type::getDoubleTy(m_pModule->getContext());
}

Type *CoerceTypes::getNonCompositeTypeAtExactOffset(Type *T,
                                                    unsigned Offset) const {
  if (auto *StructT = dyn_cast<StructType>(T)) {
    const StructLayout *SL = m_pDataLayout->getStructLayout(StructT);
    unsigned ElementIdx = SL->getElementContainingOffset(Offset);
    Offset -= SL->getElementOffset(ElementIdx);
    return getNonCompositeTypeAtExactOffset(StructT->getElementType(ElementIdx),
                                            Offset);
  }

  if (auto *ArrayT = dyn_cast<ArrayType>(T)) {
    Type *ElementT = ArrayT->getElementType();
    unsigned ElementSize = m_pDataLayout->getTypeAllocSize(ElementT);
    Offset -= (Offset / ElementSize) * ElementSize;
    return getNonCompositeTypeAtExactOffset(ElementT, Offset);
  }

  // TODO remove the assertion once vector type coercion is supported
  assert(!T->isVectorTy() && "Unexpected vector field");

  if (Offset == 0)
    return T;
  return nullptr;
}

CoerceTypes::ClassPair CoerceTypes::classify(Type *T, unsigned Offset) const {
  if (T->isEmptyTy())
    return {TypeClass::NO_CLASS, TypeClass::NO_CLASS};

  if (auto *ArrayT = dyn_cast<ArrayType>(T)) {
    Type *ElementT = ArrayT->getElementType();
    ClassPair Result = classify(ElementT, Offset);
    // If the array occupies both eightbytes, classify the high one as well
    if (Offset < 8 && m_pDataLayout->getTypeAllocSize(ArrayT) + Offset > 8)
      Result.second = Result.first;
    return Result;
  }

  if (auto *StructT = dyn_cast<StructType>(T))
    return classifyStruct(StructT, Offset);

  // TODO vector type coercion, leave them as-is by classifying as MEMORY for
  // now
  if (T->isVectorTy())
    return {TypeClass::MEMORY, TypeClass::MEMORY};

  TypeClass Class = classifyScalar(T);
  assert(((Offset < 8) == (m_pDataLayout->getTypeAllocSize(T) + Offset <= 8)) &&
         "Unexpected scalar type occupying both eightbytes");
  // Classify the eightbyte occupied by the scalar
  if (Offset < 8)
    return {Class, TypeClass::NO_CLASS};
  return {TypeClass::NO_CLASS, Class};
}

CoerceTypes::ClassPair CoerceTypes::classifyStruct(StructType *T,
                                                   unsigned Offset) const {
  const StructLayout *Layout = m_pDataLayout->getStructLayout(T);
  if (Layout->getSizeInBytes() > 16 || T->isPacked())
    return {TypeClass::MEMORY, TypeClass::MEMORY};

  ClassPair Result(TypeClass::NO_CLASS, TypeClass::NO_CLASS);
  // Iterate through all fields, merging their classes along the way
  size_t I = 0;
  for (Type *Element : T->elements()) {
    unsigned CurrentOffset = Offset + Layout->getElementOffset(I);
    Result = mergeClasses(Result, classify(Element, CurrentOffset));
    ++I;
  }

  // X86_64 ABI post-merge algorithm: if either eightbyte is MEMORY, consider
  // all of them as such. Caller checks only the first class for MEMORY, so
  // handle only that case.
  if (Result.second == TypeClass::MEMORY)
    Result.first = TypeClass::MEMORY;

  return Result;
}

CoerceTypes::TypeClass CoerceTypes::classifyScalar(Type *T) const {
  // Leave arbitrary-sized LLVM IR types as-is
  if (m_pDataLayout->getTypeAllocSize(T) > 8)
    return TypeClass::MEMORY;

  if (T->isIntegerTy() || T->isPointerTy())
    return TypeClass::INTEGER;
  if (T->isFloatingPointTy())
    return TypeClass::SSE;
  llvm_unreachable("Unexpected type");
  return TypeClass::NO_CLASS;
}

CoerceTypes::ClassPair
CoerceTypes::mergeClasses(CoerceTypes::ClassPair A,
                          CoerceTypes::ClassPair B) const {
  // Assumes that the classes are enumerated in the order of their preference
  assert(TypeClass::NO_CLASS < TypeClass::SSE &&
         TypeClass::SSE < TypeClass::INTEGER &&
         TypeClass::INTEGER < TypeClass::MEMORY);
  return {std::max(A.first, B.first), std::max(A.second, B.second)};
}

Type *CoerceTypes::getCombinedCoercedType(CoerceTypes::TypePair CoercedTypes,
                                          StringRef OriginalTypeName) const {
  return CoercedTypes.second
             ? StructType::create({CoercedTypes.first, CoercedTypes.second},
                                  Twine(OriginalTypeName + ".coerce").str())
             : CoercedTypes.first;
}

void CoerceTypes::copyAttributesAndArgNames(
    Function *OldF, Function *NewF, ArrayRef<TypePair> NewArgTypePairs) {
  AttributeList OldAttrList = OldF->getAttributes();
  SmallVector<AttributeSet, 16> ArgAttrs;
  Function::arg_iterator OldArgI = OldF->arg_begin();
  Function::arg_iterator NewArgI = NewF->arg_begin();
  size_t I = 0;
  // Collect attributes and name new arguments
  for (const auto &NewArgTypePair : NewArgTypePairs) {
    if (NewArgTypePair.first == OldArgI->getType()) {
      assert(!NewArgTypePair.second && "Unexpected second type");
      ArgAttrs.push_back(
          OldAttrList.getAttributes(I + AttributeList::FirstArgIndex));
      NewArgI->setName(OldArgI->getName());
      ++OldArgI;
      ++I;
      ++NewArgI;
      continue;
    }

    StringRef OldName = OldArgI->getName();
    if (!OldName.empty())
      NewArgI->setName(OldName + ".coerce.high");
    ArgAttrs.push_back(AttributeSet::get(m_pModule->getContext(), None));
    ++NewArgI;

    if (NewArgTypePair.second) {
      if (!OldName.empty())
        NewArgI->setName(OldName + ".coerce.low");
      ArgAttrs.push_back(AttributeSet::get(m_pModule->getContext(), None));
      ++NewArgI;
    }
    ++OldArgI;
    ++I;
  }

  AttributeSet FuncAttrs =
      OldAttrList.getAttributes(AttributeList::FunctionIndex);
  AttributeSet RetAttrs = OldAttrList.getAttributes(AttributeList::ReturnIndex);
  AttributeList NewAttrs = AttributeList::get(m_pModule->getContext(),
                                              FuncAttrs, RetAttrs, ArgAttrs);
  NewF->setAttributes(NewAttrs);
}

void CoerceTypes::moveFunctionBody(Function *OldF, Function *NewF,
                                   ArrayRef<TypePair> NewArgTypePairs) {
  // Splice the body of the old function into the new one
  NewF->getBasicBlockList().splice(NewF->begin(), OldF->getBasicBlockList());

  // Delete original function body - this is needed to remove linkage (if
  // exists)
  OldF->deleteBody();

  // Loop over the original arguments, replace the use of each with the values
  // obtained from coerced ones.
  Function::arg_iterator OldArgI = OldF->arg_begin();
  Function::arg_iterator NewArgI = NewF->arg_begin();
  IRBuilder<> Builder(&*NewF->getEntryBlock().begin());
  for (const auto &NewArgTypePair : NewArgTypePairs) {
    // If the coerced is the same as the original, replace with the new version
    if (NewArgTypePair.first == OldArgI->getType()) {
      assert(!NewArgTypePair.second && "Unexpected second type");
      OldArgI->replaceAllUsesWith(&*NewArgI);
      ++OldArgI;
      ++NewArgI;
      continue;
    }
    // Otherwise allocate the original type, store values from the coerced
    // arguments, replace uses of old argument with the allocated value
    auto OldArgT = cast<PointerType>(OldArgI->getType());
    AllocaInst *Alloca = Builder.CreateAlloca(OldArgT->getElementType(),
                                              OldArgT->getAddressSpace());
    auto OldStructT = cast<StructType>(OldArgT->getElementType());
    Value *BC = Builder.CreateBitCast(
        Alloca, PointerType::get(getCombinedCoercedType(NewArgTypePair,
                                                        OldStructT->getName()),
                                 OldArgT->getAddressSpace()));

    // If the combined coerced type is a struct itself, insert GEPs
    Value *StoreDest = BC;
    SmallVector<Value *, 2> Indices(
        2, ConstantInt::get(IntegerType::get(m_pModule->getContext(), 32), 0));
    if (NewArgTypePair.second)
      StoreDest = Builder.CreateGEP(BC, Indices);
    Builder.CreateStore(&*NewArgI, StoreDest);
    ++NewArgI;

    if (NewArgTypePair.second) {
      Indices[1] =
          ConstantInt::get(IntegerType::get(m_pModule->getContext(), 32), 1);
      StoreDest = Builder.CreateGEP(BC, Indices);
      Builder.CreateStore(&*NewArgI, StoreDest);
      ++NewArgI;
    }
    OldArgI->replaceAllUsesWith(Alloca);
    ++OldArgI;
  }
}
} // namespace intel
