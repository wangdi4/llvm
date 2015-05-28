/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  InterpreterPluggable.h


\*****************************************************************************/

#ifndef LLI_INTERPRETER_PLUGGABLE_H
#define LLI_INTERPRETER_PLUGGABLE_H

#include "llvm/ExecutionEngine/Interpreter/Interpreter.h"
#include "InterpreterPlugIn.h"
#include "WorkGroupBuiltinsNames.h"
#include <list>
namespace llvm {


class InterpreterPluggable : public Interpreter
{
    std::list<InterpreterPlugIn*> m_pPlugins;
    Validation::WorkGroupBultinsNames m_WGBuiltinsNames;
    typedef std::list<InterpreterPlugIn*>::iterator PlugInIterator;
    // flag if interpreter is still running
    bool m_stillRunning;
    // in case of blocking work group builtins
    // wait for all work items in the wg or proceed to builtin execution
    bool m_needToExecutePreMethod;

public:
    static void Register() {
        InterpCtor = create;
    }

    /// create - Create an interpreter ExecutionEngine. This can never fail.
    ///
    static ExecutionEngine *create(std::unique_ptr<Module> M, std::string *ErrorStr = nullptr);

    InterpreterPluggable(std::unique_ptr<Module> M)
        : Interpreter(std::move(M)), m_stillRunning(false), m_needToExecutePreMethod(true)
    {
    }
    /// run - Start execution with the specified function and arguments.
    ///
    virtual GenericValue runFunction(Function *F,
        const std::vector<GenericValue> &ArgValues);

    // return codes
    enum RETCODE
    {
        OK = 0,
        BARRIER = 2,
        BLOCKING_WG_FUNCTION = 3
    };

    /// run with plugins call
    /// @return OK if function ended, BARRIER if barrier was found
    InterpreterPluggable::RETCODE runWithPlugins();

    /// add plugin to interpreter
    void addPlugIn(InterpreterPlugIn& ref)
    {
        m_pPlugins.push_back(&ref);
    }

    /// add plugin to interpreter
    void removePlugIn(InterpreterPlugIn& ref)
    {
        m_pPlugins.remove(&ref);
    }

    /// public adapter function to Interpreter::getOperandValue()
    GenericValue getOperandValueAdapter(Value *V, ExecutionContext &SF)
    {
        return getOperandValue(V, SF);
    }

};

} // End llvm namespace

#endif // LLI_INTERPRETER_PLUGGABLE_H
