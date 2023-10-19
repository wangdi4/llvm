//===- CoerceTypes.cpp - CoerceTypes pass C++ -*--------------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/CoerceTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"

using namespace llvm;
using namespace SYCLKernelMetadataAPI;
using namespace CompilationUtils;

#define DEBUG_TYPE "sycl-kernel-coerce-types"

PreservedAnalyses CoerceTypesPass::run(Module &M, ModuleAnalysisManager &AM) {
  if (!runImpl(M))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}

static Value *createAllocaInst(Type *Ty, Function *F, unsigned Alignment,
                               unsigned AS) {
  Module *M = F->getParent();
  const DataLayout &DL = M->getDataLayout();
  unsigned AllocaAS = DL.getAllocaAddrSpace();
  IRBuilder<> Builder(&F->getEntryBlock().front());
  AllocaInst *AllocaRes = Builder.CreateAlloca(Ty, AllocaAS);
  // If the alignment is defined, set it.
  if (Alignment)
    AllocaRes->setAlignment(Align(Alignment));
  if (AS != AllocaAS) {
    LLVM_DEBUG(dbgs() << "Create addrspacecast for alloca instruction ("
                      << *AllocaRes << ")\n  from address space " << AllocaAS
                      << " to " << AS << "\n");
    return Builder.CreateAddrSpaceCast(AllocaRes, PointerType::get(Ty, AS));
  }
  return AllocaRes;
}

bool CoerceTypesPass::runImpl(Module &M) {
  PModule = &M;
  PDataLayout = &M.getDataLayout();
  FunctionMap.clear();
  bool Changed = false;

  // Leave kernel signatures intact
  KernelList KL(PModule);
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

  updateFunctionMetadata(PModule, FunctionMap);

  // TODO handle indirect calls
  return Changed;
}

bool CoerceTypesPass::runOnFunction(Function *F) {
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

  DenseMap<unsigned, std::pair<unsigned, uint64_t>> ValueMap;
  // Coerce function argument types
  for (auto &Arg : F->args()) {
    TypePair TP = getCoercedType(&Arg, NFreeIntRegs, NFreeSSERegs);
    if (Arg.getType() == TP.first && Arg.hasByValAttr()) {
      Changed = true;
      Type *ArgMemTy = Arg.getParamByValType();
      uint64_t MemSize = PDataLayout->getTypeAllocSize(ArgMemTy);
      // If alignment isn't available for the arg, set to default of 1 byte.
      ValueMap[Arg.getArgNo()] = {Arg.getParamAlign().valueOrOne().value(),
                                  MemSize};
      TP = {Arg.getType(), nullptr};
    }
    Changed |= (Arg.getType() != TP.first);
    OldArgTypes.push_back(Arg.getType());
    NewArgTypePairs.push_back(TP);
    NewArgTypes.push_back(TP.first);
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
    Function *NewF = Function::Create(FuncType, F->getLinkage(), Name, PModule);
    FunctionMap[F] = NewF;

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
    std::replace_if(
        std::begin(Kernels), std::end(Kernels),
        [F](llvm::Function *Func) { return F == Func; }, NewF);
    KernelList(M).set(Kernels);

    // Patch users of the old function
    for (auto UI = F->user_begin(), UE = F->user_end(); UI != UE;) {
      // Increment UI before eraseFromParent() to avoid operating on freed
      // memory.
      if (CallInst *CI = dyn_cast<CallInst>(*UI++)) {
        // replace call instruction
        SmallVector<Value *, 16> Args;
        size_t I = 0;
        IRBuilder<> Builder(CI);
        for (const auto &NewArgTypePair : NewArgTypePairs) {
          if (NewArgTypePair.first == OldArgTypes[I]) {
            // For the byval arguments that can not be split, we need to handle
            // them as we do on windows.
            if (CI->paramHasAttr(I, Attribute::ByVal)) {
              Value *ArgI = CI->getArgOperand(I);
              auto *PT = cast<PointerType>(ArgI->getType());
              Type *ElementTy = CI->getParamByValType(I);
              // In case that FE doesn't pass the size information.
              unsigned Alignment = ValueMap[I].first;
              uint64_t MemSize = ValueMap[I].second;
              Value *Alloca =
                  createAllocaInst(ElementTy, CI->getFunction(), Alignment,
                                   PT->getAddressSpace());
              Value *DstPtr = Builder.CreateInBoundsGEP(ElementTy, Alloca,
                                                        Builder.getInt32(0));
              Builder.CreateMemCpy(DstPtr, MaybeAlign(Alignment), ArgI,
                                   MaybeAlign(Alignment), MemSize);
              Args.push_back(Alloca);
            } else
              Args.push_back(CI->getArgOperand(I));
            ++I;
            continue;
          }

          auto *OldArgT = cast<PointerType>(OldArgTypes[I]);
          auto *OldStructT = cast<StructType>(CI->getParamByValType(I));
          LLVM_DEBUG(dbgs()
                     << "Old argument type: " << *(OldArgTypes[I])
                     << ". Original struct type: " << *OldStructT << "\n");
          // Bitcast the original structure to the new coerced type (a struct if
          // there are 2 eightbytes), load values of the coerced type from it
          // and pass to the new function. Example for struct.coerced = {i64,
          // double}
          // IR with opaque pointer disabled:
          // %1 = bitcast %struct.original* %0 to %struct.coerced*
          // %2 = getelementptr %struct.coerced, %1, i32 0, i32 0
          // %3 = load i64, i64* %2
          // %4 = getelementptr %struct.coerced, %1, i32 0, i32 1
          // %5 = load double, double* %4
          // call void @foo(i64 %3, double %5)
          //
          // IR with opaque pointer enabled:
          // %1 = getelementptr %struct.coerced, ptr %0, i32 0, i32 0
          // %2 = load i64, ptr %1
          // %3 = getelementptr %struct.coerced, ptr %0, i32 0, i32 1
          // %4 = load double, ptr %3
          // call void @foo(i64 %2, double %4)
          Type *PointeeTy = getCombinedCoercedType(
              NewArgTypePair,
              OldStructT->hasName() ? OldStructT->getName() : "");
          Value *BC = Builder.CreateBitCast(
              CI->getArgOperand(I),
              PointerType::get(PointeeTy, OldArgT->getAddressSpace()));
          Value *LoadSrc = BC;
          SmallVector<Value *, 2> Indices(2, Builder.getInt32(0));
          if (NewArgTypePair.second)
            LoadSrc = Builder.CreateGEP(PointeeTy, BC, Indices);

          LLVM_DEBUG(dbgs()
                     << "New argument type: " << *(NewArgTypePair.first)
                     << ".\nSource in load instruction: " << *LoadSrc << "\n");
          Value *Load = Builder.CreateLoad(NewArgTypePair.first, LoadSrc);
          LLVM_DEBUG(dbgs() << "Load instruction: " << *Load << "\n");
          Args.push_back(Load);
          if (NewArgTypePair.second) {
            Indices[1] = Builder.getInt32(1);
            LoadSrc = Builder.CreateGEP(PointeeTy, BC, Indices);
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

bool CoerceTypesPass::isFunctionSupported(Function *F) {
  // Leave functions with users that are not supported yet (function pointer
  // related) unchanged
  for (User *U : F->users()) {
    auto *CI = dyn_cast<CallInst>(U);
    if (!CI || CI->getCalledFunction() != F)
      return false;
  }
  return true;
}

CoerceTypesPass::TypePair
CoerceTypesPass::getCoercedType(Argument *Arg, unsigned &FreeIntRegs,
                                unsigned &FreeSSERegs) const {
  Type *T = Arg->getType();
  LLVM_DEBUG(dbgs() << "Argument type: " << *T << "\n");
  if (auto *PtrT = dyn_cast<PointerType>(T)) {
    if (!Arg->hasByValAttr())
      return {T, nullptr};

    // TODO Handle empty structs by excluding them from the new argument list
    if (auto *StructT = dyn_cast<StructType>(Arg->getParamByValType())) {
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

Type *CoerceTypesPass::getCoercedType(StructType *StructT, unsigned Offset,
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

Type *CoerceTypesPass::getIntegerType(StructType *T, unsigned Offset) const {
  Type *FieldT = getNonCompositeTypeAtExactOffset(T, Offset);
  if (FieldT && (FieldT->isPointerTy() || FieldT->isIntegerTy(64)))
    return FieldT;

  const StructLayout *Layout = PDataLayout->getStructLayout(T);
  unsigned Size = Layout->getSizeInBytes();

  // TODO can do better here, no need to always extend the first eightbyte type
  // of a two-eightbyte struct to 64 bits
  assert(Offset <= Size &&
         "Offset of struct element should be less than struct size");
  return IntegerType::get(PModule->getContext(),
                          std::min(Size - Offset, 8U) * 8);
}

Type *CoerceTypesPass::getSSEType(StructType *T, unsigned Offset) const {
  Type *TyAt0 = getNonCompositeTypeAtExactOffset(T, Offset);
  assert(TyAt0 && "Unexpected SSE type that occupies more than 1 eightbyte!");
  if (TyAt0->is16bitFPTy()) {
    // The other 6 bytes are either empty or a combination of half/bfloat,
    // float.
    // Since "half" and "bfloat" have the same bit width, we treat both
    // types consistently.
    Type *FP16Ty = TyAt0;
    unsigned NumOfConsecutiveFP16 = 1;
    if (Type *TyAt2 = getNonCompositeTypeAtExactOffset(T, Offset + 2)) {
      // Example struct: { half, half, ... }
      assert(TyAt2->is16bitFPTy() && "Unexpected SSE type at offset 2");
      ++NumOfConsecutiveFP16;
    }
    if (Type *TyAt4 = getNonCompositeTypeAtExactOffset(T, Offset + 4)) {
      if (TyAt4->is16bitFPTy()) {
        // { half, half, half, ... }
        ++NumOfConsecutiveFP16;
      } else {
        // { half, half, float } or { half, float }
        assert(TyAt4->isFloatTy() && "Unexpected SSE type at offset 4");
        // In this case we coerce the whole eightbyte to a double.
        return Type::getDoubleTy(PModule->getContext());
      }
    }
    if (Type *TyAt6 = getNonCompositeTypeAtExactOffset(T, Offset + 6)) {
      // { half, half, half, half }
      assert(TyAt6->is16bitFPTy() && "Unexpected SSE type at offset 6");
      ++NumOfConsecutiveFP16;
    }

    // Coerce to <N x half> or <N x bfloat> if there're multiple consecutive
    // 2-byte fp types.
    return NumOfConsecutiveFP16 == 1
               ? FP16Ty
               : FixedVectorType::get(FP16Ty, NumOfConsecutiveFP16);
  }

  if (TyAt0->isFloatTy()) {
    // Since the class is SSE, the other 4 bytes are either float or empty
    // Coerce to <2 x float> in the first case
    Type *TyAt4 = getNonCompositeTypeAtExactOffset(T, Offset + 4);
    if (TyAt4 && TyAt4->isFloatTy())
      return FixedVectorType::get(Type::getFloatTy(PModule->getContext()), 2);
    // Coerce to float in the second
    return Type::getFloatTy(PModule->getContext());
  }

  // The only other usable type is double
  return Type::getDoubleTy(PModule->getContext());
}

Type *CoerceTypesPass::getNonCompositeTypeAtExactOffset(Type *T,
                                                        unsigned Offset) const {
  if (auto *StructT = dyn_cast<StructType>(T)) {
    const StructLayout *SL = PDataLayout->getStructLayout(StructT);
    unsigned ElementIdx = SL->getElementContainingOffset(Offset);
    Offset -= SL->getElementOffset(ElementIdx);
    return getNonCompositeTypeAtExactOffset(StructT->getElementType(ElementIdx),
                                            Offset);
  }

  if (auto *ArrayT = dyn_cast<ArrayType>(T)) {
    Type *ElementT = ArrayT->getElementType();
    unsigned ElementSize = PDataLayout->getTypeAllocSize(ElementT);
    unsigned OffsetElement = Offset / ElementSize;
    if (OffsetElement >= (unsigned)ArrayT->getNumElements())
      return nullptr;
    Offset -= OffsetElement * ElementSize;
    return getNonCompositeTypeAtExactOffset(ElementT, Offset);
  }

  // TODO remove the assertion once vector type coercion is supported
  assert(!T->isVectorTy() && "Unexpected vector field");

  if (Offset == 0)
    return T;
  return nullptr;
}

CoerceTypesPass::ClassPair CoerceTypesPass::classify(Type *T,
                                                     unsigned Offset) const {
  if (T->isEmptyTy())
    return {TypeClass::NO_CLASS, TypeClass::NO_CLASS};

  if (auto *ArrayT = dyn_cast<ArrayType>(T)) {
    auto ArrayAllocSize = PDataLayout->getTypeAllocSize(ArrayT);
    // If the last eightbyte remainder of the array can not be fully represented
    // by any normal integer types (i8, i16, i32, i64), we conservatively pass
    // it by MEMORY to avoid irregular integer parameter widening issue of SG
    // emulation.
    // FIXEME: we should teach subgroup emulation to properly handle irregular
    // integer parameter (e.g. i48) instead.
    auto RemainderSize = ArrayAllocSize % 8;
    if (RemainderSize != 0 && RemainderSize != 1 && RemainderSize != 2 &&
        RemainderSize != 4) {
      LLVM_DEBUG(dbgs() << "Classify irregular array type as MEMORY: "
                        << *ArrayT << ", size = " << ArrayAllocSize << '\n');
      return {TypeClass::MEMORY, TypeClass::MEMORY};
    }
    Type *ElementT = ArrayT->getElementType();
    ClassPair Result = classify(ElementT, Offset);
    // If the array occupies both eightbytes, classify the high one as well
    if (Offset < 8 && ArrayAllocSize + Offset > 8)
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
  assert(((Offset < 8) == (PDataLayout->getTypeAllocSize(T) + Offset <= 8)) &&
         "Unexpected scalar type occupying both eightbytes");
  // Classify the eightbyte occupied by the scalar
  if (Offset < 8)
    return {Class, TypeClass::NO_CLASS};
  return {TypeClass::NO_CLASS, Class};
}

CoerceTypesPass::ClassPair
CoerceTypesPass::classifyStruct(StructType *T, unsigned Offset) const {
  const StructLayout *Layout = PDataLayout->getStructLayout(T);
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

CoerceTypesPass::TypeClass CoerceTypesPass::classifyScalar(Type *T) const {
  // Leave arbitrary-sized LLVM IR types as-is
  if (PDataLayout->getTypeAllocSize(T) > 8)
    return TypeClass::MEMORY;

  if (T->isIntegerTy() || T->isPointerTy())
    return TypeClass::INTEGER;
  if (T->isFloatingPointTy())
    return TypeClass::SSE;
  llvm_unreachable("Unexpected type");
  return TypeClass::NO_CLASS;
}

CoerceTypesPass::ClassPair
CoerceTypesPass::mergeClasses(CoerceTypesPass::ClassPair A,
                              CoerceTypesPass::ClassPair B) const {
  // Assumes that the classes are enumerated in the order of their preference
  static_assert(TypeClass::NO_CLASS < TypeClass::SSE &&
                    TypeClass::SSE < TypeClass::INTEGER &&
                    TypeClass::INTEGER < TypeClass::MEMORY,
                "");
  return {std::max(A.first, B.first), std::max(A.second, B.second)};
}

Type *
CoerceTypesPass::getCombinedCoercedType(CoerceTypesPass::TypePair CoercedTypes,
                                        StringRef OriginalTypeName) const {
  return CoercedTypes.second
             ? StructType::create({CoercedTypes.first, CoercedTypes.second},
                                  Twine(OriginalTypeName + ".coerce").str())
             : CoercedTypes.first;
}

void CoerceTypesPass::copyAttributesAndArgNames(
    Function *OldF, Function *NewF, ArrayRef<TypePair> NewArgTypePairs) {
  AttributeList OldAttrList = OldF->getAttributes();
  SmallVector<AttributeSet, 16> ArgAttrs;
  Function::arg_iterator OldArgI = OldF->arg_begin();
  Function::arg_iterator NewArgI = NewF->arg_begin();
  size_t I = 0;
  // Collect attributes and name new arguments
  bool ByValRemoved = false;
  for (const auto &NewArgTypePair : NewArgTypePairs) {
    if (NewArgTypePair.first == OldArgI->getType()) {
      assert(!NewArgTypePair.second && "Unexpected second type");
      if (OldAttrList.hasParamAttr(I, Attribute::ByVal)) {
        OldAttrList =
            OldAttrList.removeParamAttributes(PModule->getContext(), I);
        if (NewF->getName().startswith("__intel_indirect_call"))
          ByValRemoved = true;
      }
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
    ArgAttrs.push_back(AttributeSet::get(PModule->getContext(), std::nullopt));
    ++NewArgI;

    if (NewArgTypePair.second) {
      if (!OldName.empty())
        NewArgI->setName(OldName + ".coerce.low");
      ArgAttrs.push_back(
          AttributeSet::get(PModule->getContext(), std::nullopt));
      ++NewArgI;
    }
    ++OldArgI;
    ++I;
  }

  AttributeSet FuncAttrs =
      OldAttrList.getAttributes(AttributeList::FunctionIndex);
  AttributeSet RetAttrs = OldAttrList.getAttributes(AttributeList::ReturnIndex);
  AttributeList NewAttrs =
      AttributeList::get(PModule->getContext(), FuncAttrs, RetAttrs, ArgAttrs);
  NewF->setAttributes(NewAttrs);
  // Since this pass removes byval attributes from __intel_indirect_call_xxx
  // args but not from the args of the indirect call itself, this can lead to
  // inconsistencies which won't prevent SGSizeCollectorIndirect from adding
  // "vector-variant" attributes for these calls. Since these calls cannot yet
  // be vectorized, we need to ensure that any such calls retain byval
  // attribute history so that "vector-variants" attributes are not added and
  // vectorization can be prevented. In other words, the indirect call still
  // has the byval attribute but the call to __intel_indirect_call_xxx doesn't
  // and this leads to problems when we add "vector-variants" attributes later
  // in SGSizeCollectorIndirect.
  if (ByValRemoved) {
    NewF->addFnAttr("ByValRemoved");
  }
}

void CoerceTypesPass::moveFunctionBody(Function *OldF, Function *NewF,
                                       ArrayRef<TypePair> NewArgTypePairs) {
  // Splice the body of the old function into the new one
  NewF->splice(NewF->begin(), OldF);

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
    auto *OldArgT = cast<PointerType>(OldArgI->getType());
    Value *Alloca = [&]() -> Value * {
      Type *ParamTy = OldArgI->getParamByValType();
      assert(ParamTy && "No byval type for the parameter!");

      AllocaInst *AllocaRes =
          Builder.CreateAlloca(ParamTy, PDataLayout->getAllocaAddrSpace());
      MaybeAlign Alignment = OldArgI->getParamAlign();
      if (Alignment)
        AllocaRes->setAlignment(*Alignment);
      if (PDataLayout->getAllocaAddrSpace() != OldArgT->getAddressSpace()) {
        LLVM_DEBUG(dbgs() << "Create addrspacecast from address space "
                          << PDataLayout->getAllocaAddrSpace() << " to "
                          << OldArgT->getAddressSpace() << "\n");
        return Builder.CreateAddrSpaceCast(
            AllocaRes, PointerType::get(OldArgT->getContext(),
                                        OldArgT->getAddressSpace()));
      }
      return AllocaRes;
    }();
    auto *OldStructT = cast<StructType>(OldArgI->getParamByValType());
    Type *PointeeTy = getCombinedCoercedType(
        NewArgTypePair, OldStructT->hasName() ? OldStructT->getName() : "");
    Value *BC = Builder.CreateBitCast(
        Alloca, PointerType::get(PointeeTy, OldArgT->getAddressSpace()));

    // If the combined coerced type is a struct itself, insert GEPs
    Value *StoreDest = BC;
    SmallVector<Value *, 2> Indices(2, Builder.getInt32(0));
    if (NewArgTypePair.second)
      StoreDest = Builder.CreateGEP(PointeeTy, BC, Indices);
    Builder.CreateStore(&*NewArgI, StoreDest);
    ++NewArgI;

    if (NewArgTypePair.second) {
      Indices[1] = Builder.getInt32(1);
      StoreDest = Builder.CreateGEP(PointeeTy, BC, Indices);
      Builder.CreateStore(&*NewArgI, StoreDest);
      ++NewArgI;
    }
    OldArgI->replaceAllUsesWith(Alloca);
    ++OldArgI;
  }
}
