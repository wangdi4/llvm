/*********************************************************************************************
 * TODO: add Copyright © 2011, Intel Corporation
 *********************************************************************************************/

#include "WIRelatedValuePass.h"
#include "BarrierUtils.h"

#include "llvm/Support/InstIterator.h"
#include "llvm/Support/raw_ostream.h"

namespace intel {
  char WIRelatedValue::ID = 0;

  WIRelatedValue::WIRelatedValue() : ModulePass(ID) {}

  bool WIRelatedValue::runOnFunction(Function &F) {
    m_changed.clear();

    //Schedule all instruction for calculation
    inst_iterator it = inst_begin(F);
    inst_iterator  e = inst_end(F);
    for (; it != e; ++it) {
      m_changed.insert(&*it);
    }

    updateDeps();
    return false;
  }

  void WIRelatedValue::updateDeps() {
    //As lonst as we have values to update
    while(!m_changed.empty()) {
      //copy the list aside
      std::set<Value*> changed(m_changed.begin(), m_changed.end());
      //clear the original list, for next iteration
      m_changed.clear();
      //update all changed values
      for ( std::set<Value*>::iterator vi = changed.begin(), ve = changed.end();
          vi != ve; ++vi) {
        //remove first instruction
        //calculate its new dependencey value
        calculate_dep(*vi);
      }
    }
  }

  bool WIRelatedValue::getWIRelation(Value *pVal) {
    //New instruction to consider
    if ( m_specialValues.find(pVal) == m_specialValues.end() ) {
      //mark it as not related to WI Id till farther update
      m_specialValues[pVal] = false;
    }

    return m_specialValues[pVal];
  }

  void WIRelatedValue::calculate_dep(Value *pVal) {

    Instruction *pInst = dyn_cast<Instruction>(pVal);

    if ( !pInst ) {
      assert( false && "we are runing on instruction list, can we reach here?" );
      //Not an instruction, must be a constant or an argument
      //Could this vector type be of a constant whic is not uniform ?
      return;
    }

    //New instruction to consider
    if ( m_specialValues.find(pInst) == m_specialValues.end() ) {
      //mark it as not related to WI Id till farther update
      m_specialValues[pInst] = false;
    }

    //Our initial value
    bool origRelation = m_specialValues[pInst];
    bool newRelation = origRelation;

    //LLVM does not have compile time polymorphisms
    //TODO: to make things faster we may want to sort the list below according
    //to the order of their probability of appearance.
    if (BinaryOperator     *inst = dyn_cast<BinaryOperator>(pInst))    {newRelation = calculate_dep(inst); }
    else if (CallInst           *inst = dyn_cast<CallInst>(pInst))          {newRelation = calculate_dep(inst); }
    else if (CmpInst            *inst = dyn_cast<CmpInst>(pInst))           {newRelation = calculate_dep(inst); }
    else if (ExtractElementInst *inst = dyn_cast<ExtractElementInst>(pInst)){newRelation = calculate_dep(inst); }
    else if (GetElementPtrInst  *inst = dyn_cast<GetElementPtrInst>(pInst)) {newRelation = calculate_dep(inst); }
    else if (InsertElementInst  *inst = dyn_cast<InsertElementInst>(pInst)) {newRelation = calculate_dep(inst); }
    else if (InsertValueInst    *inst = dyn_cast<InsertValueInst>(pInst))   {newRelation = calculate_dep(inst); }
    else if (PHINode            *inst = dyn_cast<PHINode>(pInst))           {newRelation = calculate_dep(inst); }
    else if (ShuffleVectorInst  *inst = dyn_cast<ShuffleVectorInst>(pInst)) {newRelation = calculate_dep(inst); }
    else if (StoreInst          *inst = dyn_cast<StoreInst>(pInst))         {newRelation = calculate_dep(inst); }
    else if (TerminatorInst     *inst = dyn_cast<TerminatorInst>(pInst))    {newRelation = calculate_dep(inst); }
    else if (SelectInst         *inst = dyn_cast<SelectInst>(pInst))        {newRelation = calculate_dep(inst); }
    else if (AllocaInst         *inst = dyn_cast<AllocaInst>(pInst))        {newRelation = calculate_dep(inst); }
    else if (CastInst           *inst = dyn_cast<CastInst>(pInst))          {newRelation = calculate_dep(inst); }
    else if (ExtractValueInst   *inst = dyn_cast<ExtractValueInst>(pInst))  {newRelation = calculate_dep(inst); }
    else if (LoadInst           *inst = dyn_cast<LoadInst>(pInst))          {newRelation = calculate_dep(inst); }
    else if (VAArgInst          *inst = dyn_cast<VAArgInst>(pInst))         {newRelation = calculate_dep(inst); }

    //If the value was changed in this calculation
    if ( newRelation != origRelation ) {
      //Save the new value of this instruction
      m_specialValues[pInst] = newRelation;
      //Register for update all of the dependent values of this updated instruction.
      Value::use_iterator ui = pInst->use_begin();
      Value::use_iterator ue  = pInst->use_end();
      for (; ui != ue; ++ui) {
        m_changed.insert(*ui);
      }
    }
  }

  bool WIRelatedValue::calculate_dep(BinaryOperator *pInst) {
    //Calculate the WI relation for each of the operands
    Value *op0 = pInst->getOperand(0);
    Value *op1 = pInst->getOperand(1);

    bool dep0 = getWIRelation(op0);
    bool dep1 = getWIRelation(op1);

    return (dep0 || dep1);
  }

  bool WIRelatedValue::calculate_dep(CallInst *pInst) {
    //TODO: This function requires much more work, to be correct:
    //   2) Some functions (dot_prod, cross_prod) provide "measurable"
    //   behavior (Uniform->strided).
    //   This information should also be obtained from RuntimeServices somehow.

    //Check if call is TID-generator

    //Check if the function is in the table of functions
    Function *origFunc = pInst->getCalledFunction();
    std::string origFuncName = origFunc->getName();

    if( origFuncName == GET_GID_NAME ||
      origFuncName == GET_LID_NAME ) {
        //These functions return WI Id, they indeed related on WI Id
        return true;
    }

    //Check if function is not declared inside "this" module
    if ( !pInst->getCalledFunction()->isDeclaration() ) {
      //For functions defined (not declared) in this module - it is unsafe to assume anything
      //TODO: can we check the function and assure it is not related on WI-Id?
      return true;
    }

    //Iterate over all input dependencies. If all are not WI Id related - propagate it.
    //Otherwise - return WI Id related
    unsigned int numParams = pInst->getNumArgOperands();

    bool isWIRelated = false;
    for ( unsigned int i = 0; i < numParams; ++i )
    {
      //Operand 0 is the function's name
      Value *op = pInst->getArgOperand(i);
      isWIRelated = isWIRelated || getWIRelation(op);
      if ( isWIRelated ) {
        break; //non related check failed. no need to continue
      }
    }
    return isWIRelated;
  }

  bool WIRelatedValue::calculate_dep(CmpInst *pInst) {
    //Calculate the WI relation for each of the operands
    Value *op0 = pInst->getOperand(0);
    Value *op1 = pInst->getOperand(1);

    bool dep0 = getWIRelation(op0);
    bool dep1 = getWIRelation(op1);

    return (dep0 || dep1);
  }

  bool WIRelatedValue::calculate_dep(ExtractElementInst *pInst) {
    //Return the WI relation of the only one operand
    Value *op0 = pInst->getOperand(0);

    bool dep0 = getWIRelation(op0);

    return dep0;
  }

  bool WIRelatedValue::calculate_dep(GetElementPtrInst *pInst) {
    //Calculate the WI relation for each of the operands
    unsigned int num = pInst->getNumIndices();
    Value *op0 = pInst->getPointerOperand();

    bool dep = getWIRelation(op0);

    for ( unsigned int i=0; i < num; ++i ) {
      dep = dep || getWIRelation(pInst->getOperand(i + 1));
    }

    return dep;
  }

  bool WIRelatedValue::calculate_dep(InsertElementInst *pInst) {
    //Calculate the WI relation for each of the operands
    Value *op0 = pInst->getOperand(0);
    Value *op1 = pInst->getOperand(1);

    bool dep0 = getWIRelation(op0);
    bool dep1 = getWIRelation(op1);

    return (dep0 || dep1);
  }

  bool WIRelatedValue::calculate_dep(InsertValueInst *pInst) {
    //TODO: why should we always return related?
    assert( false && "This is a reachable place, handle it right" );
    return true;
  }

  bool WIRelatedValue::calculate_dep(PHINode *pInst) {
    //Calculate the WI relation for each of the operands
    //unsigned int num = pInst->getNumIncomingValues();
    //bool dep = false;

    //for ( unsigned int i=0; i < num; ++i ) {
    //  Value *op = pInst->getIncomingValue(i);
    //  dep = dep || getWIRelation(op);
    //}

    //return dep;
    //TODO: CSSD100007559 (should fix the following case and then remove the always return true!): %isOk.0 = phi i1 [ false, %4 ], [ true, %0 ]
    return true;
  }

  bool WIRelatedValue::calculate_dep(ShuffleVectorInst *pInst) {
    //Calculate the WI relation for each of the operands
    Value *op0 = pInst->getOperand(0);
    Value *op1 = pInst->getOperand(1);

    bool dep0 = getWIRelation(op0);
    bool dep1 = getWIRelation(op1);

    return (dep0 || dep1);
  }

  bool WIRelatedValue::calculate_dep(StoreInst *pInst) {
    //No need to handle store/load instructions as alloca is handled separately
    return false;
  }

  bool WIRelatedValue::calculate_dep(TerminatorInst *pInst) {
    //Instruction has no return value
    //Just need to know if this inst is uniform or not
    //because we may want to avoid predication if the control flows
    //in the function are uniform...
    switch (pInst->getOpcode())
    {
    case Instruction::Br:
      {
        BranchInst *pBrInst = cast<BranchInst>(pInst);
        if ( pBrInst->isConditional() ) {
          //Conditional branch is uniform, if its condition is uniform
          Value *op = pBrInst->getCondition();
          return getWIRelation(op);
        }
        //Unconditional branch is non TID-dependent
        return false;
      }
    case Instruction::IndirectBr:
      //TODO: Define the dependency requirements of indirectBr
    default:
      return true;
    }
  }

  bool WIRelatedValue::calculate_dep(SelectInst *pInst) {
    //Calculate the WI relation for each of the operands
    Value *op0 = pInst->getOperand(0);
    Value *op1 = pInst->getOperand(1);
    Value *op2 = pInst->getOperand(2);

    bool dep0 = getWIRelation(op0);
    bool dep1 = getWIRelation(op1);
    bool dep2 = getWIRelation(op2);

    return (dep0 || dep1 || dep2);
  }

  bool WIRelatedValue::calculate_dep(AllocaInst *pInst) {
    //No need to handle alloca instruction, it is handled separately
    return false;
  }

  bool WIRelatedValue::calculate_dep(CastInst *pInst) {
    //Return the WI relation of the only one operand
    Value *op0 = pInst->getOperand(0);

    bool dep0 = getWIRelation(op0);

    return dep0;
  }

  bool WIRelatedValue::calculate_dep(ExtractValueInst *pInst) {
    //TODO: why should we always return related?
    assert( false && "This is a reachable place, handle it right" );
    return true;
  }

  bool WIRelatedValue::calculate_dep(LoadInst *pInst) {
    //No need to handle store/load instructions as alloca is handled separately
    //Return the WI relation of the only one operand
    Value *op0 = pInst->getOperand(0);

    bool dep0 = getWIRelation(op0);

    return dep0;
  }

  bool WIRelatedValue::calculate_dep(VAArgInst *pInst) {
    assert(false && "Are we supporting this ??");
    return false;
  }

  void WIRelatedValue::print(raw_ostream &OS, const Module *M) const {
    if ( !M ) {
      OS << "No Module!\n";
      return;
    }
    //Print Module
    OS << *M;

    //Run on all WI related values
    OS << "\nWI related Values\n";
    TValuesMap::const_iterator vi = m_specialValues.begin();
    TValuesMap::const_iterator ve = m_specialValues.end();
    for ( ; vi != ve; ++vi ) {
      Value *pVal = dyn_cast<Value>(vi->first);
      if ( isa<StoreInst>(pVal) ) continue;
      bool isWIRelated = vi->second;
      //Print vale name is (not) WI related!
      OS << pVal->getNameStr();
      OS << ( (isWIRelated) ? " is WI related" : " is not WI related" );
      OS << "\n";
    }
  }

  //Register this pass...
  static RegisterPass<WIRelatedValue> WIRV("B-WIAnalysis",
    "Barrier Pass - Calculate WI relation per Value", false, true);


} // namespace intel

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  void* createWIRelatedValuePass() {
    return new intel::WIRelatedValue();
  }
}