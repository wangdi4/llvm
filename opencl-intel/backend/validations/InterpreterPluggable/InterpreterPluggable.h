// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#ifndef LLI_INTERPRETER_PLUGGABLE_H
#define LLI_INTERPRETER_PLUGGABLE_H

#include "InterpreterPlugIn.h"
#include "WorkGroupBuiltinsNames.h"
#include "llvm/ExecutionEngine/Interpreter/Interpreter.h"
#include <list>
namespace llvm {

class InterpreterPluggable : public Interpreter {
  std::list<InterpreterPlugIn *> m_pPlugins;
  Validation::WorkGroupBultinsNames m_WGBuiltinsNames;
  typedef std::list<InterpreterPlugIn *>::iterator PlugInIterator;
  // flag if interpreter is still running
  bool m_stillRunning;
  // in case of blocking work group builtins
  // wait for all work items in the wg or proceed to builtin execution
  bool m_needToExecutePreMethod;

public:
  static void Register() { InterpCtor = create; }

  /// create - Create an interpreter ExecutionEngine. This can never fail.
  ///
  static ExecutionEngine *create(std::unique_ptr<Module> M,
                                 std::string *ErrorStr = nullptr);

  InterpreterPluggable(std::unique_ptr<Module> M)
      : Interpreter(std::move(M)), m_stillRunning(false),
        m_needToExecutePreMethod(true) {}
  /// run - Start execution with the specified function and arguments.
  ///
  GenericValue runFunction(Function *F,
                           ArrayRef<GenericValue> ArgValues) override;

  // return codes
  enum RETCODE { OK = 0, BARRIER = 2, BLOCKING_WG_FUNCTION = 3 };

  /// run with plugins call
  /// @return OK if function ended, BARRIER if barrier was found
  InterpreterPluggable::RETCODE runWithPlugins();

  /// add plugin to interpreter
  void addPlugIn(InterpreterPlugIn &ref) { m_pPlugins.push_back(&ref); }

  /// add plugin to interpreter
  void removePlugIn(InterpreterPlugIn &ref) { m_pPlugins.remove(&ref); }

  /// public adapter function to Interpreter::getOperandValue()
  GenericValue getOperandValueAdapter(Value *V, ExecutionContext &SF) {
    return getOperandValue(V, SF);
  }
};

} // namespace llvm

#endif // LLI_INTERPRETER_PLUGGABLE_H
