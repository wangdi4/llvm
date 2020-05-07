// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#include "BuiltInFuncImport.h"
#include "CompilationUtils.h"
#include "InitializePasses.h"
#include "OCLPassSupport.h"
#include "TargetArch.h"

#include <llvm/IR/Module.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/Transforms/Utils/GlobalStatus.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Debug.h>
#include <llvm/IR/Verifier.h>

#include <string>

using namespace std;
using namespace llvm;

// Add command line to pass CPU prefix to BIImport through oclopt/ocloptimg
static cl::opt<std::string> OptCPUPrefix(
    "arch", cl::init(""), cl::desc("Set CPU prefix for Built-In Import Pass"));

namespace intel {

  char BIImport::ID = 0;

  OCL_INITIALIZE_PASS_BEGIN(BIImport, "builtin-import", "Built-in function pass", false, true)
  OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
  OCL_INITIALIZE_PASS_END(BIImport, "builtin-import", "Built-in function pass", false, true)

  BIImport::BIImport(const char *CPUPrefix)
    : ModulePass(ID)
  {
    m_cpuPrefix = CPUPrefix;
    if (m_cpuPrefix.empty())
      m_cpuPrefix = OptCPUPrefix;
  }

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
  void BIImport::UpdateSvmlBuiltin(const FunctionsVec &SvmlFunctions, Module &M)
      const {
    if (m_cpuPrefix.empty())
      return;

    // Get svml calling convention based on cpu perfix
    llvm::CallingConv::ID CC = llvm::CallingConv::C; // default
    if (m_cpuPrefix.compare(Intel::CPUId::GetCPUPrefixSSE(true)) == 0 ||
        m_cpuPrefix.compare(Intel::CPUId::GetCPUPrefixSSE(false)) == 0)
      CC = llvm::CallingConv::Intel_OCL_BI;
    else if (m_cpuPrefix.compare(Intel::CPUId::GetCPUPrefixAVX(true)) == 0 ||
             m_cpuPrefix.compare(Intel::CPUId::GetCPUPrefixAVX(false)) == 0 ||
             m_cpuPrefix.compare(Intel::CPUId::GetCPUPrefixAVX2(true)) == 0 ||
             m_cpuPrefix.compare(Intel::CPUId::GetCPUPrefixAVX2(false)) == 0)
      CC = llvm::CallingConv::Intel_OCL_BI_AVX;
    else if (m_cpuPrefix.compare(Intel::CPUId::GetCPUPrefixAVX512(true)) == 0 ||
             m_cpuPrefix.compare(Intel::CPUId::GetCPUPrefixAVX512(false)) == 0)
      CC = llvm::CallingConv::Intel_OCL_BI_AVX512;

    for (auto &SvmlF : SvmlFunctions) {
      for (auto &RTL : m_runtimeModuleList) {
        if (SvmlF->getParent() != RTL)
          continue;
        llvm::StringRef FName = SvmlF->getName();
        Function *F = M.getFunction(FName);
        if (!F)
          continue;
        std::string NewName = FName.str();
        NewName.replace(11, 6, m_cpuPrefix);
        F->setName(NewName);
        F->setCallingConv(CC);

        for (User *U : F->users())
          if (CallInst *CI = dyn_cast<CallInst>(U))
            CI->setCallingConv(CC);
      }
    }
  }

  void BIImport::GetCalledFunctions(const Function *F,
                                    FunctionsVec &CalledFuncs,
                                    FunctionsVec &SvmlFunctions) const
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

      if (CalledFunc->getName().startswith("__ocl_svml_shared"))
        SvmlFunctions.push_back(CalledFunc);

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
                             SmallPtrSetImpl<GlobalVariable*> &UsedGlobals,
                             FunctionsVec &SvmlFunctions) {
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

    if (Root->isMaterializable())
     if (llvm::Error EC = Root->materialize())
      report_fatal_error("Error materializing function: " + Root->getName());

    FunctionsVec CalledFuncs;
    GetCalledFunctions(Root, CalledFuncs, SvmlFunctions);

    for (auto Callee : CalledFuncs) {
      ExploreUses(Callee, Modules, UsedFunctions, UsedGlobals, SvmlFunctions);
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
      std::make_unique<Module>(M->getModuleIdentifier(), M->getContext());

    New->setDataLayout(M->getDataLayout());
    New->setTargetTriple(M->getTargetTriple());
    New->setModuleInlineAsm(M->getModuleInlineAsm());

    // Create globals without initializers - they may contain function
    // calls which have not been cloned yet.
    for (const auto &GV : ReqGlobals) {
      if (GV->getParent() != M) {
        continue;
      }
      GlobalVariable *NewGV = new GlobalVariable(
        *New,
        GV->getValueType(),
        GV->isConstant(), GV->getLinkage(),
        (Constant*) nullptr, GV->getName(),
        (GlobalVariable*) nullptr,
        GV->getThreadLocalMode(),
        GV->getType()->getAddressSpace());

      NewGV->copyAttributesFrom(GV);
      VMap[GV] = NewGV;
    }

    // Now do the same with the required functions
    for (const auto &FGV : ReqFunctions) {
      if (FGV->getParent() != M) {
        continue;
      }

      auto F = cast<Function>(FGV);
      VMap[F] = Function::Create(F->getFunctionType(), F->getLinkage(),
                                      F->getName(), New.get());
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
    for (const auto &FGV : ReqFunctions) {
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

    const int EST_FUNCTIONS_NUM = 32;
    const int EST_GLOBALS_NUM = 32;
    SmallPtrSet<GlobalValue*, EST_GLOBALS_NUM> UsedFunctions;
    SmallPtrSet<GlobalVariable*, EST_FUNCTIONS_NUM> UsedGlobals;
    FunctionsVec SvmlFunctions; // shared svml functions

    for (auto &F : M) {
      ExploreUses(&F, m_runtimeModuleList, UsedFunctions, UsedGlobals,
                  SvmlFunctions);
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
                          UsedFunctions, UsedGlobals, SvmlFunctions);
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

    for (const auto &RTL : ClonedRtlModules)
     if (llvm::Error EC = RTL->materializeAll())
      report_fatal_error("Error matializing module: " + RTL->getName());

    // Workaround: save StructType names to restore them later after
    // Linker::linkInModule().
    //
    // We've cloned necessary functions and globals from RTLs into
    // smaller modules, but they still refer the same Type objects,
    // because they stored in the LLVMContext.
    //
    // However, when doing linkInModule(), Linker assumes that the
    // module passed in as the Src will be destroyed. With this
    // assumption in mind, it resets the names of all struct types
    // from Src, which also have been found in the Dst module. This is
    // done to maintain the Linker internal state and it is described
    // in:
    //
    //   7a551b7c6dd012d67ddf27ab8d87c3e8742c5f11
    //   Author:     Rafael Espindola <rafael.espindola@gmail.com>
    //   git-svn-id: https://llvm.org/svn/llvm-project/llvm/trunk@222986
    //               91177308-0d34-0410-b5e6-96231b3b80d8
    //
    //   Change how we keep track of which types are in the dest module.
    //
    //   Instead of keeping an explicit set, just drop the names of types we choose
    //   to map to some other type.
    //
    //   This has the advantage that the name of the unused will not cause the context
    //   to rename types on module read.
    //
    //
    // It is perfectly valid for completely separated modules, because
    // they do not share the Type objects, but we have a full RTL and
    // small modules with such sharing. When linker changes the Type
    // names in a temporary module, it also changes the Type names
    // in the 'persistent' RTL module.
    DenseMap<StructType *, std::string> STyNames;
    for (const auto &RTL : ClonedRtlModules) {
      for (auto *Ty : RTL->getIdentifiedStructTypes()) {
        STyNames[Ty] = std::string(Ty->getName());
      }
    }

    // now perform the linking itself
    Linker LD(M);

    for (auto &RTL : ClonedRtlModules) {
      if (llvm::Error EC = RTL->materializeAll())
        report_fatal_error("Error matializing module: " + RTL->getName());

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

    for (auto &TyNamePair : STyNames) {
      using namespace Intel::OpenCL::DeviceBackend;
      TyNamePair.first->setName(
        CompilationUtils::stripStructNameTrailingDigits(TyNamePair.second));
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

    AttrBuilder IgnoreAttrs;
    for (auto A : TargetAttrs) {
      IgnoreAttrs.addAttribute(A);
    }

    for (auto &F : M) {
      F.removeAttributes(AttributeList::FunctionIndex, IgnoreAttrs);
    }

    // Update svml functions and their callers after cloning and linking, so
    // that we don't need to modify m_runtimeModuleList because that may cause
    // some caller's calling convention not updated in another clBuildProgram.
    UpdateSvmlBuiltin(SvmlFunctions, M);

    return true;
  }

} //namespace intel {

extern "C" llvm::ModulePass* createBuiltInImportPass(const char* CPUPrefix) {
  return new intel::BIImport(CPUPrefix);
}
