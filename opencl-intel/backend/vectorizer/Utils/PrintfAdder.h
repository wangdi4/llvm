/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
// This pass is a development utility that add printf calls using relatively easy interfaces
// into IR. it has the following abilities:
// - print IR for all work items or certain work item (according to m_printConditionaly flag)
// - print the name of each printed value before it's actual value (according to m_printNames flag)
// - print value with all it's operands recursively until some predefined depth
// - print arbitrary list of values
// - print the arguments of a function
// - print the current global id's

// The pass was developed mainly to be used in apple environment since we don't have the CL source
// so we can't put the printf in it. it is also usefull if we want to debug some values that are
// not in the original code (like masks)
// Author: Ran Chachick.

#ifndef __PRINTF_ADDER_H__
#define __PRINTF_ADDER_H__


#include "llvm/Pass.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/ADT/ArrayRef.h"
#include <list>
#include <vector>
#include <string>
#include <sstream>


using namespace llvm;


#define GET_GID_NAME  "get_global_id"


namespace intel {


class PrintfAdder : public FunctionPass {


public:
  static char ID; // Pass identification, replacement for typeid

  PrintfAdder(bool printNames=true, bool printConditionaly=true) : FunctionPass(ID) {
    m_printNames = printNames;
    m_printConditionally = printConditionaly;
  };
  
  ~PrintfAdder()  {}
  virtual StringRef getPassName() const {
    return "PrintfAdder";
  }
  
  ///@brief LLVM interface.
  bool runOnFunction (Function &F);
  
  ///@brief main function adds debug prints 
  ///       This function is changed according to the value we would like to print.
  void addDebugPrints(Function *F);

private :
  typedef SmallVector<Value *, 4> VVec;
  
  /// holds arguments commonly passed by debug print procedures  
  typedef struct debug_print_args {
    std::list<Value *> toPrint;         //values to be printed
    std::string prefix;                 //string to be printed at start
    std::string suffix;                 //string to be printed at the end
  } debug_print_args;

  // @brief holds get_global_id call for printing of conditinaly printing.
  VVec m_gids;

  ///@brief holds the specific global ids in which print occurs when
  ///       m_printConditionally is set.
  SmallVector<unsigned, 4> m_ngids;

  ///@brief if true will print names of the printed value before their value.
  bool m_printNames;
  
  ///@brief if true will print values only for specific global ids.
  bool m_printConditionally;



  //-------------------------------------------------------
  // main function to be used inside addDebugPrints
  //--------------------------------------------------------

  ///@brief print the global id
  void addDebugPrintGID(Function *F);
  
  ///@brief print the arguments of the function.
  void addDebugPrintFuncArgs(Function *F);  
  
  ///@brief prints conditionally on global_id the valToTrack and it's operands
  ///       recursively until depth parameter   
  void addPrintDepth(Function *F, Value *valToTrack, int depthThr); 
  
  ///@brief main function prints Values in print_arg toPrint field before 
  ///       loc instruction for all work items or specific one  according to 
  ///       m_printConditionally
  void addDebugPrint(Function *F, debug_print_args& print_args, Instruction *loc); 




  //-------------------------------------------------------
  // helper funtions
  //--------------------------------------------------------

  /// @brief return int value of environment variable and default if the variable does not exists.  
  int getIntEnvVarVal (const char *varName, int defVal);
    
  ///@brief fills m_gids with calls to get_global_id set the id to conditonally print on.
  void getConditionalGIDs (Function *F);
  
  ///@brief add Names to all values in the print list.
  void setUninitializedNames(debug_print_args& print_args); 
  
  ///@brief main function prints Values in print_arg toPrint field before 
  ///       loc instruction 
  void addDebugPrintImpl(Function *F, debug_print_args& print_args, Instruction *loc); 
  
  ///@brief prints contditionally on gid0 and gid1 values    
  void addGIDConditionPrint (Function *F, debug_print_args& print_args, VVec& gids);

  ///@brief creates a function before loc that prints values print_args conditionally
  ///       on gid0, gid1  
  void addDebugCondPrintFunction(Function *F, debug_print_args& print_args, Instruction *loc);

  ///@brief returns true iff the value is printable (constasnt, argument, instruction).
  bool canPrint(Value *v);


};

} // namespace intel

#endif // __PRINTF_ADDER_H__
