//===----  Intel_AutoCPUClone.cpp - Intel Automatic CPU Dispatch ---------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/Intel_AutoCPUClone.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/ProfileSummaryInfo.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/Intel_CPU_utils.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TargetParser/Triple.h"
#include "llvm/TargetParser/X86TargetParser.h"
#include "llvm/Transforms/IPO/Intel_InlineReport.h"
#include "llvm/Transforms/IPO/Intel_MDInlineReport.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Intel_X86EmitMultiVersionResolver.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

using namespace llvm;

#define DEBUG_TYPE "auto-cpu-clone"

// The control options for ACD multi-versioning defined below offer precise
// management of function multi-versioning. These options establish a specific
// priority hierarchy among them.
// cost-model-based < acd-enable-all < acd-disable-all < acd-force-funcs <
// < acd-force-no-funcs

// Option enables ACD multi-versioning for all functions regardless of
// cost-model decision.
static cl::opt<bool>
    ACDEnableAll("acd-enable-all", cl::init(false), cl::ReallyHidden,
                 cl::desc("Enable multi-versioning for all functions"));

// Option disables ACD multi-versioning for all functions regardless of
// cost-model decision.
static cl::opt<bool>
    ACDDisableAll("acd-disable-all", cl::init(false), cl::ReallyHidden,
                  cl::desc("Disable multi-versioning for all functions"));

// Option enables ACD multi-versioning for specific functions regardless of
// cost-model decision.
static cl::list<std::string> ForceACDFunctions(
    "acd-force-funcs", cl::CommaSeparated,
    cl::desc("Functions to always consider for auto cpu multi-versioning"),
    cl::value_desc("func1,func2,func3,..."), cl::Hidden);

// Option disables ACD multi-versioning for specific functions regardless of
// cost-model decision.
static cl::list<std::string> ForceNoACDFunctions(
    "acd-force-no-funcs", cl::CommaSeparated,
    cl::desc("Functions to always disable for auto cpu multi-versioning"),
    cl::value_desc("func1,func2,func3,..."), cl::Hidden);

// Internal option to control whether to multi-version functions per the
// targets specified in llvm.vec.auto.cpu.dispatch metadata. Added specifically
// for enabling vector variant generation in lit tests.
static cl::opt<bool>
    ClGenerateVectorVariants("generate-vector-variants", cl::init(false),
                             cl::ReallyHidden, cl::desc("Generate vector variants"));

static std::string getTargetFeatures(StringRef TargetCpu) {
  SmallVector<StringRef, 16> CPUFeatures;
  X86::getFeaturesForCPU(TargetCpu, CPUFeatures);
  llvm::sort(CPUFeatures);
  return "+" + llvm::join(CPUFeatures, ",+");
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

static void
setResolverAttributes(Function *Resolver, Function &Fn) {

  if (Fn.hasFnAttribute("tune-cpu"))
    Resolver->addFnAttr(Fn.getFnAttribute("tune-cpu"));

  // Set Resolver's "target-features" attribute to the feature-set supported
  // by the "target-cpu".
  if (Fn.hasFnAttribute("target-cpu")) {
    const Attribute Attr = Fn.getFnAttribute("target-cpu");
    Resolver->addFnAttr(Attr);
    StringRef TargetCPU = Attr.getValueAsString();
    Resolver->addFnAttr("target-features", getTargetFeatures(TargetCPU));
  }

  if (Fn.hasFnAttribute("advanced-optim"))
    Resolver->addFnAttr(Fn.getFnAttribute("advanced-optim"));
}

static void
emitWrapperBasedDispatcher(Function &Fn, std::string OrigName,
                           GlobalVariable *DispatchPtr,
                           Function*& Dispatcher) {

  // Create the dispatcher function through cloning Fn.
  // This will make sure all attributes and properties of Fn are cloned/copied
  // over to the dispatcher function.
  ValueToValueMapTy VMap;
  Dispatcher = CloneFunction(&Fn, VMap, nullptr, true);
  Dispatcher->setName(OrigName);
  getInlineReport()->updateName(Dispatcher);
  getMDInlineReport()->updateName(Dispatcher);

  // Now, create the body for the Dispatcher.
  LLVMContext &Ctx = Fn.getContext();
  BasicBlock *CurBlock = BasicBlock::Create(Ctx, "", Dispatcher);
  IRBuilder<> Builder(CurBlock, CurBlock->begin());

  // Create a load of the dispatch pointer.
  auto DispatchPtrVal = Builder.CreateAlignedLoad(DispatchPtr->getType(),
                                                  DispatchPtr, MaybeAlign(8));

  // Create an indirect call through the dispatch pointer.
  SmallVector<Value *, 10> Args;
  for_each(Dispatcher->args(), [&](Argument &Arg) { Args.push_back(&Arg); });
  CallInst *Result =
      Builder.CreateCall(FunctionCallee(Fn.getFunctionType(), DispatchPtrVal),
                         Args);
  Result->setCallingConv(Fn.getCallingConv());
  Result->setAttributes(Fn.getAttributes());

  Dispatcher->setMetadata("llvm.acd.dispatcher", MDNode::get(Ctx, {}));
  DispatchPtr->setMetadata("llvm.acd.dispatcher", MDNode::get(Ctx, {}));

  // Create a return.
  if (Fn.getReturnType()->isVoidTy())
    Builder.CreateRetVoid();
  else
    Builder.CreateRet(Result);
}

static void
emitWrapperBasedResolver(Function &Fn, std::string OrigName,
                         SmallVector<MultiVersionResolverOption> &MVOptions,
                         Function *&Resolver, GlobalValue *&Dispatcher,
                         GlobalVariable *&DispatchPtr,
                         bool PerformCPUBrandCheck) {

  Module *M = Fn.getParent();
  Resolver = M->getFunction("__intel.acd.resolver");
  if (!Resolver) {
    LLVMContext &Ctx = Fn.getContext();
    FunctionType *ResolverTy = FunctionType::get(Type::getVoidTy(Ctx), false);
    Resolver = Function::Create(ResolverTy, GlobalValue::InternalLinkage,
                                "__intel.acd.resolver", M);
    Resolver->setDSOLocal(true);
    setResolverAttributes(Resolver, Fn);
    appendToGlobalCtors(*M, Resolver, 500 /* Some number > 0 */);
  }

  std::string DispatchPtrName = OrigName + ".ptr";
  Type *DispatchPtrType = PointerType::getUnqual(M->getContext());
  DispatchPtr =
      new GlobalVariable(*M, DispatchPtrType, false, GlobalValue::InternalLinkage,
                         Constant::getNullValue(DispatchPtrType), DispatchPtrName);
  DispatchPtr->setDSOLocal(true);

  emitMultiVersionResolver(Resolver, DispatchPtr, MVOptions, false /*UseIFunc*/,
                           true /*UseLibIRC*/, PerformCPUBrandCheck);

  emitWrapperBasedDispatcher(Fn, OrigName, DispatchPtr, (Function*&)Dispatcher);
}

static void
emitIFuncBasedResolver(Function &Fn, std::string OrigName,
                       SmallVector<MultiVersionResolverOption> &MVOptions,
                       Function *&Resolver, GlobalValue *&Dispatcher,
                       bool PerformCPUBrandCheck) {

  FunctionType *ResolverTy = FunctionType::get(Fn.getType(), false /*IsVarArg*/);
  Resolver = Function::Create(ResolverTy, Fn.getLinkage(),
                              OrigName + ".resolver", Fn.getParent());
  Resolver->setVisibility(Fn.getVisibility());
  Resolver->setDSOLocal(Fn.isDSOLocal());

  Dispatcher = GlobalIFunc::create(Fn.getValueType(), 0, Fn.getLinkage(),
                                   OrigName, Resolver, Fn.getParent());
  Dispatcher->setVisibility(Fn.getVisibility());
  Dispatcher->setDSOLocal(Fn.isDSOLocal());

  emitMultiVersionResolver(Resolver, nullptr /*DispatchPtr*/, MVOptions,
                           true /*UseIFunc*/, true /*UseLibIRC*/,
                           PerformCPUBrandCheck);
  setResolverAttributes(Resolver, Fn);
}

static bool
shouldMultiVersion(Module& M, Function& Fn, bool GenerateVectorVariants,
                   function_ref<TargetLibraryInfo &(Function &)> GetTLI) {

  if (Fn.isDeclaration())
    return false;

  // Skip functions that are not intended to be auto multi-versioned.
  if (GenerateVectorVariants && !Fn.hasMetadata("llvm.vec.auto.cpu.dispatch"))
    return false;

  if (!GenerateVectorVariants &&
      !Fn.hasMetadata("llvm.auto.arch") &&
      !Fn.hasMetadata("llvm.auto.cpu.dispatch"))
    return false;

  // Skip available externally functions as they will be removed anyway.
  if (Fn.hasAvailableExternallyLinkage())
    return false;

  // Skip weakly defined functions, as GNU ld handles them correctly starting
  // from binutils 2.31 while support for older linkers is required.
  // Commit which adds support for such ifuncs:
  // https://github.com/bminor/binutils-gdb/commit/4ec0995016801cc5d5cf13baf6e10163861e6852
  //
  // TODO: Remove this restriction.
  if (Fn.isWeakForLinker())
    return false;

  // Skip functions that have addresses of their basic blocks taken, this can
  // happen when an address of a label is taken to do indirect goto later.
  // Skip them because Value::replaceAllUsesWith cannot handle them.
  // TODO: See if we can update such usages manually.
  if (any_of(Fn.users(), [](User *U) { return isa<BlockAddress>(U); }))
    return false;

  // Skip functions that have inline assembly.
  if (any_of(instructions(Fn),
             [](Instruction &I) {
               auto *CInst = dyn_cast<CallBase>(&I);
               return CInst && CInst->isInlineAsm();
             }))
    return false;

  // Skip functions that are resolvers of other ifuncs.
  if (any_of(M.ifuncs(),
             [&](GlobalIFunc &GIF) {
               return GIF.getResolverFunction() == &Fn;
             }))
    return false;

  // Names of Library functions that come from the ISO C standard are reserved
  // unconditionally. Redefining them with external linkage will result in
  // undefined behavior.
  // Skip multiversioning such redefinitions. This is to achieve consistent
  // behavior with -ax enabled vs not.
  LibFunc LF;
  if (Fn.hasExternalLinkage() && GetTLI(Fn).getLibFunc(Fn.getName(), LF))
    return false;

  return true;
}

static void
CollectCalledFunctions(SetVector<Function*>& MVFunctions, unsigned StartIndex) {
  for (size_t I = StartIndex; I < MVFunctions.size(); I++) {
    Function *Fn = MVFunctions[I];
    assert(Fn && "Pointer must point to a valid Function");
    for (inst_iterator It = inst_begin(Fn), End = inst_end(Fn); It != End; ++It) {
      auto *CInst = dyn_cast<CallBase>(&*It);
      if (!CInst)
        continue;
      Function* Callee = CInst->getCalledFunction();
      if (!Callee || Callee->isDeclaration())
        continue;
      MVFunctions.insert(Callee);
    }
  }
}

static void
CollectMVCandidates(Module &M, SetVector<Function *> &MVCandidates,
                    function_ref<LoopInfo &(Function &)> GetLoopInfo,
                    function_ref<BlockFrequencyInfo &(Function &)> GetBFI,
                    ProfileSummaryInfo &PSI) {

  SetVector<Function *> MVFunctionsCallableFromLoops;
  for (Function &Fn : M) {
    if (Fn.isDeclaration() || MVFunctionsCallableFromLoops.contains(&Fn))
      continue;
    if (!ForceNoACDFunctions.empty() && Fn.hasName() &&
        is_contained(ForceNoACDFunctions, Fn.getName()))
      continue;
    if (!ForceACDFunctions.empty() && Fn.hasName() &&
        is_contained(ForceACDFunctions, Fn.getName())) {
      MVCandidates.insert(&Fn);
      continue;
    }
    if (ACDDisableAll)
      continue;
    if (ACDEnableAll) {
      MVCandidates.insert(&Fn);
      continue;
    }
    // Cost-model based multi-versioning is done only for
    //   1) functions that carry Attribute::Hot,
    //   2) functions that contain hot code per available profile data,
    //   3) functions that contain non-annotation like intrinsics,
    //   4) functions that contain loops, or
    //   5) functions that are callable from loop bodies.
    // Collect functions that carry Attribute::Hot.
    if (Fn.hasFnAttribute(Attribute::Hot) ||
        // Collect functions that contain hot code per available profile data.
        PSI.isFunctionHotInCallGraph(&Fn, GetBFI(Fn)) ||
        // Collect functions that contain non-annotation like intrinsics.
        any_of(instructions(Fn),
               [&](Instruction &I) {
                 auto *Inst = dyn_cast<IntrinsicInst>(&I);
                 return Inst && !Inst->isAssumeLikeIntrinsic();
               })) {
      MVCandidates.insert(&Fn);
    }
    // Collect functions that contain loops.
    if (GetLoopInfo(Fn).getTopLevelLoops().empty())
      continue;
    MVCandidates.insert(&Fn);
    // Collect functions that are callable from loop bodies.
    int StartIndex = MVFunctionsCallableFromLoops.size();
    for (inst_iterator It = inst_begin(Fn), End = inst_end(Fn); It != End; ++It) {
      auto *CInst = dyn_cast<CallBase>(&*It);
      if (!CInst)
        continue;
      auto ParentBB = CInst->getParent();
      if (GetLoopInfo(Fn).getLoopDepth(ParentBB) == 0)
        continue;
      Function* Callee = CInst->getCalledFunction();
      if (!Callee || Callee->isDeclaration())
        continue;
      MVFunctionsCallableFromLoops.insert(Callee);
    }
    CollectCalledFunctions(MVFunctionsCallableFromLoops, StartIndex);
  }
  MVCandidates.set_union(MVFunctionsCallableFromLoops);
}

static bool
cloneFunctions(Module &M, function_ref<LoopInfo &(Function &)> GetLoopInfo,
               function_ref<TargetLibraryInfo &(Function &)> GetTLI,
               function_ref<TargetTransformInfo &(Function &)> GetTTI,
               function_ref<BlockFrequencyInfo &(Function &)> GetBFI,
               ProfileSummaryInfo &PSI, bool GenerateVectorVariants) {

  // Candidates for multi-versioning.
  SetVector<Function *> MVCandidates;
  CollectMVCandidates(M, MVCandidates, GetLoopInfo, GetBFI, PSI);

  if (MVCandidates.empty())
    return false;

  // Maps that are used to do to RAUW later.
  std::map</*OrigFunc*/ GlobalValue *,
           std::tuple</*Resolver*/ GlobalValue *, /*Dispatcher*/ GlobalValue *,
                      /*DispatchPtr*/ GlobalVariable *,
                      /*Target CPU to multivesioned func*/
                      std::map<std::string, GlobalValue *>>>
      Orig2MultiFuncs;

  std::map</*MultiversionedFunc*/ const GlobalValue *,
           /*Target CPU*/ std::string>
      MultiFunc2TargetCPU;

  std::map</*Dispatcher*/ const GlobalValue *,
           /*Baseline clone*/ GlobalValue *>
      Dispatcher2BaselineGV;

  // Collect functions that have GlobalAlias(es) and are in MVCandidates.
  std::set<Function *> HasGlobalAliasSet;
  for (GlobalAlias &GA : M.aliases()) {
    Function *Fn = dyn_cast<Function>(GA.getAliaseeObject());
    if (Fn && MVCandidates.contains(Fn))
      HasGlobalAliasSet.insert(Fn);
  }

  bool Changed = false;

  LLVMContext &Ctx = M.getContext();
  const Triple TT{M.getTargetTriple()};

  for (Function *Fn : MVCandidates) {

    assert(Fn && "Pointer must point to a valid Function");
    if (!shouldMultiVersion(M, *Fn, GenerateVectorVariants, GetTLI))
      continue;

    // Use wrapper based resolvers when:
    //    1) on Windows, or
    //    2) -fPIC is specified on the command line, or
    //    3) function has GlobalAlias(es)
    bool isPIC = M.getPICLevel() != PICLevel::NotPIC;
    bool UseWrapperBasedResolver =
        TT.isOSWindows() || isPIC || HasGlobalAliasSet.count(Fn) > 0;

    // Skip multiversioning variable argument functions w/ wrapper based resolvers.
    if (UseWrapperBasedResolver && Fn->isVarArg())
      continue;

    MDNode *TargetsMD = nullptr;
    bool EnableAdvancedOpts = false;

    // Erase metadata after reading it, to prevent cloning it unnecessarily
    // during multi-versioning as well as dispatcher code generation.
    if (GenerateVectorVariants) {
      TargetsMD = Fn->getMetadata("llvm.vec.auto.cpu.dispatch");
      Fn->eraseMetadata(Ctx.getMDKindID("llvm.vec.auto.cpu.dispatch"));
    } else {
      TargetsMD = Fn->getMetadata("llvm.auto.cpu.dispatch");
      EnableAdvancedOpts = TargetsMD != nullptr;
      if (!TargetsMD) {
        TargetsMD = Fn->getMetadata("llvm.auto.arch");
      }
      Fn->eraseMetadata(Ctx.getMDKindID("llvm.auto.cpu.dispatch"));
      Fn->eraseMetadata(Ctx.getMDKindID("llvm.auto.arch"));
    }

    assert(TargetsMD && "TargetsMD should not be null!");
    LLVM_DEBUG(dbgs() << Fn->getName() << ": " << *TargetsMD << "\n");

    SmallVector<MultiVersionResolverOption> MVOptions;
    std::map<std::string, GlobalValue *> Clones;

    for (const MDOperand &TargetInfoIt : TargetsMD->operands()) {
      StringRef TargetCpu = cast<MDString>(TargetInfoIt.get())->getString();
      if (!X86::getCPUDispatchSupported(TargetCpu))
        continue;
      auto TargetCpuSuffix = X86::getCPUDispatchMangling(TargetCpu);

      ValueToValueMapTy VMap;
      Function *New = CloneFunction(Fn, VMap);

      const Attribute Attr = New->getFnAttribute("target-features");
      const StringRef OldFeatures = Attr.getValueAsString();

      SmallVector<StringRef, 64> NewFeatures;
      OldFeatures.split(NewFeatures, ",", -1, false);

      NewFeatures.erase(
          remove_if(NewFeatures, [](StringRef Str) { return Str[0] == '-'; }),
          NewFeatures.end());

      // Drop leading "+".
      transform(NewFeatures, NewFeatures.begin(),
                [](StringRef Str) { return Str.substr(1); });

      X86::getFeaturesForCPU(TargetCpu, NewFeatures);
      llvm::sort(NewFeatures);
      auto Last = std::unique(NewFeatures.begin(), NewFeatures.end());
      NewFeatures.erase(Last, NewFeatures.end());

      New->addFnAttr("target-features", "+" + llvm::join(NewFeatures, ",+"));

      New->removeFnAttr("target-cpu");
      New->addFnAttr("target-cpu", TargetCpu);

      New->removeFnAttr("tune-cpu");
      New->addFnAttr("tune-cpu", TargetCpu);

      if (EnableAdvancedOpts) {
        New->addFnAttr("loopopt-pipeline", "full");
        New->addFnAttr("advanced-optim", "true");
      }

      New->setMetadata("llvm.acd.clone", MDNode::get(Ctx, {}));

      New->setName(Fn->getName() + "." + Twine(TargetCpuSuffix));
      getInlineReport()->updateName(New);
      getMDInlineReport()->updateName(New);

      MVOptions.emplace_back(New, "" /* Op(1)? */, NewFeatures);

      MultiFunc2TargetCPU[New] = TargetCpu.str();
      Clones[TargetCpu.str()] = New;
    }

    // Skip function if no supported targets were found.
    if (MVOptions.empty())
      continue;

    std::string OrigName = Fn->getName().str();
    Fn->setName(OrigName + ".A"); // "generic" suffix.
    getInlineReport()->updateName(Fn);
    getMDInlineReport()->updateName(Fn);

    // Since we are renaming the function, any comdats with the same name must
    // also be renamed. This is required when targeting COFF, as the comdat name
    // must match one of the names of the symbols in the comdat.
    if (Comdat *C = Fn->getComdat()) {
      if (C->getName() == OrigName) {
        Comdat *NewC = M.getOrInsertComdat(OrigName + ".A");
        NewC->setSelectionKind(C->getSelectionKind());
        for (GlobalObject &GO : M.global_objects())
          if (GO.getComdat() == C)
            GO.setComdat(NewC);
      }
    }

    // Set advanced-optim attribute here. Fn's attributes will be used to set
    // the attributes of resolver/dispatcher functions.
    Fn->addFnAttr("advanced-optim",
                  GetTTI(*Fn).isIntelAdvancedOptimEnabled() ? "true" : "false");

    MVOptions.emplace_back(Fn, "", ArrayRef<StringRef>()); // generic.
    stable_sort(MVOptions, libIRCMVResolverOptionComparator);

    Function* Resolver = nullptr;
    GlobalValue* Dispatcher = nullptr;
    GlobalVariable* DispatchPtr = nullptr;
    bool PerformCPUBrandCheck = EnableAdvancedOpts || GenerateVectorVariants;
    if (UseWrapperBasedResolver)
      emitWrapperBasedResolver(*Fn, OrigName, MVOptions, Resolver, Dispatcher,
                               DispatchPtr, PerformCPUBrandCheck);
    else
      emitIFuncBasedResolver(*Fn, OrigName, MVOptions, Resolver, Dispatcher,
                             PerformCPUBrandCheck);

    Orig2MultiFuncs[Fn] = {Resolver, Dispatcher, DispatchPtr, std::move(Clones)};
    Dispatcher2BaselineGV[Dispatcher] = Fn;

    Fn->setMetadata("llvm.acd.clone", MDNode::get(Ctx, {}));

    Changed = true;
  }

  // No functions were multiversioned, exiting.
  if (!Changed)
    return false;

  // Collect GlobalAliases aliasing multiversioned functions.
  SmallVector<GlobalAlias*> GlobalAliasWorklist;
  for (GlobalAlias &GA : M.aliases()) {

    GlobalValue *Fn = GA.getAliaseeObject();
    if (Fn && Orig2MultiFuncs.count(Fn) != 0)
      GlobalAliasWorklist.push_back(&GA);
  }

  // Multiversion GlobalAliases aliasing multiversioned functions.
  for (GlobalAlias *GA : GlobalAliasWorklist) {

    // Get aliasee
    GlobalValue *Aliasee =
        dyn_cast<GlobalAlias>(GA->getAliasee()->stripPointerCasts());
    if (!Aliasee)
      Aliasee = GA->getAliaseeObject();

    // Set GA's name to indicate that it aliases the generic version of Fn.
    std::string Name = GA->getName().str();
    GA->setName(Name + ".A");

    // Make a clone of GA aliasing the dispatcher.
    GlobalValue *Dispatcher = std::get<1>(Orig2MultiFuncs[Aliasee]);
    GlobalAlias *DispatcherGA =
      GlobalAlias::create(GA->getValueType(), GA->getType()->getPointerAddressSpace(),
                          GA->getLinkage(), Name, &M);
    DispatcherGA->copyAttributesFrom(GA);
    ValueToValueMapTy VMap;
    VMap[Aliasee] = Dispatcher;
    if (const Constant *C = GA->getAliasee())
      DispatcherGA->setAliasee(MapValue(C, VMap));

    // Make clones of GA each aliasing a version of Fn.
    std::map<std::string, GlobalValue *> GAClones;
    std::map<std::string, GlobalValue *> &Clones = std::get<3>(Orig2MultiFuncs[Aliasee]);
    for (auto& I : Clones) {
      const StringRef TargetCpu = I.first;

      auto *NewGA =
        GlobalAlias::create(GA->getValueType(),
                            GA->getType()->getPointerAddressSpace(), GA->getLinkage(),
                            Name + "." + X86::getCPUDispatchMangling(TargetCpu), &M);

      ValueToValueMapTy VMap;
      VMap[Aliasee] = I.second;
      if (const Constant *C = GA->getAliasee())
        NewGA->setAliasee(MapValue(C, VMap));
      NewGA->copyAttributesFrom(GA);

      MultiFunc2TargetCPU[NewGA] = I.first;
      GAClones[I.first] = NewGA;
    }

    Orig2MultiFuncs[GA] = {nullptr, DispatcherGA, nullptr, std::move(GAClones)};
    Dispatcher2BaselineGV[DispatcherGA] = GA;
  }

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
    GlobalVariable *DispatchPtr = std::get<2>(Entry.second);
    std::map<std::string, GlobalValue *> &Clones = std::get<3>(Entry.second);

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
        if (MultiFunc2TargetCPU.count(Caller))
          return false;

        return true;
      }

      return true;
    });

    for (auto It = Fn->use_begin(), End = Fn->use_end(); It != End;) {
      Use &IFUse = *It;
      ++It;

      // Resolver should operate on specific functions versions.
      auto *Inst = dyn_cast<Instruction>(IFUse.getUser());
      if (Inst && Inst->getFunction() == Resolver) {
        // When generating vector variants, there should be no references
        // to the generic clone in the resolver function for correctness.
        // Removing the references here will allow GlobalDCE to remove
        // the generic, i.e. the .A clone, from the module later in the
        // pass pipeline.
        if (GenerateVectorVariants && VFInfo::isVectorVariant(Fn->getName())) {
          if (isa<ReturnInst>(Inst)) {
            auto PTy = PointerType::getUnqual(M.getContext());
            IFUse.set(Constant::getNullValue(PTy));
          } else if (auto *Store = dyn_cast<StoreInst>(Inst))
            Store->eraseFromParent();
        }
        continue;
      }

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
        if (MultiFunc2TargetCPU.count(Caller)) {
          GlobalValue *Replacement = Clones[MultiFunc2TargetCPU[Caller]];
          assert(Replacement && "Expected that functions are multiversioned "
                                "for all requested targets now!");
          CInst->setCalledOperand(Replacement);
          continue;
        }
      }

      llvm_unreachable("Unhandled case!");
    }

    if (DispatchPtr && !GenerateVectorVariants)
      DispatchPtr->setInitializer(Fn);
  }

  // Iterate over GlobalAliases once more.
  // Earlier call to replaceUsesWithIf() caused generic versions
  // of GlobalAlias clones to incorrectly alias dispatchers functions.
  // Make generic versions of GlobalAlias clones, alias the generic
  // versions of aliasees instead.
  for (auto It : GlobalAliasWorklist) {

    GlobalAlias &GA = *It;

    GlobalValue *Dispatcher =
        dyn_cast<GlobalAlias>(GA.getAliasee()->stripPointerCasts());
    if (!Dispatcher)
      Dispatcher = GA.getAliaseeObject();
    assert(Dispatcher && "Dispatcher must exist");

    GlobalValue *BaselineGV = Dispatcher2BaselineGV[Dispatcher];
    assert(BaselineGV && "BaselineGV must exist");

    ValueToValueMapTy VMap;
    VMap[Dispatcher] = BaselineGV;
    if (const Constant *C = GA.getAliasee())
      GA.setAliasee(MapValue(C, VMap));
  }

  // If we are here then we have done modifications.
  return true;
}

static void
clearMetadataAndSetAttributes(
    Module &M, function_ref<TargetTransformInfo &(Function &)> GetTTI,
    bool SetAdvancedOptim, bool GenerateVectorVariants) {

  LLVMContext &Ctx = M.getContext();
  for (Function &Fn : M) {
    if (Fn.isDeclaration())
      continue;
    // Remove metadata, storing multi-versioning targets, from all functions.
    if (GenerateVectorVariants) {
      Fn.eraseMetadata(Ctx.getMDKindID("llvm.vec.auto.cpu.dispatch"));
    } else {
      Fn.eraseMetadata(Ctx.getMDKindID("llvm.auto.arch"));
      Fn.eraseMetadata(Ctx.getMDKindID("llvm.auto.cpu.dispatch"));
    }
    // Add "advanced-optim" attribute on functions that are skipped
    // and not multi-versioned.
    if (SetAdvancedOptim && !Fn.hasFnAttribute("advanced-optim"))
      Fn.addFnAttr("advanced-optim",
                   GetTTI(Fn).isIntelAdvancedOptimEnabled() ? "true" : "false");
  }
}

PreservedAnalyses AutoCPUClonePass::run(Module &M, ModuleAnalysisManager &AM) {

  Triple TargetTriple(M.getTargetTriple());

  // Do not run auto CPU clone pass on offload modules.
  if (TargetTriple.getArch() == Triple::spir ||
      TargetTriple.getArch() == Triple::spir64 ||
      TargetTriple.getArch() == Triple::spirv32 ||
      TargetTriple.getArch() == Triple::spirv64)
    return PreservedAnalyses::all();

  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetLoopInfo = [&FAM](Function &F) -> LoopInfo & {
    return FAM.getResult<LoopAnalysis>(F);
  };
  auto GetTLI = [&FAM](Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(F);
  };
  auto GetTTI = [&FAM](Function &F) -> TargetTransformInfo & {
    return FAM.getResult<TargetIRAnalysis>(F);
  };
  auto GetBFI = [&FAM](Function &F) -> BlockFrequencyInfo & {
    return FAM.getResult<BlockFrequencyAnalysis>(F);
  };
  ProfileSummaryInfo &PSI = AM.getResult<ProfileSummaryAnalysis>(M);

  bool Success = cloneFunctions(M, GetLoopInfo, GetTLI, GetTTI, GetBFI, PSI,
                                GenerateVectorVariants);

  clearMetadataAndSetAttributes(M, GetTTI, Success, GenerateVectorVariants);

  return Success ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

AutoCPUClonePass::AutoCPUClonePass(bool GVV) : GenerateVectorVariants(GVV) {
  // Setting ClGenerateVectorVariants on the command line overrides
  // the value of GenerateVectorVariants member variable.
  if (ClGenerateVectorVariants.getNumOccurrences() > 0)
    GenerateVectorVariants = ClGenerateVectorVariants;
}
