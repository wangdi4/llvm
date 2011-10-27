#include "llvm/Support/InstIterator.h"
#include "llvm/Support/CommandLine.h"
#include "Resolver.h"
#include "Mangler.h"
#include "Logger.h"

#include <vector>


static cl::opt<bool>
MicResolve("micresolve", cl::init(false), cl::Hidden,
  cl::desc("Resolve masked functions for MIC"));

namespace intel {

bool FuncResolver::runOnFunction(Function &F) {

  V_PRINT("resolver", "---------------- Resolver before ---------------\n"<<F<<"\n");

  std::vector<CallInst*> calls;

  LLVMContext &Con = F.getContext();
  m_v16i1  = VectorType::get(IntegerType::get(Con, 1), 16);
  m_v16i32 = VectorType::get(IntegerType::get(Con, 32), 16);
  m_v16f32 = VectorType::get(Type::getFloatTy(Con), 16);
  m_v8i1   = VectorType::get(IntegerType::get(Con, 1), 8);
  m_v8i64  = VectorType::get(IntegerType::get(Con, 64), 8);
  m_v8f64  = VectorType::get(Type::getDoubleTy(Con), 8);

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
  for (; C != CE ; ++C) {
    resolve(*C);
  }

  V_PRINT("resolver", "Found "<<m_toCF.size()<<" instructions to hide behind CCF\n");

  for (std::map<Value*, std::vector<Instruction*> >::iterator it = m_toCF.begin(),
       e = m_toCF.end(); it != e; ++it) {
    Value* pred = it->first;
    std::vector<Instruction*> elements = it->second;
    resolvePredicate(pred, elements);
  }

  m_toCF.clear();
  V_PRINT("resolver", "---------------- Resolver After ---------------\n"<<F<<"\n");
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
      if  (!to_predicate.empty()) { CFInstruction(to_predicate, pred); }
      to_predicate.clear();
      curr = *(elements.begin());
    }
  }

  // final flush
  if  (!to_predicate.empty()) {
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
    return resolveFunc(caller);
  }
}

void FuncResolver::CFInstruction(std::vector<Instruction*> insts, Value* pred) {
  V_ASSERT(!insts.empty() && pred && "Bad instruction");

  // split before and after the instruction
  BasicBlock* header = insts[0]->getParent();
  BasicBlock* body = header->splitBasicBlock(insts[0],"preload");
  BasicBlock* footer = body->splitBasicBlock(insts[0],"postload");

  // Assert, debug, print
  for (size_t i=0; i< insts.size(); ++i) {
    V_PRINT("resolver", "moving "<<*insts[i]<<"\n");
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
    UndefValue* undef = UndefValue::get(insts[i]->getType());
    PHINode* phi = PHINode::Create(insts[i]->getType(), "phi", footer->getFirstNonPHI());

    // replace all users which are not skipped (this will create a broken module)
    std::vector<Value*> users(insts[i]->use_begin(), insts[i]->use_end());
    for (std::vector<Value*>::iterator it = users.begin(), e = users.end(); it != e; ++it) {
      // If the user is an instruction
      Instruction* user = dyn_cast<Instruction>(*it);
      V_ASSERT(user && "a non-instruction user");
      // If the user is in this block, don't change it.
      if (user->getParent() != body) {
        V_PRINT("resolver", "replacing "<<*user<<"\n");
        user->replaceUsesOfWith(insts[i], phi);
      } else {
        V_PRINT("resolver", "not replacing "<<*user<<"\n");
      }
    }//end of for

    phi->addIncoming(undef, header);
    phi->addIncoming(insts[i], body);
  }
}

void FuncResolver::resolveLoad(CallInst* caller) {
  Function* called = caller->getCalledFunction();
  std::string calledName = called->getName();
  V_PRINT(DEBUG_TYPE, "Inspecting load "<<calledName<<"\n");
  if (isa<VectorType>(caller->getArgOperand(0)->getType())) return resolveLoadVector(caller);
  return resolveLoadScalar(caller);
}

void FuncResolver::resolveLoadScalar(CallInst* caller) {
  V_PRINT(DEBUG_TYPE, "Inspecting scl load\n" <<*caller<<"\n");
  // Operands: pred, ptr
  V_ASSERT(caller->getNumArgOperands() == 2 && "Bad number of operands");
  V_ASSERT(!isa<VectorType>(caller->getArgOperand(0)->getType()) && "Bad op type");

  // Create the new load
  LoadInst* loader = new LoadInst(caller->getArgOperand(1), "masked_load", caller);
  caller->replaceAllUsesWith(loader);
  // Replace predicate with control flow

  toPredicate(loader, caller->getArgOperand(0));
  // Remove original function call
  caller->eraseFromParent();
}

void FuncResolver::resolveLoadVector(CallInst* caller) {
  Value *Mask = caller->getArgOperand(0);
  Value *Ptr = caller->getArgOperand(1);
  assert(caller->getNumArgOperands() == 2 && "Bad number of operands");

  const Type *Tp = caller->getType();
  assert(Tp->isVectorTy() && "Return value must be of vector type");
  assert(Ptr->getType()->isPointerTy() && "Pointer must be of pointer type");

  // Uniform mask for vector load.
  // Perform a single wide load and a single IF.
  if (!Mask->getType()->isVectorTy()) {
    Instruction *loader = new LoadInst(Ptr, "vload", false, caller);
    toPredicate(loader, Mask);
    caller->replaceAllUsesWith(loader);
    caller->eraseFromParent();
    return;
  }

  const VectorType *VT = cast<VectorType>(Tp);
  unsigned NumElem = VT->getNumElements();
  const Type *Elem = VT->getElementType();

  // Cast the ptr back to its original form
  const PointerType *InPT = cast<PointerType>(Ptr->getType());
  PointerType *SclPtrTy = PointerType::get(Elem, InPT->getAddressSpace());
  Ptr = new BitCastInst(Ptr, SclPtrTy, "ptrTypeCast", caller);

  Value *Ret = UndefValue::get(VT);

  for (unsigned i=0; i< NumElem; ++i) {
    Constant *Idx = ConstantInt::get(Type::getInt32Ty(Elem->getContext()), i);
    Value *GEP = GetElementPtrInst::Create(Ptr, Idx, "vload", caller);
    Value *MaskBit = ExtractElementInst::Create(Mask, Idx, "exmask", caller);
    Instruction *loader = new LoadInst(GEP, "vload", false, caller);    
    Instruction* inserter = InsertElementInst::Create(
      Ret, loader, Idx, "vpack", caller);
    Ret = inserter;
    toPredicate(loader, MaskBit);
  }

  caller->replaceAllUsesWith(Ret);
  caller->eraseFromParent();
}

void FuncResolver::resolveStore(CallInst* caller) {
  Function* called = caller->getCalledFunction();
  std::string calledName = called->getName();
  V_PRINT(DEBUG_TYPE, "Inspecting store "<<calledName<<"\n");

  if (isa<VectorType>(caller->getArgOperand(0)->getType())) return resolveStoreVector(caller);
  return resolveStoreScalar(caller);
}

void FuncResolver::resolveStoreScalar(CallInst* caller) {
  V_PRINT(DEBUG_TYPE, "Inspecting scl store\n" <<*caller<<"\n");
  //Operands pred, val,  ptr
  V_ASSERT(caller->getNumArgOperands() == 3 && "Bad number of operands");
  V_ASSERT(!isa<VectorType>(caller->getType()) && "Bad return type");
  V_ASSERT(!isa<VectorType>(caller->getArgOperand(0)->getType()) && "Bad op type");

  // Create new store
  StoreInst* st = new StoreInst(
    caller->getArgOperand(1), caller->getArgOperand(2), false, caller);
  // Replace predicator with contol flow
  toPredicate(st, caller->getArgOperand(0));
  // Remove original call
  caller->eraseFromParent();
}


void FuncResolver::resolveStoreVector(CallInst* caller) {
  Value *Mask = caller->getArgOperand(0);
  Value *Data = caller->getArgOperand(1);
  Value *Ptr = caller->getArgOperand(2);
  assert(caller->getNumArgOperands() == 3 && "Bad number of operands");

  const Type *Tp = Data->getType();
  assert(Ptr->getType()->isPointerTy() && "Pointer must be of pointer type");

  const VectorType *VT = cast<VectorType>(Tp);
  unsigned NumElem = VT->getNumElements();
  const Type *Elem = VT->getElementType();

  
  // Uniform mask for vector store.
  // Perform a single wide store and a single IF.
  if (!Mask->getType()->isVectorTy()) {
    Instruction *storer = new StoreInst(Data, Ptr, caller);
    toPredicate(storer, Mask);
    caller->replaceAllUsesWith(storer);
    caller->eraseFromParent();
    return;
  }

  // Cast the ptr back to its original form
  const PointerType *InPT = cast<PointerType>(Ptr->getType());
  PointerType *SclPtrTy = PointerType::get(Elem, InPT->getAddressSpace());
  Ptr = new BitCastInst(Ptr, SclPtrTy, "ptrTypeCast", caller);

  for (unsigned i=0; i< NumElem; ++i) {
    Constant *Idx = ConstantInt::get(Type::getInt32Ty(Elem->getContext()), i);
    Value *GEP = GetElementPtrInst::Create(Ptr, Idx, "vstore", caller);
    Value *MaskBit = ExtractElementInst::Create(Mask, Idx, "exmask", caller);
    Instruction *DataElem = ExtractElementInst::Create(Data, Idx, "exData", caller);
    Instruction *storer = new StoreInst(DataElem, GEP, caller);
    toPredicate(DataElem, MaskBit);
    toPredicate(storer, MaskBit);
  }

  caller->eraseFromParent();
}


void FuncResolver::resolveFunc(CallInst* caller) {

  // Create function arguments
  std::vector<const Type*> args;

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
    func, params.begin(), params.end(), "", caller);
  caller->replaceAllUsesWith(call);
  // Replace predicate with control flow
  toPredicate(call, caller->getArgOperand(0));
  // Remove original call
  caller->eraseFromParent();
}



bool FuncResolver::TargetSpecificResolve(CallInst* caller) {

  Function* called = caller->getCalledFunction();
  std::string calledName = called->getName();
  V_PRINT(DEBUG_TYPE, "Inspecting "<<calledName<<"\n");

  if (MicResolve) {
    std::string IntrinsicName = "";

    // Use name to decide what to do
    if (Mangler::isMangledLoad(calledName)) {
      Value *Mask = caller->getArgOperand(0);
      Value *Ptr  = caller->getArgOperand(1);
      assert(Ptr->getType()->isPointerTy() && "Ptr is not a pointer!");
      assert(Mask->getType()->getScalarSizeInBits() == 1 && "Invalid mask size");
      const Type *MaskTy = Mask->getType();
      const Type *RetTy = caller->getType();
      const PointerType *PtrTy  = cast<PointerType>(Ptr->getType());
      assert(PtrTy->getElementType() == RetTy && 
        "mismatch between ptr and retval");
      
      if (RetTy == m_v16i32 && MaskTy == m_v16i1) 
        IntrinsicName = "llvm.mic.load.v16i32";
      else if (RetTy == m_v16f32 && MaskTy == m_v16i1) 
        IntrinsicName = "llvm.mic.load.v16f32";
      else if (RetTy == m_v8i64 && MaskTy == m_v8i1) 
        IntrinsicName = "llvm.mic.load.v8i64";
      else if (RetTy == m_v8f64 && MaskTy == m_v8i1) 
        IntrinsicName = "llvm.mic.load.v8f64";

      if (IntrinsicName != "") {
        std::vector<Value*> args;
        std::vector<const Type *> types;

        args.push_back(Mask);
        args.push_back(Ptr);
        types.push_back(MaskTy);
        types.push_back(PtrTy);

        FunctionType *intr = FunctionType::get(RetTy, types, false);
        Constant* new_f = caller->getParent()->getParent()->getParent()->
          getOrInsertFunction(IntrinsicName, intr);
        caller->replaceAllUsesWith(CallInst::Create(new_f, 
          args.begin(), args.end(), "", caller));
        caller->eraseFromParent();
        return true;
      }
    }

    if (Mangler::isMangledStore(calledName)) {
      Value *Mask = caller->getArgOperand(0);
      Value *Ptr  = caller->getArgOperand(2);
      Value *Data = caller->getArgOperand(1);
      assert(Ptr->getType()->isPointerTy() && "Ptr is not a pointer!");
      assert(Mask->getType()->getScalarSizeInBits() == 1 && "Invalid mask size");
      const Type *MaskTy = Mask->getType();
      const Type *DataTy = Data->getType();
      const PointerType *PtrTy  = cast<PointerType>(Ptr->getType());
      assert(PtrTy->getElementType() == DataTy && 
        "mismatch between ptr and retval");
      
      if (DataTy == m_v16i32 && MaskTy == m_v16i1) 
        IntrinsicName = "llvm.mic.store.v16i32";
      else if (DataTy == m_v16f32 && MaskTy == m_v16i1) 
        IntrinsicName = "llvm.mic.store.v16f32";
      else if (DataTy == m_v8i64 && MaskTy == m_v8i1) 
        IntrinsicName = "llvm.mic.store.v8i64";
      else if (DataTy == m_v8f64 && MaskTy == m_v8i1) 
        IntrinsicName = "llvm.mic.store.v8f64";

      if (IntrinsicName != "") {
        std::vector<Value*> args;
        std::vector<const Type *> types;

        args.push_back(Mask);
        args.push_back(Data);
        args.push_back(Ptr);
        types.push_back(MaskTy);
        types.push_back(DataTy);
        types.push_back(PtrTy);

        FunctionType *intr = FunctionType::get(
          Type::getVoidTy(DataTy->getContext()), types, false);
        Constant* new_f = caller->getParent()->getParent()->getParent()->
          getOrInsertFunction(IntrinsicName, intr);
        caller->replaceAllUsesWith(CallInst::Create(new_f, 
          args.begin(), args.end(), "", caller));
        caller->eraseFromParent();
        return true;

      }
    }

  }// mic resolve

  // Did not handle this load
  return false;
}

} // namespace

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  FunctionPass* createFuncResolver() {
    return new intel::FuncResolver();
  }
}
char intel::FuncResolver::ID = 0;
static RegisterPass<intel::FuncResolver>
CLIFuncResolver("resolve", "Resolves masked and vectorized function calls");

