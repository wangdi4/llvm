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
#include "ParameterType.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/CodeGen/IntrinsicLowering.h"
#include "llvm/IR/GetElementPtrTypeIterator.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/CodeGen/IntrinsicLowering.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Module.h"
#include "llvm/ADT/Statistic.h"

#include <algorithm>
#include <cmath>
#include <cstring>
using namespace llvm;

STATISTIC(NumDynamicInsts, "Number of dynamic instructions executed");

// TODO: add macro for switching on InterpreterPluggable
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
ExecutionEngine *InterpreterPluggable::create(std::unique_ptr<Module> M, std::string* ErrStr) {
    // Tell this Module to materialize everything and release the GVMaterializer.
    if (Error err = M->materializeAll()) {
        // We got an error, just return 0
        return nullptr;
    }

    return new InterpreterPluggable(std::move(M));
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
    // declare main() with fewer parameters than it actually gets
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
    if(ret == InterpreterPluggable::BARRIER || ret == InterpreterPluggable::BLOCKING_WG_FUNCTION)
    {
        m_stillRunning = true;
        GenericValue retcode;
        retcode.IntVal = APInt(8, ret);
        return retcode;
    }

    // if not barrier it is function exit
    // call post function methods
    if(ret != InterpreterPluggable::BARRIER && ret != InterpreterPluggable::BLOCKING_WG_FUNCTION)
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
        CallSite CS (cast<Value>(&I));
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
        std::string fName = f->getName();
        std::size_t found = fName.find("barrier");
        if (found!=std::string::npos)
        {
            DEBUG(dbgs() << "Barrier Detected: " << I << '\n');
            *SF.CurInst++;
            return InterpreterPluggable::BARRIER;
        }
        else if(m_WGBuiltinsNames.isWorkGroupBuiltin(fName))
        {
            DEBUG(dbgs() << "Blocking Work Group function Detected: " << I << '\n');
            //*SF.CurInst++; do not increment instruction counter
            //if flag is false - break execution and wait for WI
            //true - continue
            if(m_needToExecutePreMethod)
            {
                //create and execute call of _pre_exec method
                std::string pre_method_name = m_WGBuiltinsNames.getMangledPreExecMethodName(fName);
                FunctionType *CalledFT = C->getCalledFunction()->getFunctionType();
                std::vector<Type*> arg_types;
                for(FunctionType::param_iterator I = CalledFT->param_begin(),
                    E = CalledFT->param_end(); I!=E; ++I)
                    arg_types.push_back(*I);
                //construct function with void return type, call params taken from built-in description
                FunctionType *FT = FunctionType::get(Type::getVoidTy(C->getContext()), arg_types, false);
                Function *pre_method = cast<Function>(this->Modules[0]->getOrInsertFunction(pre_method_name, FT));
                pre_method->setCallingConv(CallingConv::C);
                std::vector<Value*> args;
                //provide same param values as in Called built-in
                for(uint32_t i = 0, e = C->getNumArgOperands(); i<e; ++i)
                {
                    args.push_back(C->getArgOperand(i));
                }
                Instruction *CIM = CallInst::Create(pre_method, args, "");
                visit(CIM);//call
                delete CIM;//cleanup. no-one is referencing to this instruction
            }
            else//proceed to built-in execution
            {
                // note: do not move following CurInst++ prior to calling NEAT plug-in
                // Plug ins depends on ExecutionContext unmodified prior to execution in interpreter
                *SF.CurInst++;

                // Dispatch to one of the visit* methods...
                visit(I);

            }
            //invert m_needToExecutePreMethod
            //next iteration - return InterpreterPluggable::BLOCKING_WG_FUNCTION and
            //execute function
            m_needToExecutePreMethod = !m_needToExecutePreMethod;
            return InterpreterPluggable::BLOCKING_WG_FUNCTION;
            //as execution of built-in is finished - wait for other work items to proceed
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
