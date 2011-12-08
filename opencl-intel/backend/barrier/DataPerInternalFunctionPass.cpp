/*********************************************************************************************
 * TODO: add Copyright © 2011, Intel Corporation
 *********************************************************************************************/

#include "DataPerInternalFunctionPass.h"

#include "llvm/Instructions.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/raw_ostream.h"

namespace intel {
  char DataPerInternalFunction::ID = 0;
  unsigned int DataPerInternalFunction::m_badOffset = (unsigned int)(-1);

  DataPerInternalFunction::DataPerInternalFunction() : ModulePass(ID) {}

  bool DataPerInternalFunction::runOnModule(Module &M) {
    //Initialize barrier utils class with current module
    m_util.init(&M);

    m_pDataPerValue = &getAnalysis<DataPerValue>();

    //Find all the kernel functions
    //TFunctionVector& kernelFunctions = m_util.getAllKernelFunctions();

    //Find all functions that call synchronize instructions
    TFunctionVector& functionsWithSync = m_util.getAllFunctionsWithSynchronization();

    //Collect data for each function with synchronize instruction
    for ( TFunctionVector::iterator fi = functionsWithSync.begin(),
      fe = functionsWithSync.end(); fi != fe; ++fi ) {
        runOnFunction(*(*fi));
    }

    //Collect all functions to be fix in a list ordered according to call graph
    calculateCallingOrder();

    return false;
  }

  bool DataPerInternalFunction::runOnFunction(Function &F) {

    unsigned int numOfArgs = F.getFunctionType()->getNumParams();
    bool hasReturnValue = !(F.getFunctionType()->getReturnType()->isVoidTy());
    // Keep one last argument for return value
    unsigned int numOfArgsWithReturnValue = hasReturnValue ? numOfArgs+1 : numOfArgs;

    if ( 0 == numOfArgsWithReturnValue ) {
      //Function takes no arguments, nothing to check
      return false;
    }
    //Initialize number of function uses to zero
    m_dataPerFuncMap[&F].m_numberOfUses = 0;
    TCounterVector &argsFunction = m_dataPerFuncMap[&F].m_argsInSpecialBuffer;
    //Initialize number function calls with argument value
    //stored in special buffer to zero
    argsFunction.assign(numOfArgsWithReturnValue, 0);
    bool hasArgFromSpecialBuffer = false;
    //Check each call to F function searching parameters stored in special buffer
    for ( Value::use_iterator ui = F.use_begin(),
      ue = F.use_end(); ui != ue; ++ui ) {
        CallInst *pCallInst = dyn_cast<CallInst>(*ui);
        assert( pCallInst && "Something other than CallInst is calling a function!" );
        assert( numOfArgs == pCallInst->getNumArgOperands() &&
          "calling function with different number of operands!" );
        TCounterVector &argsCall = m_dataPerCallMap[pCallInst].m_argsOffsets;
        //Initialize CallInst argument offsets to negative number
        //to indicate a bad offset
        argsCall.assign(numOfArgsWithReturnValue, m_badOffset);
        //Increment number of function uses by one
        m_dataPerFuncMap[&F].m_numberOfUses++;
        for ( unsigned int i = 0; i < numOfArgsWithReturnValue; ++i ) {
          Value *pVal = (i == numOfArgs) ?
            pCallInst : pCallInst->getArgOperand(i);
          if ( m_pDataPerValue->hasOffset(pVal) ) {
            //If reach here, means that this function has at least
            //one caller with argument value in special buffer
            hasArgFromSpecialBuffer = true;
            //Increment number function calls with argument value
            //stored in special buffer by one
            argsFunction[i]++;
            //Initialize the offset of this argument according to the value
            //stored in special buffer
            argsCall[i] = m_pDataPerValue->getOffset(pVal);
          }
        }
    }

    //Need to handle function only if it has an
    //argument value stored in special buffer
    m_dataPerFuncMap[&F].m_needToBeFixed = hasArgFromSpecialBuffer;

    return false;
  }

  void DataPerInternalFunction::calculateCallingOrder() {
    TFunctionSet functionsToHandle;
    //Initialize functionToHandle container with functions that need to be fixed
    for ( TDataPerFunctionMap::iterator fi = m_dataPerFuncMap.begin(),
      fe = m_dataPerFuncMap.end(); fi != fe; ++fi ) {
        if( fi->second.m_needToBeFixed ) {
          functionsToHandle.insert(fi->first);
        }
    }
    while ( !functionsToHandle.empty() ) {
      for ( TFunctionSet::iterator fi = functionsToHandle.begin(),
        fe = functionsToHandle.end(); fi != fe; ++fi )
      {
        Function *pFunc = dyn_cast<Function>(*fi);
        bool isRoot = true;
        for ( Value::use_iterator ui = pFunc->use_begin(),
          ue = pFunc->use_end(); ui != ue; ++ui ) {
            CallInst *pCallInst = dyn_cast<CallInst>(*ui);
            Function *pCallerFunc = pCallInst->getParent()->getParent();
            if ( functionsToHandle.count(pCallerFunc) ) {
              isRoot = false;
              break;
            }
        }
        if ( isRoot ) {
          m_orderedFunctionsToFix.insert(m_orderedFunctionsToFix.begin(), pFunc);
          functionsToHandle.erase(fi);
          break;
        }
      }
    }
  }

  void DataPerInternalFunction::print(raw_ostream &OS, const Module *M) const {
    if ( !M ) {
      OS << "No Module!\n";
      return;
    }
    //Print Module
    OS << *M;

    OS << "Data collected on functions\n";
    for ( TDataPerFunctionMap::const_iterator fi = m_dataPerFuncMap.begin(),
      fe = m_dataPerFuncMap.end(); fi != fe; ++fi ) {
        //Print function name
        OS << fi->first->getNameStr() << "\n";
        OS << "\tneed to be fixed: " << fi->second.m_needToBeFixed;
        OS << "\tnumber of usages: " << fi->second.m_numberOfUses;
        OS << "\tIn special buffer counters: (";
        for ( unsigned int i = 0; i < fi->second.m_argsInSpecialBuffer.size() ; ++i ) {
          OS << ((i == 0)?" ": ", ") << fi->second.m_argsInSpecialBuffer[i];
        }
        OS << " )\n";
    }
    OS << "Data collected on calls\n";
    for ( TDataPerCallMap::const_iterator ci = m_dataPerCallMap.begin(),
      ce = m_dataPerCallMap.end(); ci != ce; ++ci ) {
        //Print call instruction
        OS << *ci->first << "\n";
        OS << "\tOffsets in special buffer: (";
        for ( unsigned int i = 0; i < ci->second.m_argsOffsets.size() ; ++i ) {
          int offset = ci->second.m_argsOffsets[i];
          OS << ((i == 0)?" ": ", ");
          if ( offset == m_badOffset ) {
            OS << "BAD_OFFSET";
          } else {
            OS << offset;
          }
        }
        OS << " )\n";
    }

    OS << "Ordered functions to fix\n";
    for ( TFunctionVector::const_iterator fi = m_orderedFunctionsToFix.begin(),
      fe = m_orderedFunctionsToFix.end(); fi != fe; ++fi ) {
        //Print call instruction
        Function *pFunc = dyn_cast<Function>(*fi);
        OS << "\t" << pFunc->getNameStr() << "\n";
    }

    OS << "DONE\n";
  }

  //Register this pass...
  static RegisterPass<DataPerInternalFunction> DPV("B-FunctionAnalysis",
    "Barrier Pass - Collect Data per Internal Function", false, true);


} // namespace intel

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  void* createDataPerInternalFunctionPass() {
    return new intel::DataPerInternalFunction();
  }
}