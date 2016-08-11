/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "BuiltInFuncImport.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"

#include <llvm/IR/Module.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include "llvm/Transforms/Utils/GlobalStatus.h"
#include <llvm/IR/Instruction.h>
#include "llvm/IR/InstIterator.h"
#include <llvm/IR/Instructions.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Debug.h>
#include <llvm/IR/Verifier.h>

#include <string>

using namespace std;
using namespace llvm;

namespace intel {

  char BIImport::ID = 0;

  OCL_INITIALIZE_PASS_BEGIN(BIImport, "builtin-import", "Built-in function pass", false, true)
  OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
  OCL_INITIALIZE_PASS_END(BIImport, "builtin-import", "Built-in function pass", false, true)

  BIImport::BIImport(const char* CPUPrefix)
    : ModulePass(ID), m_cpuPrefix(CPUPrefix)
  { }

  Function* BIImport::FindFunctionBodyInModules(const std::string& funcName) const
  {
    for (auto rtModule : m_runtimeModuleList)
    {
      assert(rtModule && "NULL pointer detected in BIImport::FindFunctionInModules");

      Function* pRetFunction = rtModule->getFunction(funcName);

      // Test if the function body is contained in this module.
      if (pRetFunction && !pRetFunction->isDeclaration())
        return pRetFunction;
    }
    return nullptr;
  }

  // this function replaces keyword "shared" in the builtin name by current CPU prefix, for example:
  // if CPU is l9, __ocl_svml_shared_acos1f to be changed to __ocl_svml_l9_acos1f
  void BIImport::UpdateSvmlBuiltinName(Function* fn, const char* pCPUPrefix) const
  {
    llvm::StringRef fName = fn->getName();
    if (fName.startswith("__ocl_svml_shared"))
    {
      std::string s = fName.str();
      s.replace(11, 6, pCPUPrefix);
      fn->setName(s);
    }
  }

  void BIImport::GetCalledFunctions(const Function* pFunc, TFunctionsVec& calledFuncs) const
  {
    TFunctionsSet visitedSet;

    // Iterate over function instructions and look for call instructions
    for (auto &I : instructions(pFunc))
    {
      const CallInst *pInstCall = dyn_cast<CallInst>(&I);
      if (!pInstCall) continue;

      Function* pCalledFunc = pInstCall->getCalledFunction();
      if (!pCalledFunc)
      {
        // This case can occur only if CallInst is calling something other than LLVM function,
        // meaning the call is indirect. We need to check if a called value is ConstantExpr that can
        // use the function defined in source module.
        auto CE = dyn_cast<ConstantExpr>(pInstCall->getCalledValue());
        if (CE && CE->getOpcode() == Instruction::BitCast) {
          Value* CEOperand = CE->getOperand(0);
          if (auto CEFuncOperand = dyn_cast<Function>(CEOperand))
            pCalledFunc = CEFuncOperand;
          else
            continue;
        }
        else
        {
          continue;
        }
      }

      if (visitedSet.count(pCalledFunc)) continue;

      // skip svml name renaming when empty cpu prefix is provided.
      if (!m_cpuPrefix.empty())
        UpdateSvmlBuiltinName(pCalledFunc, m_cpuPrefix.c_str());

      visitedSet.insert(pCalledFunc);
      calledFuncs.push_back(pCalledFunc);
    }
  }

  static bool materialized_use_empty(const Value *v)
  {
    return v->materialized_use_begin() == v->use_end();
  }

  // nuke the unused globals so we could materializeAll() quickly
  void BIImport::CleanUnusedGlobalsInitializers (Module *src_module) const
  {
    // Linker by default imports all globals, hence the functions that are stored
    // as fp pointer there. To workaround this we delete unneeded GVs from src_module.
    for (auto &GV : src_module->globals())
    {
      if (GV.hasInitializer()) {
        bool has_materialized_uses_in_rt_modules = false;
        for (auto M : m_runtimeModuleList)
        {
          auto srsGv = M->getGlobalVariable(GV.getName());
          if (srsGv && !materialized_use_empty(srsGv))
            has_materialized_uses_in_rt_modules |= true;
        }
        if (!has_materialized_uses_in_rt_modules) {
          Constant *Init = GV.getInitializer();
          GV.setInitializer(nullptr);
          if (isSafeToDestroyConstant(Init))
            Init->destroyConstant();
        }
      }
    }
  }

  // nuke the unused functions so we could materializeAll() quickly
  void BIImport::CleanUnusedFunctionsBodies (Module *src_module) const
  {
    for (auto I = src_module->begin(), E = src_module->end(); I != E; )
    {
      auto *F = &(*I++);
      if (F->isDeclaration() || F->isMaterializable())
      {
        if (materialized_use_empty(F))
        {
          F->deleteBody();
        }
      }
    }
  }

  bool BIImport::runOnModule(Module &M) {
    BuiltinLibInfo &BLI = getAnalysis<BuiltinLibInfo>();
    auto rtlModuleBufferList = BLI.getBuiltinModuleBuffers();

    if (rtlModuleBufferList.empty()) {
      // If there are no builtin modules, then nothing can be imported.
      return false;
    }

    // Initialize members
    m_UserModuleFunctions.clear();

    // Copy buffers containing builtins bitcode so we could safely delete functions bodies
    // in order to achieve faster materializing prior to linking.
    // The lifetime of these copies is limitied to this function.

    vector<unique_ptr<MemoryBuffer>> rtlModuleBufferListCopy;
    for (auto rtlBuffer : rtlModuleBufferList)
    {
      auto rtlBufferCopy = MemoryBuffer::getMemBufferCopy(rtlBuffer->getBuffer(), rtlBuffer->getBufferIdentifier());
      rtlModuleBufferListCopy.push_back(std::move(rtlBufferCopy));
    }

    vector<unique_ptr<Module>> rtlModulesList;
    for (auto &runtimeBuffer : rtlModuleBufferListCopy)
    {
      // We could use getLazyIRModule to be able to handle not only bitcode
      // as it handles both bitcode and assembly, but it is internal to IRReader.cpp.
      // ToDo: make a patch, try to upstream.
      llvm::ErrorOr<std::unique_ptr<llvm::Module>> spModuleOrErr(llvm::getLazyBitcodeModule(std::move(runtimeBuffer), M.getContext()));
      if (!spModuleOrErr)
      {
        assert(false && "Error while getLazyBitcodeModule in BIImport");
      }
      else
      {
        rtlModulesList.push_back(std::move(spModuleOrErr.get()));
      }
    }

    for (auto &rtlModule : rtlModulesList)
      m_runtimeModuleList.push_back(rtlModule.get());

    // Copy target triple from dst module to avoid linker warnings
    for (auto &rtlModule : rtlModulesList)
    {
      rtlModule.get()->setTargetTriple(M.getTargetTriple());
      rtlModule.get()->setDataLayout(M.getDataLayout());
    }

    // Remember user module function pointers, so we could set linkonce_odr
    // to only imported functions.
    for (auto &F : M)
      if (!F.isDeclaration())
        m_UserModuleFunctions.insert(&F);

    bool changed = false;

    std::function<void(Function*)> Explore = [&](Function *pRoot) -> void
    {
      TFunctionsVec calledFuncs;
      GetCalledFunctions(pRoot, calledFuncs);

      for (auto *pCallee : calledFuncs)
      {
        Function *pFunc = nullptr;
        if (pCallee->isDeclaration())
        {
          auto funcName = pCallee->getName();
          Function* pSrcFunc = FindFunctionBodyInModules(funcName);
          if (!pSrcFunc) continue;
          pFunc = pSrcFunc;
        }
        else
        {
          pFunc = pCallee;
        }

        if (pFunc->isMaterializable())
        {
          changed = true;
          pFunc->materialize();
          Explore(pFunc);
        }
      }
    };

    for (auto &func : M)
    {
      Explore(&func);
    }

    // nuke the unused globals so we could materializeAll() quickly
    for (auto rtlModule : m_runtimeModuleList)
      CleanUnusedGlobalsInitializers(rtlModule);

    // Collect the functions mentioned in the globals.
    TFunctionsVec glbsFuncList;
    for (auto rtlModule : m_runtimeModuleList)
      for (auto &GV : rtlModule->globals())
        if (GV.hasInitializer())
          if (auto CA = dyn_cast<ConstantArray>(GV.getInitializer()))
            for (auto &operand : CA->operands())
            {
              auto func = dyn_cast<Function>(operand);
              if (!func->isDeclaration())
                glbsFuncList.push_back(func);
            }

    // Explore those functions
    for (auto func : glbsFuncList)
    {
      func->materialize();
      Explore(func);
    }

    // nuke the unused functions so we could materializeAll() quickly
    for (auto rtlModule : m_runtimeModuleList)
      CleanUnusedFunctionsBodies(rtlModule);

    for (auto rtlModule : m_runtimeModuleList)
      rtlModule->materializeAll();

    // now perform the linking itself
    Linker ld(M);

    for (auto &rtlModule : rtlModulesList)
    {
      // the flag Linker::OverrideFromSrc is needed as globals
      // can have initializers in both modules.
      if (ld.linkInModule(std::move(rtlModule), Linker::OverrideFromSrc))
      {
        assert(false && "Error linking builtin module!");
      }
    }

    rtlModulesList.clear();

    // Allow removal of function from module after it is inlined
    for (auto &F : M)
      if (!m_UserModuleFunctions.count(&F) && !F.isDeclaration())
        F.setLinkage(GlobalVariable::LinkOnceODRLinkage);

    // At link time we have a shared.rtl (with common built-ins) compiled for
    // SSE and target.rtl compiled for SSE, AVX or AVX2. Built-ins from
    // shared.rtl and target.rtl will have different "target-cpu" and
    // "target-features" function attributes, so CodeGen will generate SSE code
    // for shared built-ins and AVX2 for target ones. These built-ins will have
    // different ABI, producing incorrect results at run-time.
    //
    // We need to remove these attributes and allow CodeGen to generate all
    // built-ins for a single (target) architecture.
    //
    // This also fixes an issue with inlining, where built-ins with different
    // targets cannot be inlined together.
    const char *TargetAttrs[] = {"target-cpu", "target-features"};

    AttributeSet IgnoreAttrs;
    for (auto A : TargetAttrs) {
      IgnoreAttrs = IgnoreAttrs.addAttribute(
          M.getContext(), AttributeSet::FunctionIndex, A);

    }

    for (auto &F : M) {
      F.removeAttributes(AttributeSet::FunctionIndex, IgnoreAttrs);
    }

    return changed;
  }

} //namespace intel {

extern "C" llvm::ModulePass* createBuiltInImportPass(const char* CPUPrefix) {
  return new intel::BIImport(CPUPrefix);
}
