#include "llvm/Support/CommandLine.h"
#include "llvm/PassManager.h"
#include "VecHeuristics.h"
#include "WIAnalysis.h"


namespace intel {

static cl::opt<bool>
PrintVecHue("print-vec-hue", cl::init(false), cl::Hidden,
  cl::desc("Debug VecHeuristics analysis"));

static cl::opt<bool>
VecHueHasAVX("vec-hue-hasavx", cl::init(false), cl::Hidden,
  cl::desc("VecHeuristics may assume it has avx"));

static cl::opt<bool>
VecHueHasAVX2("vec-hue-hasavx2", cl::init(false), cl::Hidden,
  cl::desc("VecHeuristics may assume it has avx2"));

static cl::opt<bool>
VecHueIsMIC("vec-hue-ismic", cl::init(false), cl::Hidden,
  cl::desc("VecHeuristics assumes it is compiling for MIC architechture"));

static const bool enableDebugPrints = false;
static raw_ostream &dbgPrint() {
    static raw_null_ostream devNull;
    return enableDebugPrints ? errs() : devNull;
}

bool VectorizationHeuristics::runOnFunction(Function &F) {  
  dbgPrint()<<"Considering "<<F.getNameStr()<<":";

  assert(m_preferedVectorSize == 0 && "Make sure that this is the first run");
  if (PrintVecHue){
    if ( VecHueHasAVX )
      m_featureSupport |= Intel::CFS_AVX1;
    if ( VecHueHasAVX2 )
      m_featureSupport |= Intel::CFS_AVX2;
    if ( VecHueIsMIC )
      m_cpuid = Intel::MIC_KNIGHTSFERRY;
  }
  /*
   *  Legality checks
   */
  m_mayVectorize = false;
  m_preferedVectorSize = 0;
  // must not have variable access  
  if (hasVariableGetTIDAccess(F)) {dbgPrint()<<" Illegal TID\n"; }
  // CF must be reducible
  else if (!isReducibleControlFlow(F)) { dbgPrint()<<" Illegal IRCF\n"; }
  // Do not vectorize if the incoming program has types which are not supported by the codegen
  else if (hasIllegalTypes(F)) { dbgPrint()<<" Illegal types\n";  }
  else m_mayVectorize = true;
  if (!m_mayVectorize) {
    return false;
  }
  if (isMic()){
    m_preferedVectorSize = 16;
    if(PrintVecHue) {
      outs() << F.getNameStr() << " Should vectorize to " <<
      m_preferedVectorSize << "\n";
    }
    return true;
  }

  /*
   *  Profitability of vectorization checks
   */

  // We must run WIAnalysis only after we know it is safe to vectorize
  FunctionPassManager FPM(F.getParent());
  m_WIAnalysisPass = new WIAnalysis();
  FPM.add(m_WIAnalysisPass);
  FPM.run(F);


  if (isVectorHeavy(F)) { dbgPrint()<<"Too isVectorHeavy\n"; }
  else if (isPHIHeavy(F)) { dbgPrint()<<"Too isPHIHeavy\n"; }
  else if (maxLoopNest(F) > 3) { dbgPrint()<<"Too maxLoopNest"<<maxLoopNest(F)<<"\n"; }
  else if (isHeavyScatterGather(F)) { dbgPrint()<<"Too isHeavyScatterGather\n"; }
  else if (hasDifficultTypes(F)) { dbgPrint()<<"Too hasDifficultTypes\n"; }
  else if (isCFHeavy(F)) { dbgPrint()<<"Too isCFHeavy\n"; }
  else {
    /*
     *  Vector size checks
     */
    m_preferedVectorSize = 4;  
    if (hasAVX2())
	  	m_preferedVectorSize = 8;

    else if (hasAVX() && !isIntegerHeavy(F))
      m_preferedVectorSize = 8;  

    dbgPrint()<<F.getNameStr()<<" Should vectorize to " << m_preferedVectorSize << "\n";


    // Debug print for tests
    if(PrintVecHue) {
      outs()<<"Function ["<<F.getName()<<"] vec size:"<<m_preferedVectorSize<<"\n";
    }
  }
  // To be safe, ensure we get an exception if we access this member not through here.
  m_WIAnalysisPass = 0;
  return false;
}


bool VectorizationHeuristics::hasVariableGetTIDAccess(Function &F) {
  RuntimeServices* services = RuntimeServices::get();
  assert(services && "Unable to get runtime services");
  for (Function::iterator bbit = F.begin(), bbe=F.end(); bbit != bbe; ++bbit) {
    for (BasicBlock::iterator it = bbit->begin(), e=bbit->end(); it!=e;++it) {
      if (CallInst* call = dyn_cast<CallInst>(it)) {
        bool err = false;
        unsigned dim = 0;
        services->isTIDGenerator(call, &err, &dim);
        // We are unable to vectorize this code because get_global_id is messed up
        if (err) return true;
      }
    }
  }
  // TID access is okay
  return false;
}

bool VectorizationHeuristics::masksNeeded(Function &F) {
  return !m_WIAnalysisPass->isControlFlowUniform(&F);
}

unsigned VectorizationHeuristics::maxLoopNest(Function &F) {
  LoopInfo *LI = &getAnalysis<LoopInfo>();
  unsigned MaxNest = 0;
  // For each basic blocks
  for (Function::iterator it = F.begin(), e = F.end(); it != e; ++it) {
    if (LI->getLoopFor(it)) {
      MaxNest = std::max(MaxNest, LI->getLoopDepth(it));
    }
  }

  return MaxNest;
}

bool VectorizationHeuristics::isIntegerHeavy(Function &F) {
  LoopInfo *LI = &getAnalysis<LoopInfo>();
  unsigned MaxNest = maxLoopNest(F);

  unsigned Integers = 0;
  unsigned Floats = 0;

  // for each BB
  for (Function::iterator it = F.begin(), e = F.end(); it != e; ++it) {
    // Only look at inner most loops
    if ((LI->getLoopDepth(it) != MaxNest)) continue;
    
    // for each instruction
    for (BasicBlock::iterator ii = it->begin(), fi = it->end(); ii != fi; ++ii) {
      // Only consider arithmetic operations and function calls
      if (!dyn_cast<BinaryOperator>(ii) && 
        !dyn_cast<CallInst>(ii)) continue;
      if (ii->getOpcode() == Instruction::And) {Floats++; continue;}
      if (ii->getOpcode() == Instruction::Or) { Floats++; continue;}
      if (ii->getOpcode() == Instruction::Xor) { Floats++; continue;}
      
      const Type *Tp = ii->getType();
      if (Tp->isVectorTy()) {
        Tp = cast<VectorType>(Tp)->getElementType();
      }

      if (Tp->isIntegerTy()) Integers++;
      if (Tp->isFloatingPointTy()) Floats++;

    }
  }

  dbgPrint()<<"[I"<<Integers<<":F"<<Floats<<"]";
  return 3 * Integers >= Floats;
}

bool VectorizationHeuristics::isVectorHeavy(Function &F) {
  unsigned Scalars = 0;
  unsigned Vectors = 0;

  // for each BB
  for (Function::iterator it = F.begin(), e = F.end(); it != e; ++it) {
    // for each instruction
    for (BasicBlock::iterator ii = it->begin(), fi = it->end(); ii != fi; ++ii) {
      const Type* Tp = ii->getType();
      if (Tp->isIntegerTy() || Tp->isFloatingPointTy()) Scalars++;
      if (Tp->isVectorTy()) Vectors++;
    }
  }

  unsigned Total = Vectors + Scalars;
  // Vector heavy if more than 30% of the workload is of vector type;
  dbgPrint()<<"[V"<<Vectors<<":S"<<Scalars<<"]";
  return Vectors > 0.4 * Total; 
}


bool VectorizationHeuristics::isPHIHeavy(Function &F) {
  LoopInfo *LI = &getAnalysis<LoopInfo>();
  unsigned MaxNest = maxLoopNest(F);
  unsigned PHI = 0;
  unsigned NonPHI = 0;

  // for each BB
  for (Function::iterator it = F.begin(), e = F.end(); it != e; ++it) {
    if ((LI->getLoopDepth(it) != MaxNest)) continue;
    // for each instruction
    for (BasicBlock::iterator ii = it->begin(), fi = it->end(); ii != fi; ++ii) {
      if(dyn_cast<PHINode>(ii)) {
        PHI++;
      } else {
        NonPHI++;
      }
    }
  }

  unsigned Total = PHI + NonPHI;
  // If the program is more than 30% PHIs, do not vectorize
  dbgPrint()<<"[PHI"<<PHI<<":S"<<NonPHI<<"]";
  return (PHI > 0.3 * Total) && (PHI > 8); 
}

bool VectorizationHeuristics::hasLoopBarriers(Function &F) {
  LoopInfo *LI = &getAnalysis<LoopInfo>();
  unsigned MaxNest = maxLoopNest(F);

  RuntimeServices *RTS = RuntimeServices::get();
  // for each BB
  for (Function::iterator it = F.begin(), e = F.end(); it != e; ++it) {
    // for each instruction
    for (BasicBlock::iterator ii = it->begin(), fi = it->end(); ii != fi; ++ii) {
      if (CallInst* CI = dyn_cast<CallInst>(ii)) {
        // if we found a barrier
        if (RTS->isSyncFunc(CI->getCalledFunction()->getNameStr())) {
          m_hasBarrier = true;
          if ((LI->getLoopDepth(it) == MaxNest) && MaxNest) return true;
        }
      }
    }
  }
  return false;
}

// Reference:
//  Book: High Performance Compilers for Parallel Computing / Michael Wolfe
//  Page 60, Section 3.2.4 - Finding Cycles in Directed Graphs
//
// Check if the graph is irreducible using the standard algorithm:
// 1. If you ignore backedges, graph is acyclic.
// 2. backedges are edges where the target dominates the src.
bool VectorizationHeuristics::isReducibleControlFlow(Function& F) {
  DominatorTree* DT = &getAnalysis<DominatorTree>();

  llvm::SmallSet<BasicBlock*, 16> removed;
  llvm::SmallSet<BasicBlock*, 16> toProcess;
  toProcess.insert(&F.getEntryBlock());

  while (!toProcess.empty()) {
    BasicBlock* next = NULL;
    // Find a free node
    llvm::SmallSet<BasicBlock*, 16>::iterator bb(toProcess.begin());
    llvm::SmallSet<BasicBlock*, 16>::iterator bbe(toProcess.end());
    // for each of the toProcess blocks, find a block who's
    // preds are all removed
    for (; bb != bbe ; ++bb) {
      // Are all of its preds removed ?
      bool Removed = true;
      llvm::pred_iterator p = pred_begin(*bb), pe = pred_end(*bb);

      // for each pred
      for (; p != pe; ++p) {
        // This is a back-edge, ignore it
        // Note: nodes dominate themselves.
        // This is good because this is how we handle self-edges.
        if (DT->dominates(*bb, *p)) continue;
        // pred in removed list ?
        Removed &= removed.count(*p);
      }

      // Found a candidate to remove
      if (Removed) {
        next = *bb;
        break;
      }
    }

    // Did not find a free node to remove,
    // The graph has a cycle. This code is irreducible.
    if (!next) {
      return false;
    }

    // Remove this node
    toProcess.erase(next);
    removed.insert(next);
    // Insert all successors to the 'todo' queue
    llvm::succ_iterator s = succ_begin(next), se = succ_end(next);
    for (; s != se; ++s) {
      // did not visit this before
      if (!removed.count(*s)) toProcess.insert(*s);
    }
  }

  // Code is reducible
  return true;
}

bool VectorizationHeuristics::hasIllegalTypes(Function &F) {
  // For each BB
  for (Function::iterator bbit = F.begin(), bbe=F.end(); bbit != bbe; ++bbit) {
    // For each instruction
    for (BasicBlock::iterator it = bbit->begin(), e=bbit->end(); it!=e;++it) {
      const Type* tp = it->getType();
      // strip vector types
      if (const VectorType* VT = dyn_cast<VectorType>(tp)) {
        tp = VT->getElementType();
      }
      // check that integer types are legal
      if (const IntegerType* IT = dyn_cast<IntegerType>(tp)) {
        unsigned BW = IT->getBitWidth();
        if (BW > 64) return true;
      }
    }
  }
  return false;
}

class CalleeRating {
    // A pair of built-in name and its rating
    typedef SmallVector< std::pair<const char*, unsigned>,  8> CalleeLookup;
     CalleeLookup Lookup;
public:
    CalleeRating() {
        //TODO: expand list to contain all known built-ins
        Lookup.push_back(std::make_pair("fabs", 1));
        Lookup.push_back(std::make_pair("min", 1));
        Lookup.push_back(std::make_pair("max", 1));
        Lookup.push_back(std::make_pair("mad", 2));
    }

    // Returns rating of callee. High means we gain more by vectorizing
    // Returns zero if there is no information about this callee.
    unsigned findRating(const CallInst* call) const {
      StringRef functionName = call->getCalledFunction()->getName();
      for (CalleeLookup::const_iterator i = Lookup.begin(), e = Lookup.end();
          i != e; ++i) {
        if (functionName.find(i->first) != StringRef::npos) return i->second;
      }
      return 0;
    }
};

bool VectorizationHeuristics::hasDifficultTypes(Function &F) {
  LoopInfo *LI = &getAnalysis<LoopInfo>();
  unsigned MaxNest = maxLoopNest(F);

  unsigned Easy = 0;
  unsigned Hard = 0;

  // for each BB
  for (Function::iterator it = F.begin(), e = F.end(); it != e; ++it) {
    // Only look at inner most loops
    if ((LI->getLoopDepth(it) != MaxNest)) continue;
    
    // for each instruction
    for (BasicBlock::iterator ii = it->begin(), fi = it->end(); ii != fi; ++ii) {
      // Only consider arithmetic operations
      const Type *Tp =  ii->getType();

      // Code generator does not handle these compares well
      if (CmpInst* Cmp = dyn_cast<CmpInst>(ii)){
        Tp = Cmp->getOperand(1)->getType();
        if (const Type *VT = Tp) {
            unsigned ScalarSize = VT->getScalarSizeInBits();
            if (ScalarSize != 32) {Hard+=6; continue; }
        }
      }

      if (StoreInst* ST = dyn_cast<StoreInst>(ii)){
        Tp = ST->getValueOperand()->getType();
      }

      if (Tp->isVectorTy()) {
        Tp = cast<VectorType>(Tp)->getElementType();
        if (Tp->getScalarSizeInBits() != 32) { Hard++; continue; }
      }

      // Some cases of calls can be especially easier
      if (CallInst* call = dyn_cast<CallInst>(ii)) {
        if (call->getType()->isFPOrFPVectorTy()) {          
          static CalleeRating CR;
          unsigned rating = CR.findRating(call);
          if (0 == rating) rating = 20;

          //call->print(dbgPrint());
          //dbgPrint() << " rating=" << rating << '\n';

          Easy+=rating; continue; 
        }
      }

      Easy++;
    }
  }

  dbgPrint()<<"[E"<<Easy<<":H"<<Hard<<"]";
  return 8 * Hard > Easy;
}


bool VectorizationHeuristics::isGEPHeavy(Function &F) {
  LoopInfo *LI = &getAnalysis<LoopInfo>();
  unsigned MaxNest = maxLoopNest(F);

  unsigned GEP = 0;
  unsigned Bin = 0;

  // for each BB
  for (Function::iterator it = F.begin(), e = F.end(); it != e; ++it) {
    // Only look at inner most loops
    if ((LI->getLoopDepth(it) != MaxNest)) continue;
    
    // for each instruction
    for (BasicBlock::iterator ii = it->begin(), fi = it->end(); ii != fi; ++ii) {
      if (dyn_cast<BinaryOperator>(ii)) Bin++;
      if (dyn_cast<GetElementPtrInst>(ii)) GEP++;
    }
  }

  dbgPrint()<<"[B"<<Bin<<":G"<<GEP<<"]";
  return 4 * GEP > Bin;
}

bool VectorizationHeuristics::isMic()const{
  return Intel::MIC_KNIGHTSFERRY == m_cpuid;
}

bool VectorizationHeuristics::hasAVX()const{
  return 0 != (m_featureSupport & Intel::CFS_AVX1);
}
bool VectorizationHeuristics::hasAVX2()const{
  return 0 != (m_featureSupport & Intel::CFS_AVX2);
}

bool VectorizationHeuristics::isCD(const BasicBlock* x, const BasicBlock *y) {
  DominatorTree *DT = &getAnalysis<DominatorTree>();

  bool yesDom = false;
  bool noDom = false;
  // For each successor
  succ_const_iterator succ = succ_begin(x);
  succ_const_iterator succ_e = succ_end(x);
  for (; succ != succ_e ; ++succ) {
    if (DT->dominates(*succ, y)) {
      yesDom = true;
    } else {
      noDom = true;
    }
  }

  // 1. Y post-dominates some successor of X
  // 2. Y does not post-dominate all successors of X
  return (yesDom && noDom);
}

void VectorizationHeuristics::getMaxNestLevel(ControlList& control, 
                            std::map<BasicBlock*, unsigned>& MaxNest) {

  std::set<BasicBlock*> Worklist;

  std::set<BasicBlock*> Blocks;
  // Zero depth map
  for (ControlList::iterator it = control.begin(),
                              e = control.end(); it != e; ++it) {
    Blocks.insert(it->first);
    Blocks.insert(it->second);
    MaxNest[it->first] = 1;
    MaxNest[it->second] = 1;
  }

  Worklist.insert(Blocks.begin(), Blocks.end());

  while (Worklist.size()) {
    BasicBlock *BB = *Worklist.begin();
    Worklist.erase(BB);
    
    for (ControlList::iterator it = control.begin(),
                              e = control.end(); it != e; ++it) {
      if (BB != it->first) continue;
      Worklist.insert(it->second);
      MaxNest[it->second] = std::max(MaxNest[it->second], 1+ MaxNest[it->first]);
    }
  }

}

struct BBInfo { int depth; unsigned icount; };
// Maps a BB to a pair of: control flow depth, icount
typedef std::map<const BasicBlock*, BBInfo > Lookup;

bool VectorizationHeuristics::isCFHeavy(Function &F) {
  LoopInfo *LI = &getAnalysis<LoopInfo>();  
  unsigned MaxNest = maxLoopNest(F);
  // Function must have a loop to be interesting
  if (!MaxNest) return false;
  
  Lookup Dep;
  static CalleeRating CR;

  for (Function::const_iterator X = F.begin(), Xe = F.end(); X != Xe; ++X) {
    // only consider highest loop nest
    if (LI->getLoopDepth(X) != MaxNest) continue;

    for (Function::const_iterator Y = F.begin(), Ye = F.end(); Y != Ye; ++Y) {
      if (X == Y) continue;
      if (LI->getLoopDepth(Y) != MaxNest) continue;
      if (!isCD(Y,X)) continue;
      if (m_WIAnalysisPass->whichDepend(Y->getTerminator()) == WIAnalysis::UNIFORM) continue;
    
      Lookup::iterator i = Dep.find(X);
      if (i == Dep.end()) {
        BBInfo info;
        info.depth = 1;
        info.icount = X->size();
        for (BasicBlock::const_iterator ii = X->begin(), ie = X->end(); ii != ie; ++ii) {
          // Some cases of calls get a greater weight
          if (const CallInst* call = dyn_cast<CallInst>(ii)) {
            if (call->getType()->isFPOrFPVectorTy()) {
              unsigned rating = CR.findRating(call);
              if (0 == rating) rating = 20;
              info.icount+=rating; continue;
            }
          }
        }

        Dep[X] = info;
      } else
        i->second.depth++;
    }
  }

  if (Dep.empty()) return false;

  float weighted = 0.f;
  unsigned icount = 0;
  for (Lookup::const_iterator i = Dep.begin(), e = Dep.end(); i != e; ++i) {
    weighted += i->second.icount/pow(2.f, i->second.depth);
    icount += i->second.icount;
  }
  if (icount == 0) return false;

  weighted/=icount;
  dbgPrint() << "[CFLN:" << weighted << "]";
    
  if (weighted < 0.4f) return true;

  return false;
}

bool VectorizationHeuristics::isMasked(BasicBlock* BB) {
  // For each block in BB
  Function *F =  BB->getParent();
  for (Function::iterator Y = F->begin(), Ye = F->end(); Y != Ye; ++Y) {
      if (BB!=Y && isCD(Y, BB) && // Y controls BB
          (m_WIAnalysisPass->whichDepend(Y->getTerminator()) != WIAnalysis::UNIFORM)) {
              StringRef YName = Y->getName();
              StringRef BBName = BB->getName();
              if (YName.empty()) YName = "Noname";
              if (BBName.empty()) BBName = "Noname";
              //dbgPrint()<< BBName <<" is masked due to "<< YName << "\n";
              return true;
      }
  }
  return false;
}

bool VectorizationHeuristics::isHeavyScatterGather(Function &F) {
  unsigned MaxNest = maxLoopNest(F);
  LoopInfo *LI = &getAnalysis<LoopInfo>();
  
 
  unsigned illegal = 0;
  unsigned binops = 0;
  unsigned scatter = 0;
  unsigned direct = 0;
  unsigned transpose = 0;
  unsigned barriers = 0;
  unsigned calls = 0;

  // for each BB
  for (Function::iterator it = F.begin(), e = F.end(); it != e; ++it) {
    bool MaskedBlock = isMasked(it);
    // only consider highest loop nest
    if (LI->getLoopDepth(it) == MaxNest)
    // for call each instruction
    for (BasicBlock::iterator ii = it->begin(), fi = it->end(); ii != fi; ++ii) {

      if (ExtractElementInst *EEI = dyn_cast<ExtractElementInst>(ii)) {
          if (MaskedBlock || m_WIAnalysisPass->whichDepend(EEI->getOperand(0)) != WIAnalysis::UNIFORM) {
          // Gather :)
          const VectorType *VT = cast<VectorType>(EEI->getOperand(0)->getType());
          unsigned NumElt = VT->getNumElements();
          if (NumElt> 1)
            transpose++;
        }
      }

      if (InsertElementInst *IEI = dyn_cast<InsertElementInst>(ii)) {
        if (m_WIAnalysisPass->whichDepend(IEI->getOperand(0)) != WIAnalysis::UNIFORM || 
            m_WIAnalysisPass->whichDepend(IEI->getOperand(1)) != WIAnalysis::UNIFORM || 
            MaskedBlock) {
          // Scatter :(
          const VectorType *VT = cast<VectorType>(IEI->getOperand(0)->getType());
          unsigned NumElt = VT->getNumElements();
          if (NumElt> 1)
            transpose++;

        }
      }

      if (StoreInst* SI = dyn_cast<StoreInst>(ii)) {
        if (m_WIAnalysisPass->whichDepend(SI->getPointerOperand()) > WIAnalysis::PTR_CONSECUTIVE) {
          scatter += 1;
        } else {
          direct += 1;
        }
      }
      if (LoadInst* LI = dyn_cast<LoadInst>(ii)) {
        if (m_WIAnalysisPass->whichDepend(LI->getPointerOperand()) > WIAnalysis::PTR_CONSECUTIVE) {
          scatter += 1;
        } else {
          direct += 1;
        }
      }
      if (CallInst *CI = dyn_cast<CallInst>(ii)) {
        calls+=1;
        if (CI->getCalledFunction()->getNameStr().find("barrier") != std::string::npos) {
          barriers += 1;
        }
      }
      if (BinaryOperator* BI = dyn_cast<BinaryOperator>(ii)) {
        unsigned size_in_bits = BI->getType()->getScalarSizeInBits();
        if (size_in_bits > 1 && (size_in_bits < 32 || size_in_bits > 64))
          if (m_WIAnalysisPass->whichDepend(BI) > WIAnalysis::PTR_CONSECUTIVE)
            illegal++;        

        if (m_WIAnalysisPass->whichDepend(BI) != WIAnalysis::UNIFORM) {
            binops++;

        }
      }
    }
  }
  dbgPrint()<<"[SCTR"<<scatter<<" Tran"<<transpose<<" Call"<<calls<<" BIN"<<binops<<" DIR"<<direct<<"]";
                              // great memory load/transpose overhead
  return ((((3*scatter + transpose*2)) > (calls*4 + binops + direct)) && (scatter + transpose > 4)) 
    || (transpose > 50) || (scatter > 100);
}


// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  FunctionPass* createVectorizationHeuristics() {
    return new intel::VectorizationHeuristics();
  }
}

char VectorizationHeuristics::ID = 0;
INITIALIZE_PASS(VectorizationHeuristics, "vecHeuristics",
                "Vectorization Heuristics Analysis", false, true);

} // namespace intel

