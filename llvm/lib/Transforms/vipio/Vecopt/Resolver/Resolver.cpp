/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#define DEBUG_TYPE "resolver"
#include "Resolver.h"
#include "Mangler.h"
#include "Logger.h"
#include "VectorizerUtils.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"
#include "FakeExtractInsert.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Constants.h"

#include <vector>

namespace intel {

char X86Resolver::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(X86Resolver, "resolve", "Resolves masked and vectorized function calls on x86", false, false)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
OCL_INITIALIZE_PASS_END(X86Resolver, "resolve", "Resolves masked and vectorized function calls on x86", false, false)

bool FuncResolver::runOnFunction(Function &F) {

  V_STAT(
  V_PRINT(vectorizer_stat, "Resolver Statistics on function "<<F.getName()<<":\n");
  V_PRINT(vectorizer_stat, "======================================================\n");
  )

  V_PRINT(resolver, "---------------- Resolver before ---------------\n"<<F<<"\n");

  m_rtServices = getAnalysis<BuiltinLibInfo>().getRuntimeServices();

  m_IRBuilder = new IRBuilder<>(F.getContext());

  std::vector<CallInst*> calls;

  for (Function::iterator it = F.begin(), e = F.end(); it != e; ++it) {
    packPredicatedLoads(it);
  }

  // Collect call instructions inspect
  inst_iterator Inst = inst_begin(F);
  inst_iterator InstE = inst_end (F);
  // for each instruction
  for (; Inst != InstE ; ++Inst) {
    // if this is a function call
    if (CallInst* inst = dyn_cast<CallInst>(&*Inst)) {
      calls.push_back(inst);
    }
  }

  // Inspect and resolve all function calls
  std::vector<CallInst*>::iterator C  = calls.begin();
  std::vector<CallInst*>::iterator CE = calls.end();

  V_STAT(
  m_unresolvedLoadCtr  = 0;
  m_unresolvedStoreCtr = 0;
  m_unresolvedCallCtr  = 0;
  m_unresolvedInstrCtr = 0;
  )

  for (; C != CE ; ++C) {
    resolve(*C);
  }

  V_STAT(
  V_PRINT(vectorizer_stat, "Couldn't vectorize "<<m_unresolvedLoadCtr<<" load instructions\n");
  V_PRINT(vectorizer_stat, "Couldn't vectorize "<<m_unresolvedStoreCtr<<" store instructions\n");
  V_PRINT(vectorizer_stat, "Couldn't vectorize "<<m_unresolvedCallCtr<<" call instructions\n");

  V_PRINT(vectorizer_stat_excel, "\t\t\t\t\t\t\t\t\t\t"<<m_unresolvedLoadCtr <<"\t\t\t\tCouldn't vectorize load instructions\n");
  V_PRINT(vectorizer_stat_excel, "\t\t\t\t\t\t\t\t\t\t\t"<<m_unresolvedStoreCtr <<"\t\t\tCouldn't vectorize store instructions\n");
  V_PRINT(vectorizer_stat_excel, "\t\t\t\t\t\t\t\t\t\t\t\t"<<m_unresolvedCallCtr <<"\t\tCouldn't vectorize call instructions\n");
  )

  V_PRINT(resolver, "Found "<<m_toCF.size()<<" instructions to hide behind CCF\n");

  for (std::map<Value*, std::vector<Instruction*> >::iterator it = m_toCF.begin(),
       e = m_toCF.end(); it != e; ++it) {
    Value* pred = it->first;
    std::vector<Instruction*> elements = it->second;
    resolvePredicate(pred, elements);
  }

  V_STAT(
  V_PRINT(vectorizer_stat, "Overall: couldn't vectorize "<<m_unresolvedInstrCtr<<" instructions\n");
  )

  m_toCF.clear();
  delete m_IRBuilder;
  m_IRBuilder = NULL;
  V_PRINT(resolver, "---------------- Resolver After ---------------\n"<<F<<"\n");
  return (calls.begin() != calls.end());
}

void FuncResolver::packPredicatedLoads(BasicBlock* BB) {
  V_ASSERT(BB);

  bin_t curr_bin;
  std::vector<bin_t> bins;

  // this loop gathers masked-loads which appear sequentially and consqutively
  // in the code, into a bin
  for (BasicBlock::iterator it=BB->begin(), e=BB->end(); it != e; ++it) {

    // if this is a masked load
    bool load = false;
    if (CallInst* caller = dyn_cast<CallInst>(it)) {
      Function* called = caller->getCalledFunction();
      std::string calledName = called->getName().str();
      V_PRINT(DEBUG_TYPE, "Inspecting "<<calledName<<"\n");
      if (Mangler::isMangledLoad(calledName)) {
        curr_bin.push_back(caller);
        load = true;
      }
    }
    // if we found the first non-load, flush the bin
    if (!load && !curr_bin.empty()) {
      bins.push_back(curr_bin);
      curr_bin.clear();
    }
  }

  // flush the last bin
  if (!curr_bin.empty()) {
    bins.push_back(curr_bin);
  }

  // for all bins, re-order loads
  for (std::vector<bin_t>::iterator it=bins.begin(), e=bins.end(); it!=e; ++it) {
    packLoadBin(*it);
  }

}

void FuncResolver::packLoadBin(const bin_t& bin) {
  // for each consecutive loads, where it1 > it0
  for(bin_t::const_iterator it0=bin.begin(),e0=bin.end(); it0!=e0; ++it0) {
    for(bin_t::const_iterator it1=it0,e1=bin.end(); it1!=e1; ++it1) {
      // if they share the same predicate
      if (it0 != it1 && (*it0)->getOperand(0) == (*it1)->getOperand(0)) {
        V_PRINT(DEBUG_TYPE, "Found almost consecutive loads "<<**it0<<" "<<**it1<<" in BB:"<<*(*it1)->getParent()<<"\n");
        // Then they should be together.
        (*it1)->moveBefore(*it0);
        (*it0)->moveBefore(*it1);
      }
    }
  }

}

void FuncResolver::resolvePredicate(Value* pred, std::vector<Instruction*>& elements) {
  V_ASSERT(!elements.empty() && pred);
  std::vector<Instruction*> to_predicate;
  Instruction* curr = NULL;
  // while we have unresolved instructions
  while(!elements.empty()) {
    if (std::find(elements.begin(), elements.end(), curr) != elements.end()) {
      to_predicate.push_back(curr);
      elements.erase(std::find(elements.begin(), elements.end(), curr));
      //adding assert to prevent Klocwork warning
      V_ASSERT(curr && "Node must not be null");
      curr = curr->getNextNode();
    } else {
      if  (!to_predicate.empty()) {
        V_STAT(m_unresolvedInstrCtr++;)
        CFInstruction(to_predicate, pred);
      }
      to_predicate.clear();
      curr = *(elements.begin());
    }
  }

  // final flush
  if  (!to_predicate.empty()) {
    V_STAT(m_unresolvedInstrCtr++;)
    CFInstruction(to_predicate, pred);
    to_predicate.clear();
  }
}


void FuncResolver::toPredicate(Instruction* inst, Value* pred) {
  if (m_toCF.find(pred) == m_toCF.end()) {
    m_toCF[pred] = std::vector<Instruction*>();
  }

  m_toCF[pred].push_back(inst);
}

void FuncResolver::resolve(CallInst* caller) {
  Function* called = caller->getCalledFunction();
  std::string calledName = called->getName().str();
  V_PRINT(DEBUG_TYPE, "Inspecting "<<calledName<<"\n");

  if (TargetSpecificResolve(caller)) return;

  if (Mangler::isAllZero(calledName)) {
    resolveAllZero(caller);
  }
  if (Mangler::isAllOne(calledName)) {
    resolveAllOne(caller);
  }

  // Use name to decide what to do
  if (Mangler::isMangledLoad(calledName)) {
    return resolveLoad(caller);
  }
  if (Mangler::isMangledStore(calledName)) {
    return resolveStore(caller);
  }
  if (Mangler::isMangledCall(calledName)) {
    V_PRINT(vectorizer_stat, "<<<<Cannot vectorize masked call to " << calledName << "\n");
    V_STAT(m_unresolvedCallCtr++;)
    return resolveFunc(caller);
  }
  if (Mangler::isFakeInsert(calledName)) {
    return resolveFakeInsert(caller);
  }
  if (Mangler::isFakeExtract(calledName)) {
    return resolveFakeExtract(caller);
  }
  if (Mangler::isRetByVectorBuiltin(calledName)) {
    return resolveRetByVectorBuiltin(caller);
  }
}

Constant *FuncResolver::getDefaultValForType(Type *ty) {
  if (ty->isFPOrFPVectorTy()) {
    return Constant::getNullValue(ty);
  }
  return UndefValue::get(ty);
}

void FuncResolver::CFInstruction(std::vector<Instruction*> insts, Value* pred) {
  V_ASSERT(!insts.empty() && pred && "Bad instruction");

  // split before and after the instruction
  BasicBlock* header = insts[0]->getParent();
  BasicBlock* body = header->splitBasicBlock(insts[0],"preload");
  BasicBlock* footer = body->splitBasicBlock(insts[0],"postload");

  // Assert, debug, print
  for (size_t i=0; i< insts.size(); ++i) {
    V_PRINT(resolver, "moving "<<*insts[i]<<"\n");
    V_ASSERT(insts[i]->getParent() == insts[0]->getParent());
    if (i>0) { V_ASSERT(insts[i-1]->getNextNode() == insts[i]); }
  }

  // Move instructions into newly created BB
  for (size_t i=0; i< insts.size(); ++i) {
    insts[i]->moveBefore(body->getTerminator());
  }
  // replace terminator with conditional branch
  header->getTerminator()->eraseFromParent();
  BranchInst::Create(body, footer, pred, header);

  // create a phinode to join the value of the load
  for (size_t i=0; i< insts.size(); ++i) {
    // Nothing to do for void type
    if (insts[i]->getType()->isVoidTy()) continue;
    PHINode* phi = PHINode::Create(insts[i]->getType(), 2, "phi", footer->getFirstNonPHI());

    // replace all users which are not skipped (this will create a broken module)
    std::vector<Value*> users(insts[i]->user_begin(), insts[i]->user_end());
    for (std::vector<Value*>::iterator it = users.begin(), e = users.end(); it != e; ++it) {
      // If the user is an instruction
      Instruction* user = dyn_cast<Instruction>(*it);
      V_ASSERT(user && "a non-instruction user");
      // If the user is in this block, don't change it.
      if (user->getParent() != body) {
        V_PRINT(resolver, "replacing "<<*user<<"\n");
        user->replaceUsesOfWith(insts[i], phi);
      } else {
        V_PRINT(resolver, "not replacing "<<*user<<"\n");
      }
    }//end of for

    phi->addIncoming(getDefaultValForType(insts[i]->getType()), header);
    phi->addIncoming(insts[i], body);
  }
}

void FuncResolver::resolveAllZero(CallInst* caller) {
  V_PRINT(DEBUG_TYPE, "Inspecting all-zero\n" <<*caller<<"\n");
  V_ASSERT(caller->getNumArgOperands() == 1 && "Bad number of operands");
  Value* arg0 = caller->getArgOperand(0);
  Type* arg0Type = arg0->getType();
  VectorType* arg0VectorType = dyn_cast<VectorType>(arg0Type);
  V_ASSERT(arg0VectorType && "Bad op type");
  unsigned int vecWidth = arg0VectorType->getNumElements();
  Type* bitMaskType = Type::getIntNTy(caller->getContext(), vecWidth);
  Instruction *bitCastInst = new BitCastInst(arg0,
					     bitMaskType,
					     "bitMask",
					     caller);
  Instruction *cmpInst = new ICmpInst(caller,
				      CmpInst::ICMP_EQ,
				      bitCastInst,
				      Constant::getNullValue(bitMaskType),
				      "isAllZero");
  caller->replaceAllUsesWith(cmpInst);
}

void FuncResolver::resolveAllOne(CallInst* caller) {
  V_PRINT(DEBUG_TYPE, "Inspecting all-one\n" <<*caller<<"\n");
  V_ASSERT(caller->getNumArgOperands() == 1 && "Bad number of operands");
  Value* arg0 = caller->getArgOperand(0);
  Type* arg0Type = arg0->getType();
  VectorType* arg0VectorType = dyn_cast<VectorType>(arg0Type);
  V_ASSERT(arg0VectorType && "Bad op type");
  unsigned int vecWidth = arg0VectorType->getNumElements();
  //  unsigned int bitMaskWidth = std::max(vecWidth, 8u);
  APInt allOnesMaskBits = APInt::getLowBitsSet(vecWidth, vecWidth);
  Type* bitMaskType = Type::getIntNTy(caller->getContext(), vecWidth);
  Instruction *bitCastInst = new BitCastInst(arg0,
					     bitMaskType,
					     "bitMask",
					     caller);
  Instruction *cmpInst = new ICmpInst(caller,
				      CmpInst::ICMP_EQ,
				      bitCastInst,
				      ConstantInt::get(caller->getContext(),
						       allOnesMaskBits),
				      "isAllOne");
  caller->replaceAllUsesWith(cmpInst);
}

void FuncResolver::resolveLoad(CallInst* caller) {
  Function* called = caller->getCalledFunction();
  std::string calledName = called->getName().str();
  unsigned align = Mangler::getMangledLoadAlignment(calledName);
  V_PRINT(DEBUG_TYPE, "Inspecting load "<<calledName<<"\n");
  if (isa<VectorType>(caller->getArgOperand(0)->getType()))
    return resolveLoadVector(caller, align);
  return resolveLoadScalar(caller, align);
}

void FuncResolver::resolveLoadScalar(CallInst* caller, unsigned align) {
  V_STAT(m_unresolvedLoadCtr++;)
  V_PRINT(DEBUG_TYPE, "Inspecting scl load\n" <<*caller<<"\n");
  // Operands: pred, ptr
  V_ASSERT(caller->getNumArgOperands() == 2 && "Bad number of operands");
  V_ASSERT(!isa<VectorType>(caller->getArgOperand(0)->getType()) && "Bad op type");

  // Create the new load
  LoadInst* loader = new LoadInst(caller->getArgOperand(1), "masked_load",
      false, align, caller);
  VectorizerUtils::SetDebugLocBy(loader, caller);
  caller->replaceAllUsesWith(loader);
  // Replace predicate with control flow

  toPredicate(loader, caller->getArgOperand(0));
  // Remove original function call
  caller->eraseFromParent();
}

void FuncResolver::resolveLoadVector(CallInst* caller, unsigned align) {
  Value *Mask = caller->getArgOperand(0);
  Value *Ptr = caller->getArgOperand(1);
  V_ASSERT(caller->getNumArgOperands() == 2 && "Bad number of operands");

  Type *Tp = caller->getType();
  V_ASSERT(Tp->isVectorTy() && "Return value must be of vector type");
  V_ASSERT(Ptr->getType()->isPointerTy() && "Pointer must be of pointer type");

  // Uniform mask for vector load.
  // Perform a single wide load and a single IF.
  if (!Mask->getType()->isVectorTy()) {
    Instruction *loader = new LoadInst(Ptr, "vload", false, align, caller);
    VectorizerUtils::SetDebugLocBy(loader, caller);
    toPredicate(loader, Mask);
    caller->replaceAllUsesWith(loader);
    caller->eraseFromParent();
    return;
  }

  // Masked vector load - try to utilize masked load built-in (if available)
  if (isResolvedMaskedLoad(caller, align)) {
    return;
  }

  VectorType *VT = cast<VectorType>(Tp);
  unsigned NumElem = VT->getNumElements();
  Type *Elem = VT->getElementType();

  // Cast the ptr back to its original form
  PointerType *InPT = cast<PointerType>(Ptr->getType());
  PointerType *SclPtrTy = PointerType::get(Elem, InPT->getAddressSpace());
  Ptr = new BitCastInst(Ptr, SclPtrTy, "ptrTypeCast", caller);

  Value *Ret = UndefValue::get(VT);

  for (unsigned i=0; i< NumElem; ++i) {
    V_STAT(m_unresolvedLoadCtr++;)
    Constant *Idx = ConstantInt::get(Type::getInt32Ty(Elem->getContext()), i);
    Instruction *GEP = GetElementPtrInst::Create(Ptr, Idx, "vload", caller);
    Instruction *MaskBit = ExtractElementInst::Create(Mask, Idx, "exmask", caller);
    Instruction *loader = new LoadInst(GEP, "vload", false, align, caller);
    Instruction* inserter = InsertElementInst::Create(
      Ret, loader, Idx, "vpack", caller);
    VectorizerUtils::SetDebugLocBy(GEP, caller);
    VectorizerUtils::SetDebugLocBy(MaskBit, caller);
    VectorizerUtils::SetDebugLocBy(loader, caller);
    VectorizerUtils::SetDebugLocBy(inserter, caller);

    Ret = inserter;
    toPredicate(loader, MaskBit);
  }

  caller->replaceAllUsesWith(Ret);
  caller->eraseFromParent();
}

bool FuncResolver::isResolvedMaskedLoad(CallInst* caller, unsigned align) {
  Value *Mask = caller->getArgOperand(0);
  Value *Ptr = caller->getArgOperand(1);
  PointerType* ptrType = dyn_cast<PointerType>(Ptr->getType());
  VectorType* vecType = dyn_cast<VectorType>(ptrType->getElementType());
  V_ASSERT(vecType && "Pointer must be of vector type");
  m_IRBuilder->SetInsertPoint(caller);
  CallInst* newCall = m_IRBuilder->CreateMaskedLoad(Ptr, align, Mask);
  caller->replaceAllUsesWith(newCall);
  caller->eraseFromParent();
  return true;
}

void FuncResolver::resolveStore(CallInst* caller) {
  Function* called = caller->getCalledFunction();
  std::string calledName = called->getName().str();
  unsigned align = Mangler::getMangledStoreAlignment(calledName);
  V_PRINT(DEBUG_TYPE, "Inspecting store "<<calledName<<"\n");

  if (isa<VectorType>(caller->getArgOperand(0)->getType()))
    return resolveStoreVector(caller, align);
  return resolveStoreScalar(caller, align);
}

void FuncResolver::resolveStoreScalar(CallInst* caller, unsigned align) {
  V_STAT(m_unresolvedStoreCtr++;)
  V_PRINT(DEBUG_TYPE, "Inspecting scl store\n" <<*caller<<"\n");
  //Operands pred, val,  ptr
  V_ASSERT(caller->getNumArgOperands() == 3 && "Bad number of operands");
  V_ASSERT(!isa<VectorType>(caller->getType()) && "Bad return type");
  V_ASSERT(!isa<VectorType>(caller->getArgOperand(0)->getType()) && "Bad op type");

  // Create new store
  StoreInst* st = new StoreInst(
    caller->getArgOperand(1), caller->getArgOperand(2), false, align, caller);
  VectorizerUtils::SetDebugLocBy(st, caller);
  // Replace predicator with contol flow
  toPredicate(st, caller->getArgOperand(0));
  // Remove original call
  caller->eraseFromParent();
}


void FuncResolver::resolveStoreVector(CallInst* caller, unsigned align) {
  Value *Mask = caller->getArgOperand(0);
  Value *Data = caller->getArgOperand(1);
  Value *Ptr = caller->getArgOperand(2);
  V_ASSERT(caller->getNumArgOperands() == 3 && "Bad number of operands");
  V_ASSERT(caller->getType()->isVoidTy() && "Store is expected to return 'void'");
  V_ASSERT(Ptr->getType()->isPointerTy() && "Pointer must be of pointer type");

  // Uniform mask for vector store.
  // Perform a single wide store and a single IF.
  if (!Mask->getType()->isVectorTy()) {
    Instruction *storer = new StoreInst(Data, Ptr, false, align, caller);
    VectorizerUtils::SetDebugLocBy(storer, caller);
    toPredicate(storer, Mask);
    caller->replaceAllUsesWith(storer);
    caller->eraseFromParent();
    return;
  }

  // Masked vector store - try to utilize masked store built-in (if available)
  if (isResolvedMaskedStore(caller, align)) {
    return;
  }
}

bool FuncResolver::isResolvedMaskedStore(CallInst* caller, unsigned align) {
  Value *Mask = caller->getArgOperand(0);
  Value *Data = caller->getArgOperand(1);
  Value *Ptr = caller->getArgOperand(2);
  PointerType* ptrType = dyn_cast<PointerType>(Ptr->getType());
  VectorType* vecType = dyn_cast<VectorType>(ptrType->getElementType());
  V_ASSERT(vecType && "Pointer must be of vector type");
  m_IRBuilder->SetInsertPoint(caller);
  CallInst* newCall = m_IRBuilder->CreateMaskedStore(Data, Ptr, align, Mask);
  caller->replaceAllUsesWith(newCall);
  caller->eraseFromParent();
  return true;
}


void FuncResolver::resolveFunc(CallInst* caller) {

  // Create function arguments
  std::vector<Type*> args;

  //Operands: [PRED, arg0, arg1 , ...]. Omit the pred..
  for (unsigned j = 1; j < caller->getNumArgOperands(); ++j) {
    args.push_back(caller->getArgOperand(j)->getType());
  }

  // Create the new function type
  FunctionType* funcType = FunctionType::get(caller->getType(), args, false);

  Function* called = caller->getCalledFunction();
  std::string name = called->getName().str();

  // Obtain function
  Module * currModule = caller->getParent()->getParent()->getParent();
  Function* func = currModule->getFunction(Mangler::demangle(name));
  if (!func) {
    func = dyn_cast<Function>(currModule->getOrInsertFunction(Mangler::demangle(name),funcType));
    V_ASSERT(func && "should not be a cast, as function was not locally found");
  }

  std::vector<Value*> params;
  // Create arguments for new function call (omit pred)
  for (unsigned j = 1; j < caller->getNumArgOperands(); ++j) {
    params.push_back(caller->getArgOperand(j));
  }

  // Generate a Call the new function
  CallInst* pcall = CallInst::Create(
    func, ArrayRef<Value*>(params), "", caller);
  //Update new call instruction with calling convention and attributes
  pcall->setCallingConv(caller->getCallingConv());
  AttributeSet as;
  AttributeSet callAttr = caller->getAttributes();
  for (unsigned int i=0; i < caller->getNumArgOperands(); ++i) {
    //Parameter attributes starts with index 1-NumOfParams
    unsigned int idx = i+1;
    //pcall starts with mask argument, skip it when setting original argument attributes.
    as.addAttributes(func->getContext(), 1 + idx, callAttr.getParamAttributes(idx));
  }
  //set function attributes of pcall
  as.addAttributes(func->getContext(), AttributeSet::FunctionIndex, callAttr.getFnAttributes());
  //set return value attributes of pcall
  as.addAttributes(func->getContext(), AttributeSet::ReturnIndex, callAttr.getRetAttributes());
  pcall->setAttributes(as);
  VectorizerUtils::SetDebugLocBy(pcall, caller);
  caller->replaceAllUsesWith(pcall);
  // Replace predicate with control flow
  toPredicate(pcall, caller->getArgOperand(0));
  // Remove original call
  caller->eraseFromParent();
}

Instruction* FuncResolver::extendMaskAsBIParameter(Function* maskLoadStoreBI, Value* Mask) {
  // Retrieve mask argument type (assumed to be the LAST parameter in masked load/store BI)
  FunctionType* funcType = maskLoadStoreBI->getFunctionType();
  Type* extMaskType = funcType->getParamType(funcType->getNumParams() - 1);
  // SIGN-extend the mask to the argument type (as MSB of mask matters)
  V_ASSERT(extMaskType->getScalarSizeInBits() >= Mask->getType()->getScalarSizeInBits() &&
             "Extended mask type smaller than original mask type!");
  return CastInst::CreateSExtOrBitCast(Mask, extMaskType, "extmask");
}

Instruction* FuncResolver::adjustPtrAddressSpace(Function* maskLoadStoreBI, Value* Ptr) {
  // Retrieve memory pointer argument type (assumed to be the FIRST parameter in masked load/store BI)
  FunctionType* funcType = maskLoadStoreBI->getFunctionType();
  PointerType* memPtrType = dyn_cast<PointerType>(funcType->getParamType(0));
  V_ASSERT(memPtrType && dyn_cast<VectorType>(memPtrType->getElementType()) && "First parameter should be a pointer of vector type");
  // Convert the pointer to argument type
  return CastInst::CreatePointerCast(Ptr, memPtrType, "PtrCast");
}


void FuncResolver::resolveFakeInsert(CallInst* caller) {
  FakeInsert FI(*caller);
  InsertElementInst *IEI = InsertElementInst::Create(FI.getVectorArg(), FI.getNewEltArg(), FI.getIndexArg(), "insertelt", caller);
  caller->replaceAllUsesWith(IEI);
  VectorizerUtils::SetDebugLocBy(IEI, caller);
  caller->eraseFromParent();
}

void FuncResolver::resolveFakeExtract(CallInst* caller) {
  ExtractElementInst *EEI = ExtractElementInst::Create(caller->getOperand(0), caller->getOperand(1), "extractelt", caller);
  caller->replaceAllUsesWith(EEI);
  VectorizerUtils::SetDebugLocBy(EEI, caller);
  caller->eraseFromParent();
}

void FuncResolver::resolveRetByVectorBuiltin(CallInst* caller) {
  Function* currFunc = caller->getParent()->getParent();

  Function *origFunc = caller->getCalledFunction();
  std::string fakeFuncName = origFunc->getName().str();
  std::string origFuncName = Mangler::get_original_scalar_name_from_retbyvector_builtin(fakeFuncName);
  const Function *LibFunc = m_rtServices->findInRuntimeModule(origFuncName);
  V_ASSERT(LibFunc && "Function does not exists in runtime module");

  // Find (or create) declaration for newly called function
  Function *newFunction = currFunc->getParent()->getFunction(LibFunc->getName());
  if (!newFunction) {
    Constant *newFunctionConst = currFunc->getParent()->getOrInsertFunction(
        LibFunc->getName(), LibFunc->getFunctionType(), LibFunc->getAttributes());
    V_ASSERT(newFunctionConst && "failed generating function in current module");
    newFunction = dyn_cast<Function>(newFunctionConst);
    V_ASSERT(newFunction && "Function type mismatch, caused a constant expression cast!");
  }

  std::vector<Value *> newArgs;
  //Prepare first parameter
  newArgs.push_back(caller->getOperand(0));

  //Prepare second parameter
  FunctionType *LibFuncTy = LibFunc->getFunctionType();
  Instruction *loc = currFunc->getEntryBlock().begin();
  PointerType *ptrTy = dyn_cast<PointerType>(LibFuncTy->getParamType(1));
  V_ASSERT(ptrTy && "bad signature");
  Type *elTy = ptrTy->getElementType();
  AllocaInst *AI = new AllocaInst(elTy, "ret2", loc);
  newArgs.push_back(AI);

  //Create new function call
  CallInst *newCall = CallInst::Create(newFunction, ArrayRef<Value*>(newArgs), "", caller);

  //Update return value
  Type *i32Ty = Type::getInt32Ty(caller->getContext());
  Constant *constZero = ConstantInt::get(i32Ty, 0);
  Constant *constOne = ConstantInt::get(i32Ty, 1);
  Value *undefVal = UndefValue::get(caller->getType());
  Value *vectorRetVal = InsertElementInst::Create(undefVal, newCall, constZero, "insert.ret1", caller);
  Instruction* secondRetVal = new LoadInst(AI, "load.ret2", caller);
  vectorRetVal = InsertElementInst::Create(vectorRetVal, secondRetVal, constOne, "insert.ret2", caller);

  caller->replaceAllUsesWith(vectorRetVal);

  //update new call instruction location and delete old call instruction
  VectorizerUtils::SetDebugLocBy(newCall, caller);
  caller->eraseFromParent();
}

} // namespace

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  FunctionPass* createX86ResolverPass() {
    return new intel::X86Resolver();
  }
}

