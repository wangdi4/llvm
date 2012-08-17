#include "llvm/PassManager.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/DominanceFrontier.h"
#include "InstCounter.h"
#include "WIAnalysis.h"
#include "Mangler.h"
#include "LoopUtils.h"
#include "OpenclRuntime.h"

namespace intel {

const float WeightedInstCounter::RATIO_MULTIPLIER = 1;
const float WeightedInstCounter::ALL_ZERO_LOOP_PENALTY = 0;
const float WeightedInstCounter::TID_EQUALITY_PENALTY = 0.1f;

// Costs for transpose functions
WeightedInstCounter::FuncCostEntry WeightedInstCounter::CostDB[] = {
   { "load_transpose_float4x8", 60 },
   { "transpose_store_float4x8", 60 },
   { "load_transpose_char4x4", 10 },
   { "transpose_store_char4x4", 10 },

   // The line below must be the last line in the DB,
   // serving as a terminator.
   { 0, 0 }
};

static const bool enableDebugPrints = false;
static raw_ostream &dbgPrint() {
    static raw_null_ostream devNull;
    return enableDebugPrints ? errs() : devNull;
}

bool WeightedInstCounter::isMic() const {
  return m_cpuid.IsMIC();
}

bool WeightedInstCounter::hasAVX() const {
  return m_cpuid.HasAVX1();
}

bool WeightedInstCounter::hasAVX2() const {
  return m_cpuid.HasAVX2();
}

     
WeightedInstCounter::WeightedInstCounter(bool preVec = true, bool sanityOnly = false, 
                              Intel::CPUId cpuId = Intel::CPUId()): 
                              FunctionPass(ID), m_cpuid(cpuId), m_preVec(preVec),  
                              m_sanityOnly(sanityOnly), 
                              m_desiredWidth(1), m_totalWeight(0) {        
  initializeLoopInfoPass(*PassRegistry::getPassRegistry());
  initializeDominatorTreePass(*PassRegistry::getPassRegistry());
  initializePostDominatorTreePass(*PassRegistry::getPassRegistry());
  initializePostDominanceFrontierPass(*PassRegistry::getPassRegistry());

  int i = 0;
  while (CostDB[i].name) {
    const FuncCostEntry* e = &CostDB[i];       
    m_transCosts[e->name] = e->cost;
    i++;
  }
}

bool WeightedInstCounter::runOnFunction(Function &F) {  

  //This is for safety - don't return 0.
  m_totalWeight = 1;

  // If the request was only to check sanity, only set the desired
  // with for MIC to 16, and finish.
  // This used to also contain a "are we allowed to vectorize" check
  // but that was moved elsewhere.
  if (m_sanityOnly)
  {
    if (isMic())
      m_desiredWidth = 16;
    else
      m_desiredWidth = 1;
    return false;
  }

  // Check if this is the "pre" stage. 
  // If it is, compute things that are relevant only here.
  DenseMap<Instruction*, int> MemOpCostMap;
  if (m_preVec) {
    // MIC always has vectorization width 16.
    if (isMic()) {
      m_desiredWidth = 16;
      return false;
    }

    // Compute the "cost map" for stores and loads. This is only
    // done pre-vectorization. See function for extended explanation.
    estimateMemOpCosts(F, MemOpCostMap); 
  }
  else if (isMic()) {
    //Do nothing for MIC in the post stage.
    return false;
  } 

  // First, estimate the total number of iterations each loop in the
  // functions runs. Once we have this, we can multiply the count
  // by each instruction's weight.
  DenseMap<Loop*, int> IterMap;
  estimateIterations(F, IterMap);
  
  // Now compute some estimation of the probability of each basic block
  // being executed in a run.
  DenseMap<BasicBlock*, float> ProbMap;
  estimateProbability(F, ProbMap);

  // Ok, start counting with 0
  m_totalWeight = 0;

  LoopInfo *LI = &getAnalysis<LoopInfo>();
  // For each basic block, add up its weight
  for (Function::iterator BB = F.begin(), BBE = F.end(); BB != BBE; ++BB) {    
    // Check if the basic block in a loop. If it is, we want to multiply
    // all of its instruction's weights by its tripcount. 
    int TripCount = 1;
    if (Loop* ContainingLoop = LI->getLoopFor(BB))
      TripCount = IterMap.lookup(ContainingLoop);

    float Probability = ProbMap.lookup(BB);

    // And now, sum up all the instructions
    for (BasicBlock::iterator I = BB->begin(), IE=BB->end(); I != IE; ++I) 
      m_totalWeight += Probability * TripCount * 
                       getInstructionWeight(I, MemOpCostMap);
  }

  // Decide what the vectorization width should be.
  // MIC was already decided earlier. The reason the code is split
  // is that for the MIC we don't need to compute the various maps,
  // while in this part of the code we want to use them.
  // SSE is always 4. For AVX, estimate the most used type in the kernel.
  // Integers have no 8-wide operations on AVX1, so vectorize to 4,
  // otherwise, 8.
  // This logic was inherited from the old heuristic, but the types 
  // are computed slightly more rationally now.

  // Not that the below code is actually nonsense.
  // By the above logic, we should be:
  // a) always vectorizing doubles code to 4.
  // b) vectorizing i32 code (and below) to 8.
  // What's below works well in practice, but for bad reasons. 
  // The right thing to do would be to change (a) and (b), and then
  // take memory access patterns into account. (a lot of "cheap" loads
  // means 4, "expensive" loads mean 8).
  // However, in practice, this works better. It would be a good idea
  // to understand why.

  if (m_preVec) {
    if (hasAVX()) {
      Type* DominantType = estimateDominantType(F, IterMap, ProbMap);
      if (DominantType->isIntegerTy())
        m_desiredWidth = 4;
      else
        m_desiredWidth = 8;         
    }
    else
      m_desiredWidth = 4;
  }
  return false;
}

// This allows a consistent comparison between scalar and vector types.
// Under normal conditions, a pointer comparison always occures, which
// is consistent for a single run, but not between runs.
struct TypeComp {
  bool operator() (Type* Left, Type* Right) const {    
    if (Left->getNumElements() != Right->getNumElements())
      return (Left->getNumElements() < Right->getNumElements());

    if (Left->getScalarSizeInBits() != Right->getScalarSizeInBits())
      return (Left->getScalarSizeInBits() < Right->getScalarSizeInBits());
  
    Type* ScalarLeft = Left->getScalarType();
    Type* ScalarRight = Right->getScalarType();

    if (ScalarLeft->isIntegerTy() && !ScalarRight->isIntegerTy())
      return true;
    
    if (!ScalarLeft->isIntegerTy() && ScalarRight->isIntegerTy())
      return false;

    //Fallback to a pointer comparison for other types.
    return Left < Right;
  }
};

Type* WeightedInstCounter::estimateDominantType(Function &F, DenseMap<Loop*, int> &IterMap,
                                           DenseMap<BasicBlock*, float> &ProbMap) const
{
  DenseMap<Type*, float> countMap;

  // For each type, count how many times it is the first operand of a binop.
  LoopInfo *LI = &getAnalysis<LoopInfo>();
  for (Function::iterator BB = F.begin(), BBE = F.end(); BB != BBE; ++BB) {    
    int TripCount = 1;
    if (Loop* ContainingLoop = LI->getLoopFor(BB))
      TripCount = IterMap.lookup(ContainingLoop);

    float Probability = ProbMap.lookup(BB);
    for (BasicBlock::iterator I = BB->begin(), IE=BB->end(); I != IE; ++I) {
      // We only care about BinOps 
      if (BinaryOperator* BinOp = dyn_cast<BinaryOperator>(I)) {
        Type* OpType = BinOp->getOperand(0)->getType();
        // Get the base type for vectors
        OpType = OpType->getScalarType();
        countMap[OpType] += TripCount * Probability;
      }
    }
  }

  // Find the maximum. The map is expected to be small, just iterate over it.
  // Use i32 by default... if there are no binops at all. Should be pretty rare.
  // In case there are several values with equal maximum keys, use a comparator
  // to choose the maximum.
  // This is needed because otherwise the choice depends on the order of iteration
  // which, in turn, depends on the values of the Type pointers, which change
  // from run to run. This makes the choice inconsistent between runs.
  Type* DominantType = Type::getInt32Ty(F.getContext());
  int MaxCount = 0;
  for (DenseMap<Type*, float>::iterator I = countMap.begin(), 
       E = countMap.end(); I != E; ++I) {
    if ((I->second > MaxCount) || 
        (I->second == MaxCount && TypeComp()(I->first, DominantType))) {
      MaxCount = I->second;
      DominantType = I->first;
    }
  }
  return DominantType;
}

int WeightedInstCounter::estimateCall(CallInst *Call)
{  
    // TID generators are extremely common and very cheap.
    RuntimeServices* services = RuntimeServices::get();
    bool err = false;
    unsigned dim = 0;
    if(services->isTIDGenerator(Call, &err, &dim))
      return DEFAULT_WEIGHT;

    // Handle the special case of unnamed functions (call through a bitcast)
    if (!Call->getCalledFunction())
      return CALL_WEIGHT;

    StringRef Name = Call->getCalledFunction()->getName();    
    // Since we run before the resolver, masked load/stores should count
    // as load/stores, not calls. Maybe slightly better or worse.
    if (Mangler::isMangledLoad(Name) || Mangler::isMangledStore(Name)) {
      // If the mask is non-scalar, this will become a lot of memops,
      // since the CPU doesn't have gathers.
      Value *Mask = Call->getArgOperand(0);
      Type* MaskType = Mask->getType();
      // For scalar masks, it'll be a pretty little vector store/load
      if (!MaskType->isVectorTy())
        return MEM_OP_WEIGHT;

       // This is a vector type, it'll be ugly.
      int NumElements = MaskType->getNumElements();
      return MEM_OP_WEIGHT * NumElements;
      // TODO: if the vector is really large, still need to multiply...
    }

    // vloads and vstores also count as loads/stores.
    if (Name.startswith("vload") || Name.startswith("_Z6vload") ||
        Name.startswith("vstore") || Name.startswith("_Z7vstore")) {
      return MEM_OP_WEIGHT;
    }


    // Ugly hacks start here.
    if (Name.startswith("_Z5clamp") || Name.startswith("clamp"))
      return CALL_CLAMP_WEIGHT;

    if (Name.startswith("_Z5floor") || Name.startswith("floor"))
      return CALL_FLOOR_WEIGHT;

    // allZero and allOne calls are cheap, it's basically a xor/ptest
    if ((Name.startswith(Mangler::name_allZero)) || 
        (Name.startswith(Mangler::name_allOne)))
      return DEFAULT_WEIGHT;

    // See if we can find the function in the function cost table
    return getFuncCost(Name);
}

int WeightedInstCounter::getFuncCost(const std::string& name) 
{
  if (m_transCosts.find(name) != m_transCosts.end()) {
    return m_transCosts[name];
  } else {
    // Function is not in the table, return default call weight.
    return CALL_WEIGHT;
  }    
}

int WeightedInstCounter::estimateBinOp(BinaryOperator *I)
{
  int Weight = BINARY_OP_WEIGHT;
  Type* OpType = I->getOperand(0)->getType();

  // If it's a scalar op, return the base weight.
  if (!OpType->isVectorTy())
    return Weight;
  
  VectorType* VecType = cast<VectorType>(OpType); 
  int OpWidth = 0;
  //SSE
  if (!hasAVX())
    OpWidth = getOpWidth(VecType, 4, 2, 2, 4);

  // OK, we have AVX. Do we have AVX2?
  if (hasAVX2())
    OpWidth = getOpWidth(VecType, 8, 4, 4, 8);

  // Only AVX, 4-wide on ints, 2-wide on i64
  OpWidth = getOpWidth(VecType, 8, 4, 2, 4);

  return Weight * OpWidth;
}

int WeightedInstCounter::getOpWidth(VectorType* VecType, int Float, 
                                     int Double, int LongInt, int ShortInt)
{
  Type* BaseType = VecType->getScalarType();
  int BaseWidth = VecType->getScalarSizeInBits();
  int ElemCount = VecType->getNumElements();

  //ceil_div(x,y) is ceil(x/y) for positive integers.
  if (BaseType->isFloatingPointTy())
  {
    if (BaseType->isFloatTy())
      return ceil_div(ElemCount, Float);
    else
      return ceil_div(ElemCount, Double);
  }
  else if (BaseWidth > 32)
    return ceil_div(ElemCount, LongInt);
  else
    return ceil_div(ElemCount, ShortInt);
}

int WeightedInstCounter::getInstructionWeight(Instruction *I, DenseMap<Instruction*, int> &MemOpCostMap)
{
  // We could replace this by a switch on the opcode, but that introduces
  // a bit too many cases. So using this method (also used in WIAnalysis).
  // TODO: A lot of cases have been introduced as is, so, perhaps replace.
  if (BinaryOperator* BinOp = dyn_cast<BinaryOperator>(I)) 
    return estimateBinOp(BinOp);
  
  if (CallInst* called = dyn_cast<CallInst>(I))
    return estimateCall(called);
    
  // GEP and PHI nodes are free
  if (isa<GetElementPtrInst>(I) || isa<PHINode>(I) || isa<AllocaInst>(I))
    return 0;

  // Shuffles/extracts/inserts are mostly representative
  // of transposes.
  if (isa<ShuffleVectorInst>(I)) {
    // Shuffling from 4xi32, 8xi32, 4xfloat and 8xfloat is cheap,
    // everything else is expensive.
    // (This is purely empirical, probably overfitting)
    Value* Vec = I->getOperand(0);
    VectorType* OpType = dyn_cast<VectorType>(Vec->getType());
    VectorType* ResType = dyn_cast<VectorType>(I->getType());
    assert(OpType && "Shuffle with a non-vector type!");

    // A shuffle between different types can't be cheap, even if
    // the types are sensible. This should, amongst other things,
    // make revectorization from 4 to 8 less appealing.
    if (ResType != OpType)
      return EXPENSIVE_SHUFFLE_WEIGHT;

    if (((OpType->getNumElements() == 4) || 
         (OpType->getNumElements() == 8)) 
       &&
        ((OpType->getElementType()->isFloatTy()) || 
          OpType->getElementType()->isIntegerTy(32)))
      return CHEAP_SHUFFLE_WEIGHT;

    return EXPENSIVE_SHUFFLE_WEIGHT;
  }

  if (isa<ExtractElementInst>(I)) {
    //Same logic as for shuffles.
    Value* Vec = I->getOperand(0);
    VectorType* OpType = dyn_cast<VectorType>(Vec->getType());
    assert(OpType && "Extract from a non-vector type!");

    if (((OpType->getNumElements() == 4) || 
         (OpType->getNumElements() == 8)) 
       &&
        ((OpType->getElementType()->isFloatTy()) || 
          OpType->getElementType()->isIntegerTy(32)))
      return CHEAP_EXTRACT_WEIGHT;

    return EXPENSIVE_EXTRACT_WEIGHT;
  }

  if (isa<InsertElementInst>(I))
    return INSERT_WEIGHT;


  // We can't take spilling into account at this point, so counting loads 
  // and stores may be voodoo magic.
  // Still, it works.
  if (isa<LoadInst>(I) || isa<StoreInst>(I)) {
    // Look up precomputed "cheap"/"expensive" ops
    DenseMap<Instruction*, int>::iterator OpCost = MemOpCostMap.find(I);
    if (OpCost != MemOpCostMap.end())
      return OpCost->second;

    // Not known to be special, return normal weight.
    return MEM_OP_WEIGHT;
  }

  // Conditional branches are expensive.
  // This has two reasons - a direct one (misprediction)
  // and an indirect one (to punish complex control flow).
  if (BranchInst* BR = dyn_cast<BranchInst>(I))
    if (BR->isConditional())
      return COND_BRANCH_WEIGHT;

  // For everything else - use a default weight
  return DEFAULT_WEIGHT;
}

void WeightedInstCounter::estimateIterations(Function &F, 
                                              DenseMap<Loop*, int> &IterMap) 
                                              const
{
  // Walk the loop tree, "estimating" the total number of loop 
  // iterations for each loop. Since we assume control flow is sane
  // the number of iterations is simply the number of iterations of
  // the current loop multiplied by the computed total number for its
  // parent loop.
  // If the number for the current loop is unknown (because it's not
  // constant, or LoopInfo could not figure it out), we guess it to be
  // LOOP_ITER_GUESS. It may be possible to refine this, but I don't see
  // a good way right now.
  
  std::vector<Loop*> WorkList;
  LoopInfo *LI = &getAnalysis<LoopInfo>();

  // Add all the top-level loops to the worklist
  for (LoopInfo::iterator L = LI->begin(), LE = LI->end(); L != LE; ++L)
    WorkList.push_back(*L);

  // Now, walk the loop tree.
  while(!WorkList.empty()) {
    Loop* L = WorkList.back();    
    WorkList.pop_back();

    assert(IterMap.find(L) == IterMap.end() && 
      "Looking at same loop twice, loop tree is not a tree!");

    int Multiplier = 1;
    // Is this a top-level loop?
    if (Loop* Parent = L->getParentLoop()) {
      // No, it has a parent, so we want to mulitply by the parents' count
      assert(IterMap.find(Parent) != IterMap.end() && 
        "Parent of loop is not in iteration map!");
      
      Multiplier = IterMap.lookup(Parent);
    }

    int Count = L->getSmallConstantTripCount();   
    // getSmallConstantTripCount() returns 0 for non-constant trip counts
    // and on error conditions. In this case guess and hope for the best.
    if (!Count) {
      Count = LOOP_ITER_GUESS;
      
      // If we're pre-vec, we're done. However, if we're post-vec, then
      // for loops that depend on allOnes/allZero, we may want to increase 
      // the multiplier. 
      // Right now, ALL_ZERO_LOOP_PENALTY is constant zero, so the below
      // code is a NOP. Left in for completeleness.
      if (!m_preVec && (ALL_ZERO_LOOP_PENALTY != 0)) {
        BasicBlock* ExitBlock = L->getExitingBlock();
        // Only look at simple loops - those with one exit block.
        if (ExitBlock) {
          Instruction *I = &ExitBlock->back();
          // Ignore the case we don't exit with a branch.
          if (BranchInst* Branch = dyn_cast<BranchInst>(I)) {
            // We only care about conditional branches
            if (Branch->isConditional()) {
              Value* Cond = Branch->getCondition();
              // Which directly depened on a call
              if (CallInst *Call = dyn_cast<CallInst>(Cond)) {
                StringRef Name = Call->getCalledFunction()->getName();
                // Which is allZero/allOne
                if ((Name.startswith(Mangler::name_allZero)) || 
                    (Name.startswith(Mangler::name_allOne))) {
                  // With a vector type!
                  Type* CondType = Call->getOperand(0)->getType();
                  if (VectorType* VecCondType = dyn_cast<VectorType>(CondType)) {
                    // Punish.
                    int Width = VecCondType->getNumElements();
                    Count *= (1 + (Width * ALL_ZERO_LOOP_PENALTY));
                  }
                }
              }
            }
          }
        }
      }
      // End of NOP code
    }

    Count *= Multiplier;

    IterMap[L] = Count;

    // Add all subloops
    for (Loop::iterator Sub = L->begin(), SubE = L->end(); Sub != SubE; ++Sub)
      WorkList.push_back(*Sub);
  }
}

void WeightedInstCounter::
     estimateProbability(Function &F, DenseMap<BasicBlock*, float> 
                         &ProbMap) const {
  
  // What we really ant is a control-dependance graph.
  // Luckily, a node is control-dependent exactly on its postdom
  // frontier.
  PostDominanceFrontier* PDF = &getAnalysis<PostDominanceFrontier>();
  PostDominanceFrontier::DomSetMapType ControlDepMap;

  //Debug output
  dbgPrint() << F.getName();
  if (m_preVec)
    dbgPrint() << " Before";
  else
    dbgPrint() << " After";
  dbgPrint() << "\n";
  if (enableDebugPrints)
    PDF->dump();

  // Check which instructions depend on "data" (results of loads and stores). 
  DenseSet<Instruction*> DepSet;
  estimateDataDependence(F, DepSet);

  // Now do a VERY coarse measurement. The number of nodes on which a block 
  // is control-dependent is the number of decision points. For a block to be
  // reached, all decisions need to go "its way". 
  // This code makes all sorts of silly assumptions.

  for (Function::iterator BB = F.begin(), BE = F.end(); BB != BE; BB++) { 
    PostDominanceFrontier::iterator iter = PDF->find(BB);
    // It's actually possible that a BB has no PDF (as opposed to an empty one)
    // This happens for infinite loops. If this is the case, 
    // we don't really care to evaluate this block.
    if (iter == PDF->end()) {
      ProbMap[BB] = 0;
      continue;
    }

    PostDominanceFrontier::DomSetType &Frontier = iter->second;
    // Before vectorization, the probability is simple 1/(2^k) where k 
    // is the number of branches the block is control-dependent on.
    int count = (int)Frontier.size();
    
    if (!m_preVec) {
      // After vectorization, we consider dependence on an allZero/allOne
      // condition to be a bad thing. The normal structure for an allZero branch
      // is (A => B, A => C, B => C), where the A => B branch is taken only
      // if all workitems satisfy a condition. Note that block C does not depend
      // on the branch in A(since it's always executed), only B does. 
      // Because of the "all workitems" conditions,  it is a good idea to assume 
      // that block B is executed more often than a block that depends on a "normal"
      // branch.
      // For loops, the stucture is similar.   
      for(PostDominanceFrontier::DomSetType::iterator Anc = Frontier.begin(),
          AE = Frontier.end(); Anc != AE; Anc++) {
        // find allZero/allOne conditions, and filter them out.
        BasicBlock *Ancestor = *Anc;
        BranchInst* BR = dyn_cast<BranchInst>(Ancestor->getTerminator());
        if (!BR) 
          // The ancestor's terminator isn't a branch, definitely not an allZero one
          continue;

        Value* Cond = BR->getCondition();
        CallInst* CondCall = dyn_cast<CallInst>(Cond);
        if (!CondCall) 
          // A branch, but not directly based on a call.
          continue;
        
        // Ok, so it's a call.
        // If it has no operands, it's definitely not allOne/allZero.
        // This is basically just a sanity check, there's no good reason for
        // anyone to branch on the result of a call with no arguments.
        if (CondCall->getNumOperands() < 1) 
          continue;

        // Handled unnamed functions (bitcasts)
        if (!CondCall->getCalledFunction())
          continue;

        StringRef Name = CondCall->getCalledFunction()->getName();
        Value* AllZOp = CondCall->getOperand(0);
        Type* AllZOpType = AllZOp->getType();
        // Do not count the ancestor if it's all1/all0, has a vector type, and is data
        // dependent! Not counting penalizes this block, because the less blocks you're 
        // control-dependent on, the higher your probability of being executed is.
        // The reason data dependence is taken into account is that an allZero call
        // that does not depend on data is a pretty weird beast. There are two cases:
        // a) The branch is a guard ("if (x > width)")
        // b) Different workitems perform different computations, according to their GID.
        // We believe case (a) is more common. There is no reason to punish guards, since
        // the logic of "all workitems must agree for the block to be skipped, which is rarer
        // then a single workitem satisfying the condition" no longer applies.
        if ((Name.startswith(Mangler::name_allZero) || Name.startswith(Mangler::name_allOne))
          && AllZOpType->isVectorTy() 
          && (DepSet.find(CondCall) != DepSet.end())) 
            count--;

        // A degenerate case - an allZero(true) branch is never taken. 
        // If you depend on one of those, ignore this dependency.
        ConstantInt* ConstOp = dyn_cast<ConstantInt>(AllZOp);
        if (Name.startswith(Mangler::name_allZero) &&
            ConstOp && ConstOp->isOne())
              count--;
      }      
    }

    ProbMap[BB] = 1.0/(pow(2.0, count));
  }
}

void WeightedInstCounter::estimateMemOpCosts(Function &F, DenseMap<Instruction*, int> &CostMap) const
{
  dbgPrint() << "Estimate MemOp Cost for : ";
  dbgPrint() << F.getName() << "\n";

  std::vector<Instruction*> TIDUsers;
  std::vector<Instruction*> Muls;

  std::vector<Instruction*> CheapGEP;
  std::vector<Instruction*> ExpensiveGEP;

  DenseSet<Instruction*> Visited;

  RuntimeServices* services = RuntimeServices::get();
  assert(services && "Unable to get runtime services");

  // The idea is that some access patterns are good for vectorization and some are bad. 
  // What we try to do here is look at each memory access, and decide 
  // if vectorization will help or hurt it. However, looking at the actual pattern is hard
  // so each operation is looked at in isolation, and some very ugly heuristics is used.
  // We classify each access as "cheap" or "expensive". 
  // An access is "cheap" if we expect the same workitem to access several adjacent locations.
  // It's "expensive" if we expect the pointers to be consecutive in the workitem dimension.
  // The basic heuristic is that if the access is ptr[j] where j depends on the TID, then:
  // 1) if j is some multiple of the TID, then the kernel is probably doing either single-cell
  // or row accesses, so the op is "cheap"
  // 2) if j is not a multiple of the TID, then the kernel is probably doing column accesses,
  // so the op is expensive. That is, if the accessed location if K+id where K is uniform,
  // then we expect accesses to be consecutive.
  // (Using WIAnalisys would be better, but we can't use it here, since this is pre-scalarization)
  // The above is only true for GEPs where the TID is the last index.
  // However, if the ID is not the last index, then your accesses are in fact of the form (K+id*L)+M
  // so after vectorization they are strided (not good), but before vectorization, if you're accessing 
  // several fields of a struct (with different M) this is in fact "cheap", even when not in a loop.

  LoopInfo *LI = &getAnalysis<LoopInfo>();

  // First, find all the TID generators. 
  for (Function::iterator bbit = F.begin(), bbe=F.end(); bbit != bbe; ++bbit) {
    for (BasicBlock::iterator it = bbit->begin(), e=bbit->end(); it!=e;++it) {
      if (CallInst* Call = dyn_cast<CallInst>(it)) {
        bool err = false;
        unsigned dim = 0;
        StringRef Name = Call->getCalledFunction()->getName();
        // get_group_id() is not a TID generator, but plays the same role here.
        if (services->isTIDGenerator(Call, &err, &dim) || Name.equals("get_group_id"))
        {
          assert(!err && "Should not have variable TID access at this stage");
          addUsersToWorklist(Call, Visited, TIDUsers);
        }
      }
    }
  }
  
  // Now run a DFS from each TID generator. We want to find two kinds of thing:
  // a) GEP instructions that use the generator.
  // b) MUL/SHL instructions that use it.
  while (!TIDUsers.empty())
  {
    Instruction* I = TIDUsers.back();
    TIDUsers.pop_back();
    Visited.insert(I);
    if (GetElementPtrInst* GEP = dyn_cast<GetElementPtrInst>(I)) { 
      // Check which index is TID-dependent. If it's the last one this is expensive,
      // otherwise, it's cheap. However, accesses to the local (addrspace 3) addresses 
      // are never expensive.
      // Another possible exception is that wide accesses are never expensive.
      // e.g. use something like:
      // int RetSize = GEP->getType()->getElementType()->getPrimitiveSizeInBits();      
      // and then check whether (RetSize < 128).
      // Not in right now since it doesn't appear to be helpful.

      Value* LastOp = GEP->getOperand(GEP->getNumOperands() - 1);
      Instruction* LastOpInst = dyn_cast<Instruction>(LastOp);
      // If this is not in a loop, we don't care.
      if (!(LI->getLoopFor(GEP->getParent())))
        continue;

      if (LastOpInst && (Visited.find(LastOpInst) != Visited.end()) && 
           (GEP->getPointerAddressSpace() != 3))
        ExpensiveGEP.push_back(GEP);
      else
        CheapGEP.push_back(GEP);
    }
    else if (BinaryOperator* BinOp = dyn_cast<BinaryOperator>(I)) {
      if ((BinOp->getOpcode() == Instruction::Mul) || 
          (BinOp->getOpcode() == Instruction::FMul) ||
          (BinOp->getOpcode() == Instruction::Shl)) {
        Muls.push_back(BinOp);
        // We shouldn't look for uses here, will be done later.
        continue;
      }
    }
    else if (isa<LoadInst>(I) || isa<StoreInst>(I) || isa<CallInst>(I))
      // Don't propagate through loads/stores/calls.
      continue;

    addUsersToWorklist(I, Visited, TIDUsers);
  }

  // Ok, now we have the MULs and SHLs
  // Find the GEPs that depend on them, and mark as cheap.
  Visited.clear();
  while(!Muls.empty()) {
    Instruction* I = Muls.back();
    Muls.pop_back();
    Visited.insert(I);
    if (GetElementPtrInst* GEP = dyn_cast<GetElementPtrInst>(I))
        CheapGEP.push_back(GEP);

    if (!isa<LoadInst>(I) && !isa<StoreInst>(I) && !isa<CallInst>(I))
      addUsersToWorklist(I, Visited, Muls);
  }

  // Now, actually fill in the cost maps.
  // The user of a GEP should normally be either a load, a store, or a call (such as vload/vstore)
  // You wouldn't expect them to propagate through anything, but in fact, phi nodes happen.
  // Could run a DFS, instead support one level of phi.
  // Mark the expensive ones first, since if something is reachable through both types of paths,
  // it should be cheap.
  for (std::vector<Instruction*>::iterator I = ExpensiveGEP.begin(), E = ExpensiveGEP.end();
    I != E; ++I) {
    for (Instruction::use_iterator U = (*I)->use_begin(), UE = (*I)->use_end();
        U != UE; ++U)
    {
      if (Instruction* User = dyn_cast<Instruction>(*U))
        CostMap[User] = EXPENSIVE_MEMOP_WEIGHT;

      dbgPrint() << "Expensive: ";
      if (enableDebugPrints)
        U->dump();
    }
  }
  
  for (std::vector<Instruction*>::iterator I = CheapGEP.begin(), E = CheapGEP.end();
    I != E; I++) {
    for (Instruction::use_iterator U = (*I)->use_begin(), UE = (*I)->use_end();
        U != UE; ++U)
    {
      if (Instruction* User = dyn_cast<Instruction>(*U))
        CostMap[User] = CHEAP_MEMOP_WEIGHT;
      dbgPrint() << "Cheap: ";
      if (enableDebugPrints)
        U->dump();
    }
  }
}

void WeightedInstCounter::addUsersToWorklist(Instruction *I, 
                                DenseSet<Instruction*> &Visited,
                                std::vector<Instruction*> &WorkList) const
{
  // Find all users, add them to the worklist if they haven't been visited yet
  for (Instruction::use_iterator U = I->use_begin(), UE = I->use_end();
       U != UE; U++)
    if (Instruction* User = dyn_cast<Instruction>(*U))
      if (Visited.find(User) == Visited.end())
        WorkList.push_back(User);
}


void WeightedInstCounter::estimateDataDependence(Function &F, 
                     DenseSet<Instruction*> &DepSet) const
{
  // Finds every instruction that depends on loaded data.

  // TODO: Add image reads?  
  std::vector<Instruction*> DataUsers;

  // First, find everything all GEP instructions. 
  // This used to be LoadInst but was changed to GEP.
  // LoadInst seems to make intuitive sense, but in fact also counts loads from allocas.
  // Why would there even be allocas at this stage? Because of soa builtins that return
  // results through local pointers. Oops.
  for (Function::iterator BB = F.begin(), BBE = F.end(); BB != BBE; ++BB) {
    for (BasicBlock::iterator I = BB->begin(), IE = BB->end(); I != IE; ++I) {
      if (dyn_cast<GetElementPtrInst>(I)) 
        DataUsers.push_back(I);
    }
  }
  
  // Now run a DFS from each GEP, and mark all its (indirect) users.  
  while (!DataUsers.empty()) {
    Instruction* I = DataUsers.back();
    DataUsers.pop_back();
    DepSet.insert(I);
    addUsersToWorklist(I, DepSet, DataUsers);
  }
}

bool VectorizationPossibilityPass::runOnFunction(Function & F)
{
  DominatorTree &DT = getAnalysis<DominatorTree>();
  m_canVectorize = CanVectorizeImpl::canVectorize(F, DT);
  return false;
}


bool CanVectorizeImpl::canVectorize(Function &F, DominatorTree &DT)
{
  if (hasVariableGetTIDAccess(F)) {
    dbgPrint() << "Variable TID access, can not vectorize\n";
    return false;
  }

  if (!isReducibleControlFlow(F, DT)) { 
    dbgPrint()<< "Irreducible control flow, can not vectorize\n";
    return false;
  }
  
  if (hasIllegalTypes(F)) { 
    dbgPrint() << "Types unsupported by codegen, can not vectorize\n";
    return false;
  }

  if (hasNonInlineUnsupportedFunctions(F)) { 
    dbgPrint() << "Call to unsupported functions, can not vectorize\n";
    return false;
  }

  return true;
}

bool CanVectorizeImpl::hasVariableGetTIDAccess(Function &F) {
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

// Reference:
//  Book: High Performance Compilers for Parallel Computing / Michael Wolfe
//  Page 60, Section 3.2.4 - Finding Cycles in Directed Graphs
//
// Check if the graph is irreducible using the standard algorithm:
// 1. If you ignore backedges, graph is acyclic.
// 2. backedges are edges where the target dominates the src.
bool CanVectorizeImpl::isReducibleControlFlow(Function& F, DominatorTree& DT) {  
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
        if (DT.dominates(*bb, *p)) continue;
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

bool CanVectorizeImpl::hasIllegalTypes(Function &F) {
  // For each BB
  for (Function::iterator bbit = F.begin(), bbe=F.end(); bbit != bbe; ++bbit) {
    // For each instruction
    for (BasicBlock::iterator it = bbit->begin(), e=bbit->end(); it!=e;++it) {
      Type* tp = it->getType();
      // strip vector types
      if (VectorType* VT = dyn_cast<VectorType>(tp)) {
        tp = VT->getElementType();
      }
      // check that integer types are legal
      if (IntegerType* IT = dyn_cast<IntegerType>(tp)) {
        unsigned BW = IT->getBitWidth();
        if (BW > 64) 
          return true;
      }
    }
  }
  return false;
}

bool CanVectorizeImpl::hasNonInlineUnsupportedFunctions(Function &F) {
  Module *pM = F.getParent();
  std::set<Function *> unsupportedFunctions;
  std::set<Function *> roots;

  // Add all kernels to root functions
  // Kernels assumes to have implicit barrier
  SmallVector<Function *, 4> kernels;
  LoopUtils::GetOCLKernel(*pM, kernels);
  roots.insert(kernels.begin(), kernels.end());

  // Add all functions that contains barrier/get_local_id/get_global_id to root functions
  llvm::StringRef oclFunctionName[3] = {BARRIER_FUNC_NAME, GET_LID_NAME, GET_GID_NAME};
  for(unsigned int i=0; i<3; i++) {
    Function *F = pM->getFunction(oclFunctionName[i]);
    if (!F) continue;
    for (Function::use_iterator ui = F->use_begin(), ue = F->use_end();
         ui != ue; ++ui ) {
      CallInst *CI = dyn_cast<CallInst> (*ui);
      if (!CI) continue;
      Function *pCallingFunc = CI->getParent()->getParent();
      roots.insert(pCallingFunc);
    }
  }

  // Fill unsupportedFunctions set with all functions that calls directly or undirectly
  // functions from the root functions set
  LoopUtils::fillFuncUsersSet(roots, unsupportedFunctions);
  return unsupportedFunctions.count(&F);
}

extern "C" {
  FunctionPass* createWeightedInstCounter(bool preVec = true, bool sanityOnly = false, 
                                          Intel::CPUId cpuId = Intel::CPUId()) {
    return new intel::WeightedInstCounter(preVec, sanityOnly, cpuId);
  }
}

char intel::VectorizationPossibilityPass::ID = 0;
static RegisterPass<intel::VectorizationPossibilityPass>
CLIVectorPossibility("vectorpossible", "Check whether vectorization is possible");

char intel::WeightedInstCounter::ID = 0;
static RegisterPass<intel::WeightedInstCounter>
CLIInstCount("winstcounter", "Weighted Instruction Counter");

} // namespace intel

