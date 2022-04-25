//===----  Intel_AutoCPUClone.cpp - Intel Automatic CPU Dispatch ---------===//
//
// Copyright (C) Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/Intel_AutoCPUClone.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/Triple.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/Intel_CPU_utils.h"
#include "llvm/Support/X86TargetParser.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Intel_X86EmitMultiVersionResolver.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

using namespace llvm;

#define DEBUG_TYPE "auto-cpu-clone"

static StringRef getTargetCPUFromMD(MDNode *TargetInfoMD) {
  // Expected format:
  //
  //   !{!"auto-cpu-dispatch-target", !"target-cpu"}
  assert(all_of(TargetInfoMD->operands(),
                [](const MDOperand &Op) { return isa<MDString>(Op.get()); }) &&
         "Auto CPU Dispatch target metadata must consists of MDString's "
         "only!");
  assert(TargetInfoMD->getNumOperands() == 2 &&
         "Expected 2 entries in Auto CPU Dispatch target metadata!");

  auto Op = [TargetInfoMD](unsigned Idx) {
    return cast<MDString>(TargetInfoMD->getOperand(Idx).get())->getString();
  };

  assert(Op(0) == "auto-cpu-dispatch-target" &&
         "Invalid Auto CPU Dispatch target metadata format!");

  StringRef TargetCpu = Op(1);

  return TargetCpu;
}

static std::string getTargetFeatures(StringRef TargetCpu) {
  SmallVector<StringRef, 16> CPUFeatures;
  X86::getFeaturesForCPU(TargetCpu, CPUFeatures);

  ListSeparator LS(",");
  std::string TargetFeaturesStr;
  raw_string_ostream TargetFeatures(TargetFeaturesStr);
  for (auto Feature : CPUFeatures)
    TargetFeatures << LS << "+" << Feature;

  return TargetFeatures.str();
}

static Twine getTargetSuffix(StringRef TargetCpu) {
  return Twine(StringSwitch<char>(TargetCpu)
#define CPU_SPECIFIC(NAME, TUNE_NAME, MANGLING, FEATURES) .Case(NAME, MANGLING)
#include "llvm/Support/X86TargetParser.def"
                   .Default(0));
}

static StringRef getLibIRCDispatchFeatures(StringRef TargetCpu) {
  StringRef Features = StringSwitch<StringRef>(TargetCpu)
#define CPU_SPECIFIC(NAME, TUNE_NAME, MANGLING, FEATURES) .Case(NAME, FEATURES)
#include "llvm/Support/X86TargetParser.def"
                            .Default("");
  return Features;
}

static StringRef CPUSpecificCPUDispatchNameDealias(StringRef Name) {
  return llvm::StringSwitch<StringRef>(Name)
#define CPU_SPECIFIC_ALIAS(NEW_NAME, TUNE_NAME, NAME) .Case(NEW_NAME, NAME)
#define CPU_SPECIFIC_ALIAS_ADDITIONAL(NEW_NAME, NAME) .Case(NEW_NAME, NAME)
#include "llvm/Support/X86TargetParser.def"
      .Default(Name);
}

static bool
libIRCMVResolverOptionComparator(const MultiVersionResolverOption &LHS,
                                 const MultiVersionResolverOption &RHS) {
  std::array<uint64_t, 2> LHSBits =
      X86::getCpuFeatureBitmap(LHS.Conditions.Features, /*OnlyAutoGen=*/true);
  std::array<uint64_t, 2> RHSBits =
      X86::getCpuFeatureBitmap(RHS.Conditions.Features, /*OnlyAutoGen=*/true);
  return LHSBits[1] > RHSBits[1] ||
         (LHSBits[1] == RHSBits[1] && LHSBits[0] > RHSBits[0]);
}

static bool cloneFunctions(Module &M,
                           function_ref<TargetLibraryInfo &(Function &)> GetTLI) {
  // Windows is not supported so far.
  const Triple TT{M.getTargetTriple()};
  if (!TT.isOSLinux())
    return false;

  // Maps that are used to do to RAUW later.
  std::map</*OrigFunc*/ GlobalValue *,
           std::tuple</*Resolver*/ GlobalValue *, /*Dispatch*/ GlobalValue *,
                      /*Target extension to multivesioned func*/
                      std::map<std::string, GlobalValue *>>>
      Orig2MultiFuncs;

  std::map</*MultiversionedFunc*/ const GlobalValue *,
           /*Target extension*/ std::string>
      MultiFunc2TargetExt;

  bool Changed = false;

  // Multiversion functions marked for auto cpu dispatching.
  for (Function &Fn : M) {

    if (Fn.isDeclaration())
      continue;

    if (!Fn.hasMetadata("llvm.auto.cpu.dispatch"))
      continue;

    // Skip available externally functions as they will be removed anyway.
    if (Fn.hasAvailableExternallyLinkage())
      continue;

    // Skip weakly defined functions, as GNU ld handles them correctly starting
    // from binutils 2.31 while support for older linkers is required.
    // Commit which adds support for such ifuncs:
    // https://github.com/bminor/binutils-gdb/commit/4ec0995016801cc5d5cf13baf6e10163861e6852
    //
    // TODO: Remove this restriction.
    if (Fn.isWeakForLinker())
      continue;

    // Skip functions that have addresses of their basic blocks taken, this can
    // happen when an address of a label is taken to do indirect goto later.
    // Skip them because Value::replaceAllUsesWith cannot handle them.
    // TODO: See if we can update such usages manually.
    if (any_of(Fn.users(), [](User *U) { return isa<BlockAddress>(U); }))
      continue;

    // Skip functions that have inline assembly.
    if (any_of(instructions(Fn),
               [](Instruction &I) {
                 auto *CInst = dyn_cast<CallBase>(&I);
                 return CInst && CInst->isInlineAsm();
               })) {
      continue;
    }

    // Names of Library functions that come from the ISO C standard are reserved
    // unconditionally. Redefining them with external linkage will rsult in
    // undefined behavior.
    // Skip multiversioning such redefinitions. This is to achieve consistent
    // behavior with -ax enabled vs not.
    LibFunc LF;
    if (Fn.hasExternalLinkage() &&
        GetTLI(Fn).getLibFunc(Fn.getName(), LF)) {
      continue;
    }

    MDNode *AutoCPUDispatchMD = Fn.getMetadata("llvm.auto.cpu.dispatch");
    LLVM_DEBUG(dbgs() << Fn.getName() << ": " << *AutoCPUDispatchMD << "\n");

    SmallVector<MultiVersionResolverOption> MVOptions;

    std::map<std::string, GlobalValue *> Clones;

    for (const MDOperand &TargetInfoIt : AutoCPUDispatchMD->operands()) {
      const StringRef TargetCpu =
          getTargetCPUFromMD(cast<MDNode>(TargetInfoIt.get()));

      // Get llvm/Support/X86TargetParser.def friendly target name.
      const StringRef TargetCpuDealiased =
          CPUSpecificCPUDispatchNameDealias(TargetCpu);

      const StringRef LibIRCDispatchFeatures =
          getLibIRCDispatchFeatures(TargetCpuDealiased);
      // Skip target if it is not recognized by
      // llvm/Support/X86TargetParser.def.
      assert(LibIRCDispatchFeatures != "" && "A target is not recognized!");
      if (LibIRCDispatchFeatures == "")
        continue;

      ValueToValueMapTy VMap;
      Function *New = CloneFunction(&Fn, VMap);

      New->setMetadata("llvm.auto.cpu.dispatch", nullptr);

      std::string Features =
          LibIRCDispatchFeatures.str();

      const Attribute Attr = New->getFnAttribute("target-features");
      const StringRef OldFeatures = Attr.getValueAsString();

      // Keep old target features as some of them can be added as a result
      // of command line arguments(e.g. -msha)
      if (OldFeatures.empty()) {
        New->addFnAttr("target-features", Features);
      }
      else {
        SmallString<256> Appended(OldFeatures);
        Appended.push_back(',');
        Appended.append(Features);
        New->addFnAttr("target-features", Appended);
      }

      New->removeFnAttr("target-cpu");
      New->addFnAttr("target-cpu", TargetCpu);

      New->removeFnAttr("tune-cpu");
      New->addFnAttr("tune-cpu", TargetCpu);

      New->addFnAttr("loopopt-pipeline", "full");

      New->setName(Fn.getName() + "." + getTargetSuffix(TargetCpuDealiased));

      SmallVector<StringRef> FeaturesArray;
      LibIRCDispatchFeatures.split(FeaturesArray, ',', /*MaxSplit=*/-1,
                                   /*KeepEmpty=*/false);
      // Drop leading "+".
      transform(FeaturesArray, FeaturesArray.begin(),
                [](StringRef Str) { return Str.substr(1); });

      MVOptions.emplace_back(New, "" /* Op(1)? */, FeaturesArray);

      MultiFunc2TargetExt[New] = TargetCpu.str();
      Clones[TargetCpu.str()] = New;
    }

    // Skip function if no supported targets were found.
    if (MVOptions.empty())
      continue;

    std::string OrigName = Fn.getName().str();
    Fn.setName(OrigName + ".A");                            // "generic" suffix.
    MVOptions.emplace_back(&Fn, "", ArrayRef<StringRef>()); // generic.

    stable_sort(MVOptions, libIRCMVResolverOptionComparator);

    FunctionType *ResolverTy =
        FunctionType::get(Fn.getType(), false /*IsVarArg*/);

    auto *Resolver = Function::Create(ResolverTy, Fn.getLinkage(),
                                      OrigName + ".resolver", Fn.getParent());
    Resolver->setVisibility(Fn.getVisibility());

    GlobalIFunc *GIF = GlobalIFunc::create(
        Fn.getValueType(), 0, Fn.getLinkage(), "", Resolver, Fn.getParent());
    GIF->setVisibility(Fn.getVisibility());
    GIF->setName(OrigName);

    if (Fn.getLinkage() != GlobalValue::LinkageTypes::InternalLinkage &&
        Fn.getLinkage() != GlobalValue::LinkageTypes::PrivateLinkage)
      appendToCompilerUsed(M, {GIF});

    emitMultiVersionResolver(Resolver, MVOptions, true /*UseIFunc*/,
                             true /*UseLibIRC*/);
    Fn.setMetadata("llvm.auto.cpu.dispatch", nullptr);

    // TODO: Comdat?
    // TODO: CodeGenModule::SetCommonAttributes ?

    Orig2MultiFuncs[&Fn] = {Resolver, GIF, std::move(Clones)};
    Changed = true;
  }

  // No functions were multiversioned, exiting.
  if (!Changed)
    return false;

  // Update uses of the original functions.
  // At this point all functions(except resolvers) use original functions (those
  // with .A suffix). The loop below iterates over all the functions that we
  // multiversioned earlier and replaces all of their uses with the
  // corresponding ifunc's. This is done for _all_ cases except:
  // 1) when a multiversioned function calls another multiversioned function
  //    whose definition is in the same module and,
  // 2) when a multiversioned function initializes a function pointer to point
  //    to another multiversioned function whose definition is in the same module.
  // For such cases, calls and function pointer initializations are replaced with
  // calls and uses to the correct multiversioned analogs respectively.
  for (auto &Entry : Orig2MultiFuncs) {
    GlobalValue *Fn = Entry.first;
    const GlobalValue *Resolver = std::get<0>(Entry.second);
    GlobalValue *Dispatcher = std::get<1>(Entry.second);
    std::map<std::string, GlobalValue *> &Clones = std::get<2>(Entry.second);

    Fn->replaceUsesWithIf(Dispatcher, [&](Use &IFUse) {

      // Resolver should operate on specific function versions.
      const auto *Inst = dyn_cast<Instruction>(IFUse.getUser());
      if (Inst && Inst->getFunction() == Resolver)
        return false;

      auto *CInst = dyn_cast<CallBase>(IFUse.getUser());
      if (CInst && CInst->isCallee(&IFUse)) {
        Function *Caller = CInst->getFunction();
        // If caller is a generic function skip it, as it already calls
        // the correct version:
        // foo.A
        //   call bar.A
        if (Orig2MultiFuncs.count(Caller))
          return false;

        // If caller is a specialized version of a function then find the
        // corresponding specialized version of the callee.
        if (MultiFunc2TargetExt.count(Caller))
          return false;

        return true;
      }

      return true;
    });

    for (auto It = Fn->use_begin(), End = Fn->use_end(); It != End;) {
      Use &IFUse = *It;
      ++It;

      // Resolver should operate on specific functions versions.
      const auto *Inst = dyn_cast<Instruction>(IFUse.getUser());
      if (Inst && Inst->getFunction() == Resolver)
        continue;

      auto *CInst = dyn_cast<CallBase>(IFUse.getUser());
      if (CInst && CInst->isCallee(&IFUse)) {
        Function *Caller = CInst->getFunction();
        // If caller is a generic function skip it, as it already calls
        // the correct version:
        // foo.A
        //   call bar.A
        if (Orig2MultiFuncs.count(Caller))
          continue;

        // If caller is a specialized version of a function then find the
        // corresponding specialized version of the callee.
        if (MultiFunc2TargetExt.count(Caller)) {
          GlobalValue *Replacement = Clones[MultiFunc2TargetExt[Caller]];
          assert(Replacement && "Expected that functions are multiversioned "
                                "for all requested targets now!");
          CInst->setCalledOperand(Replacement);
          continue;
        }
      }

      llvm_unreachable("Unhandled case!");
    }
  }

  // If we are here then we have done modifications.
  return true;
}

PreservedAnalyses AutoCPUClonePass::run(Module &M, ModuleAnalysisManager &AM) {

  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(F);
  };

  if (cloneFunctions(M, GetTLI))
    return PreservedAnalyses::none();

  return PreservedAnalyses::all();
}

namespace {
class AutoCPUCloneLegacyPass : public ModulePass {
public:
  static char ID; // Pass identification, replacement for typeid.
  explicit AutoCPUCloneLegacyPass() : ModulePass(ID) {
    initializeAutoCPUCloneLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetLibraryInfoWrapperPass>();
  }

  bool runOnModule(Module &M) override {

    auto GetTLI = [this](Function &F) -> TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };

    if (skipModule(M))
      return false;

    bool anyFunctionsCloned = cloneFunctions(M, GetTLI);
    return anyFunctionsCloned;
  }
};
} // namespace

char AutoCPUCloneLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(AutoCPUCloneLegacyPass, "auto-cpu-clone",
                      "Clone functions for Auto CPU Dispatch", false, false)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(AutoCPUCloneLegacyPass, "auto-cpu-clone",
                    "Clone functions for Auto CPU Dispatch", false, false)

Pass *llvm::createAutoCPUCloneLegacyPass() {
  return new AutoCPUCloneLegacyPass();
}
