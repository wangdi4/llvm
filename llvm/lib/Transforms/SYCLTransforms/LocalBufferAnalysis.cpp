//===- LocalBufferAnalysis.cpp - DPC++ kernel local buffer analysis -------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/LocalBufferAnalysis.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/User.h"
#include "llvm/Transforms/SYCLTransforms/DevLimits.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/SYCLTransforms/Utils/WGBoundDecoder.h"

using namespace llvm;

#define DEBUG_TYPE "sycl-kernel-local-buffer-analysis"

namespace llvm {

class LocalBufferInfoImpl {
public:
  LocalBufferInfoImpl(Module &M, CallGraph &CG);

  using TUsedLocals = LocalBufferInfo::TUsedLocals;
  using TUsedLocalsMap = LocalBufferInfo::TUsedLocalsMap;

  TUsedLocalsMap &getDirectLocalsMap() { return DirectLocalUseMap; }

  void computeLocalsSizeOffset();

  size_t getLocalsSize(Function *F) const { return LocalSizeMap.lookup(F); }

  size_t getLocalGVToOffset(GlobalVariable *GV) const {
    return LocalGVToOffsetMap.lookup(GV);
  }

  void print(raw_ostream &OS);

private:
  void analyze();

  /// Adds the given local value to the set of used locals of all functions
  /// that are using the given user directly. It recursively searches the first
  /// useres (and users of a users) that are functions.
  /// \param LocalVal local value (which is represented by a global value
  /// with address space 3).
  /// \param U direct user of pLocalVal.
  void updateLocalsMap(GlobalVariable *LocalVal, User *U);

  /// Goes over all local values in the module and over all their direct users
  /// and maps between functions and the local values they use.
  /// \param M the module which need to go over its local values.
  void updateDirectLocals();

  /// Get the map from function to local variables which are directly or
  /// indirectly used in the function.
  void updateLocalUseMap();

  /// The llvm module this pass needs to update.
  Module &M;

  CallGraph &CG;

  /// Map between function pointer and the set of local values the
  /// function uses directly.
  DenseMap<const Function *, TUsedLocals> DirectLocalUseMap;

  /// Map between function pointer and the set of local values used in
  /// the function's callgraph.
  DenseMap<const Function *, TUsedLocals> LocalUseMap;

  /// Map between kernel function and its local buffer size.
  SmallDenseMap<Function *, size_t, 8> LocalSizeMap;

  /// Map between local variable and its offset in containing kernel's local
  /// buffer.
  DenseMap<GlobalVariable *, size_t> LocalGVToOffsetMap;
};

} // namespace llvm

LocalBufferInfoImpl::LocalBufferInfoImpl(Module &M, CallGraph &CG)
    : M(M), CG(CG) {
  analyze();
}

void LocalBufferInfoImpl::updateLocalsMap(GlobalVariable *LocalVal, User *U) {
  for (auto It = df_begin(U), E = df_end(U); It != E;) {
    if (auto *I = dyn_cast<Instruction>(*It)) {
      // declaring variables for debugging purposes shouldn't affect local
      // buffers.
      if (MDNode *mdn = I->getMetadata("dbg_declare_inst")) {
        if (mdconst::extract<ConstantInt>(mdn->getOperand(0))
                ->isAllOnesValue()) {
          ++It;
          continue;
        }
      }
      // Add LocalVal to the set of local values used by F
      Function *F = I->getFunction();
      DirectLocalUseMap[F].insert(LocalVal);
      LocalUseMap[F].insert(LocalVal);
      It.skipChildren();
      continue;
    }
    ++It;
  }
}

void LocalBufferInfoImpl::updateDirectLocals() {
  // Find globals that appear in the origin kernel as local variables and add
  // update mapping accordingly
  for (GlobalVariable &GV : M.globals()) {
    if (GV.getAddressSpace() != CompilationUtils::ADDRESS_SPACE_LOCAL)
      continue;

    // If we reached here, then Val is a global value that was originally a
    // local value.
    for (User *U : GV.users())
      updateLocalsMap(&GV, U);
  }
}

/// Iterate all functions in module by postorder traversal. For each
/// function, calculate local variables used.
void LocalBufferInfoImpl::updateLocalUseMap() {
  for (auto It = po_begin(&CG), E = po_end(&CG); It != E; ++It) {
    Function *F = It->getFunction();
    if (!F || F->isDeclaration() ||
        WGBoundDecoder::isWGBoundFunction(F->getName()))
      continue;
    const CallGraphNode *N = CG[F];
    for (const auto &Pair : *N) {
      Function *Callee = Pair.second->getFunction();
      if (!Callee || Callee->isDeclaration() ||
          WGBoundDecoder::isWGBoundFunction(Callee->getName()))
        continue;
      if (auto MapIt = LocalUseMap.find(Callee); MapIt != LocalUseMap.end()) {
        auto Copy = MapIt->second;
        LocalUseMap[F].insert(Copy.begin(), Copy.end());
      }
    }
  }
}

void LocalBufferInfoImpl::computeLocalsSizeOffset() {
  DataLayout DL(&M);
  for (Function *F : CompilationUtils::getKernels(M)) {
    // Calculate total local buffer size.
    size_t LocalSize = 0;
    if (auto MapIt = LocalUseMap.find(F); MapIt != LocalUseMap.end()) {
      for (auto *GV : MapIt->second) {
        assert(!LocalGVToOffsetMap.count(GV) &&
               "Global variable with local address space can't be used in two "
               "kernels");
        size_t ArraySize = DL.getTypeAllocSize(GV->getValueType());
        assert(0 != ArraySize && "local buffer size is zero!");
        Align A = GV->getAlign().valueOrOne();
        LocalSize = alignTo(LocalSize, A);
        LocalGVToOffsetMap[GV] = LocalSize;
        LocalSize += ArraySize;
      }
    }
    LocalSizeMap[F] = LocalSize;

    // Handle kernel with barrier path case, in which a local variable could
    // still be shared between a kernel and its vectorized kernel. The local
    // variable layout is the same between a kernel and its vectorized kernel.
    // Need not to handle vectorized masked kernel because at this point masked
    // kernel is already inlined.
    SYCLKernelMetadataAPI::KernelInternalMetadataAPI KIMD(F);
    if (auto *VectorizedF = KIMD.VectorizedKernel.hasValue()
                                ? KIMD.VectorizedKernel.get()
                                : nullptr)
      LocalSizeMap[VectorizedF] = LocalSize;
  }
}

void LocalBufferInfoImpl::print(raw_ostream &OS) {
  OS << "LocalBufferInfo\n";
  OS.indent(2) << "Local variables used in kernel\n";
  CompilationUtils::FuncSet Kernels = CompilationUtils::getAllKernels(M);
  for (auto *F : Kernels) {
    OS.indent(4) << F->getName() << "\n";
    if (auto It = LocalUseMap.find(F); It != LocalUseMap.end()) {
      for (auto *GV : It->second)
        OS.indent(6) << GV->getName() << "\n";
    }
  }
  computeLocalsSizeOffset();
  OS.indent(2) << "Kernel local buffer size\n";
  for (auto *F : Kernels)
    OS.indent(4) << F->getName() << " : " << getLocalsSize(F) << "\n";
  OS.indent(2)
      << "Offset of local variable in containing kernel's local buffer\n";
  for (auto &Pair : LocalGVToOffsetMap)
    OS.indent(4) << Pair.first->getName() << " : " << Pair.second << "\n";
}

void LocalBufferInfoImpl::analyze() {
  // Initialize DirectLocalUseMap.
  updateDirectLocals();

  updateLocalUseMap();
}

LocalBufferInfo::LocalBufferInfo(Module &M, CallGraph &CG) {
  Impl.reset(new LocalBufferInfoImpl(M, CG));
}

LocalBufferInfo::LocalBufferInfo(LocalBufferInfo &&Other) {
  Impl = std::move(Other.Impl);
}

LocalBufferInfo &LocalBufferInfo::operator=(LocalBufferInfo &&Other) {
  Impl = std::move(Other.Impl);
  return *this;
}

LocalBufferInfo::~LocalBufferInfo() = default;

void LocalBufferInfo::print(raw_ostream &OS) { return Impl->print(OS); }

LocalBufferInfo::TUsedLocalsMap &LocalBufferInfo::getDirectLocalsMap() {
  return Impl->getDirectLocalsMap();
}

void LocalBufferInfo::computeSize() { Impl->computeLocalsSizeOffset(); }

size_t LocalBufferInfo::getLocalsSize(Function *F) const {
  return Impl->getLocalsSize(F);
}

size_t LocalBufferInfo::getLocalGVToOffset(GlobalVariable *GV) const {
  return Impl->getLocalGVToOffset(GV);
}

LocalBufferInfo LocalBufferAnalysis::run(Module &M,
                                         AnalysisManager<Module> &AM) {
  CallGraph &CG = AM.getResult<CallGraphAnalysis>(M);
  LocalBufferInfo WPAResult(M, CG);

  return WPAResult;
}

PreservedAnalyses LocalBufferAnalysisPrinter::run(Module &M,
                                                  ModuleAnalysisManager &MAM) {
  MAM.getResult<LocalBufferAnalysis>(M).print(OS);
  return PreservedAnalyses::all();
}

// Provide a definition for the static class member used to identify passes.
AnalysisKey LocalBufferAnalysis::Key;
