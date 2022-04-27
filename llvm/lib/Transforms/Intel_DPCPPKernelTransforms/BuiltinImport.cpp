//===- BuiltinImport.cpp - Import DPC++ builtin modules -------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BuiltinImport.h"
#include "CPUDetect.h"
#include "llvm/ADT/EquivalenceClasses.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/InitializePasses.h"
#include "llvm/Linker/Linker.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BuiltinLibInfoAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/ImplicitArgsAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;
using namespace DPCPPKernelCompilationUtils;

#define DEBUG_TYPE "dpcpp-kernel-builtin-import"

static cl::opt<std::string>
    OptCPUPrefix("dpcpp-kernel-cpu-prefix", cl::init(""),
                 cl::desc("Set CPU prefix for BuiltinImport Pass"));

namespace {

/// Legacy BuiltinImport pass.
class BuiltinImportLegacy : public ModulePass {
  BuiltinImportPass Impl;

public:
  static char ID;

  BuiltinImportLegacy(const SmallVector<Module *, 2> &BuiltinModules =
                          SmallVector<Module *, 2>(),
                      StringRef CPUPrefix = "")
      : ModulePass(ID), Impl(BuiltinModules, CPUPrefix) {
    initializeBuiltinImportLegacyPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return "BuiltinImportLegacy"; }

  bool runOnModule(Module &M) override {
    BuiltinLibInfo *BLI =
        &getAnalysis<BuiltinLibInfoAnalysisLegacy>().getResult();
    return Impl.runImpl(M, BLI);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<BuiltinLibInfoAnalysisLegacy>();
    AU.addPreserved<ImplicitArgsAnalysisLegacy>();
  }
};

} // namespace

char BuiltinImportLegacy::ID = 0;

INITIALIZE_PASS_BEGIN(BuiltinImportLegacy, DEBUG_TYPE,
                      "DPCPP builtin import pass", false, false)
INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfoAnalysisLegacy)
INITIALIZE_PASS_END(BuiltinImportLegacy, DEBUG_TYPE,
                    "DPCPP builtin import pass", false, false)

ModulePass *llvm::createBuiltinImportLegacyPass(
    const SmallVector<Module *, 2> &BuiltinModules, StringRef CPUPrefix) {
  return new BuiltinImportLegacy(BuiltinModules, CPUPrefix);
}

static Function *FindFunctionDef(const Function *F,
                                 ArrayRef<Module *> Modules) {
  assert(F && "Invalid function.");
  for (auto M : Modules) {
    assert(M && "Invalid module.");

    Function *Ret = M->getFunction(F->getName());
    // Test if the function body is contained in this module.
    if (Ret && !Ret->isDeclaration())
      return Ret;
  }
  return nullptr;
}

static GlobalVariable *FindGlobalDef(const GlobalVariable *GV,
                                     ArrayRef<Module *> Modules) {
  assert(GV && "Invalid global variable.");
  for (auto M : Modules) {
    assert(M && "Invalid module.");

    auto Ret = M->getGlobalVariable(GV->getName());
    // check if it is a definition
    if (Ret && Ret->hasInitializer())
      return Ret;
  }

  return nullptr;
}

BuiltinImportPass::BuiltinImportPass(const SmallVector<Module *, 2> &,
                                     StringRef CPUPrefix)
    : CPUPrefix(CPUPrefix) {
  initializeBuiltinImportLegacyPass(*PassRegistry::getPassRegistry());
}

// This function replaces keyword "shared" in the builtin name by current CPU
// prefix, for example:
// If CPU is l9, __ocl_svml_shared_acos1f to be changed to __ocl_svml_l9_acos1f
void BuiltinImportPass::UpdateSvmlBuiltin(const FuncVec &SvmlFunctions,
                                          Module &M) const {
  if (CPUPrefix.empty())
    return;

  // Get svml calling convention based on cpu perfix.
  CallingConv::ID CC = CallingConv::C; // default
  std::string SVMLCPUPrefix = CPUPrefix.str();
  if (CPUPrefix.compare(CPUDetect::GetCPUPrefixSSE(true)) == 0 ||
      CPUPrefix.compare(CPUDetect::GetCPUPrefixSSE(false)) == 0)
    CC = CallingConv::Intel_OCL_BI;
  else if (CPUPrefix.compare(CPUDetect::GetCPUPrefixAVX(true)) == 0 ||
           CPUPrefix.compare(CPUDetect::GetCPUPrefixAVX(false)) == 0 ||
           CPUPrefix.compare(CPUDetect::GetCPUPrefixAVX2(true)) == 0 ||
           CPUPrefix.compare(CPUDetect::GetCPUPrefixAVX2(false)) == 0)
    CC = CallingConv::Intel_OCL_BI_AVX;
  else if (CPUPrefix.compare(CPUDetect::GetCPUPrefixAVX512(true)) == 0 ||
           CPUPrefix.compare(CPUDetect::GetCPUPrefixAVX512(false)) == 0)
    CC = CallingConv::Intel_OCL_BI_AVX512;
  else if (CPUPrefix.compare(CPUDetect::GetCPUPrefixAMX(true)) == 0) {
    CC = CallingConv::Intel_OCL_BI_AVX512;
    // FIXME:
    // ocl_svml lib for AMX 64bit is not available yet (__ocl_svml_z1.so/dll)
    // Use AVX512 implementations as a workaround
    SVMLCPUPrefix = CPUDetect::GetCPUPrefixAVX512(true);
  } else if (CPUPrefix.compare(CPUDetect::GetCPUPrefixAMX(false)) == 0) {
    CC = llvm::CallingConv::Intel_OCL_BI_AVX512;
    // FIXME:
    // ocl_svml lib for AMX 32bit is not available yet (__ocl_svml_x1.so/dll)
    // Use AVX512 implementations as a workaround
    SVMLCPUPrefix = CPUDetect::GetCPUPrefixAVX512(false);
  }

  for (auto &SvmlF : SvmlFunctions) {
    for (auto &RTL : BuiltinModules) {
      if (SvmlF->getParent() != RTL)
        continue;
      StringRef FName = SvmlF->getName();
      Function *F = M.getFunction(FName);
      if (!F)
        continue;
      std::string NewName = FName.str();
      NewName.replace(11, 6, SVMLCPUPrefix);
      F->setName(NewName);
      F->setCallingConv(CC);

      for (User *U : F->users())
        if (CallInst *CI = dyn_cast<CallInst>(U))
          CI->setCallingConv(CC);
    }
  }
}

void BuiltinImportPass::GetCalledFunctions(const Function *F,
                                           FuncVec &CalledFuncs,
                                           FuncVec &SvmlFunctions) const {
  DPCPPKernelCompilationUtils::FuncSet VisitedSet;

  // Iterate over function instructions and look for call instructions.
  for (auto &I : instructions(F)) {
    const CallInst *InstCall = dyn_cast<CallInst>(&I);
    if (!InstCall)
      continue;

    Function *CalledFunc = InstCall->getCalledFunction();
    if (!CalledFunc) {
      // This case can occur only if CallInst is calling something other than
      // LLVM function, meaning the call is indirect. We need to check if a
      // called value is ConstantExpr that can use the function defined in
      // source module.
      auto CE = dyn_cast<ConstantExpr>(InstCall->getCalledOperand());
      if (CE && CE->getOpcode() == Instruction::BitCast) {
        Value *CEOperand = CE->getOperand(0);
        if (auto CEFuncOperand = dyn_cast<Function>(CEOperand))
          CalledFunc = CEFuncOperand;
        else
          continue;
      } else {
        continue;
      }
    }

    if (VisitedSet.count(CalledFunc))
      continue;

    if (CalledFunc->getName().startswith("__ocl_svml_shared"))
      SvmlFunctions.push_back(CalledFunc);

    VisitedSet.insert(CalledFunc);
    CalledFuncs.push_back(CalledFunc);
  }
}

static void ExploreOperand(Value *Op, ArrayRef<Module *> Modules,
                           SetVector<GlobalValue *> &UsedFunctions,
                           SetVector<GlobalVariable *> &UsedGlobals) {
  // Operand may be a ConstantExpr, so we need to recursively check its
  // operands.
  if (auto *CE = dyn_cast<ConstantExpr>(Op)) {
    for (size_t i = 0; i < CE->getNumOperands(); ++i)
      ExploreOperand(CE->getOperand(i), Modules, UsedFunctions, UsedGlobals);
    return;
  }

  if (auto GV = dyn_cast<GlobalVariable>(Op)) {
    UsedGlobals.insert(GV);
    if (auto G = FindGlobalDef(GV, Modules))
      UsedGlobals.insert(G);
  }
}

void BuiltinImportPass::ExploreUses(Function *Root, ArrayRef<Module *> Modules,
                                    SetVector<GlobalValue *> &UsedFunctions,
                                    SetVector<GlobalVariable *> &UsedGlobals,
                                    FuncVec &SvmlFunctions) {
  assert(Root && "Invalid function");

  if (Root->isDeclaration()) {
    Function *Def = FindFunctionDef(Root, Modules);
    if (Def) {
      UsedFunctions.insert(Root);
      Root = Def;
    } else {
      UsedFunctions.insert(Root);
      return;
    }
  }

  bool FirstUse = UsedFunctions.insert(Root);
  if (!FirstUse)
    return;

  if (Root->isMaterializable())
    if (Error EC = Root->materialize())
      report_fatal_error("Error materializing function: " + Root->getName());

  FuncVec CalledFuncs;
  GetCalledFunctions(Root, CalledFuncs, SvmlFunctions);

  for (auto Callee : CalledFuncs)
    ExploreUses(Callee, Modules, UsedFunctions, UsedGlobals, SvmlFunctions);

  for (const BasicBlock &BB : *Root) {
    for (const Instruction &I : BB)
      for (Value *Op : I.operands())
        ExploreOperand(Op, Modules, UsedFunctions, UsedGlobals);
  }
}

static std::unique_ptr<Module>
CloneModuleOnlyRequired(const Module *M, ValueToValueMapTy &VMap,
                        SetVector<GlobalValue *> &ReqFunctions,
                        SetVector<GlobalVariable *> &ReqGlobals) {
  std::unique_ptr<Module> New =
      std::make_unique<Module>(M->getModuleIdentifier(), M->getContext());

  New->setDataLayout(M->getDataLayout());
  New->setTargetTriple(M->getTargetTriple());
  New->setModuleInlineAsm(M->getModuleInlineAsm());

  // Create globals without initializers - they may contain function calls which
  // have not been cloned yet.
  for (const auto &GV : ReqGlobals) {
    if (GV->getParent() != M)
      continue;

    GlobalVariable *NewGV = new GlobalVariable(
        *New, GV->getValueType(), GV->isConstant(), GV->getLinkage(),
        (Constant *)nullptr, GV->getName(), (GlobalVariable *)nullptr,
        GV->getThreadLocalMode(), GV->getType()->getAddressSpace());

    NewGV->copyAttributesFrom(GV);
    VMap[GV] = NewGV;
  }

  // Now do the same with the required functions.
  for (const auto &FGV : ReqFunctions) {
    if (FGV->getParent() != M)
      continue;

    auto F = cast<Function>(FGV);
    VMap[F] = Function::Create(F->getFunctionType(), F->getLinkage(),
                               F->getName(), New.get());
  }

  // Clone global initializers.
  for (auto GV : ReqGlobals) {
    if (GV->getParent() != M || !GV->hasInitializer())
      continue;

    auto NewGV = cast<GlobalVariable>(VMap[GV]);
    NewGV->setInitializer(MapValue(GV->getInitializer(), VMap));
  }

  // ... and the functions bodies.
  for (const auto &FGV : ReqFunctions) {
    if (FGV->getParent() != M)
      continue;

    auto F = cast<Function>(FGV);
    Function *NF = cast<Function>(VMap[F]);

    // Track args changes.
    Function::arg_iterator DestI = NF->arg_begin();
    for (const auto &Arg : F->args()) {
      DestI->setName(Arg.getName());
      VMap[&Arg] = &*DestI++;
    }

    SmallVector<ReturnInst *, 8> Returns; // Ignore returns cloned.
    CloneFunctionInto(NF, F, VMap, CloneFunctionChangeType::ClonedModule,
                      Returns);

    if (F->hasPersonalityFn())
      NF->setPersonalityFn(MapValue(F->getPersonalityFn(), VMap));
  }

  return New;
}

static const char MinLegalVectorWidthStrAttr[] = "min-legal-vector-width";

// Return true if F has a valid "min-legal-vector-width".
static bool getMinLegalVectorWidthAttrVal(Function *F, unsigned &Val) {
  if (!F)
    return false;
  Attribute Attr = F->getFnAttribute(MinLegalVectorWidthStrAttr);
  // getAsInteger() returns true on error.
  if (!Attr.isValid() || Attr.getValueAsString().getAsInteger(0, Val))
    return false;
  return true;
}

// A directed graph is said to be a WCC (Weakly Connected Component), if every
// node is reachable from every other node, ignoring the direction of the
// edge. (For undirected graphs, WCC is equivalent to SCC).
// We need to make sure that all function nodes in the same WCC have the same
// "min-legal-vector-width" attribute, so that they have agreement on calling
// conventions (especially passing arguments and returning results via ZMM/YMM
// registers).
static void unifyMinLegalVectorWidthAttr(Module &M) {
  EquivalenceClasses<Function *> WeaklyConnectedComponents;
  CallGraph CG = CallGraph(M);
  for (auto &F : M) {
    if (F.isDeclaration())
      continue;
    CallGraphNode *CGN = CG[&F];
    WeaklyConnectedComponents.insert(&F);
    // Visit all functions called by F.
    for (auto CallRecord : *CGN) {
      Function *Callee = CallRecord.second->getFunction();
      if (!Callee)
        continue;
      // If F calls Callee, merge them into the same equivalence class.
      WeaklyConnectedComponents.unionSets(&F, Callee);
    }
  }

  for (auto I = WeaklyConnectedComponents.begin(),
            E = WeaklyConnectedComponents.end();
       I != E; ++I) {
    // Find the leader set.
    if (!I->isLeader())
      continue;
    // Traverse the WCC to get the largest 'min-legal-vector-width' value.
    unsigned LargestMinLegalVectorWidth = 0;
    for (auto MI = WeaklyConnectedComponents.member_begin(I),
              ME = WeaklyConnectedComponents.member_end();
         MI != ME; ++MI) {
      unsigned AttrVal = 0;
      getMinLegalVectorWidthAttrVal(*MI, AttrVal);
      LargestMinLegalVectorWidth =
          std::max(LargestMinLegalVectorWidth, AttrVal);
    }
    // Doesn't have to update the attributes.
    if (LargestMinLegalVectorWidth == 0)
      continue;
    // Set 'min-legal-vector-width' to the largest one among the WCC.
    LLVM_DEBUG(dbgs() << "The following functions are in the same WCC, setting "
                         "'min-legal-vector-width' to "
                      << LargestMinLegalVectorWidth << ":\n");
    for (auto MI = WeaklyConnectedComponents.member_begin(I),
              ME = WeaklyConnectedComponents.member_end();
         MI != ME; ++MI) {
      (*MI)->addFnAttr(MinLegalVectorWidthStrAttr,
                       utostr(LargestMinLegalVectorWidth));
      LLVM_DEBUG(dbgs() << (*MI)->getName() << '\n');
    }
  }
}

bool BuiltinImportPass::runImpl(Module &M, BuiltinLibInfo *BLI) {
  if (CPUPrefix.empty())
    CPUPrefix = OptCPUPrefix;
  BuiltinModules = BLI->getBuiltinModules();

  // Remember user module function pointers, so we could set linkonce_odr
  // to only imported functions.
  FuncSet UserModuleFunctions;

  SetVector<GlobalValue *> UsedFunctions;
  SetVector<GlobalVariable *> UsedGlobals;
  FuncVec SvmlFunctions; // shared svml functions

  for (auto &F : M) {
    if (F.isDeclaration()) {
      if (!F.use_empty())
        ExploreUses(&F, BuiltinModules, UsedFunctions, UsedGlobals,
                    SvmlFunctions);
    } else {
      UserModuleFunctions.insert(&F);
    }
  }
  // Globals can have other function calls in their initializers, which can have
  // other globals in their bodies, so we must loop until no new globals
  // discovered.
  size_t GlobalsNumBefore;
  do {
    GlobalsNumBefore = UsedGlobals.size();
    for (auto GV : UsedGlobals) {
      if (GV->hasInitializer()) {
        Constant *Init = GV->getInitializer();
        for (auto &Op : Init->operands())
          if (auto func = dyn_cast<Function>(Op))
            ExploreUses(func, BuiltinModules, UsedFunctions, UsedGlobals,
                        SvmlFunctions);
      }
    }
  } while (GlobalsNumBefore < UsedGlobals.size());

  // We now known which globals and functions we need.
  // Lets clone rtl modules and filter out everything we don't need.
  SmallVector<std::unique_ptr<Module>, 2> ClonedRtlModules;
  ValueToValueMapTy VMap;
  for (auto RTL : BuiltinModules)
    ClonedRtlModules.push_back(
        CloneModuleOnlyRequired(RTL, VMap, UsedFunctions, UsedGlobals));

  for (const auto &RTL : ClonedRtlModules)
    if (Error EC = RTL->materializeAll())
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
  //   Instead of keeping an explicit set, just drop the names of types we
  //   choose to map to some other type.
  //
  //   This has the advantage that the name of the unused will not cause the
  //   context to rename types on module read.
  //
  //
  // It is perfectly valid for completely separated modules, because
  // they do not share the Type objects, but we have a full RTL and
  // small modules with such sharing. When linker changes the Type
  // names in a temporary module, it also changes the Type names
  // in the 'persistent' RTL module.
  DenseMap<StructType *, std::string> STyNames;
  for (const auto &RTL : ClonedRtlModules)
    for (auto *Ty : RTL->getIdentifiedStructTypes())
      STyNames[Ty] = std::string(Ty->getName());

  // now perform the linking itself.
  Linker LD(M);

  for (auto &RTL : ClonedRtlModules) {
    if (Error EC = RTL->materializeAll())
      report_fatal_error("Error matializing module: " + RTL->getName());

    // Copy target triple from dst module to avoid linker warnings.
    // FIXME: remove x86_64-pc-windows-gnu-elf triple on Linux
    RTL->setTargetTriple(M.getTargetTriple());
    RTL->setDataLayout(M.getDataLayout());

    // The flag Linker::OverrideFromSrc is needed as the same global variable
    // may be initialized in both modules. It is an error for the linker, but we
    // don't care unless they initialized with different values.
    if (LD.linkInModule(std::move(RTL), Linker::OverrideFromSrc))
      assert(false && "Error linking builtin module!");
  }

  for (auto &TyNamePair : STyNames)
    TyNamePair.first->setName(
        DPCPPKernelCompilationUtils::stripStructNameTrailingDigits(
            TyNamePair.second));

  // Allow removal of function from module after it is inlined.
  for (auto &F : M)
    if (!UserModuleFunctions.count(&F) && !F.isDeclaration())
      F.setLinkage(GlobalVariable::InternalLinkage);

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

  AttributeMask IgnoreAttrs;
  for (auto A : TargetAttrs)
    IgnoreAttrs.addAttribute(A);

  for (auto &F : M)
    F.removeFnAttrs(IgnoreAttrs);

  // Update svml functions and their callers after cloning and linking, so
  // that we don't need to modify BuiltinModules because that may cause
  // some caller's calling convention not updated in another clBuildProgram.
  UpdateSvmlBuiltin(SvmlFunctions, M);

  // Unify the 'min-legal-vector-width' attribute if caller/callee functions
  // mismatch on how to pass/read arguments/return values.
  unifyMinLegalVectorWidthAttr(M);

  return true;
}

PreservedAnalyses BuiltinImportPass::run(Module &M, ModuleAnalysisManager &AM) {
  BuiltinLibInfo *BLI = &AM.getResult<BuiltinLibInfoAnalysis>(M);
  if (!runImpl(M, BLI))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<ImplicitArgsAnalysis>();
  PA.preserveSet<CFGAnalyses>();
  return PA;
}
