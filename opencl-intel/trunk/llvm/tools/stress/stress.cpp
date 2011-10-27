#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/PassManager.h"
#include "llvm/Instruction.h"
#include "llvm/CallGraphSCCPass.h"
#include "llvm/Assembly/PrintModulePass.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Support/PassNameParser.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PluginLoader.h"
#include "llvm/Support/PrettyStackTrace.h"
#include <memory>
#include <sstream>
#include <algorithm>
using namespace llvm;


//Command line arguments:
static cl::opt<unsigned> SeedCL("seed", 
  cl::desc("Seed used for randomness"), cl::init(0));
static cl::opt<unsigned> SizeCL("size", 
  cl::desc("Estimated size of the generated function (# of instrs)"), cl::init(100));
static cl::opt<bool> VectorSelect("vectorselect", 
  cl::desc("Allow the generation of vector-select instructions"), cl::init(false));


/// A utility class to provide a pseudo-random number generator which is 
///  constant across all platforms. This is close to the libc implementation.
class Random {
public:
	/// C'tor
	Random(unsigned seed):m_seed(seed) {}
	/// Return the next random value.
	unsigned Rand() {
	  unsigned val = m_seed + 0x000b07a1;
	  m_seed = (val * 0x3c7c0ac1);
    // Only lowest 19 bits are random-ish.
	  return m_seed & 0x7ffff; 
	}

private:
	unsigned m_seed;
};


/// Generate an empty function with a default argument list.
Function *GenEmptyFunction(Module *M) {
  // Type Definitions
  std::vector<const Type*> ArgsTy;
  // Define a few arguments
  ArgsTy.push_back(PointerType::get(IntegerType::getInt8Ty(M->getContext()),0));
  ArgsTy.push_back(PointerType::get(IntegerType::getInt32Ty(M->getContext()),0));
  ArgsTy.push_back(PointerType::get(IntegerType::getInt64Ty(M->getContext()),0));
  ArgsTy.push_back(IntegerType::getInt32Ty(M->getContext()));
  ArgsTy.push_back(IntegerType::getInt8Ty(M->getContext()));
  ArgsTy.push_back(IntegerType::getInt64Ty(M->getContext()));
  
  FunctionType *FuncTy = FunctionType::get(
	  /*Result=*/Type::getVoidTy(M->getContext()),
    /*Params=*/ArgsTy,
    /*isVarArg=*/false);
  // Pick a unique name/
  std::stringstream ss;
  ss<<"autogen_"<<SeedCL<<"_"<<SizeCL<<VectorSelect;
  
  Function *Func = Function::Create(
    /*Type=*/FuncTy,
    /*Linkage=*/GlobalValue::ExternalLinkage,
    /*Name=*/ss.str(), M); 

  Func->setCallingConv(CallingConv::C);
  return Func;
}


/// A base class, implementing utilities needed for
/// modifying and adding new random instructions.
struct Modifier {
  /// Used to store the randomly generated values.
  typedef std::vector<Value*> PieceTable;

public:
	/// C'tor
	Modifier(BasicBlock *BB, PieceTable *PT, Random *R):m_BB(BB), m_PT(PT),m_R(R) {};
	/// Add a new instruction
	virtual void Act() = 0;
	/// Add new instructions in a loop
	virtual void ActN(unsigned n) { for (unsigned i=0; i<n; i++) Act(); }

protected:
  Value *getRandomVal() {assert(m_PT->size()); return m_PT->at(m_R->Rand() % m_PT->size());}

	Value *getTypedValue(const Type *Tp) {
		unsigned index = m_R->Rand();
		for (unsigned i=0; i<m_PT->size(); i++) {
			Value *V = m_PT->at((index + i) % m_PT->size());
			if (V->getType() == Tp) return V;
		}
    // If no value of this type was found, generate a constant value
    if (Tp->isIntegerTy()) {
      if (m_R->Rand() & 1)
        return ConstantInt::getAllOnesValue(Tp);
      return ConstantInt::getNullValue(Tp);
    } else if (Tp->isFloatingPointTy()) {
      if (m_R->Rand() & 1)
        return ConstantFP::getAllOnesValue(Tp);
      return ConstantFP::getNullValue(Tp);
    }
    
		return UndefValue::get(Tp);
	}

  Value *getPointerValue() {
		unsigned index = m_R->Rand();
		for (unsigned i=0; i<m_PT->size(); i++) {
			Value *V = m_PT->at((index + i) % m_PT->size());
			if (V->getType()->isPointerTy()) return V;
		}
		return UndefValue::get(getPointerType());
	}

	Value *getVectorValue() {
		unsigned index = m_R->Rand();
		for (unsigned i=0; i<m_PT->size(); i++) {
			Value *V = m_PT->at((index + i) % m_PT->size());
			if (V->getType()->isVectorTy()) return V;
		}
		return UndefValue::get(getVectorType());
	}

	const Type *getType() {
		return (m_R->Rand() & 3 ? getVectorType() : getScalarType());
	}

	const Type *getPointerType() {
		const Type *Ty = getType();
		return PointerType::get(Ty, 0);
	}

	const Type *getVectorType(unsigned len = (unsigned)-1) {
		const Type *Ty = getScalarType();
		unsigned width = 1<<((m_R->Rand() % 3) + (m_R->Rand() % 3));
		if (len != (unsigned)-1) width = len;
		return VectorType::get(Ty, width);
	}

	const Type *getScalarType() {
		switch (m_R->Rand() % 15) {

		  case 0: return Type::getInt1Ty(m_BB->getContext());
		  case 1: return Type::getInt8Ty(m_BB->getContext());
	    case 2: return Type::getInt16Ty(m_BB->getContext());
      case 3: case 4:
      case 5: return Type::getFloatTy(m_BB->getContext());
      case 6: case 7:
		  case 8: return Type::getDoubleTy(m_BB->getContext());		  
      case 9: case 10:
		  case 11: return Type::getInt32Ty(m_BB->getContext());
      case 12: case 13:
		  case 14: return Type::getInt64Ty(m_BB->getContext());
		}

    return Type::getInt32Ty(m_BB->getContext());
  }

  /// Basic block to populate
	BasicBlock *m_BB;
  /// Value table
	PieceTable *m_PT;
  /// Random number generator
  Random *m_R;
};

struct LoadModifier: public Modifier {
	LoadModifier(BasicBlock *BB, PieceTable *PT, Random *R):Modifier(BB, PT, R) {};
	virtual void Act() {
		// Try to use predefined pointers. If non exist, use undef pointer value;
    Value *Ptr = getPointerValue();
    Value *V = new LoadInst(Ptr, "L", m_BB->getTerminator()); 
		m_PT->push_back(V);
	}
};


struct StoreModifier: public Modifier {
	StoreModifier(BasicBlock *BB, PieceTable *PT, Random *R):Modifier(BB, PT, R) {}
	virtual void Act() {
		// Try to use predefined pointers. If non exist, use undef pointer value;		
		Value *Ptr = getPointerValue();
    Value *Val = getTypedValue(Ptr->getType()->getContainedType(0));
		new StoreInst(Val, Ptr, m_BB->getTerminator()); 
	}
};


struct BinModifier: public Modifier {
	BinModifier(BasicBlock *BB, PieceTable *PT, Random *R):Modifier(BB, PT, R) {}
	virtual void Act() {
		Value *Val0 = getRandomVal();
    Value *Val1 = getTypedValue(Val0->getType());
		
		if (Val0->getType()->isPointerTy() || 
        Val1->getType()->isPointerTy()) return;
		
		bool isFloat = Val0->getType()->getScalarType()->isFloatingPointTy();
		Value *V = NULL;
    switch (m_R->Rand() % 13) {
		case 0:{ V = BinaryOperator::Create(isFloat?Instruction::FAdd : Instruction::Add, Val0, Val1, "B", m_BB->getTerminator()); break; }
		case 1:{ V = BinaryOperator::Create(isFloat?Instruction::FSub : Instruction::Sub, Val0, Val1, "B", m_BB->getTerminator()); break; }
		case 2:{ V = BinaryOperator::Create(isFloat?Instruction::FMul : Instruction::Mul, Val0, Val1, "B", m_BB->getTerminator()); break; }
	    case 3:{ V = BinaryOperator::Create(isFloat?Instruction::FDiv : Instruction::SDiv, Val0, Val1, "B", m_BB->getTerminator()); break; }
		case 4:{ V = BinaryOperator::Create(isFloat?Instruction::FDiv : Instruction::UDiv, Val0, Val1, "B", m_BB->getTerminator()); break; }
	    case 5:{ V = BinaryOperator::Create(isFloat?Instruction::FRem : Instruction::SRem, Val0, Val1, "B", m_BB->getTerminator()); break; }
		case 6:{ V = BinaryOperator::Create(isFloat?Instruction::FRem : Instruction::URem, Val0, Val1, "B", m_BB->getTerminator()); break; }
		case 7:{ if (!isFloat) V = BinaryOperator::Create(Instruction::Shl, Val0, Val1, "B", m_BB->getTerminator()); break; }
        case 8:{ if (!isFloat) V = BinaryOperator::Create(Instruction::LShr, Val0, Val1, "B", m_BB->getTerminator()); break; }
        case 9:{ if (!isFloat) V = BinaryOperator::Create(Instruction::AShr, Val0, Val1, "B", m_BB->getTerminator()); break; }
	    case 10:{ if (!isFloat) V = BinaryOperator::Create(Instruction::And, Val0, Val1, "B", m_BB->getTerminator()); break; }
		case 11:{ if (!isFloat) V = BinaryOperator::Create(Instruction::Or, Val0, Val1, "B", m_BB->getTerminator()); break; }
		case 12:{ if (!isFloat) V = BinaryOperator::Create(Instruction::Xor, Val0, Val1, "B", m_BB->getTerminator()); break; }
		}
		if (V) m_PT->push_back(V);
	}
};


struct ConstModifier: public Modifier {
	ConstModifier(BasicBlock *BB, PieceTable *PT, Random *R):Modifier(BB, PT, R) {}
	virtual void Act() {
    const Type *Ty = getType();

    if (Ty->isVectorTy()) {
      switch (m_R->Rand() % 2) {
        case 0: if (Ty->isIntegerTy()) return m_PT->push_back(ConstantVector::getAllOnesValue(Ty));
        case 1: if (Ty->isIntegerTy()) return m_PT->push_back(ConstantVector::getNullValue(Ty));		    
		  }
    } 

    if (Ty->isFloatingPointTy()) {
      if (m_R->Rand() & 1) 
        return m_PT->push_back(ConstantFP::getNullValue(Ty));
      return m_PT->push_back(ConstantFP::get(Ty, ((double)1)/m_R->Rand()));
    }

    if (Ty->isIntegerTy()) {
      switch (m_R->Rand() % 7) {
		    case 0: if (Ty->isIntegerTy()) return m_PT->push_back(ConstantInt::get(Ty, APInt::getAllOnesValue(Ty->getPrimitiveSizeInBits())));
		    case 1: if (Ty->isIntegerTy()) return m_PT->push_back(ConstantInt::get(Ty, APInt::getNullValue(Ty->getPrimitiveSizeInBits())));
        case 2: case 3: case 4: case 5:
        case 6: if (Ty->isIntegerTy()) m_PT->push_back(ConstantInt::get(Ty, m_R->Rand()));
		  }
    }
    
	}
};

struct AllocaModifier: public Modifier {
	AllocaModifier(BasicBlock *BB, PieceTable *PT, Random *R):Modifier(BB, PT, R) {}
	virtual void Act() {
      const Type *Tp = getType();
	  m_PT->push_back(new AllocaInst(Tp, "A", m_BB->getFirstNonPHI()));
	}
};


struct ExtractElementModifier: public Modifier {
	ExtractElementModifier(BasicBlock *BB, PieceTable *PT, Random *R):Modifier(BB, PT, R) {}
	virtual void Act() {
	  Value *Val0 = getVectorValue();
	  Value *V = ExtractElementInst::Create(Val0, ConstantInt::get(Type::getInt32Ty(m_BB->getContext()), 
      m_R->Rand() % cast<VectorType>(Val0->getType())->getNumElements()),"E", m_BB->getTerminator());
	  return m_PT->push_back(V);
	}
};

struct ShuffModifier: public Modifier {
	ShuffModifier(BasicBlock *BB, PieceTable *PT, Random *R):Modifier(BB, PT, R) {}
	virtual void Act() {
		
		for (int i=0; i<10; ++i) {
			Value *Val0 = getVectorValue();
			Value *Val1 = getTypedValue(Val0->getType());

			unsigned Width = cast<VectorType>(Val0->getType())->getNumElements();
			std::vector<Constant*> Idxs;

			const Type *I32 = Type::getInt32Ty(m_BB->getContext());
			for (unsigned i=0; i<Width; ++i) {				
        Constant *CI = ConstantInt::get(I32, m_R->Rand() % (Width*2));
        if (!(m_R->Rand() % 5)) CI = UndefValue::get(I32);
				Idxs.push_back(CI);
			}

			Constant *Mask = ConstantVector::get(VectorType::get(I32, Width), Idxs);

			Value *V = new ShuffleVectorInst(Val0, Val1, Mask, "Shuff", m_BB->getTerminator());
			return m_PT->push_back(V);
		}
	}
};



struct InsertElementModifier: public Modifier {
	InsertElementModifier(BasicBlock *BB, PieceTable *PT, Random *R):Modifier(BB, PT, R) {}
	virtual void Act() {
		Value *Val0 = getVectorValue();
		Value *Val1 = getTypedValue(Val0->getType()->getScalarType());
			
		Value *V = InsertElementInst::Create( Val0, Val1, 
				ConstantInt::get(Type::getInt32Ty(m_BB->getContext()), m_R->Rand() % cast<VectorType>(Val0->getType())->getNumElements()), 
				"I",  m_BB->getTerminator());
		return m_PT->push_back(V);
	}
};

struct CastModifier: public Modifier {
	CastModifier(BasicBlock *BB, PieceTable *PT, Random *R):Modifier(BB, PT, R) {}
	virtual void Act() {

		Value *V = getRandomVal();
		const Type *VTy = V->getType();		
		const Type *DestTy = getScalarType();

		// Vectors
		if (VTy->isVectorTy()) {
			const VectorType *VecTy = cast<VectorType>(VTy);
			DestTy = getVectorType(VecTy->getNumElements());
		}

		if (VTy == DestTy) return;

		// Pointers
		if (VTy->isPointerTy()) {
      if (!DestTy->isPointerTy())
			  DestTy = PointerType::get(DestTy, 0);
			return m_PT->push_back(new BitCastInst(V, DestTy, "PC", m_BB->getTerminator()));
		}

    if ((m_R->Rand() & 1) && VTy->getPrimitiveSizeInBits() == DestTy->getPrimitiveSizeInBits()) {
			return m_PT->push_back(new BitCastInst(V, DestTy, "BC", m_BB->getTerminator()));
		}

		// Both Integers
		if (VTy->getScalarType()->isIntegerTy() && DestTy->getScalarType()->isIntegerTy()) {
			if (VTy->getScalarType()->getPrimitiveSizeInBits() > DestTy->getScalarType()->getPrimitiveSizeInBits()) {
				return m_PT->push_back(new TruncInst(V, DestTy, "Tr", m_BB->getTerminator()));
			} else {
        if (m_R->Rand() & 1)
				return m_PT->push_back(new ZExtInst(V, DestTy, "ZE", m_BB->getTerminator()));
				return m_PT->push_back(new SExtInst(V, DestTy, "Se", m_BB->getTerminator()));
			}
		}

		// fp to int
		if (VTy->getScalarType()->isFloatingPointTy() && DestTy->getScalarType()->isIntegerTy()) {
      if (m_R->Rand() & 1)
			return m_PT->push_back(new FPToSIInst(V, DestTy, "FC", m_BB->getTerminator()));
			return m_PT->push_back(new FPToUIInst(V, DestTy, "FC", m_BB->getTerminator()));
		}

		// int to fp
		if (VTy->getScalarType()->isIntegerTy() && DestTy->getScalarType()->isFloatingPointTy()) {
      if (m_R->Rand() & 1)
				return m_PT->push_back(new SIToFPInst(V, DestTy, "FC", m_BB->getTerminator()));
				return m_PT->push_back(new UIToFPInst(V, DestTy, "FC", m_BB->getTerminator()));
			
		}

		// Both floats
		if (VTy->getScalarType()->isFloatingPointTy() && DestTy->getScalarType()->isFloatingPointTy()) {
			if (VTy->getScalarType()->getPrimitiveSizeInBits() > DestTy->getScalarType()->getPrimitiveSizeInBits()) {
				return m_PT->push_back(new FPTruncInst(V, DestTy, "Tr", m_BB->getTerminator()));
			} else {
					return m_PT->push_back(new FPExtInst(V, DestTy, "ZE", m_BB->getTerminator()));
			}
		}
	}

};

struct SelectModifier: public Modifier {
	SelectModifier(BasicBlock *BB, PieceTable *PT, Random *R):Modifier(BB, PT, R) {}
	virtual void Act() {
    // Try a bunch of different select configuration until a valid one is found
		for (int i=0; i<100; ++i) {
			Value *Cond = getRandomVal();
			Value *Val0 = getRandomVal();
			Value *Val1 = getTypedValue(Val0->getType());
			if (Cond->getType()->getScalarType() != IntegerType::getInt1Ty(m_BB->getContext())) continue;
			if (Val1->getType()->isVectorTy() != Cond->getType()->isVectorTy()) continue;
			if (Val1->getType()->isVectorTy()) {
        if (!VectorSelect) continue; // Disallow vector select
				if (!Cond->getType()->isVectorTy()) continue;
				if (cast<VectorType>(Val1->getType())->getNumElements() != 
					  cast<VectorType>(Cond->getType())->getNumElements()) continue;
			}

			Value *V = SelectInst::Create(Cond, Val0, Val1, "S", m_BB->getTerminator());
			return m_PT->push_back(V);
		}
	}
};


struct CmpModifier: public Modifier {
	CmpModifier(BasicBlock *BB, PieceTable *PT, Random *R):Modifier(BB, PT, R) {}
	virtual void Act() {
		
		Value *Val0 = getRandomVal();
		Value *Val1 = getTypedValue(Val0->getType());
			
    if (Val0->getType()->isPointerTy()) return;
		bool fp = Val0->getType()->getScalarType()->isFloatingPointTy();

		int op;
		if (fp) {
      op = m_R->Rand() % (CmpInst::LAST_FCMP_PREDICATE - CmpInst::FIRST_FCMP_PREDICATE) +  CmpInst::FIRST_FCMP_PREDICATE;
		} else {
      op = m_R->Rand() % (CmpInst::LAST_ICMP_PREDICATE - CmpInst::FIRST_ICMP_PREDICATE) +  CmpInst::FIRST_ICMP_PREDICATE;
		}
			
		Value *V = CmpInst::Create(fp ? Instruction::FCmp : Instruction::ICmp, op, Val0, Val1, "S", m_BB->getTerminator());
		return m_PT->push_back(V);
	}
};


void FillFunction(Function *F) {
  // Create a legal entry block.
  BasicBlock *BB = BasicBlock::Create(F->getContext(), "BB", F);
  ReturnInst::Create(F->getContext(), BB);

  // Create the value table.
  Modifier::PieceTable PT;
  // Pick an initial seed value
  Random R(SeedCL);
  
  // Consider arguments as legal values.
  for (Function::arg_iterator it = F->arg_begin(), e = F->arg_end(); it != e; ++it) {
	  PT.push_back(it);
  }
  
  // List of modifiers which add new random instructions.
  std::vector<Modifier*> Modifiers;
  std::auto_ptr<Modifier> LM(new LoadModifier(BB, &PT, &R)); 
  std::auto_ptr<Modifier> SM(new StoreModifier(BB, &PT, &R)); 
  std::auto_ptr<Modifier> EE(new ExtractElementModifier(BB, &PT, &R)); 
  std::auto_ptr<Modifier> SHM(new ShuffModifier(BB, &PT, &R)); 
  std::auto_ptr<Modifier> IE(new InsertElementModifier(BB, &PT, &R)); 
  std::auto_ptr<Modifier> BM(new BinModifier(BB, &PT, &R)); 
  std::auto_ptr<Modifier> CM(new CastModifier(BB, &PT, &R)); 
  std::auto_ptr<Modifier> SLM(new SelectModifier(BB, &PT, &R)); 
  std::auto_ptr<Modifier> PM(new CmpModifier(BB, &PT, &R)); 
  Modifiers.push_back(LM.get());
  Modifiers.push_back(SM.get());
  Modifiers.push_back(EE.get());
  Modifiers.push_back(SHM.get());
  Modifiers.push_back(IE.get());
  Modifiers.push_back(BM.get());
  Modifiers.push_back(CM.get());
  Modifiers.push_back(SLM.get());
  Modifiers.push_back(PM.get());
  
  // Generate the random instructions
  AllocaModifier AM(BB, &PT, &R); AM.ActN(5); // Throw in a few allocas
  ConstModifier COM(BB, &PT, &R);  COM.ActN(40); // Throw in a few constants

  for (unsigned i=0; i< SizeCL / Modifiers.size(); ++i)
    for (std::vector<Modifier*>::iterator it = Modifiers.begin(), e = Modifiers.end(); it != e; ++it) {
	    (*it)->Act();
    }  

  SM->ActN(5); // Throw in a few stores.
}

void IntroduceControlFlow(Function *F) {
	std::set<Instruction*> Instrs;
	for (BasicBlock::iterator it = F->begin()->begin(), e = F->begin()->end(); it != e; ++it) {
		if (it->getType() == IntegerType::getInt1Ty(F->getContext())) Instrs.insert(it);
	}

	for (std::set<Instruction*>::iterator it = Instrs.begin(), e = Instrs.end(); it != e; ++it) {
		Instruction *Instr = *it;
		BasicBlock *Curr = Instr->getParent();
		BasicBlock::iterator Loc= Instr;
		BasicBlock *Next = Curr->splitBasicBlock(Loc, "CF");
		Instr->moveBefore(Curr->getTerminator());
		if (Curr != &F->getEntryBlock()) {
			BranchInst::Create(Curr, Next, Instr, Curr->getTerminator());
			Curr->getTerminator()->eraseFromParent();
		}
	}
}


int main(int argc, char **argv) {
  // Init LLVM, call llvm_shutdown() on exit, parse args, etc.
  llvm::PrettyStackTraceProgram X(argc, argv);
  cl::ParseCommandLineOptions(argc, argv, "llvm codegen stress-tester\n");  
  llvm_shutdown_obj Y;  

  Module *M = new Module("/tmp/autogen.bc", getGlobalContext());
  Function *F = GenEmptyFunction(M);
  FillFunction(F);
  IntroduceControlFlow(F);
  verifyModule(*M, PrintMessageAction);
  M->dump();
  delete M;
  return 0;
}
