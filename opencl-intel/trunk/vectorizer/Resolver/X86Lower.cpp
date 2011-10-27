#include "llvm/Analysis/Verifier.h"

#include "X86Lower.h"
#include "Logger.h"
#include <sstream>
namespace intel {


static void AddComperator(Module &M, std::string name, std::string intrinsic) {

  // Type Definitions
  const Type* Vec8Type = VectorType::get(IntegerType::get(M.getContext(), 32), 8);
  const Type* Vec4Type = VectorType::get(IntegerType::get(M.getContext(), 32), 4);
  const Type* Int32T = IntegerType::get(M.getContext(), 32);
  std::vector<const Type*> T8Func;
  T8Func.push_back(Vec8Type);
  T8Func.push_back(Vec8Type);
  FunctionType* Func8Ty_0 = FunctionType::get(Vec8Type, T8Func, false);
  
  // Function Declarations
  
  Function* FuncCMP = Function::Create(Func8Ty_0, GlobalValue::ExternalLinkage,
   name.c_str(), &M); 

  std::vector<Constant*> v03;
  std::vector<Constant*> v47;
  std::vector<Constant*> v07;
  for (int i=0; i<4; i++) v03.push_back(ConstantInt::get(Int32T, i));
  for (int i=4; i<8; i++) v47.push_back(ConstantInt::get(Int32T, i));
  for (int i=0; i<8; i++) v07.push_back(ConstantInt::get(Int32T, i));
  Value* M03 = ConstantVector::get(v03);
  Value* M47 = ConstantVector::get(v47);
  Value* M07 = ConstantVector::get(v07);

  BasicBlock* entry = BasicBlock::Create(M.getContext(), "entry", FuncCMP,0);

  Function::arg_iterator args = FuncCMP->arg_begin();
  Value* ArgA = args; args++;
  Value* ArgB = args;
     
  Value *ALow = new ShuffleVectorInst(ArgA, UndefValue::get(ArgA->getType()), M03, "ALow",entry);
  Value *BLow = new ShuffleVectorInst(ArgB, UndefValue::get(ArgB->getType()), M03, "BLow",entry);
  Value *AHigh = new ShuffleVectorInst(ArgA, UndefValue::get(ArgA->getType()), M47, "AHigh",entry);
  Value *BHigh = new ShuffleVectorInst(ArgB, UndefValue::get(ArgB->getType()), M47, "BHigh",entry);

  std::vector<const Type*> params;
  params.push_back(Vec4Type);
  params.push_back(Vec4Type);
  const FunctionType* FType = FunctionType::get(Vec4Type, params, false);

  Constant* IntrinsicFunc = M.getOrInsertFunction(intrinsic.c_str(), FType);

  std::vector<Value*> ArgsL;
  ArgsL.push_back(ALow);
  ArgsL.push_back(BLow);
  std::vector<Value*> ArgsH;
  ArgsH.push_back(AHigh);
  ArgsH.push_back(BHigh);

  Value* CmpLow = CallInst::Create(IntrinsicFunc, ArgsL.begin(), ArgsL.end(), "callLow", entry);
  Value* CmpHigh = CallInst::Create(IntrinsicFunc, ArgsH.begin(), ArgsH.end(), "callLow", entry);

  Value *Ret = new ShuffleVectorInst(CmpLow, CmpHigh, M07, "join",entry);

  ReturnInst::Create(M.getContext(), Ret, entry);
}

bool X86Lower::doInitialization(Module &M) {
  AddComperator(M, "local.avx256.pcmpeq.d", "llvm.x86.sse2.pcmpeq.d");
  AddComperator(M, "local.avx256.pcmpgt.d", "llvm.x86.sse2.pcmpgt.d");
  return true;
}

Value* X86Lower::convertToI32(Value* A, Instruction* loc) {
  V_ASSERT(A && loc);
  // if this is a 1-bit predicate
  if (needTranslate(A)) {
    // if it has an alloca
    if (m_trans.find(A) != m_trans.end()) {
      // load its value
      A = new LoadInst(m_trans[A], "bin_a", loc);
    } else {
      // otherwise, it must be a constant
      A = TranslateConst(A);
    }
  }
  V_ASSERT(A);
  return A;
}

bool X86Lower::needTranslate(Value* val) {
  const Type *tp = val->getType();
  if (tp->isVectorTy()) {
    // if this is a vector, look at the element type
    tp = dyn_cast<VectorType>(tp)->getElementType();
  }
  if (tp->isIntegerTy()) {
    // if this is an integer and has a bitwidth of one
    return (dyn_cast<IntegerType>(tp)->getBitWidth() == 1);
  }
  // this is not a 1-bit predicate value
  return false;
}

const Type* X86Lower::TranslateType(const Type* tp) {
  if (tp->isVectorTy()) {
    const VectorType *vt = dyn_cast<VectorType>(tp);
    return  VectorType::get(m_i32, vt->getNumElements());
  }

  if (tp->isIntegerTy()) {
    return m_i32;
  }

  V_ASSERT(false && "bad type");
  return m_i32;
}

Constant* X86Lower::TranslateConst(Value* val) {
  V_PRINT("x86lower", "Doing TranslateConst --  "<<*val<<"\n");

  // translate vector of integers
  if (val->getType()->isVectorTy()) {
    ConstantVector* cv = dyn_cast<ConstantVector>(val);
    const VectorType *vt = dyn_cast<VectorType>(val->getType());
    V_ASSERT(vt);
    // Zero initializer

    std::vector<Constant*> cvect;
    for (unsigned i = 0; i< vt->getNumElements(); i++) {

      Constant* elem;
      if (val->getValueID() == Value::ConstantAggregateZeroVal) {
        elem = ConstantInt::get(m_i32, 0);
      } else if (val->getValueID() == Value::UndefValueVal) {
        elem = UndefValue::get(m_i32);
      } else {
        V_ASSERT(cv);
        elem = TranslateConst(cv->getOperand(i));
      }
      cvect.push_back(elem);
    }
    return ConstantVector::get(cvect);
  }

  // translate scalar
  if (val->getType()->isIntegerTy()) {

    // This is an i1 type with an undef value
    if (dyn_cast<UndefValue>(val)) {
      return UndefValue::get(m_i32);
    }

    // This is an integer type, which may be zero.
    if (ConstantInt *ci = dyn_cast<ConstantInt>(val)) {
      if (ci->isZero()) {
        return m_i32_0;
      } else {
        return m_i32_111;
      }
    } else {
      Constant *C = dyn_cast<Constant>(val);
      assert(C && "Invalid constant expression");
      return ConstantExpr::getSExtOrBitCast(C, m_i32);
    }
  }

  V_ASSERT(false && "unknown type"); // has to return something.
  return UndefValue::get(Type::getInt32Ty(*m_context));
}


const char* X86Lower::getIntrinsicNameForCMPType(int predicate, const Type* vec) {
  const VectorType* vt = dyn_cast<VectorType>(vec);
  V_PRINT("x86lower", "looking for intrinsic for "<<*vec<<" of type "<<predicate<<"\n");
  if (! vt) return NULL;
  unsigned numElem = vt->getNumElements();
  const Type* elTp = vt->getElementType();
  if (! elTp->isIntegerTy()) return NULL;
  /*if (predicate == CmpInst::ICMP_EQ && numElem == 2 && elTp->isIntegerTy() && dyn_cast<IntegerType>(elTp)->getBitWidth() == 64) {
    if (m_arch < SSE4) return NULL;
    return "llvm.x86.sse41.pcmpeq.q";
  }*/
  if (predicate == CmpInst::ICMP_EQ && numElem == 4 && elTp->isIntegerTy() && dyn_cast<IntegerType>(elTp)->getBitWidth() == 32) {
    return "llvm.x86.sse2.pcmpeq.d";
  }
  /*if (predicate == CmpInst::ICMP_EQ && numElem == 8 && elTp->isIntegerTy() && dyn_cast<IntegerType>(elTp)->getBitWidth() == 16) {
    return "llvm.x86.sse2.pcmpeq.w";
  }*/
  /*if (predicate == CmpInst::ICMP_EQ && numElem == 16 && elTp->isIntegerTy() && dyn_cast<IntegerType>(elTp)->getBitWidth() == 8) {
    return "llvm.x86.sse2.pcmpeq.b";
  }*/
  /*if (predicate == CmpInst::ICMP_SGT && numElem == 2 && elTp->isIntegerTy() && dyn_cast<IntegerType>(elTp)->getBitWidth() == 64) {
    if (m_arch < SSE4) return NULL;
    return "llvm.x86.sse41.pcmpgt.q";
  }*/
  if (predicate == CmpInst::ICMP_SGT && numElem == 4 && elTp->isIntegerTy() && dyn_cast<IntegerType>(elTp)->getBitWidth() == 32) {
    return "llvm.x86.sse2.pcmpgt.d";
  }
  /*if (predicate == CmpInst::ICMP_SGT && numElem == 8 && elTp->isIntegerTy() && dyn_cast<IntegerType>(elTp)->getBitWidth() == 16) {
    return "llvm.x86.sse2.pcmpgt.w";
  }*/
  /*if (predicate == CmpInst::ICMP_SGT && numElem == 16 && elTp->isIntegerTy() && dyn_cast<IntegerType>(elTp)->getBitWidth() == 8) {
    return "llvm.x86.sse2.pcmpgt.b";
  }*/
  if (m_arch >= AVX2) {

    if (predicate == CmpInst::ICMP_EQ && numElem == 8 && elTp->isIntegerTy() && dyn_cast<IntegerType>(elTp)->getBitWidth() == 32) {
      return "llvm.x86.avx2.pcmpeq.d";
    }
    if (predicate == CmpInst::ICMP_SGT && numElem == 8 && elTp->isIntegerTy() && dyn_cast<IntegerType>(elTp)->getBitWidth() == 32) {
      return "llvm.x86.avx2.pcmpgt.d";
    }

  } else {
    if (predicate == CmpInst::ICMP_EQ && numElem == 8 && elTp->isIntegerTy() && dyn_cast<IntegerType>(elTp)->getBitWidth() == 32) {
      return "local.avx256.pcmpeq.d";
    }
    if (predicate == CmpInst::ICMP_SGT && numElem == 8 && elTp->isIntegerTy() && dyn_cast<IntegerType>(elTp)->getBitWidth() == 32) {
      return "local.avx256.pcmpgt.d";
    }
  }

  V_PRINT("x86lower", "did not find intrinsic for "<<*vec<<" of type "<<predicate<<"\n");
  return NULL;
}

void X86Lower::Translate(BinaryOperator* bin) {
  Value *A= bin->getOperand(0);
  Value *B= bin->getOperand(1);
  A = convertToI32(A, bin);
  B = convertToI32(B, bin);
  // create a new binary operator of the converted values
  BinaryOperator* bio = BinaryOperator::Create(
    bin->getOpcode(), A, B, bin->getName() + "_32", bin);
  new StoreInst(bio, m_trans[bin], bin);
  return;
}

void X86Lower::Translate(SelectInst* select) {
  Value *A= select->getTrueValue();
  Value *B= select->getFalseValue();
  A = convertToI32(A, select);
  B = convertToI32(B, select);
  // create a new select of the converted values
  Value* new_sel = SelectInst::Create(select->getCondition(), A, B, select->getName() + "_sel", select);
  new StoreInst(new_sel, m_trans[select], select);
  return;
}

void X86Lower::Translate(CallInst* ci) {
  std::vector<Value*> args;
  std::vector<const Type *> types;
  for (unsigned i=0; i<ci->getNumArgOperands();++i) {
    Value *A= ci->getArgOperand(0);
    A = convertToI32(A, ci);
    args.push_back(A);
    types.push_back(A->getType());
  }
  // Create a function call with a similar name with a _32 suffix
  std::string name = std::string(ci->getCalledFunction()->getName()) +std::string("_i32");
  FunctionType *intr = FunctionType::get(ci->getType(), types, false);
  Constant* new_f = m_func->getParent()->getOrInsertFunction(name, intr);
  Instruction* call = CallInst::Create(new_f, args.begin(), args.end(), "", ci);
  m_rauw[ci] = call;
  return;
}

void X86Lower::Translate(PHINode* phi) {
  PHINode* new_phi = PHINode::Create(TranslateType(phi->getType()), phi->getName() + "_32", phi);
  for (unsigned i=0; i <  phi->getNumIncomingValues(); ++i) {
    Value* in = phi->getIncomingValue(i);
    if (m_trans.find(in) != m_trans.end()) {
      in = new LoadInst(m_trans[in], "in", phi->getIncomingBlock(i)->getTerminator());
    } else if (dyn_cast<Constant>(in)){
      in = TranslateConst(in);
    }
    new_phi->addIncoming(in, phi->getIncomingBlock(i));
  }
  new StoreInst(new_phi, m_trans[phi], phi->getParent()->getFirstNonPHI());
  return;
}

void X86Lower::Translate(ShuffleVectorInst* sv) {
  Value *A= sv->getOperand(0);
  Value *B= sv->getOperand(1);
  A = convertToI32(A, sv);
  B = convertToI32(B, sv);
  Instruction *new_sv = new ShuffleVectorInst(A, B, sv->getOperand(2), sv->getName() + "_32", sv);
  new StoreInst(new_sv, m_trans[sv], sv);
  return;
}

void X86Lower::Translate(InsertElementInst* ie){
  Value *A= ie->getOperand(0);
  Value *B= ie->getOperand(1);
  A = convertToI32(A, ie);
  B = convertToI32(B, ie);
  Instruction *new_ie = InsertElementInst::Create(A, B, ie->getOperand(2), ie->getName() + "_32", ie);
  new StoreInst(new_ie, m_trans[ie], ie);
  return;
}

void X86Lower::Translate(ExtractElementInst* ee) {
  Value *A= ee->getOperand(0);
  A = convertToI32(A, ee);
  Instruction *new_ee = ExtractElementInst::Create(A, ee->getOperand(1), ee->getName() + "_32", ee);
  new StoreInst(new_ee, m_trans[ee], ee);
  return;
}


void X86Lower::TranslateVector(ICmpInst* cmp) {
  V_PRINT("x86lower", "Doing vector compare "<<*cmp<<" \n");
  bool negate = false;

  // In case we compare bit vectors, switch to 32-bit vectors.
  Value *A = cmp->getOperand(0);
  Value *B = cmp->getOperand(1);
  A = convertToI32(A, cmp);
  B = convertToI32(B, cmp);

  const VectorType* vt = dyn_cast<VectorType>(A->getType());
  V_ASSERT(vt);
  const Type* elTp = vt->getElementType();
  unsigned numElem = vt->getNumElements();

  // look for an intrinsic
  const char* intr_name = NULL;
  intr_name = getIntrinsicNameForCMPType(cmp->getPredicate(), vt);

  // Didn't find ? Try to swap operands
  if (! intr_name) {
    // try to swap
    cmp->swapOperands();
    Value* t = A; A = B; B = t;
    intr_name = getIntrinsicNameForCMPType(cmp->getPredicate(), vt);
  }

  //Didn't help ? swap back
  if (! intr_name) {
    cmp->swapOperands();
    Value* t = A; A = B; B = t;
  }

  // Try to negate the computation
  if (! intr_name) {
    if (cmp->getPredicate() == CmpInst::ICMP_NE) {
      negate = true;
      intr_name = getIntrinsicNameForCMPType(CmpInst::ICMP_EQ, vt);
    }
  }
  // try unsigned hack (twist and shout)
  if (! intr_name) {
    V_PRINT("x86lower", "Trying unsigned hack for compare "<<*cmp<<" \n");
    const IntegerType *intType = dyn_cast<IntegerType>(elTp); V_ASSERT(intType);
    unsigned bw = intType->getBitWidth();
    std::vector<Constant*> cvect (numElem, ConstantInt::get(elTp, 1 << (bw -1)));
    Constant *cv = ConstantVector::get(cvect);
    Value * A2 = BinaryOperator::Create(Instruction::Xor, cv , A, "negate", cmp);
    Value * B2 = BinaryOperator::Create(Instruction::Xor, cv , B, "negate", cmp);

    // Once you do, we can use signed operands
    if (cmp->getPredicate() == CmpInst::ICMP_UGT) {
      intr_name = getIntrinsicNameForCMPType(CmpInst::ICMP_SGT, vt);
    } else if (cmp->getPredicate() == CmpInst::ICMP_ULT) {
      intr_name = getIntrinsicNameForCMPType(CmpInst::ICMP_SGT, vt);
      Value* t; t = A2; A2 = B2; B2 = t;
    } else if (cmp->getPredicate() == CmpInst::ICMP_UGE) {
      intr_name = getIntrinsicNameForCMPType(CmpInst::ICMP_SGT, vt);
      negate = true;
      Value* t; t = A2; A2 = B2; B2 = t;
    } else if (cmp->getPredicate() == CmpInst::ICMP_ULE) {
      intr_name = getIntrinsicNameForCMPType(CmpInst::ICMP_SGT, vt);
      negate = true;
    }
    if (intr_name) {
      V_PRINT("x86lower", "Found unsigned hack for compare "<<*cmp<<" \n");
      A = A2;
      B = B2;
    } else {
      negate = false;
    }
  }

  std::vector<Value*> args;
  std::vector<const Type *> types;

  // If you were able to find an intinsic
  if (intr_name) {
    args.push_back(A);
    args.push_back(B);
    types.push_back(A->getType());
    types.push_back(B->getType());

    V_PRINT("x86lower", "Doing compare using integer intrinsic "<<intr_name<<" for "<<*cmp<<" \n");
    FunctionType *intr = FunctionType::get(TranslateType(cmp->getType()), types, false);
    Constant* new_f = m_func->getParent()->getOrInsertFunction(intr_name, intr);
    Instruction* call = CallInst::Create(new_f, args.begin(), args.end(), "", cmp);

    if (negate) {
      std::vector<Constant*> cvect (numElem, ConstantInt::get(elTp, -1));
      Constant *cv = ConstantVector::get(cvect);
      call = BinaryOperator::Create(Instruction::Xor, call, cv, "negate", cmp);
    }
    new StoreInst(call, m_trans[cmp], cmp);
    return;
  }

  // If nothing helps, fallback to scalar execution
  return TranslateFallback(cmp);
}

void X86Lower::TranslateVector(FCmpInst* cmp) {
  const VectorType* vt = dyn_cast<VectorType>(cmp->getOperand(0)->getType());
  V_ASSERT(vt);
  const Type* elTp = vt->getElementType();
  unsigned numElem = vt->getNumElements();

  std::vector<Value*> args;
  std::vector<const Type *> types;

  if ((numElem == 4 || numElem == 8) && elTp->isFloatTy()) {
    V_PRINT("x86lower", "Trying  floating point for "<<*cmp<<" \n");

    /*
    CMPEQPS  xmm1,xmm2   => CMPPS xmm1,xmm2,0 
    CMPLTPS  xmm1,xmm2   => CMPPS xmm1,xmm2,1 
    CMPLEPS  xmm1,xmm2   => CMPPS xmm1,xmm2,2 
    CMPUNORDPS xmm1,xmm2 => CMPPS xmm1,xmm2,3 
    CMPNEQPS xmm1,xmm2   => CMPPS xmm1,xmm2,4 
    CMPNLTPS xmm1,xmm2   => CMPPS xmm1,xmm2,5 
    CMPNLEPS xmm1,xmm2   => CMPPS xmm1,xmm2,6 
    CMPORDPS xmm1,xmm2   => CMPPS xmm1,xmm2,7
    */
    bool neg = false;
    unsigned char imm = 255;
    if (cmp->getPredicate() == CmpInst::FCMP_OGT) {cmp->swapOperands();}
    if (cmp->getPredicate() == CmpInst::FCMP_OGE) {cmp->swapOperands();}
    if (cmp->getPredicate() == CmpInst::FCMP_UGE) {cmp->swapOperands();}
    if (cmp->getPredicate() == CmpInst::FCMP_UGT) {cmp->swapOperands();}
    
    V_PRINT(x86,"After swaping fcmp, opcode is "<<*cmp<<"\n");
    
    if (cmp->getPredicate() == CmpInst::FCMP_OEQ)  imm = 0x0;
    if (cmp->getPredicate() == CmpInst::FCMP_UEQ) {imm = 0x4; neg = true;}
    if (cmp->getPredicate() == CmpInst::FCMP_OLT)  imm = 0x1;
    if (cmp->getPredicate() == CmpInst::FCMP_ULT) {imm = 0x5; neg = true;}
    if (cmp->getPredicate() == CmpInst::FCMP_OLE)  imm = 0x2;
    if (cmp->getPredicate() == CmpInst::FCMP_UNO)  imm = 0x3;
    if (cmp->getPredicate() == CmpInst::FCMP_UNE)  imm = 0x4;
    if (cmp->getPredicate() == CmpInst::FCMP_ONE) {imm = 0x0; neg = true;}
    if (cmp->getPredicate() == CmpInst::FCMP_ULE) {imm = 0x6; neg = true;}
    if (cmp->getPredicate() == CmpInst::FCMP_ORD)  imm = 0x7;
    
    V_ASSERT(imm < 8 && "unable to find predicate");

    Value *A= cmp->getOperand(0);
    Value *B= cmp->getOperand(1);

    if (imm != 255) {
      args.push_back(A);
      args.push_back(B);
      types.push_back(A->getType());
      types.push_back(B->getType());

      V_PRINT("x86lower", "Using cmp.ps for "<<*cmp<<" with value "<<int(imm)<<"\n");
      Constant* cmmi = ConstantInt::get(m_i8, imm);
      args.push_back(cmmi);
      types.push_back(cmmi->getType());
      FunctionType *intr = FunctionType::get(A->getType(), types, false);
      Constant* new_f = NULL;
      if (numElem == 4) {
        new_f = m_func->getParent()->getOrInsertFunction("llvm.x86.sse.cmp.ps", intr);
      } else if (numElem == 8 && m_arch == AVX)  {
        new_f = m_func->getParent()->getOrInsertFunction("llvm.x86.avx.cmp.ps.256", intr);
      } else {
        TranslateFallback(cmp);
        return;
      }
      Instruction* call = CallInst::Create(new_f, args.begin(), args.end(), "call_cmps", cmp);
      call = new BitCastInst(call, TranslateType(cmp->getType()), "cmp_toi32", cmp);
      
      if (neg) {
          std::vector<Constant*> cvect (numElem, ConstantInt::get(m_i32, -1));
        Constant *cv = ConstantVector::get(cvect);
        call = BinaryOperator::Create(Instruction::Xor, call, cv, "negate", cmp);
      }

      new StoreInst(call, m_trans[cmp], cmp);
      return;
    }
  }

  return TranslateFallback(cmp);
}


void X86Lower::TranslateFallback(CmpInst* cmp) {
  const VectorType* vt = dyn_cast<VectorType>(cmp->getOperand(0)->getType());
  if  (vt) {
    V_PRINT("x86lower", "Using fallback for :( "<<*cmp<<" \n");
    unsigned numElem = vt->getNumElements();
    Value* cmp32 =  UndefValue::get(TranslateType(cmp->getType()));
    for (unsigned i=0; i< numElem; ++i) {
      // create regular cmp
      Constant* index = ConstantInt::get(m_i32, i);
      Value* A = ExtractElementInst::Create(cmp->getOperand(0) , index, "A"   , cmp);
      Value* B = ExtractElementInst::Create(cmp->getOperand(1) , index, "B"   , cmp);
      Value* scmp = CmpInst::Create(cmp->getOpcode(),cmp->getPredicate(),  A, B, "scalar_cmp", cmp);
      Instruction *SExt = new SExtInst(scmp, m_i32,"cmp32_vect", cmp);
      cmp32 = InsertElementInst::Create(cmp32, SExt , index, "Merge_X_", cmp);
    }
    new StoreInst(cmp32, m_trans[cmp], cmp);
  } else {
    // scalar
    V_PRINT("x86lower", "Using fallback bitcasr cmp for  scalar"<<*cmp<<" \n");
    Instruction *SExt = new SExtInst(cmp, m_i32,"cmp32_scalar");
    StoreInst *st = new StoreInst(SExt, m_trans[cmp]);
    SExt->insertAfter(cmp);
    st->insertAfter(SExt);
  }

}



void X86Lower::Translate(CmpInst* cmp) {
  V_PRINT("x86lower", "Doing compare "<<*cmp<<" \n");

  const VectorType* vt = dyn_cast<VectorType>(cmp->getOperand(0)->getType());
  if (vt) {
    if (ICmpInst *icmp = dyn_cast<ICmpInst>(cmp)) return TranslateVector(icmp);
    if (FCmpInst *fcmp = dyn_cast<FCmpInst>(cmp)) return TranslateVector(fcmp);
  } else {
    return TranslateFallback(cmp);
  }

}



void X86Lower::Translate(Value* val) {

  V_PRINT("x86lower", "Doing "<<*val<<" \n");
  Instruction* inst = dyn_cast<Instruction>(val);
  V_ASSERT(inst && "not an instruction!");

  if (BinaryOperator* bin = dyn_cast<BinaryOperator>(inst)) { return Translate(bin); }
  if (SelectInst* select = dyn_cast<SelectInst>(inst)) { return Translate(select);}
  if (CallInst* ci = dyn_cast<CallInst>(inst)) { return Translate(ci); }
  if (PHINode* phi = dyn_cast<PHINode>(inst)) { return Translate(phi); }
  if (ShuffleVectorInst* sv = dyn_cast<ShuffleVectorInst>(inst)) { return Translate(sv); }
  if (InsertElementInst* ie = dyn_cast<InsertElementInst>(inst)) { return Translate(ie); }
  if (ExtractElementInst* ee = dyn_cast<ExtractElementInst>(inst)) { return Translate(ee); }
  if (CmpInst* cmp = dyn_cast<CmpInst>(inst)) { return Translate(cmp); }

  V_ASSERT(false);
}

void X86Lower::LowerInst(ZExtInst* ex) {
  V_PRINT("x86lower", "Doing ZExtInst --  "<<*ex<<"\n");
  Value *A = ex->getOperand(0);
  if (!needTranslate(A)) return;

  // No need to handle zext instructions which are 
  // not i1 -> i32
  const Type* target = ex->getType();
  if (target->isVectorTy()) {
    // Check that we extend this instruction to vector of i32
    if (cast<VectorType>(target)->getElementType() != m_i32) return;  
  } else {
    // scalar test that the user is a 32bit one
    if (target != m_i32) return; 
  }
  
  // Note: In the case of zext i1 -> i64, we may have an unoptimized because 
  // we do not handle this case, and the original i1 code will remain.
 
  A = convertToI32(A, ex);
  Value* one;
  // Implement trunc using AND 0x1
  if (const VectorType *vt = dyn_cast<VectorType>(A->getType())) {
    const Type* elTp = vt->getElementType();
    unsigned numElem = vt->getNumElements();
    std::vector<Constant*> cvect (numElem, ConstantInt::get(elTp, 1));
    one = ConstantVector::get(cvect);
  } else {
    one = ConstantInt::get(A->getType(), 1);
  }
  Instruction* msb = BinaryOperator::Create(Instruction::And, one , A , "msb", ex);
  ex->replaceAllUsesWith(msb);

}

void X86Lower::LowerInst(BranchInst* bi) {
  if (bi->isConditional()) {
    Value* cond;
    cond=bi->getCondition();
    // no need to handle conditions which are coming from allOne/allZero functions
    if (dyn_cast<CallInst>(cond)) return;
    if (m_trans.find(cond) != m_trans.end()) {
      cond = m_trans[cond];
      cond = new LoadInst(cond, "cond", bi);
    } else if (isa<Constant>(cond)) {
      cond = TranslateConst(cond);
    } else {
      V_PRINT("x86lower", "problem with "<<*cond<<" \n");
      V_ASSERT(false);
    }
    Instruction* msb = BinaryOperator::Create(Instruction::AShr, cond , m_i32_31 , "msb", bi);
    CastInst* bit = new TruncInst(msb, m_i1, "bit",bi);
    bi->setCondition(bit);
  }
}

void X86Lower::LowerInst(SelectInst* si) {

  Value* cond = si->getCondition();
  cond = convertToI32(cond, si);
  Value *A = si->getTrueValue();
  Value *B = si->getFalseValue();
  if (const VectorType *vt = dyn_cast<VectorType>(cond->getType())) {
    const Type* elTp = vt->getElementType();
    unsigned numElem = vt->getNumElements();

    const VectorType *value_vt = dyn_cast<VectorType>(A->getType());
    V_ASSERT(value_vt);
    const Type* value_elTp = value_vt->getElementType();
    unsigned value_numElem = value_vt->getNumElements();

    //put assembly code
    bool Type_4xFloat = (value_numElem == 4 && value_elTp->isFloatTy());
    bool Type_8xFloat = (value_numElem == 8 && value_elTp->isFloatTy());
    bool Type_4xi32 = (value_numElem == 4 && value_elTp->isIntegerTy() && (dyn_cast<IntegerType>(value_elTp)->getBitWidth() == 32));
    bool Type_8xi32 = (value_numElem == 8 && value_elTp->isIntegerTy() && (dyn_cast<IntegerType>(value_elTp)->getBitWidth() == 32));

    if (((Type_4xFloat || Type_4xi32) && m_arch >= SSE4) ||
        ((Type_8xFloat || Type_8xi32) && m_arch >= AVX)) {
      V_PRINT("x86lower", "Doing vector selet because of  <w x type>:"<<Type_4xFloat<<Type_8xFloat<<Type_4xi32<<Type_8xi32 <<" \n");

      if (!value_elTp->isFloatTy()) {
        A = convertToI32(A, si);
        B = convertToI32(B, si);
        A = new BitCastInst(A, VectorType::get(Type::getFloatTy(*m_context),numElem), "", si);
        B = new BitCastInst(B, VectorType::get(Type::getFloatTy(*m_context),numElem), "", si);
      }
      if (!elTp->isFloatTy()) {
        cond = new BitCastInst(cond, VectorType::get(Type::getFloatTy(*m_context), numElem), "", si);
      }

      std::vector<Value*> args;
      args.push_back(B);
      args.push_back(A);
      args.push_back(cond);
      std::vector<const Type *> types;
      types.push_back(B->getType());
      types.push_back(A->getType());
      types.push_back(cond->getType());

      FunctionType *intr = FunctionType::get(A->getType(), types, false);
      Constant* new_f = NULL;
      if (Type_4xFloat || Type_4xi32) {
        new_f = m_func->getParent()->getOrInsertFunction("llvm.x86.sse41.blendvps", intr);
      } else {
        new_f = m_func->getParent()->getOrInsertFunction("llvm.x86.avx.blendv.ps.256", intr);
      }
      Instruction* call = CallInst::Create(new_f, args.begin(), args.end(), "", si);
      if (!value_elTp->isFloatTy()) {
        call = new BitCastInst(call, si->getType(), "", si);
      }
      si->setName("tokill");
      si->replaceAllUsesWith(call);
    } else {
      unsigned numelem = vt->getNumElements();
      Value* scalar_select = UndefValue::get(A->getType());
      // for each component
      for (unsigned i=0; i< numelem; ++i) {
        // create regular select
        Constant* index = ConstantInt::get(m_i32, i);
        Value* As = ExtractElementInst::Create(A, index, "trueVal"   , si);
        Value* Bs = ExtractElementInst::Create(B, index, "falseVal"   , si);
        Value* Cs = ExtractElementInst::Create(cond , index, "cond", si);
        Instruction* msb = BinaryOperator::Create(Instruction::AShr, Cs , m_i32_31 , "msb", si);
        CastInst* bit = new TruncInst(msb, m_i1, "bit", si);
        Value* new_sel = SelectInst::Create(bit, As, Bs, si->getName() + "_comp", si);
        scalar_select = InsertElementInst::Create(scalar_select, new_sel , index, "nselect_sclr", si);
      }
      si->setName("tokill");
      si->replaceAllUsesWith(scalar_select);
    }
  } else {
    V_PRINT("x86lower", "Doing scalar select #"<<*si<<"\n");
    Instruction* msb = BinaryOperator::Create(Instruction::AShr, cond , m_i32_31 , "msb", si);
    CastInst* bit = new TruncInst(msb, m_i1, "bit", si);
    Value* new_sel = SelectInst::Create(bit, A, B, si->getName() + "_comp", si);
    si->setName("tokill");
    si->replaceAllUsesWith(new_sel);
  }

}


void X86Lower::scalarizeSingleVectorInstruction(Instruction* inst) {

  // must be vector type
  if (! inst->getType()->isVectorTy()) return;
  for (Value::use_iterator it = inst->use_begin(),e=inst->use_end();it!=e;++it) {
    if (! isa<ExtractElementInst>(*it)) return;
  }

  // must be binary operator
  BinaryOperator *op = dyn_cast<BinaryOperator>(inst);
  if (!op) return;

  Value* A = op->getOperand(0);
  Value* B = op->getOperand(1);

  // must be created by either a constant or insert element
  if (isa<Instruction>(A) && !isa<InsertElementInst>(A)) return;
  if (isa<Instruction>(B) && !isa<InsertElementInst>(B)) return;

  V_PRINT("x86lower", "Scalarizing bottleneck instuction "<<*op<<"\n");

  const VectorType *vt = dyn_cast<VectorType>(inst->getType());
  unsigned numElem = vt->getNumElements();

  Value* join = UndefValue::get(op->getType());
  for (unsigned i=0; i< numElem; ++i) {
    // create regular op
    Constant* index = ConstantInt::get(m_i32, i);
    Value* As = ExtractElementInst::Create(A, index, "A", inst);
    Value* Bs = ExtractElementInst::Create(B, index, "B", inst);
    Value *sclop = BinaryOperator::Create(op->getOpcode(), As , Bs, "sclnck", inst);
    join = InsertElementInst::Create(join, sclop , index, "join", inst);
  }

  // replace all uses with new scalarized instruction
  op->replaceAllUsesWith(join);
}


bool X86Lower::runOnFunction(Function &F) {
  m_trans.clear();
  m_func = &F;
  m_rauw.clear();
  m_context = &F.getParent()->getContext();
  m_i1 =  IntegerType::get(*m_context, 1);
  m_i8 =  IntegerType::get(*m_context, 8);
  m_i32 =  IntegerType::get(*m_context, 32);
  m_i32_0 = ConstantInt::get(Type::getInt32Ty(*m_context), 0);
  m_i32_111 = ConstantInt::get(Type::getInt32Ty(*m_context), -1);
  m_i32_31 = ConstantInt::get(Type::getInt32Ty(*m_context), 31);


  // Create an alloca for each converted type in Func
  V_PRINT("x86lower", "Creating alloca for "<<F.getName()<<"\n");
  for (Function::iterator bb = F.begin(), bbe = F.end(); bb != bbe ; ++bb) {
    BasicBlock *block = bb;
    for (BasicBlock::iterator it = block->begin(), e = block->end(); it != e ; ++it) {
      if (needTranslate(it)) {
        m_trans[it] = new AllocaInst(TranslateType(it->getType()),
                                     it->getName() + "_32", F.getEntryBlock().begin());
      }
    }
  }

  // Translate all predicates to 32bit values
  V_PRINT("x86lower", "Translating -- "<<F.getName()<<"\n");
  for (Function::iterator bb = F.begin(), bbe = F.end(); bb != bbe ; ++bb) {
    BasicBlock *block = bb;
    for (BasicBlock::iterator it = block->begin(), e = block->end(); it != e ; ++it) {
      if (needTranslate(it)) {
        Translate(it);
      }
    }
  }

  // Replace all functions which were converted to 32bit version
  V_PRINT("x86lower", "Replacing --  "<<F.getName()<<"\n");
  for (std::map<Instruction*, Instruction*>::iterator it = m_rauw.begin(), e = m_rauw.end();
       it != e; ++it) {
    it->first->replaceAllUsesWith(it->second);
  }


  // Collect instructions of each type
  std::vector<SelectInst*> selects;
  std::vector<BranchInst*> branches;
  std::vector<ZExtInst*> zext;
  findAllInstructions<SelectInst>(selects, F);
  findAllInstructions<BranchInst>(branches, F);
  findAllInstructions<ZExtInst>(zext, F);

  // Handle ZExt of predicates
  V_PRINT("x86lower", "Doing ZExtInst -- "<<F.getName()<<"\n");
  for (std::vector<ZExtInst*>::iterator it = zext.begin(), e = zext.end(); it != e; ++it) {
    ZExtInst *ex = *it;
    LowerInst(ex);
  }

  // handle branches which use 1-bit predicates
  V_PRINT("x86lower", "Doing branches -- "<<F.getName()<<"\n");
  for (std::vector<BranchInst*>::iterator it = branches.begin(), e = branches.end(); it != e; ++it) {
    BranchInst *bi = *it;
    LowerInst(bi);
  }
  // handle selects which consume 1bit predicates
  V_PRINT("x86lower", "Doing selects -- "<<F.getName()<<"\n");
  for (std::vector<SelectInst*>::iterator it = selects.begin(), e = selects.end(); it != e ;  ++it) {
    SelectInst* si = *it;
    // No need to handle cases which are already manually onverted
    // This is because selects can be producers and consumers
    if (m_trans.find(si) != m_trans.end()) {
      continue;
    }
    LowerInst(si);
  }

  V_ASSERT(!verifyFunction(F) && "I broke this module");

  V_PRINT("x86lower", "Finished x86lowering of "<<F.getName()<<"\n");
  return true;
}


} // namespace

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  void* createX86LowerPass() {
    return new intel::X86Lower();
  }
}

char intel::X86Lower::ID = 0;
static RegisterPass<intel::X86Lower> CLIX86LowerX("x86lower", "Lower predicates to x86 words");

