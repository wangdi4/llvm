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

File Name:  InterpreterPluggable.cpp


\*****************************************************************************/

#define DEBUG_TYPE "interpreterPluggable"

#include "InterpreterPlugIn.h"
#include "InterpreterPluggable.h"

#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Instructions.h"
#include "llvm/CodeGen/IntrinsicLowering.h"
#include "llvm/Support/GetElementPtrTypeIterator.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/CodeGen/IntrinsicLowering.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Module.h"
#include "llvm/ADT/Statistic.h"

#include <algorithm>
#include <cmath>
#include <cstring>
using namespace llvm;

STATISTIC(NumDynamicInsts, "Number of dynamic instructions executed");

// todo: add macro for switching on InterpreterPluggable
#if 0
#if 1
static struct RegisterInterp {
    RegisterInterp() { InterpreterPluggable::Register(); }
} InterpRegistrator;

#else
static struct RegisterInterp {
    RegisterInterp() { Interpreter::Register(); }
} InterpRegistrator;

}
#endif
#endif

extern "C" void LLVMLinkInInterpreterPluggable() {
    InterpreterPluggable::Register();
}

/// create - Create a new interpreter object.  This can never fail.
///
ExecutionEngine *InterpreterPluggable::create(Module *M, std::string* ErrStr) {
    // Tell this Module to materialize everything and release the GVMaterializer.
    if (M->MaterializeAllPermanently(ErrStr))
        // We got an error, just return 0
        return 0;

    return new InterpreterPluggable(M);
}


/// run - Start execution with the specified function and arguments.
/// It is copy paste from from base class Interpreter::runFunction() method 
/// except calling our own run() function
GenericValue
InterpreterPluggable::runFunction(Function *F,
                                  const std::vector<GenericValue> &ArgValues) 
{
    assert (F && "Function *F was null at entry to run()");

    // Try extra hard not to pass extra args to a function that isn't
    // expecting them.  C programmers frequently bend the rules and
    // declare main() with fewer parameters than it actucally gets
    // passed, and the interpreter barfs if you pass a function more
    // parameters than it is declared to take. This does not attempt to
    // take into account gratuitous differences in declared types,
    // though.
    std::vector<GenericValue> ActualArgs;
    const unsigned ArgCount = F->getFunctionType()->getNumParams();
    for (unsigned i = 0; i < ArgCount; ++i)
        ActualArgs.push_back(ArgValues[i]);

    // if first time call
    if(m_stillRunning == false)
    {
        // Set up the function call.
        callFunction(F, ActualArgs);
        // call Plugins
        for(PlugInIterator it = m_pPlugins.begin(); it != m_pPlugins.end(); ++it)
            (*it)->handlePreFunctionRun(ECStack, *this);
    }

    // Start executing the function.
    InterpreterPluggable::RETCODE ret = runWithPlugins();

    // if barrier mark flag still Running
    if(ret == InterpreterPluggable::BARRIER)
    {
        m_stillRunning = true;
        GenericValue retbarriercode;
        retbarriercode.IntVal = APInt(8, InterpreterPluggable::BARRIER);
        return retbarriercode;
    }
    
    // if not barrier it is function exit
    // call post function methods
    if(ret != InterpreterPluggable::BARRIER)
    {
       // if execution is not interrupted and it is normal exit
       // set running flag to false 
        m_stillRunning = false;
        // call post function event
        for(PlugInIterator it = m_pPlugins.begin(); it != m_pPlugins.end(); ++it)
            (*it)->handlePostFunctionRun();

    }

    return ExitValue;
}

// checks if I is intrinsic then it can be lowered
// at execution of Interpreter
static bool IsLoweringPossible(Instruction& I)
{
    if(I.getOpcode() == Instruction::Call)
    {
        CallSite CS = cast<Value>(&I);
        // Check to see if this is an intrinsic function call...
        Function *F = CS.getCalledFunction();
        if (F && F->isDeclaration())
        {
            switch (F->getIntrinsicID()) 
            {
            case Intrinsic::not_intrinsic:
                break;
            case Intrinsic::vastart: // va_start
                break;
            case Intrinsic::vaend:    // va_end is a noop for the interpreter
                break;
            case Intrinsic::vacopy:   // va_copy: dest = src
                break;
            default:
                // If it intrinsics it can be lowered
                return true;
            }
        }
        return false;
    }
    return false;
}
InterpreterPluggable::RETCODE InterpreterPluggable::runWithPlugins()
{
  while (!ECStack.empty()) {
    // Interpret a single instruction & increment the "PC".
    ExecutionContext &SF = ECStack.back();  // Current stack frame
    Instruction &I = *SF.CurInst;         // Increment before execute
    const bool isLoweringPossible = IsLoweringPossible(I);

    // Track the number of dynamic instructions executed.
    ++NumDynamicInsts;

    // hack to catch barrier instruction
    // note: in case of barrier instruction plug in methods are not called
    if(I.getOpcode() == Instruction::Call)
    {
        CallInst *C= cast<CallInst>(&I);
        // To handle indirect function call getOperandValue is used instead of casting to the Function pointer.
        Function *f = (Function*)GVTOP(getOperandValue(C->getCalledValue(), SF));

        if(f->getName() == "barrier")
        {
            DEBUG(dbgs() << "Barrier Detected: " << I << '\n');
            *SF.CurInst++;
            return InterpreterPluggable::BARRIER;
        }
    }

    // call Plug in pre-exec method
    for(PlugInIterator it = m_pPlugins.begin(); it != m_pPlugins.end(); ++it)
      (*it)->handlePreInstExecution(I);

    DEBUG(dbgs() << "About to interpret: " << I << '\n');

    // note: do not move following CurInst++ prior to calling NEAT plug-in
    // Plug ins depends on ExecutionContext unmodified prior to execution in interpreter
    *SF.CurInst++;

    // Dispatch to one of the visit* methods...
    visit(I);

    // if lowering of instruction I is possible
    // do not call post instruction method 
    // since I can be already removed by visit(I) method
    // during lowering. In this case reference to I is not valid
    if(isLoweringPossible == false)
    {
        // call Plug in post-exec method
        for(PlugInIterator it = m_pPlugins.begin(); it != m_pPlugins.end(); ++it)
            (*it)->handlePostInstExecution(I);
    }

  }
    return InterpreterPluggable::OK;
}
