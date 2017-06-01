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

File Name:  DebugInfoPass.cpp

\*****************************************************************************/

#include "BuiltinLibInfo.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"
#include "CompilationUtils.h"

#include <llvm/Pass.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/IntrinsicInst.h>

#include <list>
#include <vector>

using namespace llvm;
using namespace std;
using namespace Intel::OpenCL::DeviceBackend;

namespace intel  {


// Inserts debugging information for OpenCL debugging.
//
class DebugInfoPass : public ModulePass
{
public:
    DebugInfoPass()
        : ModulePass(ID), m_llvm_context(0)
    {
    }

    bool runOnModule(Module& M);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<BuiltinLibInfo>();
    }

    static char ID; // LLVM pass ID
private:
    struct FunctionContext;

    // Invoked on each user-implemented function in the module
    //
    void runOnUserFunction(Function* pFunc);

    // In case get_global_id wasn't declared, add its declaration to the module
    //
    void addGlobalIdDeclaration();

    // Adds declarations of the debug builtins to the module
    //
    void addDebugBuiltinDeclarations();

    // Adds instructions to the beginning of the given function to compute the
    // global IDs for 3 dimensions.
    // Fills in the FunctionContext.
    //
    void insertComputeGlobalIds(Function* pFunc, FunctionContext& fContext);

    // Adds the debug builtin calls to the function.
    // gids: Values computed for global IDs by computeGlobalIds
    //
    void addDebugCallsToFunction(Function* pFunc, const FunctionContext& fContext);

    // Insert a call to the stoppoint builtin before the given instruction.
    //
    void insertDbgStoppointCall(Instruction* instr, const FunctionContext& fContext);

    // Insert a call to the "enter function" builtin into the beginning of the
    // function.
    //
    void insertDbgEnterFunctionCall(Function* pFunc, const FunctionContext& fContext);

    // Insert a call to the "exit function" builtin before the given
    // instruction.
    //
    void insertDbgExitFunctionCall(Instruction* instr, Function* pFunc, const FunctionContext& fContext);

    // Insert calls to the "declare global" builtin into the beginning of the
    // function.
    //
    void insertDbgDeclareGlobalCalls(Function* pFunc, const FunctionContext& fContext);

    // Insert a call to the "declara local" builtin before the given call,
    // using that call's arguments (assuming it's a llvm.dbg.declare)
    //
    void insertDbgDeclaraLocalCall(DbgDeclareInst* call_instr, const FunctionContext& fContext);

    // Extract the subprogram descriptor metadata for the function. Return
    // the address to its metadata.
    //
    Value* extractSubprogramDescriptorMetadata(Function* pFunc);

    // Create a constant integer Value from the pointer. The Value encodes the
    // address the pointer points to. This is used to pass the address of
    // metadata nodes to native builtins.
    //
    Value* makeAddressValueFromPointer(Metadata * ptr) const;

    // Extract the number of the C source line from which this instruction was
    // compiled.
    //
    unsigned getCLinenoFromDbgMetadata(Instruction* instr);

    // Find function in the list of RTL modules
    Function* findFunctionsInModule(StringRef funcName) const;

private:

    // Convenience object passed around between methods of this pass.
    // gids:
    //      The values for computation of get_global_id[n] for n in 0..2,
    //      that were placed in the beginning of the function.
    // original_first_instr:
    //      The original first instruction of the function (before the gid
    //      computations were inserted)
    //
    struct FunctionContext {
        vector<Value*> gids;
        Instruction* original_first_instr;
    };

    Module* m_pModule;

    LLVMContext* m_llvm_context;
    SmallVector<Module*, 2> m_RtlModuleList;

    DebugInfoFinder m_DbgInfoFinder;

    // Names of the debugging info builtins
    //
    static const char* BUILTIN_DBG_DECLARE_LOCAL_NAME;
    static const char* BUILTIN_DBG_DECLARE_GLOBAL_NAME;
    static const char* BUILTIN_DBG_ENTER_FUNCTION_NAME;
    static const char* BUILTIN_DBG_EXIT_FUNCTION_NAME;
    static const char* BUILTIN_DBG_STOPPOINT_NAME;
};

char DebugInfoPass::ID = 0;

const char* DebugInfoPass::BUILTIN_DBG_DECLARE_LOCAL_NAME = "__opencl_dbg_declare_local";
const char* DebugInfoPass::BUILTIN_DBG_DECLARE_GLOBAL_NAME = "__opencl_dbg_declare_global";
const char* DebugInfoPass::BUILTIN_DBG_ENTER_FUNCTION_NAME = "__opencl_dbg_enter_function";
const char* DebugInfoPass::BUILTIN_DBG_EXIT_FUNCTION_NAME = "__opencl_dbg_exit_function";
const char* DebugInfoPass::BUILTIN_DBG_STOPPOINT_NAME = "__opencl_dbg_stoppoint";


Function* DebugInfoPass::findFunctionsInModule(StringRef funcName) const {
    for (SmallVector<Module*, 2>::const_iterator it = m_RtlModuleList.begin();
        it != m_RtlModuleList.end();
        ++it) {
        Function* pRetFunction = (*it)->getFunction(funcName);
        if (pRetFunction != NULL)
            return pRetFunction;
    }
    return NULL;
}

bool DebugInfoPass::runOnModule(Module& M)
{
    m_llvm_context = &M.getContext();
    m_RtlModuleList = getAnalysis<intel::BuiltinLibInfo>().getBuiltinModules();
    assert(m_RtlModuleList.size() != 0 && "Empty module list in DebugInfoPass::runOnModule");
    m_pModule = &M;

    // Prime a DebugInfoFinder that can be queried about various bits of
    // debug information in the module.
    m_DbgInfoFinder = DebugInfoFinder();
    m_DbgInfoFinder.processModule(M);

    addGlobalIdDeclaration();
    addDebugBuiltinDeclarations();

    // Run runOnUserFunction on all the functions in the module.
    //
    Module::iterator func_iter = m_pModule->begin();
    for (; func_iter != m_pModule->end(); ++func_iter) {
        // Ignore declarations and builtins placed from the RT module.
        //
        if (func_iter->isDeclaration() || findFunctionsInModule(func_iter->getName()))
            continue;

        runOnUserFunction(&(*func_iter));
    }
    return true;
}


void DebugInfoPass::addGlobalIdDeclaration()
{
    std::string gid =
      Intel::OpenCL::DeviceBackend::CompilationUtils::mangledGetGID();
    if (findFunctionsInModule(gid))
        return;

    // No such declaration; let's add it
    //
    Type* i_size_t = IntegerType::get(*m_llvm_context, 8 * sizeof(size_t));
    Type* uint_type = IntegerType::get(*m_llvm_context, 32);
    vector<Type*> params;
    params.push_back(uint_type);

    FunctionType* gid_type = FunctionType::get(i_size_t, params, false);
    Function::Create(gid_type, Function::ExternalLinkage, gid, m_pModule);
}


void DebugInfoPass::addDebugBuiltinDeclarations()
{
    Type* pointer_i8 = IntegerType::getInt8PtrTy(*m_llvm_context);
    Type* i64 = IntegerType::getInt64Ty(*m_llvm_context);
    Type* void_type = Type::getVoidTy(*m_llvm_context);

    // BUILTIN_DBG_DECLARE_LOCAL_NAME
    // Arguments of this builtin:
    // 1. Pointer to the variable alloca
    // 2. Address of the metadata for variable
    // 3. Address of the metadata for complex expression
    // 4-6. global IDs for dimensions 0-2
    //
    vector<Type*> declare_params;
    declare_params.push_back(pointer_i8);
    for (int i = 0; i < 5; ++i)
      declare_params.push_back(i64);
    FunctionType* local_functype = FunctionType::get(void_type, declare_params, false);
    Function::Create(local_functype, Function::ExternalLinkage,
        BUILTIN_DBG_DECLARE_LOCAL_NAME, m_pModule);

    // BUILTIN_DBG_DECLARE_GLOBAL_NAME
    // Arguments: similar to BUILTIN_DBG_DECLARE_LOCAL_NAME
    // except it doesn't have complex expression
    declare_params.pop_back();
    FunctionType* global_functype = FunctionType::get(void_type, declare_params, false);
    Function::Create(global_functype, Function::ExternalLinkage,
        BUILTIN_DBG_DECLARE_GLOBAL_NAME, m_pModule);

    // BUILTIN_DBG_ENTER_FUNCTION_NAME
    // Arguments of this builtin:
    // 1. Address of the metadata for the subprogram entry
    // 2-4. global IDs for dimensions 0-2
    //
    vector<Type*> enter_params;
    for (int i = 0; i < 4; ++i)
        enter_params.push_back(i64);
    FunctionType* enter_functype = FunctionType::get(void_type, enter_params, false);
    Function::Create(enter_functype, Function::ExternalLinkage,
        BUILTIN_DBG_ENTER_FUNCTION_NAME, m_pModule);

    // BUILTIN_DBG_EXIT_FUNCTION_NAME
    // Arguments: similar to BUILTIN_DBG_ENTER_FUNCTION_NAME
    //
    Function::Create(enter_functype, Function::ExternalLinkage,
        BUILTIN_DBG_EXIT_FUNCTION_NAME, m_pModule);

    // BUILTIN_DBG_STOPPOINT_NAME
    // Arguments: similar to BUILTIN_DBG_ENTER_FUNCTION_NAME
    //
    Function::Create(enter_functype, Function::ExternalLinkage,
        BUILTIN_DBG_STOPPOINT_NAME, m_pModule);
}


void DebugInfoPass::runOnUserFunction(Function* pFunc)
{
    FunctionContext fContext;
    insertComputeGlobalIds(pFunc, fContext);

    addDebugCallsToFunction(pFunc, fContext);
}


void DebugInfoPass::insertComputeGlobalIds(Function* pFunc, FunctionContext& fContext)
{
    const std::string gid =
      Intel::OpenCL::DeviceBackend::CompilationUtils::mangledGetGID();
    Type* uint_type = IntegerType::get(*m_llvm_context, 32);
    Function* get_global_id_func = m_pModule->getFunction(gid);
    assert(get_global_id_func);

    // Find the first instruction in the function
    //
    BasicBlock& entry_block = pFunc->getEntryBlock();
    Instruction& first_instr = entry_block.front();

    vector<Value*> gids;
    for (unsigned i = 0; i <= 2; ++i) {
        Value* const_dim = ConstantInt::get(uint_type, i, false);
        Value* gid_at_dim = CallInst::Create(
            get_global_id_func, const_dim,
            Twine(gid) + Twine(i), &first_instr);

        // get_global_id returns size_t, but we want to always pass a 64-bit
        // number. So we may need to extend the result to 64 bits.
        //
        const IntegerType* gid_type = dyn_cast<IntegerType>(gid_at_dim->getType());
        if (gid_type && gid_type->getBitWidth() != 64) {
            Value* zext_gid = new ZExtInst(
                gid_at_dim,
                IntegerType::getInt64Ty(*m_llvm_context),
                Twine("gid") + Twine(i) + Twine("_i64"),
                &first_instr);
            gids.push_back(zext_gid);
        }
        else {
            gids.push_back(gid_at_dim);
        }
    }

    fContext.gids = gids;
    fContext.original_first_instr = &first_instr;
}


void DebugInfoPass::addDebugCallsToFunction(Function* pFunc, const FunctionContext& fContext)
{
    // Try to find this function in the debug information metadata.
    // If it's not there - it's not a function the user compiled, so we
    // don't instrument it.
    //
    Value* func_metadata = extractSubprogramDescriptorMetadata(pFunc);
    if (!func_metadata) {
        return;
    }

    // A note on the order of instructions inserted into the function:
    // fContext.original_first_instr contains the original first instruction
    // of the function, before the GID computations have been added (they are
    // added *before* this first instruction).
    // When we want to add other instructions to the beginning of the function
    // we'll use this original_first_instr as the "before instruction" argument
    // in LLVM instruction constructors. This way, the new instructions will
    // be added *in order* one after the other, before the first instruction.
    //

    // Insert a call to the "enter function" builtin at the beginning.
    //
    insertDbgEnterFunctionCall(pFunc, fContext);

    // Declare global vars
    //
    insertDbgDeclareGlobalCalls(pFunc, fContext);

    list<Instruction*> instrs_to_remove;
    unsigned prev_c_lineno = 0;

    // Go over all instructions in the function:
    // * Replace local variable declarations by builtin calls
    // * Insert stoppoint calls
    // * Insert function exit builtins before returns
    //
    for (auto &BB : *pFunc) {
        for (auto &I : BB) {
            auto pInst = &I;
            // To insert stoppoints:
            // - Find the current C line for this instruction
            // - If the line was found and it's greater ( greater because we don't want
            //   to stop on declaration of variable imported by block scope) than the previous
            //   saved C line, insert a stoppoint.
            //
            unsigned cur_c_lineno = getCLinenoFromDbgMetadata(pInst);
            if (cur_c_lineno != 0) {
                // Currently this will insert a stoppoint also for
                // declarations in the beginning of a function, including
                // declarations of function arguments.
                //
                if (cur_c_lineno > prev_c_lineno){
                    insertDbgStoppointCall(pInst, fContext);
                    prev_c_lineno = cur_c_lineno;
                }
            }

            // Now handle specific instructions. We're interested in:
            // 1. Call instructions to llvm.dbg.declare, which will be replaced
            //    by a debug builtin call.
            // 2. Return instruction, before which a builtin call will be
            //    inserted.
            //
            if (DbgDeclareInst * dbgDeclareInst = dyn_cast<DbgDeclareInst>(pInst)) {
                // The new call is inserted before the existing call, and
                // the existing call is scheduled for removal.
                //
                insertDbgDeclaraLocalCall(dbgDeclareInst, fContext);
                instrs_to_remove.push_back(dbgDeclareInst);
            }
            else if (ReturnInst* ret_instr = dyn_cast<ReturnInst>(pInst)) {
                insertDbgExitFunctionCall(ret_instr, pFunc, fContext);
            }
        }
    }

    // Clean up the instructions scheduled for removal
    //
    for (list<Instruction*>::const_iterator i = instrs_to_remove.begin();
         i != instrs_to_remove.end(); ++i) {
        (*i)->eraseFromParent();
    }
}


Value* DebugInfoPass::extractSubprogramDescriptorMetadata(Function* pFunc)
{
    for (auto subprogDIE : m_DbgInfoFinder.subprograms()) {
        if (subprogDIE->describes(pFunc)) {
            return makeAddressValueFromPointer(subprogDIE);
        }
    }

    return 0;
}


Value* DebugInfoPass::makeAddressValueFromPointer(Metadata * ptr) const
{
    uint64_t addr = reinterpret_cast<uint64_t>(ptr);
    return ConstantInt::get(
        IntegerType::getInt64Ty(m_pModule->getContext()), addr);
}


void DebugInfoPass::insertDbgEnterFunctionCall(Function* pFunc, const FunctionContext& fContext)
{
    Function* enter_function_func = m_pModule->getFunction(BUILTIN_DBG_ENTER_FUNCTION_NAME);
    assert(enter_function_func);

    Value* subprogram_md_addr = extractSubprogramDescriptorMetadata(pFunc);
    assert(subprogram_md_addr && "Did not find subprogram descriptor");

    vector<Value*> params;
    params.push_back(subprogram_md_addr);
    for (int i = 0; i <= 2; ++i)
        params.push_back(fContext.gids[i]);
    CallInst::Create(enter_function_func, params, "",
        fContext.original_first_instr);
}


void DebugInfoPass::insertDbgDeclareGlobalCalls(Function* pFunc,
                                const FunctionContext& fContext)
{
    Function* GlobalFuncDecl = m_pModule->
                               getFunction(BUILTIN_DBG_DECLARE_GLOBAL_NAME);
    assert(GlobalFuncDecl && "GlobalFuncDecl can't be nullptr");
    Type* Ptri8 = IntegerType::getInt8PtrTy(*m_llvm_context);

    // Generate the builtin call for each global var in the module
    for (auto DIE : m_DbgInfoFinder.global_variables()) {

        // Take the var address from the metadata as a Value, and bitcast it
        // to i8*.
        Value *GVasValue = nullptr;
        if (auto *V = cast_or_null<ValueAsMetadata>(DIE->getRawVariable()))
            GVasValue = dyn_cast<Value>(V->getValue());

        // Some special globals are inserted by clang with a non-pointer value
        // (for example samplers). We currently don't know how to handle them.
        //
        if (!GVasValue->getType()->isPointerTy())
            continue;

        CastInst* VarAddr = CastInst::CreatePointerCast(GVasValue,
                            Ptri8, "var_addr", fContext.original_first_instr);

        MDNode *MDN = MDNode::get(*m_llvm_context, 
            ConstantAsMetadata::get(
              ConstantInt::getAllOnesValue(
                IntegerType::getInt1Ty(*m_llvm_context))));
        VarAddr->setMetadata("dbg_declare_inst", MDN);

        // The metadata itself is passed as an address
        Value* MDAddr = makeAddressValueFromPointer(DIE);

        vector<Value*> Params;
        Params.push_back(VarAddr);
        Params.push_back(MDAddr);
        for (int i = 0; i <= 2; ++i)
            Params.push_back(fContext.gids[i]);
        // [LLVM 3.6 UPGRADE] FIXME: Most likely the new call site mist be
        // additionally handled somewhere else to properly handle the
        // Metadata/Value split.
        CallInst::Create(GlobalFuncDecl, Params, "",
            fContext.original_first_instr);
    }
}


unsigned DebugInfoPass::getCLinenoFromDbgMetadata(Instruction* instr)
{
    if (MDNode* mdn = instr->getMetadata("dbg")) {
        DebugLoc di_loc(mdn);
        return di_loc.getLine();
    }
    return 0;
}


void DebugInfoPass::insertDbgStoppointCall(Instruction* instr, const FunctionContext& fContext)
{
    MDNode* mdn = instr->getMetadata("dbg");
    assert(mdn && "Expected an instruction with dbg metadata");

    Function* stoppoint_func = m_pModule->getFunction(BUILTIN_DBG_STOPPOINT_NAME);
    assert(stoppoint_func);
    vector<Value*> params;
    params.push_back(makeAddressValueFromPointer(mdn));
    for (int i = 0; i <= 2; ++i)
        params.push_back(fContext.gids[i]);
    CallInst::Create(stoppoint_func, params, "", instr);
}


void DebugInfoPass::insertDbgDeclaraLocalCall(DbgDeclareInst* dbgDeclInstr, const FunctionContext& fContext)
{
    // The first argument is an alloca for the variable.
    // Take the alloca from the metadata and cast it to i8*
    Type* pointer_i8 = IntegerType::getInt8PtrTy(*m_llvm_context);
    CastInst* var_ref_cast = CastInst::CreatePointerCast(dbgDeclInstr->getAddress(), pointer_i8,
                                                         "", dbgDeclInstr);
    // Pass address of metadata description of the variable as an integral argument.
    Value* var_metadata_addr = makeAddressValueFromPointer(dbgDeclInstr->getVariable());
    // Pass the metadata description of the complex expression address as an integral
    // argument.
    Value* expr_metadata_addr = makeAddressValueFromPointer(dbgDeclInstr->getExpression());

    Function* declare_local_func = m_pModule->getFunction(BUILTIN_DBG_DECLARE_LOCAL_NAME);
    assert(declare_local_func);
    vector<Value*> params;
    params.push_back(var_ref_cast);
    params.push_back(var_metadata_addr);
    params.push_back(expr_metadata_addr);
    for (int i = 0; i < 3; ++i)
        params.push_back(fContext.gids[i]);
    CallInst::Create(declare_local_func, params, "", dbgDeclInstr);
}


void DebugInfoPass::insertDbgExitFunctionCall(Instruction* instr, Function* pFunc, const FunctionContext& fContext)
{
    Function* exit_function_func = m_pModule->getFunction(BUILTIN_DBG_EXIT_FUNCTION_NAME);
    assert(exit_function_func);

    Value* subprogram_md_addr = extractSubprogramDescriptorMetadata(pFunc);
    assert(subprogram_md_addr && "Did not find subprogram descriptor");

    vector<Value*> params;
    params.push_back(subprogram_md_addr);
    for (int i = 0; i <= 2; ++i)
        params.push_back(fContext.gids[i]);
    CallInst::Create(exit_function_func, params, "", instr);
}

OCL_INITIALIZE_PASS_BEGIN(DebugInfoPass, "debug-info", "Debug Info", false, false)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
OCL_INITIALIZE_PASS_END(DebugInfoPass, "debug-info", "Debug Info", false, false)

} // namespace intel {

extern "C"
{
    ModulePass* createDebugInfoPass()
    {
        return new intel::DebugInfoPass();
    }
}

