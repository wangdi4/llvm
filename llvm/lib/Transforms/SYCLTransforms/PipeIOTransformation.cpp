//= PipeIOTransformation.cpp - Transform pipe builtins to IO builtins - C++ -//
//
// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// ===--------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/PipeIOTransformation.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/NoFolder.h"
#include "llvm/Transforms/SYCLTransforms/BuiltinLibInfoAnalysis.h"
#include "llvm/Transforms/SYCLTransforms/Utils/BarrierUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/RuntimeService.h"
#include "llvm/Transforms/SYCLTransforms/Utils/SYCLChannelPipeUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

#define DEBUG_TYPE "sycl-kernel-pipe-io-transform"

using namespace llvm;
using namespace SYCLChannelPipeUtils;
using namespace CompilationUtils;

// Vector of <Pipe, Pipe Id> pairs.
using PipesWithIdVector = SmallVector<std::pair<Value *, unsigned>, 4>;
using PipeNameIdMap = StringMap<unsigned>;
using FuncToPipeArgVec = MapVector<Function *, PipesWithIdVector>;
using PipesBuiltinsMap = MapVector<CallInst *, unsigned>;
// Map from CallInst to a set of <Argument number, Pipe Id> pairs.
// Pairs are sorted so that iteration is in the order of argument number.
using PipesCallToArgNos =
    MapVector<CallInst *, std::set<std::pair<unsigned, unsigned>>>;

static bool processGlobalIOPipes(Module &M, PipesWithIdVector &PipesWithIdVec,
                                 RuntimeService &RTS, unsigned &PipeId,
                                 PipeNameIdMap &PipeNameIds) {
  bool Changed = false;
  Function *GlobalDtor = nullptr;

  for (auto &PipeGV : M.globals()) {
    if (!isGlobalPipe(&PipeGV))
      continue;

    // If IO pipe MD string is empty or GV has no IO pipe MD at all - skip it
    auto *IOMetadata = PipeGV.getMetadata("io");
    if (!IOMetadata ||
        (cast<MDString>(IOMetadata->getOperand(0))->getLength() == 0))
      continue;

    if (!GlobalDtor)
      GlobalDtor = createPipeGlobalDtor(M);

    auto *PipeReleaseFunc = importFunctionDecl(
        &M, RTS.findFunctionInBuiltinModules("__pipe_release_fpga"));
    initializeGlobalPipeReleaseCall(GlobalDtor, PipeReleaseFunc, &PipeGV);

    ChannelPipeMD MD = getChannelPipeMetadata(&PipeGV);
    if (PipeNameIds.count(MD.IO))
      PipesWithIdVec.push_back({&PipeGV, PipeNameIds[MD.IO]});
    else {
      PipesWithIdVec.push_back({&PipeGV, PipeId});
      PipeNameIds[MD.IO] = PipeId++;
    }

    Changed = true;
  }

  return Changed;
}

static bool processIOPipesFromKernelArg(Module &M,
                                        FuncToPipeArgVec &FuncPipeArg,
                                        unsigned &PipeId,
                                        PipeNameIdMap &PipeNameIds) {
  bool Changed = false;

  for (auto *Kernel : SYCLKernelMetadataAPI::KernelList(M)) {
    auto ArgIOAttributeList =
        SYCLKernelMetadataAPI::KernelMetadataAPI(Kernel).ArgIOAttributeList;
    PipesWithIdVector Pipes;
    if (ArgIOAttributeList.hasValue()) {
      auto IOList = ArgIOAttributeList.getList();
      auto *It = Kernel->arg_begin();
      for (auto &IO : IOList) {
        Value *Pipe = It++;
        std::string IOName = IO;
        if (IOName.empty())
          continue;
        if (PipeNameIds.count(IOName))
          Pipes.push_back({Pipe, PipeNameIds[IOName]});
        else {
          Pipes.push_back({Pipe, PipeId});
          PipeNameIds[IOName] = PipeId++;
        }
        Changed = true;
      }
    }
    FuncPipeArg[Kernel] = Pipes;
  }

  return Changed;
}

/// Find CallInst users of io pipe in a function. Pipes are either function
/// arguments or global pipes. It is known at this point whether an argument
/// is io pipe and which io pipe it is. Therefore, there is no need to traverse
/// CallGraph recursively.
static void getPipeUsersInFunc(const Function *F, const Value *V,
                               unsigned PipeId,
                               SmallPtrSetImpl<User *> &VisitedUsers,
                               PipesCallToArgNos &PCA, PipesBuiltinsMap &PBV) {
  for (auto &VU : V->uses()) {
    User *U = VU.getUser();
    if (VisitedUsers.count(U))
      continue;
    VisitedUsers.insert(U);
    Instruction *I = dyn_cast<Instruction>(U);
    // Ignore user that is not in current function.
    if (I && I->getFunction() != F)
      continue;
    if (CallInst *CI = dyn_cast<CallInst>(U)) {
      Function *CF = CI->getCalledFunction();
      assert(CF && "Indirect function call?");
      StringRef CFName = CF->getName();
      if (CFName.find("__spirv_CreatePipeFromPipeStorage") !=
          std::string::npos) {
        for (auto *UU : CI->users()) {
          // Result of __spirv_CreatePipeFromPipeStorage can be stored to a
          // pointer, that represents a read/write pipe. Stores are having no
          // users, but their pointer operand, as it was said, is a pipe itself,
          // that is going to be used right after in read/write built-in calls.
          if (auto *SI = dyn_cast<StoreInst>(UU))
            getPipeUsersInFunc(F, SI->getPointerOperand(), PipeId, VisitedUsers,
                               PCA, PBV);
          else
            getPipeUsersInFunc(F, U, PipeId, VisitedUsers, PCA, PBV);
        }
      }
      if (isPipeBuiltin(CFName))
        PBV[CI] = PipeId;
      else if (!CF->isDeclaration()) {
        // Found a user.
        unsigned ArgNo = CI->getArgOperandNo(&VU);
        PCA[CI].insert({ArgNo, PipeId});
        LLVM_DEBUG(dbgs() << "getPipeUsersInFunc: insert ArgNo " << ArgNo
                          << " PipeId " << PipeId << " of CI " << *CI << "\n");
      }
    } else {
      if (auto *SI = dyn_cast<StoreInst>(U))
        getPipeUsersInFunc(F, SI->getPointerOperand(), PipeId, VisitedUsers,
                           PCA, PBV);
      else
        getPipeUsersInFunc(F, U, PipeId, VisitedUsers, PCA, PBV);
    }
  }
}

/// Clone called function, add prefix to the name of cloned function, and
/// replace called function with the newly cloned function. In order to
/// differentiate combinations of io pipe arguments, IO pipe's arg operand
/// number and pipe id is encoded into the prefix.
/// E.g. io pile arg operand number is 1 and pipe id is 0,
///   %1 = call i32 @foo(%opencl.pipe_ro_t.5 addrspace(1)* %p, ...
/// foo is cloned and new function name is __io_pipe_1_0_foo:
///   %1 = call i32 @__io_pipe_1_0_foo(%opencl.pipe_ro_t.5 addrspace(1)* %p, ...
static void clonePipeFunctions(CallGraph &CG, const PipesCallToArgNos &PCA) {
  for (auto &CallArgNos : PCA) {
    if (CallArgNos.second.empty())
      continue;
    CallInst *CI = CallArgNos.first;
    Function *CF = CI->getCalledFunction();
    assert(CF && "Indirect function call");
    StringRef CFName = CF->getName();
    static const std::string Prefix = "__io_pipe_";
    std::string ArgNoPipeIdStr;
    for (auto &ArgNoPair : CallArgNos.second) {
      unsigned ArgNo = ArgNoPair.first;
      unsigned PipeId = ArgNoPair.second;
      ArgNoPipeIdStr += (Twine(ArgNo) + "_" + Twine(PipeId) + "_").str();
    }
    std::string NewName = Prefix + ArgNoPipeIdStr + CFName.str();

    // Check if new function exists.
    Function *NewF = CF->getParent()->getFunction(NewName);
    if (!NewF) {
      ValueToValueMapTy VMap;
      NewF = CloneFunction(CF, VMap);
      NewF->setName(NewName);
      LLVM_DEBUG(dbgs() << "clonePipeFunctions: new function " << NewName
                        << "\n");
    }
    CI->replaceUsesOfWith(CF, NewF);
    CG.addToCallGraph(NewF);
  }
}

/// If io pipe is used as argument in a CallInst, clone called function.
/// Run BFS to find pipe usages in a function and clone if needed. Run DFS to
/// traverse all called functions in it.
static void
cloneFunctionsWithIOPipe(CallGraph &CG, const PipesWithIdVector &GlobalPipes,
                         const FuncToPipeArgVec &FuncPipeArg,
                         PipesBuiltinsMap &PBV,
                         SmallPtrSetImpl<Function *> &VisitedFuncs) {
  SmallPtrSet<User *, 8> VisitedUsers;
  for (auto &FA : FuncPipeArg) {
    Function *F = FA.first;
    PipesCallToArgNos PCA;
    // Find CallInst users of each function argument that is a io pipe.
    for (auto &Arg : FA.second) {
      VisitedUsers.clear();
      getPipeUsersInFunc(F, Arg.first, Arg.second, VisitedUsers, PCA, PBV);
    }

    // Find CallInst users of each global io pipe.
    for (auto &PipeWithId : GlobalPipes) {
      Value *V = PipeWithId.first;
      unsigned PipeId = PipeWithId.second;
      VisitedUsers.clear();
      getPipeUsersInFunc(F, V, PipeId, VisitedUsers, PCA, PBV);
    }

    // Clone called functions.
    clonePipeFunctions(CG, PCA);
    VisitedFuncs.insert(F);

    // Process called functions recursively.
    CallGraphNode *Node = CG[F];
    for (auto &N : *Node) {
      auto *CI = cast<CallInst>(*N.first);
      Function *CF = CI->getCalledFunction();
      assert(CF && "Indirect function call?");
      if (CF->isDeclaration() || VisitedFuncs.count(CF))
        continue;
      FuncToPipeArgVec CFuncPipeArg;
      PipesWithIdVector ArgNoPipeIds;
      for (auto &ArgNoPId : PCA[CI]) {
        unsigned ArgNo = ArgNoPId.first;
        unsigned PipeId = ArgNoPId.second;
        auto *A = CF->getArg(ArgNo);
        ArgNoPipeIds.push_back({A, PipeId});
      }
      CFuncPipeArg[CF] = ArgNoPipeIds;
      cloneFunctionsWithIOPipe(CG, GlobalPipes, CFuncPipeArg, PBV,
                               VisitedFuncs);
    }
  }
}

static GlobalVariable *createGlobalTextConstant(Module &M, StringRef Name) {
  ArrayType *Ty =
      ArrayType::get(Type::getInt8Ty(M.getContext()), Name.size() + 1);
  auto *ConstStringGV = new GlobalVariable(
      M, Ty, /*isConstant=*/true, GlobalValue::PrivateLinkage,
      /*Initializer=*/ConstantDataArray::getString(M.getContext(), Name),
      Name + ".str", /*InsertBefore=*/nullptr);

  ConstStringGV->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
  const auto &DL = M.getDataLayout();
  ConstStringGV->setAlignment(MaybeAlign(DL.getPrefTypeAlign(Ty)));
  return ConstStringGV;
}

static void replacePipeBuiltinCall(CallInst *PipeCall, GlobalVariable *TC,
                                   RuntimeService &RTS) {
  IRBuilder<NoFolder> Builder(PipeCall);
  Function *CF = PipeCall->getCalledFunction();
  assert(CF && "Indirect function call");
  PipeKind PK = getPipeKind(CF->getName());
  PK.IO = true;
  PK.FPGA = true;

  Function *Builtin = getPipeBuiltin(*PipeCall->getModule(), RTS, PK);
  FunctionType *FTy = Builtin->getFunctionType();
  Value *Args[] = {PipeCall->getArgOperand(0), PipeCall->getArgOperand(1),
                   Builder.CreatePointerCast(TC, FTy->getParamType(2), ""),
                   PipeCall->getArgOperand(2), PipeCall->getArgOperand(3)};

  auto *PC = Builder.CreateCall(Builtin, Args, PipeCall->getName());
  PipeCall->replaceAllUsesWith(PC);
  PipeCall->eraseFromParent();
}

bool PipeIOTransformationPass::runImpl(Module &M, BuiltinLibInfo *BLI) {
  PipeTypesHelper PipeTypes(M);
  if (!PipeTypes.hasPipeTypes())
    return false;

  bool Changed = false;
  // Each io pipe has a unique id. PipeId is incremented whenever an io pipe is
  // found in processGlobalIOPipes and processIOPipesFromKernelArg.
  unsigned PipeId = 0;
  PipesWithIdVector GlobalPipes;
  // Map from io pipe name to its id.
  PipeNameIdMap PipeNameIds;
  assert(BLI && "Invalid builtin lib info!");
  auto &RTS = BLI->getRuntimeService();
  Changed |= processGlobalIOPipes(M, GlobalPipes, RTS, PipeId, PipeNameIds);

  FuncToPipeArgVec FuncPipeArg;
  Changed |= processIOPipesFromKernelArg(M, FuncPipeArg, PipeId, PipeNameIds);

  PipesBuiltinsMap PBV;
  SmallPtrSet<Function *, 4> VisitedFuncs;
  CallGraph CG(M);
  cloneFunctionsWithIOPipe(CG, GlobalPipes, FuncPipeArg, PBV, VisitedFuncs);

  unsigned PipeCount = PipeNameIds.size();
  std::vector<GlobalVariable *> TCs(PipeCount);
  for (auto &NameId : PipeNameIds)
    TCs[NameId.second] = createGlobalTextConstant(M, NameId.first());

  for (auto &PC : PBV) {
    unsigned PipeId = PC.second;
    assert(PipeId < PipeCount && "Pipe id is out of range");
    replacePipeBuiltinCall(PC.first, TCs[PipeId], RTS);
  }

  return Changed;
}

PreservedAnalyses PipeIOTransformationPass::run(Module &M,
                                                ModuleAnalysisManager &AM) {
  auto *BLI = &AM.getResult<BuiltinLibInfoAnalysis>(M);
  if (!runImpl(M, BLI))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}
