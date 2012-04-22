#undef DEBUG_TYPE
#define DEBUG_TYPE "micresolver"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/CommandLine.h"
#include "MICResolver.h"
#include "Mangler.h"
#include "Logger.h"
#include "llvm/Constants.h"

#include <vector>



static bool isGatherScatterType(VectorType *VecTy) {
  unsigned NumElements = VecTy->getNumElements();
  Type *ElemTy = VecTy->getElementType();
  return ((NumElements == 16) &&
          (ElemTy->isFloatTy() ||
           ElemTy->isIntegerTy(32) ||
           ElemTy->isDoubleTy() ||
           ElemTy->isIntegerTy(64)));
}

namespace intel {


static Value* getConsecutiveConstantVector(Type* type, unsigned count) {
  std::vector<Constant*> constList;
  uint64_t constVal = 0;

  for (unsigned j=0; j < count; ++j) {
    constList.push_back(ConstantInt::get(type, constVal++));
  }
  return ConstantVector::get(ArrayRef<Constant*>(constList));
}


bool MICResolver::TargetSpecificResolve(CallInst* caller) {
  Function* called = caller->getCalledFunction();
  std::string calledName = called->getName();
  V_PRINT(DEBUG_TYPE, "MICSpecificResolve Inspecting "<<calledName<<"\n");

  // Use name to decide what to do
  if (Mangler::isMangledLoad(calledName)) {
    Value *Mask = caller->getArgOperand(0);
    Value *Ptr  = caller->getArgOperand(1);
    assert(Mask->getType()->getScalarSizeInBits() == 1 && "Invalid mask size");
    assert(Ptr->getType()->isPointerTy() && "Ptr is not a pointer!");
    VectorType *MaskTy = dyn_cast<VectorType>(Mask->getType());
    VectorType *RetTy = dyn_cast<VectorType>(caller->getType());
    if (!MaskTy || !RetTy) {
      V_PRINT(DEBUG_TYPE, "Non vector type unsupported for gather "<<calledName<<"\n");
      if (!MaskTy && !RetTy)
        V_PRINT(gather_scatter_stat, "RESOLVER: SCALAR FOR GATHER\n");
      else
        V_PRINT(gather_scatter_stat, "RESOLVER: NON VECTOR TYPE FOR GATHER " << *caller << "\n");
      return false;
    }

    assert(MaskTy->getNumElements() == RetTy->getNumElements() &&
      "mismatch between mask and data num elements");
    PointerType *PtrTy  = cast<PointerType>(Ptr->getType());
    assert(PtrTy->getElementType() == RetTy && "mismatch between ptr and retval");
  
    if (!isGatherScatterType(RetTy)) {
      V_PRINT(DEBUG_TYPE, "Type unsupported for gather "<<calledName<<"\n");
      V_PRINT(gather_scatter_stat, "RESOLVER: UNSUPPORTED TYPE FOR GATHER " << *caller << "\n");
      return false;
    }

    std::string IntrinsicName = Mangler::getGatherScatterName(true, true, RetTy);
    std::vector<Value*> args;
    std::vector<Type *> types;

    V_PRINT(DEBUG_TYPE, "Generating gather for "<<calledName<<"\n");
    assert(8 * Mangler::getMangledLoadAlignment(calledName) >=
           RetTy->getScalarSizeInBits() && "Not naturally aligned!");

    // Remove address space from pointer type
    PtrTy = PointerType::get(RetTy->getScalarType(), 0);
    Ptr = new BitCastInst(Ptr, PtrTy, "ptrTypeCast", caller);

    Type *IndTy = IntegerType::get(caller->getContext(), 32);

    Value *Index = getConsecutiveConstantVector(IndTy, RetTy->getNumElements());

    args.push_back(Mask);     // load according to mask
    args.push_back(Ptr);      // pointer to loaded data
    args.push_back(Index);

    types.push_back(MaskTy);
    types.push_back(PtrTy);
    types.push_back(Index->getType());

    FunctionType *intr = FunctionType::get(RetTy, types, false);
    Constant *new_f = caller->getParent()->getParent()->getParent()->
      getOrInsertFunction(IntrinsicName, intr);
    CallInst *gather = CallInst::Create(new_f, ArrayRef<Value*>(args), "", caller);
    V_PRINT(DEBUG_TYPE, "Generated gather "<<*gather<<"\n");
    caller->replaceAllUsesWith(gather);
    caller->eraseFromParent();
    return true;
  }

  if (Mangler::isMangledStore(calledName)) {
    Value *Mask = caller->getArgOperand(0);
    Value *Ptr  = caller->getArgOperand(2);
    Value *Data = caller->getArgOperand(1);
    assert(Mask->getType()->getScalarSizeInBits() == 1 && "Invalid mask size");
    assert(Ptr->getType()->isPointerTy() && "Ptr is not a pointer!");
    VectorType *MaskTy = dyn_cast<VectorType>(Mask->getType());
    VectorType *DataTy = dyn_cast<VectorType>(Data->getType());
    if (!MaskTy || !DataTy) {
      V_PRINT(DEBUG_TYPE, "Non vector type unsupported for scatter "<<calledName<<"\n");
      if (!MaskTy && !DataTy)
        V_PRINT(gather_scatter_stat, "RESOLVER: SCALAR FOR SCATTER\n");
      else
        V_PRINT(gather_scatter_stat, "RESOLVER: NON VECTOR TYPE FOR SCATTER " << *caller << "\n");
      return false;
    }

    assert(DataTy && "mangled store of non vector type data");
    assert(MaskTy->getNumElements() == DataTy->getNumElements() &&
      "mismatch between mask and data num elements");
    PointerType *PtrTy  = cast<PointerType>(Ptr->getType());
    assert(PtrTy->getElementType() == DataTy && 
      "mismatch between ptr and retval");
    
    if (!isGatherScatterType(DataTy)) {
      V_PRINT(DEBUG_TYPE, "Type unsupported for scatter "<<calledName<<"\n");
      V_PRINT(gather_scatter_stat, "RESOLVER: UNSUPPORTED TYPE FOR SCATTER " << *caller << "\n");
      return false;
    }

    std::string IntrinsicName = Mangler::getGatherScatterName(true, false, DataTy);
    std::vector<Value*> args;
    std::vector<Type *> types;

    V_PRINT(DEBUG_TYPE, "Generating scatter for "<<calledName<<"\n");
    assert(8 * Mangler::getMangledStoreAlignment(calledName) >=
           DataTy->getScalarSizeInBits() && "Not naturally aligned!");

    // Remove address space from pointer type
    PtrTy = PointerType::get(DataTy->getScalarType(), 0);
    Ptr = new BitCastInst(Ptr, PtrTy, "ptrTypeCast", caller);

    Type *IndTy = IntegerType::get(caller->getContext(), 32);

    Value *Index = getConsecutiveConstantVector(IndTy, DataTy->getNumElements());
    args.push_back(Mask);
    args.push_back(Ptr);
    args.push_back(Index);
    args.push_back(Data);

    types.push_back(MaskTy);
    types.push_back(PtrTy);
    types.push_back(Index->getType());
    types.push_back(DataTy);

    FunctionType *intr = FunctionType::get(
      Type::getVoidTy(DataTy->getContext()), types, false);
    Constant *new_f = caller->getParent()->getParent()->getParent()->
      getOrInsertFunction(IntrinsicName, intr);
    CallInst *scatter = CallInst::Create(new_f, ArrayRef<Value*>(args), "", caller);
    V_PRINT(DEBUG_TYPE, "Generated scatter "<<*scatter<<"\n");
    caller->replaceAllUsesWith(scatter);
    caller->eraseFromParent();
    return true;
  }

  V_PRINT(DEBUG_TYPE, "Unhandled call: not mangled load nor store "<<calledName<<"\n");
  return false;
}

} // namespace

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  FunctionPass* createMICResolverPass() {
    return new intel::MICResolver();
  }
}
char intel::MICResolver::ID = 0;
static RegisterPass<intel::MICResolver>
CLIMICResolver("micresolve", "Resolves masked and vectorized function calls on MIC");
