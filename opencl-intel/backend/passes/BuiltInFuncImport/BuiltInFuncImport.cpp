/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "BuiltInFuncImport.h"
#include "CompilationUtils.h"
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

  BIImport::BIImport(const char *CPUPrefix)
    : ModulePass(ID), m_cpuPrefix(CPUPrefix)
  { }

  static Function *FindFunctionDef(const Function *F,
                                   SmallVectorImpl<Module *> &Modules) {
    assert(F && "Invalid function.");
    for (auto M : Modules ) {
      assert(M && "Invalid module.");

      Function* Ret = M->getFunction(F->getName());

      // Test if the function body is contained in this module.
      if (Ret && !Ret->isDeclaration()) {
        return Ret;
      }
    }
    return nullptr;
  }

  static GlobalVariable *FindGlobalDef(const GlobalVariable *GV,
                                       SmallVectorImpl<Module *> &Modules) {
    assert(GV && "Invalid global variable.");
    for (auto M : Modules ) {
      assert(M && "Invalid module.");

      auto Ret = M->getGlobalVariable(GV->getName());

      // check if it is a definition
      if (Ret && Ret->hasInitializer()) {
        return Ret;
      }
    }

    return nullptr;
  }

  // this function replaces keyword "shared" in the builtin name by
  // current CPU prefix, for example:
  // if CPU is l9, __ocl_svml_shared_acos1f to be changed to
  // __ocl_svml_l9_acos1f
  void BIImport::UpdateSvmlBuiltinName(Function *F, const char *CPUPrefix) const
  {
    llvm::StringRef FName = F->getName();
    if (FName.startswith("__ocl_svml_shared"))
    {
      std::string NewName = FName.str();
      NewName.replace(11, 6, CPUPrefix);
      F->setName(NewName);
    }
  }

  void BIImport::GetCalledFunctions(const Function *F,
                                    FunctionsVec &CalledFuncs) const
  {
    FunctionsSet VisitedSet;

    // Iterate over function instructions and look for call instructions
    for (auto &I : instructions(F))
    {
      const CallInst *InstCall = dyn_cast<CallInst>(&I);
      if (!InstCall) continue;

      Function* CalledFunc = InstCall->getCalledFunction();
      if (!CalledFunc)
      {
        // This case can occur only if CallInst is calling something other than
        // LLVM function, meaning the call is indirect. We need to check if a
        // called value is ConstantExpr that can use the function defined in
        // source module.
        auto CE = dyn_cast<ConstantExpr>(InstCall->getCalledValue());
        if (CE && CE->getOpcode() == Instruction::BitCast) {
          Value *CEOperand = CE->getOperand(0);
          if (auto CEFuncOperand = dyn_cast<Function>(CEOperand))
            CalledFunc = CEFuncOperand;
          else
            continue;
        }
        else
        {
          continue;
        }
      }

      if (VisitedSet.count(CalledFunc)) continue;

      // skip svml name renaming when empty cpu prefix is provided.
      if (!m_cpuPrefix.empty())
        UpdateSvmlBuiltinName(CalledFunc, m_cpuPrefix.c_str());

      VisitedSet.insert(CalledFunc);
      CalledFuncs.push_back(CalledFunc);
    }
  }

  static void ExploreOperand(Value *Op,
                             SmallVectorImpl<Module*> &Modules,
                             SmallPtrSetImpl<GlobalValue*> &UsedFunctions,
                             SmallPtrSetImpl<GlobalVariable*> &UsedGlobals) {
    // operand may be a ConstantExpr, so we need to recursively check its
    // operands
    if (auto *CE = dyn_cast<ConstantExpr>(Op)) {
      for (size_t i = 0; i < CE->getNumOperands(); ++i) {
        ExploreOperand(CE->getOperand(i), Modules, UsedFunctions, UsedGlobals);
      }
      return;
    }

    if (auto GV = dyn_cast<GlobalVariable>(Op))
      if (auto G = FindGlobalDef(GV, Modules))
        UsedGlobals.insert(G);
  }

  void BIImport::ExploreUses(Function *Root,
                             SmallVectorImpl<Module*> &Modules,
                             SmallPtrSetImpl<GlobalValue*> &UsedFunctions,
                             SmallPtrSetImpl<GlobalVariable*> &UsedGlobals) {
    assert(Root && "Invalid function.");

    if (Root->isDeclaration()) {
      Root = FindFunctionDef(Root, Modules);
      if (!Root) {
        return;
      }
    }

    bool FirstUse = UsedFunctions.insert(Root).second;
    if (!FirstUse) {
      return;
    }

    if (Root->isMaterializable()) {
      Root->materialize();
    }

    FunctionsVec CalledFuncs;
    GetCalledFunctions(Root, CalledFuncs);

    for (auto Callee : CalledFuncs) {
      ExploreUses(Callee, Modules, UsedFunctions, UsedGlobals);
    }

    for (const BasicBlock &BB : *Root)
      for (const Instruction &I : BB)
        for (Value *Op : I.operands())
          ExploreOperand(Op, Modules, UsedFunctions, UsedGlobals);
  }

  static std::unique_ptr<Module>
  CloneModuleOnlyRequired(const Module *M, ValueToValueMapTy &VMap,
                          SmallPtrSetImpl<GlobalValue*> &ReqFunctions,
                          SmallPtrSetImpl<GlobalVariable*> &ReqGlobals) {
    std::unique_ptr<Module> New =
      llvm::make_unique<Module>(M->getModuleIdentifier(), M->getContext());

    New->setDataLayout(M->getDataLayout());
    New->setTargetTriple(M->getTargetTriple());
    New->setModuleInlineAsm(M->getModuleInlineAsm());

    // Create globals without initializers - they may contain function
    // calls which have not been cloned yet.
    for (auto GV : ReqGlobals) {
      if (GV->getParent() != M) {
        continue;
      }
      GlobalVariable *NewGV = new GlobalVariable(
        *New,
        GV->getType()->getElementType(),
        GV->isConstant(), GV->getLinkage(),
        (Constant*) nullptr, GV->getName(),
        (GlobalVariable*) nullptr,
        GV->getThreadLocalMode(),
        GV->getType()->getAddressSpace());

      NewGV->copyAttributesFrom(GV);
      VMap[GV] = NewGV;
    }

    // Now do the same with the required functions
    for (auto FGV : ReqFunctions) {
      if (FGV->getParent() != M) {
        continue;
      }

      auto F = cast<Function>(FGV);
      Function *NF =
        Function::Create(cast<FunctionType>(F->getType()->getElementType()),
                         F->getLinkage(), F->getName(), New.get());
      NF->copyAttributesFrom(NF);
      VMap[F] = NF;
    }

    // Clone global initializers
    for (auto GV : ReqGlobals) {
      if (GV->getParent() != M) {
        continue;
      }

      auto NewGV = cast<GlobalVariable>(VMap[GV]);

      if (GV->hasInitializer()) {
        NewGV->setInitializer(MapValue(GV->getInitializer(), VMap));
      }
    }

    // ... and the functions bodies
    for (auto FGV : ReqFunctions) {
      if (FGV->getParent() != M) {
        continue;
      }

      auto F = cast<Function>(FGV);
      Function *NF = cast<Function>(VMap[F]);

      // Track args changes
      Function::arg_iterator DestI = NF->arg_begin();
      for (const auto &Arg : F->args()) {
        DestI->setName(Arg.getName());
        VMap[&Arg] = &*DestI++;
      }

      SmallVector<ReturnInst*, 8> Returns;  // Ignore returns cloned.
      CloneFunctionInto(NF, F, VMap, /*ModuleLevelChanges=*/true, Returns);

      if (F->hasPersonalityFn())
        NF->setPersonalityFn(MapValue(F->getPersonalityFn(), VMap));
    }

    return New;
  }

  bool BIImport::runOnModule(Module &M) {
    BuiltinLibInfo &BLI = getAnalysis<BuiltinLibInfo>();
    m_runtimeModuleList = BLI.getBuiltinModules();

    FunctionsSet UserModuleFunctions;
    // Remember user module function pointers, so we could set linkonce_odr
    // to only imported functions.
    for (auto &F : M)
      if (!F.isDeclaration())
        UserModuleFunctions.insert(&F);

    const int EST_FUNCTIONS_NUM = 64;
    const int EST_GLOBALS_NUM = 64;
    SmallPtrSet<GlobalValue*, EST_FUNCTIONS_NUM> UsedFunctions;
    SmallPtrSet<GlobalVariable*, EST_GLOBALS_NUM> UsedGlobals;

    for (auto &F : M) {
      ExploreUses(&F, m_runtimeModuleList, UsedFunctions, UsedGlobals);
    }

    // Globals can have other function calls in their initializers,
    // which can have other globals in their bodies, so we must loop
    // until no new globals discovered.
    size_t GlobalsNumBefore;
    do {
      GlobalsNumBefore = UsedGlobals.size();
      for (auto GV : UsedGlobals) {
        if (GV->hasInitializer()) {
          Constant *Init = GV->getInitializer();
          for (auto &Op : Init->operands())
            if (auto func = dyn_cast<Function>(Op))
              ExploreUses(func, m_runtimeModuleList,
                          UsedFunctions, UsedGlobals);
        }
      }
    } while (GlobalsNumBefore < UsedGlobals.size());

    // We now known which globals and functions we need.
    // Lets clone rtl modules and filter out everything we don't need.
    SmallVector<std::unique_ptr<Module>, 2> ClonedRtlModules;
    ValueToValueMapTy VMap;
    for (auto RTL : m_runtimeModuleList) {
      ClonedRtlModules.push_back(
        CloneModuleOnlyRequired(RTL, VMap, UsedFunctions, UsedGlobals));
    }

    for (const auto &RTL : ClonedRtlModules) {
      RTL->materializeAll();
    }

    // now perform the linking itself
    Linker LD(M);

    for (auto &RTL : ClonedRtlModules) {
      RTL->materializeAll();

      // Copy target triple from dst module to avoid linker warnings
      // FIXME: remove x86_64-pc-windows-gnu-elf triple on Linux
      RTL->setTargetTriple(M.getTargetTriple());
      RTL->setDataLayout(M.getDataLayout());


      // The flag Linker::OverrideFromSrc is needed as the same global
      // variable may be initialized in both modules.  It is an error
      // for the linker, but we don't care unless they initialized
      // with different values.
      if (LD.linkInModule(std::move(RTL), Linker::OverrideFromSrc)) {
        assert(false && "Error linking builtin module!");
      }
    }

    // Allow removal of function from module after it is inlined
    for (auto &F : M)
      if (!UserModuleFunctions.count(&F) && !F.isDeclaration())
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

    return true;
  }

} //namespace intel {

extern "C" llvm::ModulePass* createBuiltInImportPass(const char* CPUPrefix) {
  return new intel::BIImport(CPUPrefix);
}
