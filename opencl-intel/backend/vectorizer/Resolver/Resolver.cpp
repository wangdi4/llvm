#define DEBUG_TYPE "resolver"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "Resolver.h"
#include "Mangler.h"
#include "Logger.h"
#include "VectorizerUtils.h"
#include "llvm/Constants.h"

#include <vector>

namespace intel {

bool FuncResolver::runOnFunction(Function &F) {

  V_STAT(
  V_PRINT(vectorizer_stat, "Resolver Statistics on function "<<F.getName()<<":\n");
  V_PRINT(vectorizer_stat, "======================================================\n");
  )

  V_PRINT(resolver, "---------------- Resolver before ---------------\n"<<F<<"\n");

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
      std::string calledName = called->getName();
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
  std::string calledName = called->getName();
  V_PRINT(DEBUG_TYPE, "Inspecting "<<calledName<<"\n");

  if (TargetSpecificResolve(caller)) return;

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
    std::vector<Value*> users(insts[i]->use_begin(), insts[i]->use_end());
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

void FuncResolver::resolveLoad(CallInst* caller) {
  Function* called = caller->getCalledFunction();
  std::string calledName = called->getNameStr();
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
  assert(caller->getNumArgOperands() == 2 && "Bad number of operands");

  Type *Tp = caller->getType();
  assert(Tp->isVectorTy() && "Return value must be of vector type");
  assert(Ptr->getType()->isPointerTy() && "Pointer must be of pointer type");

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

void FuncResolver::resolveStore(CallInst* caller) {
  Function* called = caller->getCalledFunction();
  std::string calledName = called->getNameStr();
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
  assert(caller->getNumArgOperands() == 3 && "Bad number of operands");

  Type *Tp = Data->getType();
  assert(Ptr->getType()->isPointerTy() && "Pointer must be of pointer type");

  VectorType *VT = cast<VectorType>(Tp);
  unsigned NumElem = VT->getNumElements();
  Type *Elem = VT->getElementType();

  
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

  // Cast the ptr back to its original form
  PointerType *InPT = cast<PointerType>(Ptr->getType());
  PointerType *SclPtrTy = PointerType::get(Elem, InPT->getAddressSpace());
  Ptr = new BitCastInst(Ptr, SclPtrTy, "ptrTypeCast", caller);

  for (unsigned i=0; i< NumElem; ++i) {
    V_STAT(m_unresolvedStoreCtr++;)
    Constant *Idx = ConstantInt::get(Type::getInt32Ty(Elem->getContext()), i);
    Instruction *GEP = GetElementPtrInst::Create(Ptr, Idx, "vstore", caller);
    Instruction *MaskBit = ExtractElementInst::Create(Mask, Idx, "exmask", caller);
    Instruction *DataElem = ExtractElementInst::Create(Data, Idx, "exData", caller);
    Instruction *storer = new StoreInst(DataElem, GEP, false, align, caller);
    VectorizerUtils::SetDebugLocBy(GEP, caller);
    VectorizerUtils::SetDebugLocBy(MaskBit, caller);
    VectorizerUtils::SetDebugLocBy(DataElem, caller);
    VectorizerUtils::SetDebugLocBy(storer, caller);
    toPredicate(DataElem, MaskBit);
    toPredicate(storer, MaskBit);
  }

  caller->eraseFromParent();
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
  std::string name = called->getName();

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
  CallInst* call = CallInst::Create(
    func, ArrayRef<Value*>(params), "", caller);
  VectorizerUtils::SetDebugLocBy(call, caller);
  caller->replaceAllUsesWith(call);
  // Replace predicate with control flow
  toPredicate(call, caller->getArgOperand(0));
  // Remove original call
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
char intel::X86Resolver::ID = 0;
static RegisterPass<intel::X86Resolver>
CLIX86Resolver("resolve", "Resolves masked and vectorized function calls on x86");

