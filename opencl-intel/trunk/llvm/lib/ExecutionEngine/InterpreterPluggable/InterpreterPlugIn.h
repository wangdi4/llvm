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

File Name:  InterpreterPlugIn.h


\*****************************************************************************/

#ifndef LLI_INTERPRETER_PLUGIN_H
#define LLI_INTERPRETER_PLUGIN_H

#include <vector>

namespace llvm {

    class InterpreterPluggable;
    struct ExecutionContext;
    class Instruction;

    /// interface to LLVM interpreter plugin
    class InterpreterPlugIn
    {
    public:
        /// event before function call
        virtual void handlePreFunctionRun(
            std::vector<ExecutionContext>& ECStack,
            llvm::InterpreterPluggable& Interp) = 0;

        /// event after function call
        virtual void handlePostFunctionRun() = 0;

        /// event before instruction execution
        virtual void handlePreInstExecution(Instruction& ) = 0;

        /// event after instruction execution
        virtual void handlePostInstExecution(Instruction& ) = 0;

    };

} // namespace llvm

#endif // LLI_INTERPRETER_PLUGIN_H