/*********************************************************************************************
 * Copyright ? 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#include "PrintfAdder.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/raw_ostream.h"

namespace intel {

int PrintfAdder::getIntEnvVarVal (const char *varName, int defVal) {
  char *envVar;
  envVar = getenv(varName);
  if (!envVar) return defVal;
  return atoi(envVar);
}
    
void PrintfAdder::getConditionalGIDs(Function *F) {
  unsigned numDim = 0;
  for (inst_iterator sI = inst_begin(F); sI != inst_end(F); ++sI) {
    if (CallInst *CI = dyn_cast<CallInst>(&*sI)) {
      if (Function *func = CI->getCalledFunction()) {
        if (func->getName().find(GET_GID_NAME) != std::string::npos) {
          if (ConstantInt *C = dyn_cast<ConstantInt>(CI->getArgOperand(CI->getNumArgOperands()-1))) {
            unsigned dim = C->getZExtValue();
            numDim = numDim > dim ? numDim : dim +1; 
          }
        }
      }
    }
  }
    
  assert(numDim < 4 && "bad dimensions");
  if (numDim>0) m_ngids.push_back(getIntEnvVarVal("VECT_DEBUG_GID0", 0));
  if (numDim>1) m_ngids.push_back(getIntEnvVarVal("VECT_DEBUG_GID1", 0));
  if (numDim>2) m_ngids.push_back(getIntEnvVarVal("VECT_DEBUG_GID2", 0));
  
  BasicBlock &entry = F->getEntryBlock();
  Instruction *loc = entry.getFirstNonPHI();
  Module *M = F->getParent();
  Function *gidFunc = M->getFunction(GET_GID_NAME);
  FunctionType *fTy = gidFunc->getFunctionType();

  m_gids.clear();
  for (unsigned i=0; i<numDim; ++i) {
    Value * constVal = ConstantInt::get(fTy->getParamType(0), i);
    std::stringstream gidName;
    gidName << "gid" << i;
    m_gids.push_back(CallInst::Create(gidFunc, constVal, gidName.str(), loc));
  }
}

void PrintfAdder::setUninitializedNames(debug_print_args& print_args) {
  // Enumerator for values without names
  static unsigned emptyNameIndex = 1;
  std::list<Value *>& toPrint = print_args.toPrint;
  for (std::list<Value *>::iterator it = toPrint.begin(), e = toPrint.end();
       it != e; ++it){
    Value *val = *it;
    if (val->getName() == "") {
      std::stringstream newName;
      newName << "serial_print_name_" << emptyNameIndex << "_";
      emptyNameIndex++;
      val->setName(newName.str());  
    }
  }
}

void PrintfAdder::addDebugPrintGID(Function *F) {
  debug_print_args print_args;  
  print_args.prefix = "";  
  print_args.suffix = "\n";
  for (unsigned i=0; i<m_gids.size(); ++i) {
    print_args.toPrint.push_back(m_gids[i]);
  }
  Instruction *loc = F->getEntryBlock().getTerminator();
  addDebugPrintImpl(F, print_args, loc);
}

void PrintfAdder::addDebugPrintFuncArgs(Function *F) {
  debug_print_args print_args;  
  print_args.prefix = (std::string)F->getName() + " " ;
  print_args.suffix = "\n";
  Function::arg_iterator args_it = F->arg_begin();
  Function::arg_iterator args_e = F->arg_end();
  unsigned argInd = 0;
  for ( ; args_it != args_e; ++args_it, ++argInd) {
    Value *curArg= args_it;
    print_args.toPrint.push_back(curArg);  
  }

  Instruction *loc = F->getEntryBlock().getTerminator();
  addDebugPrint(F, print_args, loc);
}


void PrintfAdder::addDebugPrint(Function *F, debug_print_args& print_args, Instruction *loc) {
  if (m_printConditionally) {
    addDebugCondPrintFunction(F, print_args, loc);
  } else {
    addDebugPrintImpl(F, print_args, loc);
  }
}


bool PrintfAdder::canPrint(Value *v) {
  return isa<Instruction>(v) || isa<Argument>(v) || isa<Constant>(v);
}

void PrintfAdder::addPrintDepth(Function *F, Value *valToTrack, int depthThr) {
  std::pair<Value *, int> orig (valToTrack, 0);
  std::list<std::pair<Value *, int> > queue;
  queue.push_back(orig);
  while (!queue.empty()) {
    Value *val = queue.front().first;
    int depth = queue.front().second;
    queue.pop_front();

    // Exit conditions.
    if (depth == depthThr) continue;
    if (!canPrint(val)) continue;

    // Print the current value.
    debug_print_args printArgs;  
    printArgs.prefix = "";
    printArgs.toPrint.push_back(val);
    std::stringstream suffstream;
    suffstream << "   depth=" << depth << "\n";
    printArgs.suffix = suffstream.str();
    Instruction *inst = dyn_cast<Instruction>(val);
    Instruction *loc = inst ? inst->getParent()->getTerminator() : F->getEntryBlock().getTerminator();
    addDebugPrint(F, printArgs, loc);

    // On instructions add the operands to the queue.
    if (inst) {
      for (unsigned i = 0; i < inst->getNumOperands(); ++i) {
        Value *op = inst->getOperand(i);
        queue.push_back(std::pair<Value *, int>(op, depth+1));
      }
    }
  }
}
    
void PrintfAdder::addGIDConditionPrint (Function *F, debug_print_args& print_args, VVec &gids) {
  assert(gids.size() == m_ngids.size());
  assert(m_printConditionally);
  BasicBlock * ifgid = BasicBlock::Create(F->getContext(), "ifgid", F);
  Value *cond = ConstantInt::get(F->getContext(), APInt(1,1));
  for (unsigned i=0; i<gids.size(); ++i) {
    Value * const_i = ConstantInt::get(gids[i]->getType(), m_ngids[i]);
    Instruction *compare_i = new ICmpInst(*ifgid, CmpInst::ICMP_EQ, gids[i], const_i, "gidieq");
    if (i == 0) {
      cond = compare_i;
    } else {
      cond = BinaryOperator::Create(Instruction::And, cond, compare_i, "and_eq" , ifgid);
    }
  }
  
  BasicBlock * printBlock = BasicBlock::Create(F->getContext(), "print", F);
  BasicBlock * newret = BasicBlock::Create(F->getContext(), "newret", F);
  BranchInst::Create(printBlock, newret, cond, ifgid);
  BranchInst::Create(newret, printBlock);
        
  for(Function::iterator bbi=F->begin(), bbe=F->end(); bbi!=bbe; ++bbi) {
    // replace ret with branch to tail
    if(TerminatorInst *TI = bbi->getTerminator() ) {
      if (isa<ReturnInst>(TI)) {
        TI->eraseFromParent();
        BranchInst::Create(ifgid, bbi);
      }
    }
  }
  ReturnInst::Create(F->getContext(), newret);
  addDebugPrintImpl(F, print_args, printBlock->getTerminator());
}
    

void PrintfAdder::addDebugCondPrintFunction(Function *F, debug_print_args& print_args,
                                            Instruction *loc) {
  assert(m_gids.size() == m_ngids.size());
  static unsigned print_index = 1;
  std::stringstream  fName;
  fName << "debug_print_" << print_index;
  print_index++;
   
  std::vector<Type *> argTypes;
  std::vector<Value *> args;
  for (unsigned i=0; i<m_gids.size(); i++) {
    argTypes.push_back(m_gids[i]->getType());
    args.push_back(m_gids[i]);
  }
  for (std::list<Value *>::iterator it = print_args.toPrint.begin(), e = print_args.toPrint.end();
       it != e ; ++it) {
    Value *v = *it;
    argTypes.push_back(v->getType()); 
    args.push_back(v);
  }

  Module *M = F->getParent();
  FunctionType *fType = FunctionType::get(Type::getVoidTy(F->getContext()), argTypes, false);
  Function *printFunc = Function::Create(fType, F->getLinkage(), fName.str(), M);
    
  BasicBlock *entry = BasicBlock::Create(F->getContext(), "entry" , printFunc);
  ReturnInst::Create(F->getContext(), entry);
        
  Function::arg_iterator print_args_it = printFunc->arg_begin();
  Function::arg_iterator print_args_e = printFunc->arg_end();
        
  SmallVector<Value *, 4> func_gids;
  for (unsigned i=0; i<m_gids.size(); ++i) {
    func_gids.push_back(print_args_it++);
  }
    
  debug_print_args func_print_args;
  func_print_args.prefix = print_args.prefix;
  func_print_args.suffix = print_args.suffix;
  
  while (print_args_it != print_args_e) {
    func_print_args.toPrint.push_back(print_args_it++);
  }

  // Taking names of values in the original function and use them in the print function
  setUninitializedNames(print_args);
  Function::arg_iterator argIt =printFunc->arg_begin();  
  for (unsigned i=0; i<args.size(); ++i, ++argIt) {
    argIt->setName(args[i]->getName());
  }
        
  addGIDConditionPrint(printFunc, func_print_args, func_gids);
    
  CallInst::Create(printFunc, ArrayRef<Value*>(args), "", loc);

  
}   

void addPrintScalar(Value *v, std::vector<Value *> &printf_inputs, std::string &printf_str, Instruction *loc) {
  Type *T = v->getType();
  if (T->isIntegerTy()) {
    Value *toPrint = v;
    if (!T->isIntegerTy(32)) {
      toPrint = CastInst::CreateIntegerCast(v, Type::getInt32Ty(v->getContext()), false, "cast", loc);
    }
    printf_str = printf_str + "%d";  
    printf_inputs.push_back(toPrint);  
  } else if (T->isFloatTy()) {
    printf_str = printf_str + "%f"; 
    v = FPExtInst::Create(Instruction::FPExt, v, Type::getDoubleTy(v->getContext()),"cast", loc);
    printf_inputs.push_back(v);  
  } else if (T->isDoubleTy()) {
    printf_str = printf_str + "%f"; 
    printf_inputs.push_back(v);  
  } else if (T->isPointerTy())  {
    printf_str = printf_str + "%p";
    printf_inputs.push_back(v);
  } else {
    printf_str = printf_str + "xxx"; 
  }
}

void addPrintVector(Value *v, std::vector<Value *> &printf_inputs, std::string &printf_str, Instruction *loc) {
  Type *T = v->getType();
  const VectorType *vType = cast<VectorType>(T);
  unsigned numElts = vType->getNumElements();
  printf_str = printf_str + "<";
  for (unsigned i=0; i<numElts; i++) {
    if (i > 0) printf_str = printf_str + ", ";
    Value *extract =  ExtractElementInst::Create(v, ConstantInt::get(Type::getInt32Ty(v->getContext()), i), "", loc);
    addPrintScalar(extract, printf_inputs, printf_str, loc);
  }
  printf_str = printf_str + ">";
}
    
   
void PrintfAdder::addDebugPrintImpl(Function *F, debug_print_args& print_args, Instruction *loc) {  
  std::string& prefix = print_args.prefix;
  std::string& suffix = print_args.suffix;
  std::list<Value *>& toPrint = print_args.toPrint;
    
  setUninitializedNames(print_args);     
    
  Module *currentModule = F->getParent();
  std::string printf_str = prefix;
  std::vector<Value *> printf_inputs;
  printf_inputs.push_back(0); // reserved for printf_str 

  std::string type_str;  
  std::string gap = "   ";
  for (std::list<Value *>::iterator it = toPrint.begin(), e = toPrint.end();
       it != e; ++it) {
    Value *cur = *it;
    printf_str = printf_str + gap;
    if (m_printNames) printf_str = printf_str + (std::string)cur->getName() + " ";
    
    if (cur->getType()->isVectorTy()) {
      addPrintVector(cur, printf_inputs, printf_str, loc);
    } else {
      addPrintScalar(cur, printf_inputs, printf_str, loc);
    }
  }
  printf_str = printf_str + suffix;  
  
  // Create global
  Constant * strVal = ConstantDataArray::getString(F->getContext(), printf_str.c_str(), true);
  Type * arrayType = strVal->getType();
  unsigned strAddrSpace = 2;
  GlobalVariable * newGV = new GlobalVariable(*currentModule,
                                              arrayType,
                                              true,
                                              GlobalValue::InternalLinkage,
                                              strVal,
                                              "ptrstr",
                                              0,
                                              GlobalVariable::NotThreadLocal,
                                              strAddrSpace);

  // Declare printf function
  Type *strType = PointerType::get(Type::getInt8Ty(F->getContext()), strAddrSpace);
  std::vector<Type*> arglist(1, strType);
  FunctionType * prtFuncType = FunctionType::get(Type::getInt32Ty(F->getContext()), arglist, true);
  Constant * printFuncConst = currentModule->getOrInsertFunction("printf", prtFuncType);
  Function * printFunc = dyn_cast<Function>(printFuncConst);
  std::vector<Value*> inputIters(2, ConstantInt::get(Type::getInt32Ty(F->getContext()), 0));
  Instruction *strPtr = GetElementPtrInst::Create(newGV, ArrayRef<Value*>(inputIters), "", loc);
  //if (strPtr->getType() != strType ) {
  //  strPtr = new BitCastInst(strPtr, strType, "ptrTypeCast", loc);
  //}
  printf_inputs[0] = strPtr;

  CallInst::Create(printFunc, ArrayRef<Value*>(printf_inputs), "", loc);
}



void PrintfAdder::addDebugPrints(Function *F) {
  
  // Create get_global_uid calls to use for printing condionally from certain wok item.
  // Also reads form environment variables the gobal_id to print in case m_printConditionally flag is set.
  getConditionalGIDs(F);
  
  // Will print the arguments of the function 
  //addDebugPrintFuncArgs(F);

  // Will print the gid\s of each work item
  //addDebugPrintGID(F);
  
  for (inst_iterator sI = inst_begin(F); sI != inst_end(F); ++sI) {
    Instruction *I =&*sI;
    //if (I->getNameStr().find("test_test_load_in_mask") != std::string::npos) {
    if (I->getOpcode() == Instruction::UDiv) {
      // Will print I for all work items \ certain work item depeneds on m_printConditionally flag.
      debug_print_args print_args;
      print_args.toPrint.push_back(I);
      print_args.prefix = "";
      print_args.suffix = "\n";
      addDebugPrint(F, print_args, I->getParent()->getTerminator());

      // Will print I and it's ops recursively till depth 2 for all work items \ certain work
      // item depeneds on m_printConditionally flag.
      addPrintDepth(F, I, 2);
    }
  }
}  

bool PrintfAdder::runOnFunction(Function &F) {
  addDebugPrints(&F);
  return true;
}

}//nampespace intel

char intel::PrintfAdder::ID = 0;
extern "C" {
  
  FunctionPass* createPrintfAdderPass(bool printNames=true, bool printConditionaly=true) {
    return new intel::PrintfAdder(printNames, printConditionaly);
  }
}
//static RegisterPass<intel::PrintfAdder> PrintfAdder("PrintfAdder", "add printf call to a function");

